#include "sdf_pr_online_model.hpp"

SDFPROnlineModel::SDFPROnlineModel(Mapping* p_mapping, Config* _cfg):
    apps(p_mapping->getApplications()),
    platform(p_mapping->getPlatform()),
    mapping(p_mapping),
    cfg(_cfg),
    next(*this, apps->n_SDFActors()+platform->nodes(), 0, apps->n_SDFActors()+platform->nodes()),
    rank(*this, apps->n_SDFActors(), 0, apps->n_SDFActors()-1),
    proc(*this, apps->n_programEntities(), 0, platform->nodes()-1),
    proc_mode(*this, platform->nodes(), 0, platform->getMaxModes()),
    tdmaAlloc(*this, platform->nodes(), 0, platform->tdmaSlots()),
    tdnTable(*this, platform->getTDNGraph().size(), 0, platform->nodes()+1),
    noc_mode(*this, 0, platform->getInterconnectModes()-1),
    chosenRoute(*this, apps->n_programChannels(), 0, platform->getTDNCycles()),
    sendNext(*this, apps->n_programChannels()+platform->nodes(), 0, apps->n_programChannels()+platform->nodes()),
    recNext(*this, apps->n_programChannels()+platform->nodes(), 0, apps->n_programChannels()+platform->nodes()),
    sendbufferSz(*this, apps->n_programChannels(), 0, Int::Limits::max),
    recbufferSz(*this, apps->n_programChannels(), 0, Int::Limits::max),
    period(*this, apps->n_SDFApps(), 0, Int::Limits::max),
    proc_period(*this, platform->nodes(), 0, Int::Limits::max),
    latency(*this, apps->n_SDFApps(), 0, Int::Limits::max),
    procsUsed(*this, 1, platform->nodes()),
    utilization(*this, platform->nodes(), 0, p_mapping->max_utilization),
    sys_utilization(*this, 0, p_mapping->max_utilization),
    procsUsed_utilization(*this, 0, p_mapping->max_utilization),
    proc_power(*this, platform->nodes(), 0, Int::Limits::max),
    noc_power(*this, 0, Int::Limits::max),
    //sys_power(*this, mapping->getLeastPowerConsumption(), Int::Limits::max),
    sys_power(*this, 0, Int::Limits::max),
    proc_area(*this, platform->nodes(), 0, Int::Limits::max),
    noc_area(*this, 0, Int::Limits::max),
    sys_area(*this, 0, Int::Limits::max),
    proc_cost(*this, platform->nodes(), 0, Int::Limits::max),
    noc_cost(*this, 0, Int::Limits::max),
    sys_cost(*this, 0, Int::Limits::max),
    wcct_b(*this, apps->n_programChannels(), 0, Int::Limits::max),
    wcct_s(*this, apps->n_programChannels(), 0, Int::Limits::max),
    wcct_r(*this, apps->n_programChannels(), 0, Int::Limits::max),
    least_power_est(mapping->getLeastPowerConsumption()){

    std::ostream debug_stream(nullptr); /**< debuging stream, it is printed only in debug mode. */
    debug_stream << "\n==========\ndebug log:\n..........\n";
    vector<SDFChannel*> channels = apps->getChannels();

    cout << "Creating Model..." << endl;
    cout << "  Platform with " << platform->nodes() << " nodes is ";
    if(!mapping->homogeneousPlatform())
        cout << "not ";
    cout << "homogeneous." << endl;
    if(!mapping->homogeneousPlatform()){
        cout << "  Homogeneous nodes: ";
        for(size_t ji = 0; ji < platform->nodes(); ji++){
            for(size_t jj = ji + 1; jj < platform->nodes(); jj++){
                if(mapping->homogeneousNodes(ji, jj)){
                    cout << "{" << ji << ", " << jj << "} ";
                }
            }
        }
        cout << endl;
        cout << "  Mode-homogeneous nodes: ";
        for(size_t ji = 0; ji < platform->nodes(); ji++){
            for(size_t jj = ji + 1; jj < platform->nodes(); jj++){
                if(mapping->homogeneousModeNodes(ji, jj)){
                    cout << "{" << ji << ", " << jj << "} ";
                }
            }
        }
        cout << endl;
    }

    cout << "  Found " << apps->n_programEntities() << " program entities (", cout << apps->n_SDFActors() << " actors of ";
    cout << apps->n_SDFApps() << " SDFGs and ";
    cout << apps->n_IPTTasks() << " ipt tasks)." << endl;
    cout << "   --Actors and Tasks------------------\n";
    for(size_t ii = 0; ii < apps->n_programEntities(); ii++){
        cout << "   " << ii << ": " << apps->getName(ii) << endl;
    }

    cout << "   --Channels------------------\n";
    for(size_t ki = 0; ki < channels.size(); ki++){
        int src_ch1 = channels[ki]->source;
        int dst_ch1 = channels[ki]->destination;
        int tok_ch1 = channels[ki]->initTokens;

        cout << "   Ch " << ki << ": ";
        cout << apps->getName(src_ch1) << " -> ";
        cout << apps->getName(dst_ch1) << "   (";
        cout << src_ch1 << " -> ";
        cout << dst_ch1 << ")";
        if(tok_ch1 > 0)
            cout << " *";
        cout << endl;
    }
    cout << endl;

    cout << "Design Constraints: " << endl;
    for(size_t a = 0; a < apps->n_SDFApps(); a++){
        cout << "   SDF " << apps->getGraphName(a) << ":\n \t period const: ";
        if(apps->getPeriodConstraint(a)!=-1){
            cout<< apps->getPeriodConstraint(a);
        }else{
            cout << "none";
        }
        cout << "\n \t latency const: ";
        if(apps->getLatencyConstraint(a)!=-1){
            cout<< apps->getLatencyConstraint(a);
        }else{
            cout << "none";
        }
        cout << endl;
    }
    cout << endl;

    if(apps->n_programEntities() <= 0){
        cout << "No program entities found !!!" << endl;
        throw 42;
    }
    cout << "Inserting mapping constraints \n";

    /**
     * MAPPING
     */
    IntVarArgs nSDFAsOnProc(*this, platform->nodes(), 0, apps->n_SDFActors()); /**< number of SDF actors on proc[i]. */
    IntVarArgs nTasksOnProc(*this, platform->nodes(), 0, apps->n_IPTTasks()); /**< number of IPTs on proc[i]. */
    IntVarArgs nEntitiesOnProc(*this, platform->nodes(), 0, apps->n_programEntities()); /**< number of SDF actors and IPTs combined on proc[i]. */
    IntVarArgs proc_SDF_wcet_sum(*this, platform->nodes(), 0, Int::Limits::max); /**< sum of WCET on each proccessor for sdf apps. */
#include "mapping.constraints"

    /**
     * Independent periodic tasks
     */
    cout << "Inserting IPT scheduling constraints \n";
    if(apps->n_IPTTasks() > 0){
        IntVarArgs n_instances(*this, apps->n_IPTTasks(), 0, apps->getMaxNumberOfIPTInstances());
        IntVarArgs instancesOnNode(*this, apps->n_IPTTasks() * platform->nodes(), 0, apps->getMaxNumberOfIPTInstances() * platform->nodes());
        IntVarArgs nActorsOnProc(*this, platform->nodes(), 0, apps->n_programEntities());
        IntVarArgs taskUtil(*this, apps->n_programEntities(), 0, Int::Limits::max); /**< the main utilization variable for pr tasks. */
#include "partitioning.constraints"
    }

    /**
     * SDF scheduling, Communication and Throughput
     */
    if(apps->n_SDFActors() > 0){
        cout << "Inserting scheduling constraints \n";
        //IntVarArgs rank(*this, apps->n_SDFActors(), 0, apps->n_SDFActors()-1);                                                /**< rank of each actor. */
#include "scheduling.constraints"
        /**
         * Communication
         */
        cout << "Inserting communication constraints \n";
        IntVarArgs blockingTime_s(*this, apps->n_programChannels(), 0, Int::Limits::max);
        IntVarArgs blockingTime_r(*this, apps->n_programChannels(), 0, Int::Limits::max);
        IntVarArgs transferTime_s(*this, apps->n_programChannels(), 0, Int::Limits::max);
        IntVarArgs transferTime_r(*this, apps->n_programChannels(), 0, Int::Limits::max);
        //IntVarArgs sendbufferSz(*this, apps->n_programChannels(), 0, Int::Limits::max);                               /**< //sending buffer sizes. */
        //IntVarArgs recbufferSz(*this, apps->n_programChannels(), 1, Int::Limits::max);                                /**< //receiving buffer sizes. */
#include "wcct.constraints"

        /**
         * Memory
         */
        cout << "Inserting memory constraints \n";
#include "memory.constraints"

        /**
         * Throughput
         */
        cout << "Inserting throughput constraints \n";
        vector<int> maxMinWcet(apps->n_SDFApps(), 0);
        vector<int> maxMinWcetActor(apps->n_SDFActors(), 0);
        vector<int> sumMinWCETs(apps->n_SDFApps(), 0);
        vector<int> minA(apps->n_SDFApps(), -1);
        vector<int> maxA(apps->n_SDFApps(), 0);
        for(size_t a = 0; a < apps->n_SDFApps(); a++){
            for(size_t i = 0; i < apps->n_SDFActors(); i++){
                if(apps->getSDFGraph(i) == a){
                    if(minA[a] == -1)
                        minA[a] = i;
                    maxA[a] = i;
                    vector<int> wcets;
                    for(size_t j = 0; j < platform->nodes(); j++){
                        vector<int> wcets_proc = mapping->getValidWCETs(i, j);
                        if(wcets_proc.size() > 0)
                            wcets.push_back(*min_element(wcets_proc.begin(), wcets_proc.end()));
                    }
                    if(maxMinWcet[a] < *min_element(wcets.begin(), wcets.end()))
                        maxMinWcet[a] = *min_element(wcets.begin(), wcets.end());
                    if(maxMinWcetActor[i] < *min_element(wcets.begin(), wcets.end()))
                        maxMinWcetActor[i] = *min_element(wcets.begin(), wcets.end());

                    sumMinWCETs[a] += maxMinWcetActor[i];
                }
      }
    }
#include "throughput.constraints"

    cout << "Inserting presolver constraints \n";
//#include "presolve.constraints"

    if (cfg->is_presolved()) {
      LOG_INFO("sdf_pr_online_model.cpp: The model is presolved");
      if (cfg->getPresolverResults()->it_mapping < cfg->getPresolverResults()->oneProcMappings.size()) {
        vector<tuple<int, int>> oneProcMapping =
            cfg->getPresolverResults()->oneProcMappings[cfg->getPresolverResults()->it_mapping];

        for (size_t a = 0; a < apps->n_SDFActors(); a++) {
          rel(*this, proc[a] == get<0>(oneProcMapping[apps->getSDFGraph(a)]));
          rel(*this, proc_mode[get<0>(oneProcMapping[apps->getSDFGraph(a)])] == get<1>(oneProcMapping[apps->getSDFGraph(a)]));
        }
      } else { //...otherwise forbid all mappings in oneProcMappings
        cout << "Now forbidding " << cfg->getPresolverResults()->oneProcMappings.size() << " mappings." << endl;
        cout << endl;
        for (size_t i = 0;
            i < cfg->getPresolverResults()->oneProcMappings.size(); i++) {
          vector<tuple<int, int>> oneProcMapping =
              cfg->getPresolverResults()->oneProcMappings[i];
          IntVarArgs t_mapping(*this, apps->n_programEntities(), 0,
              platform->nodes() - 1);
          for (size_t a = 0; a < apps->n_SDFActors(); a++) {
            rel(*this,
                t_mapping[a] == get<0>(oneProcMapping[apps->getSDFGraph(a)]));
          }
          rel(*this, t_mapping, IRT_NQ, proc);
        }
      }
    }

    switch (cfg->settings().criteria[0]) {
    case (Config::POWER):
      for (size_t i = 0;
          i < cfg->getPresolverResults()->sys_energys.size(); i++) {
        rel(*this, sys_power < cfg->getPresolverResults()->sys_energys[i]);
      }
      break;
    case (Config::THROUGHPUT):
      for (size_t i = 0; i < apps->n_SDFApps(); i++) {
        if (apps->getPeriodConstraint(i) == -1) {
          for (size_t j = 0; j < cfg->getPresolverResults()->periods.size(); j++) {
            rel(*this, period[i] < cfg->getPresolverResults()->periods[j][i]);
          }
          break;
        }
      }
      break;
    default:
      cout << "unknown optimization criterion !!!\n";
      throw 42;
      break;
    }

        for(size_t i = 0; i < channels.size(); i++){
            delete channels[i];
        }

        /**
         * Creating the branching strategy
         */
        cout << endl << "  Branching:" << endl;

        vector<double> ratio;
        vector<int> ids;
        bool heaviestFirst = true;
        for(size_t a = 0; a < apps->n_SDFApps(); a++){
            if(apps->getPeriodConstraint(a) < 0){
                heaviestFirst = false;
            }else{
                double new_ratio = (double) apps->getPeriodConstraint(a) / sumMinWCETs[a];
                if(ratio.size() == 0 || new_ratio > ratio.back()){
                    ratio.push_back(new_ratio);
                    ids.push_back(a);
                }else{
                    for(unsigned i = 0; i < ratio.size(); i++){
                        if(new_ratio < ratio[i]){
                            ratio.emplace(ratio.begin() + i, new_ratio);
                            ids.emplace(ids.begin() + i, a);
                            break;
                        }
                    }
                }
            }
        }
        cout << "    ";
        if(!heaviestFirst)
            cout << "not ";
        cout << "heaviestFirst" << endl;

        IntVarArgs procBranchOrderSAT;
        IntVarArgs procBranchOrderOPT;
        IntVarArgs procBranchOrderOther;
        cout << "    procBranchOrderSAT: " << endl;
        for(unsigned a = 0; a < ids.size(); a++){
            if(apps->getPeriodConstraint(ids[a]) > 0 && !cfg->doOptimize()){
                vector<int> branchProc = mapping->sortedByWCETs(ids[a]);
                cout << "      " << apps->getGraphName(ids[a]) << " [";
                for(int i = minA[ids[a]]; i <= maxA[ids[a]]; i++){
                    procBranchOrderSAT << proc[i];
                    //procBranchOrderSAT << rank[i];
                    cout << i << " ";
                }
                cout << "]" << endl;
            }
        }
        cout << "    procBranchOrderOPT: " << endl;
        for(size_t a = 0; a < ids.size(); a++){
            if(apps->getPeriodConstraint(ids[a]) > 0 && cfg->doOptimize()){
                vector<int> branchProc = mapping->sortedByWCETs(ids[a]);
                cout << "      " << apps->getGraphName(ids[a]) << " [";
                for(int i = minA[ids[a]]; i <= maxA[ids[a]]; i++){
                    procBranchOrderOPT << proc[i];
                    //procBranchOrderOPT << rank[i];
                    cout << i << " ";
                }
                cout << "]" << endl;
            }
        }
        for(size_t a = 0; a < apps->n_SDFApps(); a++){
            if(apps->getPeriodConstraint(a) < 0 && cfg->doOptimize()){
                vector<int> branchProc = mapping->sortedByWCETs(a);
                cout << "      " << apps->getGraphName(a) << " [";
                for(int i = minA[a]; i <= maxA[a]; i++){
                    procBranchOrderOPT << proc[i];
                    //procBranchOrderOPT << rank[i];
                    cout << i << " ";
                }
                cout << "]" << endl;
            }
        }
        cout << "    procBranchOrderOther: " << endl;
        for(unsigned a = 0; a < apps->n_SDFApps(); a++){
            if(apps->getPeriodConstraint(a) <= 0 && !cfg->doOptimize()){
                cout << "      " << apps->getGraphName(a) << " [";
                for(int i = minA[a]; i <= maxA[a]; i++){
                    procBranchOrderOther << proc[i];
                    //procBranchOrderOther << rank[i];
                    cout << i << " ";
                }
                cout << "]" << endl;
            }
        }
        cout << endl;
        //branch(*this, next, INT_VAR_NONE(), INT_VAL_MIN());

        if(!heaviestFirst && (procBranchOrderSAT.size() > 0 || procBranchOrderOPT.size() > 0)){
            //branch(*this, proc_mode, INT_VAR_AFC_MAX(0.99), INT_VAL_MIN());
            branch(*this, procBranchOrderSAT, INT_VAR_NONE(), INT_VAL_MIN());
            branch(*this, procBranchOrderOPT, INT_VAR_NONE(), INT_VAL_MIN());
            branch(*this, proc_mode, INT_VAR_NONE(), INT_VAL_MIN());
        }else if(heaviestFirst && (procBranchOrderSAT.size() > 0 || procBranchOrderOPT.size() > 0)){
            branch(*this, procBranchOrderSAT, INT_VAR_NONE(), INT_VAL_MIN());
            branch(*this, procBranchOrderOPT, INT_VAR_NONE(), INT_VAL_MIN());
            branch(*this, proc_mode, INT_VAR_AFC_MAX(0.99), INT_VAL_MIN());
        }else{
            branch(*this, procBranchOrderOther, INT_VAR_NONE(), INT_VAL_MIN());
            branch(*this, proc_mode, INT_VAR_NONE(), INT_VAL_MIN());
        }
        
        if(platform->getInterconnectType() == TDN_NOC){
          branch(*this, chosenRoute, INT_VAR_NONE(), INT_VAL_MIN()); 
          assign(*this, tdnTable, INT_ASSIGN_MAX()); 
          branch(*this, noc_mode, INT_VAL_MIN());
        }else if(platform->getInterconnectType() == TDMA_BUS){
          branch(*this, tdmaAlloc, INT_VAR_NONE(), INT_VAL_MIN());  
        }

        //branch(*this, rank, INT_VAR_NONE(), INT_VAL_MIN());
        branch(*this, next, INT_VAR_NONE(), INT_VAL_MIN());
        /**
         * ordering of sending and receiving messages with same
         * source (send) or destination (rec) for unresolved cases
         */
        for(size_t k = 0; k < apps->n_programChannels() + platform->nodes(); k++){
            assign(*this, recNext[k], INT_ASSIGN_MIN());
        }
        branch(*this, sendNext, INT_VAR_NONE(), INT_VAL_MIN());

        branch(*this, proc, INT_VAR_NONE(), INT_VAL(&valueProc));
    }else{ /**< end of SDF related constraints and branching. */
        /**
         * Memory
         */
        cout << "Inserting memory constraints \n";
#include "memory.constraints"
        /**
         * Branching for the periodic tasks
         * Since the tasks are ordered from heavy to light,
         * We use INT_VAR_NONE() to select the heaviest.
         * We also use valueProc to select the minimu slack proccessor
         * to mimic bestfit algorithm
         */
        branch(*this, proc, INT_VAR_NONE(), INT_VAL(&valueProc));
        branch(*this, proc_mode, INT_VAR_AFC_MAX(0.99), INT_VAL_MIN());
    }
}

SDFPROnlineModel::SDFPROnlineModel(bool share, SDFPROnlineModel& s):
    Space(share, s),
    apps(s.apps),
    platform(s.platform),
    mapping(s.mapping),
    cfg(s.cfg),
    least_power_est(s.least_power_est){

    next.update(*this, share, s.next);
    rank.update(*this, share, s.rank);
    proc.update(*this, share, s.proc);
    proc_mode.update(*this, share, s.proc_mode);
    tdmaAlloc.update(*this, share, s.tdmaAlloc);
    tdnTable.update(*this, share, s.tdnTable);
    chosenRoute.update(*this, share, s.chosenRoute);
    sendNext.update(*this, share, s.sendNext);
    recNext.update(*this, share, s.recNext);
    sendbufferSz.update(*this, share, s.sendbufferSz);
    recbufferSz.update(*this, share, s.recbufferSz);
    period.update(*this, share, s.period);
    proc_period.update(*this, share, s.proc_period);
    latency.update(*this, share, s.latency);
    procsUsed.update(*this, share, s.procsUsed);
    utilization.update(*this, share, s.utilization);
    sys_utilization.update(*this, share, s.sys_utilization);
    procsUsed_utilization.update(*this, share, s.procsUsed_utilization);
    proc_power.update(*this, share, s.proc_power);
    sys_power.update(*this, share, s.sys_power);
    proc_area.update(*this, share, s.proc_area);
    sys_area.update(*this, share, s.sys_area);
    proc_cost.update(*this, share, s.proc_cost);
    sys_cost.update(*this, share, s.sys_cost);
    wcct_b.update(*this, share, s.wcct_b);
    wcct_s.update(*this, share, s.wcct_s);
    wcct_r.update(*this, share, s.wcct_r);
}

Space* SDFPROnlineModel::copy(bool share) {
    return new SDFPROnlineModel(share, *this);
}

void SDFPROnlineModel::print(std::ostream& out) const {
    out << "----------------------------------------" << endl;
    out << "Proc: " << proc << endl;
    out << "proc mode: " << proc_mode << endl;
    out << "Latency: " << latency << endl;
    out << "Period: " << period << endl;
    out << "Procs used: " << procsUsed << endl;
    out << "Proc_period: " << proc_period << endl;
    out << "Proc utilization: " << utilization << endl;
    out << "Sys utilization: " << sys_utilization << endl;
    out << "ProcsUsed utilization: " << procsUsed_utilization << endl;
    out << "proc power: " << proc_power << endl;
    out << "sys power: " << sys_power << endl;
    out << "least_power_est: " << least_power_est << endl;
    out << "proc area: " << proc_area << endl;
    out << "sys area: " << sys_area << endl;
    out << "proc cost: " << proc_cost << endl;
    out << "sys cost: " << sys_cost << endl;
    out << "Next: ";
    for(size_t ii = 0; ii < apps->n_SDFActors(); ii++){
        out << next[ii] << " ";
    }
    out << "|| ";
    for(size_t ii = 0; ii < platform->nodes(); ii++){
        out << next[apps->n_SDFActors() + ii] << " ";
    }
    out << endl;
    out << "Rank: ";
    for(size_t ii = 0; ii < apps->n_SDFActors(); ii++){
        out << rank[ii] << " ";
    }
    out << endl;
    out << "TDMA slots: " << tdmaAlloc << endl;
    //print TDN table
    out << endl << "Chosen routes: " << chosenRoute << endl;
    
    vector<tdn_graphNode> tdn_graph = platform->getTDNGraph();
    out << endl << "TDN table: " << endl;
    for(size_t ii = 0; ii < tdnTable.size(); ii++){
      if(ii!=0 && ii%platform->getTDNCycles()==0){out << endl;}
      if(ii%platform->getTDNCycles()==0){
        out << ((tdn_graph[ii].link.from==-1)?"NI":("SW"+tools::toString(tdn_graph[ii].link.from))) << " -> ";
        out << ((tdn_graph[ii].link.to==-1)?"NI":("SW"+tools::toString(tdn_graph[ii].link.to))) << ": ";
      }
      if(tdnTable[ii].assigned() && tdnTable[ii].val()==platform->nodes()){
        out << "_";
      }else{
        out << tdnTable[ii];
      }
      out << " ";
      
      if(ii == platform->getTDNCycles()*platform->nodes()-1)
        out << endl << "-------------------------------------------";
    }
    out << endl << endl;
    out << "S-order: " << sendNext << endl;
    out << "wcct_b: " << wcct_b << endl;
    out << "wcct_s: " << wcct_s << endl;
    out << "R-order: " << recNext << endl;
    out << "wcct_r: " << wcct_r << endl;
    out << "----------------------------------------" << endl;

}

void SDFPROnlineModel::printCSV(std::ostream& out) const {
    const char sep = ',';
    for(auto i = 0; i < latency.size(); i++)
        out << latency[i] << sep;
    for(auto i = 0; i < period.size(); i++)
        out << period[i] << sep;
    out << procsUsed << sep;
    out << sys_utilization << sep;
    out << sys_power << sep;
    out << least_power_est << sep;
    out << sys_area << sep;
    out << sys_cost << sep;
    out << endl;
}

void SDFPROnlineModel::printMappingCSV(std::ostream& out) const {
    const char sep = ',';
    for(auto i = 0; i < proc.size(); i++)
        out << proc[i] << sep;
    for(auto i = 0; i < proc_mode.size(); i++)
        out << proc_mode[i] << sep;
    out << endl;
}

vector<int> SDFPROnlineModel::getPeriodResults() {
    vector<int> periods;
    for(auto i = 0; i < period.size(); i++){
        periods.push_back(period[i].min());
    }
    return periods;
}

int SDFPROnlineModel::valueProc(const Space& home, IntVar x, int i) {
    int min_slack_proc = x.min();

    int min_slack = static_cast<const SDFPROnlineModel&>(home).mapping->max_utilization;
    for(IntVarValues k(x); k(); ++k){
        int slack = static_cast<const SDFPROnlineModel&>(home).mapping->max_utilization - static_cast<const SDFPROnlineModel&>(home).utilization[k.val()].min();
        if(slack > 0 && slack < min_slack){
            min_slack = slack;
            min_slack_proc = k.val();
        }
    }
    return min_slack_proc;
}

Mapping* SDFPROnlineModel::extractResult() {
    //cout << "Creating mapping..." << endl;

    vector<int> mem_load;
    //TODO: memory consumption data should be available
    //mem_load.push_back(memConsData[j].min()+memConsCode[j].min());
    // !!!! mapping->setMemoryLoad(mem_load);

    //cout << "  ..set schedules and mapping for mapping \n";
    vector<vector<int>> t_mappingSched = extractSchedules();
    mapping->setMappingScheds(t_mappingSched);

    //cout << "  ..set processor-related results. \n";
    for(size_t n = 0; n < platform->nodes(); n++){
        mapping->setProcMode(n, proc_mode[n].val());
        mapping->setProcPeriod(n, proc_period[n].min());
        mapping->setProcUtilization(n, utilization[n].val());
        mapping->setProcEnergy(n, proc_power[n].val());
        mapping->setProcArea(n, proc_area[n].val());
        mapping->setProcCost(n, proc_cost[n].val());
    }

    vector<int> tdma_slots;
    //cout << "  ..set tdma_slots for mapping \n";
    for(size_t j = 0; j < platform->nodes(); j++){
        tdma_slots.push_back(tdmaAlloc[j].min());
    }
    mapping->setTDMAslots(tdma_slots);

    //cout << "  ..set system-related results. \n";
    mapping->setProcsUsed(procsUsed.val());
    mapping->setSysUtilization(sys_utilization.val());
    mapping->setProcsUsedUtilization(procsUsed_utilization.val());
    mapping->setSysEnergy(sys_power.val());
    mapping->setSysArea(sys_area.val());
    mapping->setSysCost(sys_cost.val());

    //cout << "  .. set channel schedule and communication delays \n";

    //extract the static schedule for messages
    vector<vector<int>> commSched = extractCommSchedules();
    mapping->setCommScheds(commSched);

    for(size_t k = 0; k < apps->n_programChannels(); k++){
        mapping->setSendBuff(k, sendbufferSz[k].min());
        mapping->setRecBuff(k, recbufferSz[k].min());
        div_t tmp_wcct;
        tmp_wcct.quot = wcct_b[k].val();
        tmp_wcct.rem = wcct_s[k].val();
        mapping->setCommDelay(k, tmp_wcct);
    }

    //cout << "  .. set period and initial latency \n";
    for(size_t i = 0; i < apps->n_SDFApps(); i++){
        mapping->setPeriod(i, period[i].min());
        mapping->setInitLatency(i, latency[i].min());
    }

    //TODO: check if this part is still needed
    vector<int> proc_tmp; /** a temp vector for converting IntVarArray to int vector. */
    vector<int> proc_mode_tmp; /** a temp vector for converting IntVarArray to int vector. */

    for(size_t j = 0; j < platform->nodes(); j++)
        proc_mode_tmp.push_back(proc_mode[j].val());
    for(size_t i = 0; i < apps->n_programEntities(); i++)
        proc_tmp.push_back(proc[i].val());

    mapping->setMappingMode(proc_tmp, proc_mode_tmp);

    return mapping;
}

vector<vector<int>> SDFPROnlineModel::extractSchedules() const {
    //cout << "---extractSchedules start---" << endl;
    size_t checkSize = 0;
    vector<vector<int>> schedules(platform->nodes(), vector<int>());
    size_t nextActor = apps->n_SDFActors() + platform->nodes() - 1;
    int t_proc;
    while(checkSize < apps->n_SDFActors()){
        //find the first/next actor on the first/next allocated proc
        while(nextActor >= apps->n_SDFActors()){
            nextActor = next[nextActor].val();
        }
        //identify which proc the current schedule is for
        t_proc = proc[nextActor].val();
        //cout << "Schedule on proc " << t_proc << ": \n\t";
        //add actors to schedule in schedule-order
        while(nextActor < apps->n_SDFActors()){
            schedules[t_proc].push_back(nextActor);
            //cout << apps->getName(nextActor) << " -> ";
            checkSize++;
            nextActor = next[nextActor].val();
        }
        //cout << endl;
    }

    //cout << "---extractSchedules end---" << endl;
    return schedules;
}

vector<vector<int>> SDFPROnlineModel::extractCommSchedules() const {
    vector<SDFChannel*> channelList = apps->getChannels();
    vector<vector<int>> schedules(platform->nodes(), vector<int>());

    //cout << "---extractCommSchedules start---" << endl;
    size_t checkSize = 0;
    size_t nextChannel = apps->n_programChannels() + platform->nodes() - 1;
    int t_proc;
    while(checkSize < apps->n_programChannels()){
        //find the first/next actor on the first/next allocated proc
        while(nextChannel >= apps->n_programChannels()){
            nextChannel = sendNext[nextChannel].val();
        }
        //identify which proc the current schedule is for
        t_proc = proc[channelList[nextChannel]->source].val();
        //cout << "Comm-Schedule on proc " << t_proc << ": \n\t";
        //add channels to schedule in schedule-order
        while(nextChannel < apps->n_programChannels()){
            schedules[t_proc].push_back(nextChannel);
            //cout << channelList[nextChannel]->id << "(" << channelList[nextChannel]->src_name << " -> ";
            //cout << channelList[nextChannel]->dst_name << ") -- ";
            checkSize++;
            nextChannel = sendNext[nextChannel].val();
        }
        //cout << endl;
    }

    return schedules;
}
