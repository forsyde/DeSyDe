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
  cout << "  Found " << apps->n_programEntities() << " program entities (",
    cout << apps->n_SDFActors()<< " actors of ";
  cout << apps->n_SDFApps() << " SDFGs and ";
  cout << apps->n_IPTTasks() << " ipt tasks)." << endl;
  cout << "   --Actors and Tasks------------------\n";
  for (size_t ii=0; ii<apps->n_programEntities(); ii++){
    cout << "   " << ii << ": " << apps->getName(ii) << endl;
  }
  

  
  cout << endl << "  Branching:" << endl;
  

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
