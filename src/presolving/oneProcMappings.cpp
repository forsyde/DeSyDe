#include "oneProcMappings.hpp"

OneProcModel::OneProcModel(Mapping* p_mapping, DSESettings* dseSettings):
    apps(p_mapping->getApplications()),
    platform(p_mapping->getPlatform()),
    mapping(p_mapping),
    settings(dseSettings),
    proc(*this, apps->n_programEntities(), 0, platform->nodes()-1),
    proc_mode(*this, platform->nodes(), 0, platform->getMaxModes()) {
    
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
  

  
  cout << endl << "  Branching:" << endl;
  branch(*this, proc, INT_VAR_NONE(), INT_VAL_MIN());
  branch(*this, proc_mode, INT_VAR_NONE(), INT_VAL_MIN());
}
  
OneProcModel::OneProcModel(bool share, OneProcModel& s):
  Space(share, s),
  apps(s.apps),
  platform(s.platform),
  mapping(s.mapping),
  settings(s.settings){
  proc.update(*this, share, s.proc);
  proc_mode.update(*this, share, s.proc_mode);
}
  
// Copies the space
Space* OneProcModel::copy(bool share){
  return new OneProcModel(share, *this);
}
  
// Prints the variables
void OneProcModel::print(std::ostream& out) const{
  out << "Proc: " << proc << endl;
  out << "proc mode: " << proc_mode << endl;
}

vector<tuple<int,int>> OneProcModel::getResult() const{
  vector<tuple<int,int>> result;
  for(int i=0; i<proc.size(); i++){
    if(proc[i].assigned() && proc_mode[proc[i].val()].assigned()){
      result.push_back(tuple<int,int>(proc[i].val(), proc_mode[proc[i].val()].val()));
    }else{
      //throw some exception
    }
  }
  
  return result;
}
