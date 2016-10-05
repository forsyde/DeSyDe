#include "taskPartitioningModel.hpp"

taskPartitioningModel::taskPartitioningModel(Mapping* p_mapping):
  apps(p_mapping->getApplications()),
  platform(p_mapping->getPlatform()),
  mapping(p_mapping),
  proc(*this, apps->n_programEntities(), 0, platform->nodes()-1),
  proc_mode(*this, platform->nodes(), 0, 2),
  procsUsed(*this, 1, platform->nodes()),
  utilization(*this, platform->nodes(), 0, Int::Limits::max),//precision: 1/10
  sys_utilization(*this, 0, p_mapping->max_utilization),
  procsUsed_utilization(*this, 0, p_mapping->max_utilization),
  proc_power(*this, platform->nodes(), 0, Int::Limits::max),
  sys_power(*this, 0, Int::Limits::max),
  proc_area(*this, platform->nodes(), 0, Int::Limits::max),
  sys_area(*this, 0, Int::Limits::max),
  proc_cost(*this, platform->nodes(), 0, Int::Limits::max),
  sys_cost(*this, 0, Int::Limits::max){
  std::ostream debug_streasm(nullptr); /**< debuging stream, it is printed only in debug mode. */
	std::stringbuf debug_strbuf;
	debug_streasm.rdbuf(&debug_strbuf); // uses str
	debug_streasm << "\n==========\ndebug log:\n..........\n";
  cout << "Creating Model..." << endl;
  cout << "  Platform with " << platform->nodes() << " nodes is ";
  if(!mapping->homogeneousPlatform()) 
    cout << "not ";
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
  IntVarArgs nActorsOnProc(*this, platform->nodes(), 0, apps->n_programEntities());
  //Utilization of actors, depending on mapping
  IntVarArray wcet(*this, apps->n_programEntities(), 0, Int::Limits::max);
  IntVarArgs taskUtil(*this, apps->n_programEntities(), 0, Int::Limits::max);
  IntVarArgs nEntitiesOnProc(*this, platform->nodes(), 0, apps->n_programEntities()); //number of SDF actors and IPTs combined on proc[i]
  IntVarArgs nSDFAsOnProc(*this, platform->nodes(), 0, apps->n_SDFActors()); //number of SDF actors on proc[i]
  IntVarArgs nTasksOnProc(*this, platform->nodes(), 0, apps->n_IPTTasks());  //number of IPTs on proc[i]
  
  count(*this, proc.slice(0, 1, apps->n_SDFActors()), nSDFAsOnProc);
  count(*this, proc.slice(apps->n_SDFActors()), nTasksOnProc);
  count(*this, proc, nEntitiesOnProc);
  
  #include "partitioning.constraints"
  branch(*this, proc, INT_VAR_NONE(), INT_VAL_MIN());
  branch(*this, proc_mode, INT_VAR_NONE(), INT_VAL_MIN());
    
}
  
taskPartitioningModel::taskPartitioningModel(bool share, taskPartitioningModel& s):
  Space(share, s),
  apps(s.apps),
  platform(s.platform),
  mapping(s.mapping){
    proc.update(*this, share, s.proc);
    proc_mode.update(*this, share, s.proc_mode);
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
  
}
  
  // Copies the space
Space* taskPartitioningModel::copy(bool share){
  return new taskPartitioningModel(share, *this);
}
  
  // Prints the variables
void taskPartitioningModel::print(std::ostream& out) const
{
  out << "Proc: " << proc << endl;
  out << "Utils: " << utilization << endl;
  out << "Proc_modes: " << proc_mode << endl;
  out << "-----------------------------------"<< endl;
}



