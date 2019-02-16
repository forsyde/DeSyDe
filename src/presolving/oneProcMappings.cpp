#include "oneProcMappings.hpp"

OneProcModel::OneProcModel(Mapping* p_mapping, Config& cfg):
    apps(p_mapping->getApplications()),
    platform(p_mapping->getPlatform()),
    mapping(p_mapping),
    settings(cfg),
    proc(*this, apps->n_SDFApps(), 0, platform->nodes()-1),
    proc_mode(*this, platform->nodes(), 0, platform->getMaxModes()),
    noc_mode(*this, 0, platform->getInterconnectModes()-1) {
    
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
  cout << "  Found " << apps->n_SDFApps() << " SDF applications: " << endl;
  for (size_t ii=0; ii<apps->n_SDFApps(); ii++){
    cout << "   " << ii << ": " << apps->getGraphName(ii) << endl;
  }

  //CONSTRAINTS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  for (size_t ji=0; ji<platform->nodes(); ji++){
    rel(*this, proc_mode[ji] < platform->getModes(ji));
  }

  IntVarArgs nAppsOnProc(*this, platform->nodes(), 0, apps->n_SDFApps()); /**< number of SDF apps on proc[i]. */
  count(*this, proc, nAppsOnProc);

  if(mapping->homogeneousPlatform()){
    for (size_t ii=0; ii<apps->n_SDFApps()-1; ii++){
      for (size_t ij=ii+1; ij<apps->n_SDFApps(); ij++){
        rel(*this, proc[ii]==proc[ij] || proc[ii]<proc[ij]);
      }
    }
  }else{//platform not homogeneous
    for (size_t ji=0; ji<platform->nodes(); ji++){
      rel(*this, (nAppsOnProc[ji]==0) >> (proc_mode[ji]==0));
      for (size_t jj=ji+1; jj<platform->nodes(); jj++){
        if(mapping->homogeneousModeNodes(ji, jj)){
          rel(*this, !(nAppsOnProc[ji]==0 && nAppsOnProc[jj]>0));
          for (size_t ii=0; ii<apps->n_SDFApps()-1; ii++){
            for (size_t ij=ii+1; ij<apps->n_SDFApps(); ij++){
              rel(*this, !(proc[ii]==jj && proc[ij]==ji));
            }
          }
        }
      }
    }
  }
  

  
//  cout << endl << "  Branching:" << endl;
  branch(*this, proc, INT_VAR_NONE(), INT_VAL_MIN());
  branch(*this, proc_mode, INT_VAR_NONE(), INT_VAL_MIN());
  branch(*this, noc_mode, INT_VAL_MIN());
}
  
OneProcModel::OneProcModel(bool share, OneProcModel& s):
  Space(share, s),
  apps(s.apps),
  platform(s.platform),
  mapping(s.mapping),
  settings(s.settings){
  proc.update(*this, share, s.proc);
  proc_mode.update(*this, share, s.proc_mode);
  noc_mode.update(*this, share, s.noc_mode);
}
  
// Copies the space
Space* OneProcModel::copy(bool share){
  return new OneProcModel(share, *this);
}
  
// Prints the variables
void OneProcModel::print(std::ostream& out) const{
  out << "Proc: " << proc << endl;
  out << "proc mode: " << proc_mode << endl;
  out << "noc mode: " << noc_mode << endl;
}

tuple<int, vector<tuple<int,int>>> OneProcModel::getResult() const{
  vector<tuple<int,int>> result_v;
  for(int i=0; i<proc.size(); i++){
    if(proc[i].assigned() && proc_mode[proc[i].val()].assigned()){
      result_v.push_back(tuple<int,int>(proc[i].val(), proc_mode[proc[i].val()].val()));
    }else{
      //throw some exception
    }
  }
  tuple<int, vector<tuple<int,int>>> result = tuple<int, vector<tuple<int,int>>>(noc_mode.val(), result_v);
  return result;
}

