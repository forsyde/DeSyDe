#include "model.hpp"

ADSEModel::ADSEModel(Mapping* p_mapping, Config* _cfg)
  : apps     (p_mapping->getApplications()),
    platform (p_mapping->getPlatform()),
    mapping  (p_mapping),
    cfg(_cfg),
    next                  (*this, apps->n_SDFActors()+platform->nodes(), 0, 
                           apps->n_SDFActors()+platform->nodes()), 
    proc                  (*this, apps->n_programEntities(), 0, platform->nodes()-1),
    proc_mode             (*this, platform->nodes(), 0, 2),
    tdmaAlloc             (*this, platform->nodes(), 0, platform->tdmaSlots()),
    sendNext              (*this, apps->n_programChannels()+platform->nodes(), 
                           0, apps->n_programChannels()+platform->nodes()),
    recNext               (*this, apps->n_programChannels()+platform->nodes(), 
                           0, apps->n_programChannels()+platform->nodes()),
    hyperPeriod           (*this, platform->nodes(), 0, apps->getMaxHyperperiod()),
    start                 (*this, apps->n_IPTTasks()*apps->getMaxNumberOfIPTInstances(),  
                           0, apps->getMaxHyperperiod()+1),
    end                   (*this, apps->n_IPTTasks()*apps->getMaxNumberOfIPTInstances(), 
                           0, apps->getMaxHyperperiod()+1),
    exec                  (*this, apps->n_IPTTasks()*apps->getMaxNumberOfIPTInstances(),
                           IntSet::empty, IntSet(0, apps->getMaxHyperperiod()-1),
                           0, p_mapping->getMaxWCET_IPT()),
    period                (*this, apps->n_SDFApps(), 0, Int::Limits::max), 
    proc_period           (*this, platform->nodes(), 0, Int::Limits::max),
    latency               (*this, apps->n_SDFApps(), 0, Int::Limits::max), 
    procsUsed             (*this, 1, platform->nodes()),
    utilization           (*this, platform->nodes(), 0, 1000),//precision: 1/10
    sys_utilization       (*this, 0, 1000),
    procsUsed_utilization (*this, 0, 1000),
    proc_power            (*this, platform->nodes(), 0, Int::Limits::max),
    sys_power             (*this, 0, Int::Limits::max),
    proc_area             (*this, platform->nodes(), 0, Int::Limits::max),
    sys_area              (*this, 0, Int::Limits::max),
    proc_cost             (*this, platform->nodes(), 0, Int::Limits::max),
    sys_cost              (*this, 0, Int::Limits::max),
    wcct_b                (*this, apps->n_programChannels(), 0, Int::Limits::max),
    wcct_s                (*this, apps->n_programChannels(), 0, Int::Limits::max),
    wcct_r                (*this, apps->n_programChannels(), 0, Int::Limits::max) {
    
  vector<SDFChannel*> channels = apps->getChannels();
  
  cout << "Creating Model..." << endl;
  cout << "  Platform with " << platform->nodes() << " nodes is ";
  if(!mapping->homogeneousPlatform()) cout << "not ";
  cout << "homogeneous." << endl;

  if(!mapping->homogeneousPlatform()){
    cout << "  Homogeneous nodes: ";
    for (size_t ji=0; ji<platform->nodes(); ji++){
      for (size_t jj=ji+1; jj<platform->nodes(); jj++){
        if(mapping->homogeneousNodes(ji, jj)){
          cout << "{" << ji << ", " << jj << "} ";
        }
      }
    }
    cout << endl;
    cout << "  Mode-homogeneous nodes: ";
    for (size_t ji=0; ji<platform->nodes(); ji++){
      for (size_t jj=ji+1; jj<platform->nodes(); jj++){
        if(mapping->homogeneousModeNodes(ji, jj)){
          cout << "{" << ji << ", " << jj << "} ";
        }
      }
    }
    cout << endl;
  }
  cout << "  Found " << apps->n_programEntities() << " program entities (",
    cout << apps->n_SDFActors()<< " actors of ";
  cout << apps->n_SDFApps() << " SDFGs and ";
  cout << apps->n_IPTTasks() << " ipt tasks)." << endl;
  cout << "   --Actors and Tasks------------------\n";
  for (size_t ii=0; ii<apps->n_programEntities(); ii++){
    cout << "   " << ii << ": " << apps->getName(ii) << endl;
  }
  
  cout << "   --Channels------------------\n";
  for (size_t ki=0; ki<channels.size(); ki++){
    int src_ch1 = channels[ki]->source;
    int dst_ch1 = channels[ki]->destination;
    int tok_ch1 = channels[ki]->initTokens;
   
    cout << "   Ch " << ki << ": ";
    cout << apps->getName(src_ch1) << " -> ";
    cout << apps->getName(dst_ch1) <<  "   (";
    cout << src_ch1 << " -> ";
    cout << dst_ch1 << ")";
    if(tok_ch1>0) cout << " *";
    cout << endl;
  }
  cout << endl;
  
  for (size_t a=0; a<apps->n_SDFApps(); a++){
    if(apps->getPeriodBound(a) > 0){
      cout << "   Period bound for " 
           << apps->getGraphName(a) << ": " 
           << apps->getPeriodBound(a) << endl;
    }
  }
  
  // ### MAPPING ###
  //number of SDF actors on proc[i]
  IntVarArgs nSDFAsOnProc      (*this, platform->nodes(), 0, apps->n_SDFActors()); 
  //number of IPTs on proc[i]
  IntVarArgs nTasksOnProc      (*this, platform->nodes(), 0, apps->n_IPTTasks());  
  //number of SDF actors and IPTs combined on proc[i]
  IntVarArgs nEntitiesOnProc   (*this, platform->nodes(), 0, apps->n_programEntities()); 
  IntVarArgs proc_SDF_wcet_sum (*this, platform->nodes(), 0, Int::Limits::max);
#include "mapping.constraints"

  // ### SCHEDULING ###
  IntVarArgs rank              (*this, apps->n_SDFActors(), 0, apps->n_SDFActors()-1);
#include "scheduling.constraints"

  // ### IPT SCHEDULING
  IntVarArgs n_instances              (*this, apps->n_IPTTasks(), 0,
                                       apps->getMaxNumberOfIPTInstances());
  IntVarArgs instancesOnNode          (*this, apps->n_IPTTasks()*platform->nodes(), 0,
                                       apps->getMaxNumberOfIPTInstances()*platform->nodes());
  Matrix<IntVarArgs> instancesOnNodeM (instancesOnNode, apps->n_IPTTasks(), 
                                       platform->nodes());
  Matrix<IntVarArray> startM          (start, apps->getMaxNumberOfIPTInstances(),
                                       apps->n_IPTTasks());
  Matrix<IntVarArray> endM            (end, apps->getMaxNumberOfIPTInstances(), 
                                       apps->n_IPTTasks());
  Matrix<SetVarArray> execM           (exec, apps->getMaxNumberOfIPTInstances(),
                                       apps->n_IPTTasks());
#include "ipt_scheduling.constraints"
  
  // ### COMMUNICATION ###
  IntVarArgs blockingTime_s (*this, apps->n_programChannels(), 0, Int::Limits::max);
  IntVarArgs blockingTime_r (*this, apps->n_programChannels(), 0, Int::Limits::max);
  IntVarArgs transferTime_s (*this, apps->n_programChannels(), 0, Int::Limits::max);
  IntVarArgs transferTime_r (*this, apps->n_programChannels(), 0, Int::Limits::max);
  //sending and receiving buffer sizes
  IntVarArgs sendbufferSz   (*this, apps->n_programChannels(), 0, Int::Limits::max);
  IntVarArgs recbufferSz    (*this, apps->n_programChannels(), 1, Int::Limits::max);
#include "wcct.constraints"
  
  // ### MEMORY ###
#include "memory.constraints"
  
  // ### THROUGHPUT ###
  vector<int> maxMinWcet(apps->n_SDFApps(), 0);
  vector<int> maxMinWcetActor(apps->n_SDFActors(), 0);
  vector<int> sumMinWCETs(apps->n_SDFApps(), 0);
  vector<int> minA(apps->n_SDFApps(), -1);
  vector<int> maxA(apps->n_SDFApps(), 0);
  for (size_t a=0; a<apps->n_SDFApps(); a++){
    for (size_t i=0; i<apps->n_SDFActors(); i++){
      if(apps->getSDFGraph(i)==a){
        if(minA[a] == -1) minA[a] = i;
        maxA[a] = i;
        vector<int> wcets;
        for (size_t j=0; j<platform->nodes(); j++){
          vector<int> wcets_proc = mapping->getValidWCETs(i, j);
          if(wcets_proc.size()>0)
            wcets.push_back(*min_element(wcets_proc.begin(),wcets_proc.end()));
        }
        if(maxMinWcet[a]<*min_element(wcets.begin(),wcets.end()))
          maxMinWcet[a] = *min_element(wcets.begin(),wcets.end());
        if(maxMinWcetActor[i]<*min_element(wcets.begin(),wcets.end()))
          maxMinWcetActor[i] = *min_element(wcets.begin(),wcets.end());
          
        sumMinWCETs[a] += maxMinWcetActor[i];
      }
    }
  }
#include "throughput.constraints"

  for (size_t i=0; i<channels.size(); i++){
    delete channels[i];  
  }
  
  cout << endl << "  Branching:" << endl;  
  
  vector<double> ratio;
  vector<int> ids;
  
  bool heaviestFirst = true;
  for (size_t a=0; a<apps->n_SDFApps(); a++){
    if(apps->getPeriodBound(a) < 0){
      heaviestFirst = false;
    }else{
      double new_ratio = (double)apps->getPeriodBound(a)/sumMinWCETs[a];
      if(ratio.size() == 0 || new_ratio > ratio.back()){
        ratio.push_back(new_ratio);
        ids.push_back(a);
      }else{
        for(unsigned i=0; i<ratio.size(); i++){
          if(new_ratio < ratio[i]){
            ratio.emplace(ratio.begin()+i, new_ratio);
            ids.emplace(ids.begin()+i, a);
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
  
  //if(heaviestFirst){
  IntVarArgs procBranchOrderSAT;
  IntVarArgs procBranchOrderOPT;
  cout << "    procBranchOrderSAT: " << endl;
  for(unsigned a=0; a<ids.size(); a++){
    if(apps->getPeriodBound(ids[a]) > 0 && apps->getConstrType(ids[a]) != FIRSTOPT){
      vector<int> branchProc = mapping->sortedByWCETs(ids[a]);
      cout << "     app" << ids[a] << " [";
      for (int i=minA[ids[a]]; i<=maxA[ids[a]]; i++){
        //procBranchOrderSAT << proc[branchProc[i]];
        //cout << branchProc[i] << " ";
        procBranchOrderSAT << proc[i];
        cout << i << " ";
      }
      cout << "]" << endl;
    }
  }
  cout << "    procBranchOrderOPT: " << endl;
  for (size_t a=0; a<ids.size(); a++){
    if(apps->getPeriodBound(ids[a]) > 0 && apps->getConstrType(ids[a]) == FIRSTOPT){
      vector<int> branchProc = mapping->sortedByWCETs(ids[a]);
      cout << "     app" << ids[ids[a]] << " [";
      for (int i=minA[ids[a]]; i<=maxA[ids[a]]; i++){
        //procBranchOrderOPT << proc[branchProc[i]];
        ////cout << branchProc[i] << " ";
        procBranchOrderOPT << proc[i];
        cout << i << " ";
      }
      cout << "]" << endl;
    }
  }
  for (size_t a=0; a<apps->n_SDFApps(); a++){
    if(apps->getPeriodBound(a) < 0 && apps->getConstrType(a) == FIRSTOPT){
      vector<int> branchProc = mapping->sortedByWCETs(a);
      cout << "   app" << a << " [";
      for (int i=minA[a]; i<=maxA[a]; i++){
        //procBranchOrderOPT << proc[branchProc[i]];
        //cout << branchProc[i] << " ";
        procBranchOrderOPT << proc[i];
        cout << i << " ";
      }
      cout << "]" << endl;
    }
  }
  //just testing
  //branch(*this, proc, INT_VAR_NONE(), INT_VAL_MIN());
  if(!heaviestFirst){
    branch(*this, proc_mode, INT_VAR_NONE(), INT_VAL_MAX());
    branch(*this, procBranchOrderSAT, INT_VAR_NONE(), INT_VAL_MIN());
    branch(*this, procBranchOrderOPT, INT_VAR_NONE(), INT_VAL_MIN());
    //cout << "branch(*this, proc_mode, INT_VAR_NONE(), INT_VAL_MAX());" << endl;
  }else{
    branch(*this, procBranchOrderSAT, INT_VAR_NONE(), INT_VAL_MIN());
    branch(*this, procBranchOrderOPT, INT_VAR_NONE(), INT_VAL_MIN());
    branch(*this, proc_mode, INT_VAR_NONE(), INT_VAL_MIN());
    //cout << "branch(*this, proc_mode, INT_VAR_NONE(), INT_VAL_MIN());" << endl;
  }
  //}else{
  //branch(*this, proc_mode, INT_VAR_NONE(), INT_VAL_MAX());
  //branch(*this, proc, INT_VAR_NONE(), INT_VAL_MIN());
  //}
  IntVarArgs nextBranch;
  /*for (size_t j=0; j<platform->nodes(); j++){
    nextBranch << next[apps->n_programEntities()+j];
    }
    for (size_t i=0; i<apps->n_programEntities(); i++){
    nextBranch << next[i];
    }*/
  branch(*this, next, INT_VAR_NONE(), INT_VAL_MIN());
  branch(*this, tdmaAlloc, INT_VAR_NONE(), INT_VAL_MIN());
  //ordering of sending and receiving messages with same 
  //source (send) or destination (rec) for unresolved cases
  //branch(*this, recNext, INT_VAR_NONE(), INT_VAL_MIN());
  for (size_t k=0; k<apps->n_programChannels()+platform->nodes(); k++){
    assign(*this, recNext[k], INT_ASSIGN_MIN());
  }
  branch(*this, sendNext, INT_VAR_NONE(), INT_VAL_MIN());
  
  //Branching for IPTs
  branch(*this, proc, INT_VAR_NONE(), INT_VAL_MIN());
  for (size_t i=0; i<apps->n_IPTTasks(); i++){
    for (auto k=0; k<apps->getMaxNumberOfIPTInstances(); k++){
      branch(*this, start[i*apps->getMaxNumberOfIPTInstances()+k], INT_VAL_MIN());
      branch(*this, end[i*apps->getMaxNumberOfIPTInstances()+k], INT_VAL_MIN());
    }
  }
}
  
ADSEModel::ADSEModel(bool share, ADSEModel& s):
  Space(share, s),
  apps(s.apps),
  platform(s.platform),
  mapping(s.mapping){
  next.update(*this, share, s.next);
  proc.update(*this, share, s.proc);
  proc_mode.update(*this, share, s.proc_mode);
  tdmaAlloc.update(*this, share, s.tdmaAlloc);
  sendNext.update(*this, share, s.sendNext);
  recNext.update(*this, share, s.recNext);
  hyperPeriod.update(*this, share, s.hyperPeriod);
  start.update(*this, share, s.start);
  end.update(*this, share, s.end);
  exec.update(*this, share, s.exec);
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
  
// Copies the space
Space* ADSEModel::copy(bool share){
  return new ADSEModel(share, *this);
}
  
// Prints the variables
void ADSEModel::print(std::ostream& out) const{
  out << "Latency: " << latency << endl;
  out << "Period: " << period << endl << endl;
  out << "Procs used: " << procsUsed << endl;
  out << "Proc_period: " << proc_period << endl;
  out << "Proc utilization: " << utilization << endl;
  out << "Sys utilization: " << sys_utilization << endl;
  out << "ProcsUsed utilization: " << procsUsed_utilization << endl << endl;
  out << "proc mode: " << proc_mode << endl;
  out << "proc power: " << proc_power << endl;
  out << "sys power: " << sys_power << endl;
  out << "proc area: " << proc_area << endl;
  out << "sys area: " << sys_area << endl;
  out << "proc cost: " << proc_cost << endl;
  out << "sys cost: " << sys_cost << endl << endl;
  out << "Proc: " << proc << endl;
  out << "Next: " ;
  for (size_t ii=0; ii<apps->n_SDFActors(); ii++){
    out << next[ii] << " ";
  }
  out << "|| ";
  for (size_t ii=0; ii<platform->nodes(); ii++){
    out << next[apps->n_SDFActors()+ii] << " ";
  }
  out << endl;
  out << endl;
  out << "TDMA slots: " << tdmaAlloc << endl;
  out << "S-order: " << sendNext << endl;
  out << "wcct_b: " << wcct_b << endl;
  out << "wcct_s: " << wcct_s << endl;
  out << "R-order: " << recNext << endl;
  out << "wcct_r: " << wcct_r << endl;
  out << endl;
  out << "hyperPeriod: " << hyperPeriod << endl;
  out << "start: " << endl;
  for (size_t i=0; i<apps->n_IPTTasks(); i++){
    for (auto k=0; k<apps->getMaxNumberOfIPTInstances(); k++){
      if(start[i*apps->getMaxNumberOfIPTInstances()+k].assigned() &&
         (start[i*apps->getMaxNumberOfIPTInstances()+k].val()==apps->getMaxHyperperiod()+1))
        out << "X ";
      else
        out << start[i*apps->getMaxNumberOfIPTInstances()+k] << " ";
    }
    out << endl;
  }
  out << "end: " << endl;
  for (size_t i=0; i<apps->n_IPTTasks(); i++){
    for (auto k=0; k<apps->getMaxNumberOfIPTInstances(); k++){
      if(end[i*apps->getMaxNumberOfIPTInstances()+k].assigned() &&
         (end[i*apps->getMaxNumberOfIPTInstances()+k].val()==apps->getMaxHyperperiod()+1))
        out << "X ";
      else
        out << end[i*apps->getMaxNumberOfIPTInstances()+k] << " ";
    }
    out << endl;
  }
  out << "exec: " << endl;
  for (size_t i=0; i<apps->n_IPTTasks(); i++){
    for (auto k=0; k<apps->getMaxNumberOfIPTInstances(); k++){
      /*if(exec[i*apps->getMaxNumberOfIPTInstances()+k].assigned() &&
        (exec[i*apps->getMaxNumberOfIPTInstances()+k].val()==apps->getMaxHyperperiod()+1))
        out << "X ";
        else*/
      out << exec[i*apps->getMaxNumberOfIPTInstances()+k] << " ";
    }
    out << endl;
  }
}

vector<int> ADSEModel::getPeriodResults(){
  vector<int> periods;
  for (auto i=0; i<period.size(); i++){
    periods.push_back(period[i].min());
  }  
  return periods;
}
