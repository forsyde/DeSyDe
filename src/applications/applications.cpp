#include "applications.hpp"

using namespace DeSyDe;

Applications::Applications() {
  n_sdfActors       = 0;
  n_sdfParentActors = 0;
  n_sdfChannels     = 0;
  n_iptTasks        = 0;
}

Applications::~Applications() {
  for (size_t i=0; i<sdfApps.size(); i++)
    delete sdfApps[i];
  for (size_t i=0; i<desContr.size(); i++)
    delete desContr[i];
}

Applications::Applications(vector<SDFGraph*> _sdfApps, TaskSet* _iptApps)
  : sdfApps(_sdfApps), iptApps(_iptApps) {

  n_sdfActors       = 0;
  n_sdfParentActors = 0;
  n_sdfChannels	    = 0;
  for (size_t i=0; i<sdfApps.size(); i++){
    offsets.push_back(n_sdfActors);
    n_sdfActors += sdfApps[i]->n_actors();
    n_sdfParentActors += sdfApps[i]->n_parentActors();
    n_sdfChannels += sdfApps[i]->n_channels();
  }
  
  n_iptTasks = iptApps->getNumberOfTasks();

}

Applications::Applications(vector<SDFGraph*> _sdfApps, TaskSet* _iptApps, XMLdoc& xml)
  : sdfApps(_sdfApps), iptApps(_iptApps) {

  n_sdfActors       = 0;
  n_sdfParentActors = 0;
  n_sdfChannels	    = 0;
  for (size_t i=0; i<sdfApps.size(); i++){
    offsets.push_back(n_sdfActors);
    n_sdfActors += sdfApps[i]->n_actors();
    n_sdfParentActors += sdfApps[i]->n_parentActors();
    n_sdfChannels += sdfApps[i]->n_channels();
  }
  
  n_iptTasks = iptApps->getNumberOfTasks();
  
  load_const(xml);
}

void Applications::load_const(XMLdoc& xml)
{
  const char* my_xpathString = "///designConstraints/appConstraint";
	LOG_DEBUG("running xpathString  " + tools::toString(my_xpathString) + " on desConst file ...");
	auto xml_constraints = xml.xpathNodes(my_xpathString);
    LOG_DEBUG("xml_constraints size="+tools::toString(xml_constraints.size()));
	for (const auto& cons : xml_constraints)
	{
		string app_name = xml.getProp(cons, "app_name");
    string period_cons_s;
    int period_cons;
    if(xml.hasProp(cons, "period")){
      period_cons_s = xml.getProp(cons, "period");
      period_cons = atoi(period_cons_s.c_str());
    }else{
      period_cons = 0;
    }
    string latency_cons_s;
    int latency_cons;
    if(xml.hasProp(cons, "latency")){
      latency_cons_s = xml.getProp(cons, "latency");
      latency_cons = atoi(latency_cons_s.c_str());
    }else{
      latency_cons = 0;
    }
        
    set_const(app_name, period_cons, latency_cons);
        
		LOG_DEBUG("Reading constraints for app: " + app_name + "...");		
		
	}	
}
void Applications::set_const(string app_name, int period_const, int latency_const)
{
    for(const auto& sdf : sdfApps)
    {
        if(sdf->getName().compare(app_name) == 0)
        {
            sdf->setPeriodConstraint(period_const);
            sdf->setLatencyConstraint(latency_const);            
        }
    }
}

Applications::Applications(vector<SDFGraph*> _sdfApps, vector<DesignConstraints*> _desContr, TaskSet* _iptApps)
  : sdfApps(_sdfApps), desContr(_desContr), iptApps(_iptApps) {

  n_sdfActors       = 0;
  n_sdfParentActors = 0;
  n_sdfChannels     = 0;
  for (size_t i=0; i<sdfApps.size(); i++){
    offsets.push_back(n_sdfActors);
    n_sdfActors += sdfApps[i]->n_actors();
    n_sdfParentActors += sdfApps[i]->n_parentActors();
    n_sdfChannels += sdfApps[i]->n_channels();
  }
  
  n_iptTasks = iptApps->getNumberOfTasks();
  }

Applications::Applications(vector<SDFGraph*> _sdfApps, vector<DesignConstraints*> _desContr)
  : sdfApps(_sdfApps), desContr(_desContr) { 

  n_sdfActors       = 0;
  n_sdfParentActors = 0;
  n_sdfChannels     = 0;
  for (size_t i=0; i<sdfApps.size(); i++){
    offsets.push_back(n_sdfActors);
    n_sdfActors += sdfApps[i]->n_actors();
    n_sdfParentActors += sdfApps[i]->n_parentActors();
    n_sdfChannels += sdfApps[i]->n_channels();
  }
  
  n_iptTasks = 0; 
}

Applications::Applications(TaskSet* _iptApps)
  : iptApps(_iptApps){

  n_sdfActors       = 0;
  n_sdfParentActors = 0;
  n_sdfChannels     = 0;
  n_iptTasks = iptApps->getNumberOfTasks();
}

//Does id belong to an IPT task?
bool Applications::isIPT(size_t id){
  return (id >= n_sdfActors && id < n_sdfActors+n_iptTasks);
}

//Does id belong to an SDF actor?
bool Applications::isSDF(size_t id){
  return (id >= 0 && id < n_sdfActors);
}

//Is id1 dependent on id0?
bool Applications::dependsOn(size_t id0, size_t id1){
  if(isIPT(id0) || isIPT(id1)){
    return false;
  }else{ //TODO add later: if id0 and id1 are sdf, but in different applications
    int app0 = getSDFGraph(id0);
    int app1 = getSDFGraph(id1);
    if(app0 != app1){
      return false;
    }else{
      return sdfApps[app0]->precedes(id0-offsets[app0], id1-offsets[app0]);
    }
  }
}


// Gives the name of the program entity id
std::string Applications::getName(size_t id){
  std::string name;
  if(isIPT(id)){
    //std::string name("p(" + to_string(getPhase(id)) + ", " +
    //to_string(getPeriod(id)) + ", " + to_string(getMaxWCET(id)) + ",
    //" + to_string(getDeadline(id)) + ")");
    /* [Nima] Modified this to retun task name*/
    return getTaskName(id); 
  }else if(isSDF(id)){
    int app = getSDFGraph(id);
    return sdfApps[app]->getActorName(id-offsets[app]);
  }
  return name;
}

std::string Applications::getType(size_t id) {
  if(isIPT(id)) {
    //Subtracting number of actors because task ids start from zero
    return iptApps->getTaskType(id - n_sdfActors); 
  }
  else if(isSDF(id)) {
    return getName(id); //TODO it has to be changed to get the type
  }
  return "";
}

// Gives the name of the actor
std::string Applications::getParentActorName(size_t id){
  if(isIPT(id)) { 
    //std::string name("p(" + to_string(getPhase(id)) + ", " +
    //to_string(getPeriod(id)) + ", " + to_string(getMaxWCET(id)) +
    //to_string(getDeadline(id)) + ", " + ")");
    return getTaskName(id);
  }else if(isSDF(id)){
    size_t app = getSDFGraph(id);
    return sdfApps[app]->getParentName(id-offsets[app]);
  }
  std::string name;
  return name;
}

// Gives the name of the graph
std::string Applications::getGraphName(size_t g_id){
  std::string name;
  if(g_id<sdfApps.size())
    name = sdfApps[g_id]->getName();
  
  return name;
}

//get number of SDF applications
size_t Applications::n_SDFApps(){
  return sdfApps.size();
}

//get number of SDF parent actors
size_t Applications::n_SDFParentActors(){
  return n_sdfParentActors;
}

//get number of SDF actors
size_t Applications::n_SDFActors(){
  return n_sdfActors;
}

//get number of SDF channels
size_t Applications::n_SDFchannels(){
  return n_sdfChannels;
}

//get number of IPT tasks
size_t Applications::n_IPTTasks(){
  return n_iptTasks;
}

//get total number of program entities (actors + tasks)
size_t Applications::n_programEntities(){
  return n_sdfActors + n_iptTasks;
}

//get total number of program channels
size_t Applications::n_programChannels(){
  return n_sdfChannels;
}

size_t Applications::getMaxChannelId(size_t app) const{
  if(app==0 && sdfApps.size()>0) return sdfApps[0]->n_channels()-1;
  
  int id = 0;
  for(size_t i=0; i<=app; i++){
    id += sdfApps[i]->n_channels();
  }
  return id-1;
  
}

size_t Applications::getAppIdForChannel(size_t ch) const{
  int id = 0;
  for(size_t i=0; i<sdfApps.size(); i++){
    id += sdfApps[i]->n_channels();
    if(ch < id) return i;
  }
  return -1;
}

//Maximum code size of all program entities (actors + tasks)
size_t Applications::getMaxCodeSize(){
  size_t maxCodeSize = 0;
  if(n_iptTasks>0 && n_sdfActors>0)
    maxCodeSize = iptApps->getMaxCodeSize();
  for (size_t i=0; i<sdfApps.size(); i++){
    if(maxCodeSize<sdfApps[i]->getMaxCodeSize()) maxCodeSize=sdfApps[i]->getMaxCodeSize();
  }
  if(n_iptTasks>0 && n_sdfActors==0)
    maxCodeSize = iptApps->getMaxCodeSize();
  if(n_iptTasks==0 && n_sdfActors>0)
    for (size_t i=0; i<sdfApps.size(); i++){
      if(maxCodeSize<sdfApps[i]->getMaxCodeSize()) maxCodeSize=sdfApps[i]->getMaxCodeSize();
    }
  return maxCodeSize;
}

//get code size of program entity id
size_t Applications::getCodeSize(size_t id){
  if(isIPT(id)){
    return iptApps->getCodeSize(id-n_sdfActors); //TODO split into applications
  }else if(isSDF(id)){
    int app = getSDFGraph(id);
    return sdfApps[app]->getCodeSize(id-offsets[app]);
  }
  return -1;
}

//get code size of actor of program entity id
/*size_t Applications::getCodeSizeActor(size_t id){
  if(id>=n_sdfParentActors && id<n_sdfParentActors+n_iptTasks){
  return iptApps->getCodeSize(id-n_sdfParentActors); //TODO split into applications
  }else if(id>=0 && id<n_sdfParentActors){
  return sdfApps->codeSizeActor(id);
  }
  return -1;
  }*/

//get code size of program entity id
size_t Applications::getDataSize(size_t id){
  if(isIPT(id)){
    return iptApps->getMemCons(id-n_sdfActors); //TODO split into applications
  }else if(isSDF(id)){
    int app = getSDFGraph(id);
    return sdfApps[app]->getDataSize(id-offsets[app]);
  }
  return -1;
}

//get memory consumption of program entity id
//TODO: for ipt already proc-dependent, for sdf not!
/*int Applications::getMemCons(size_t id){
  if(isIPT(id)){
  return iptApps->getMemCons(id-n_sdfActors)[0]; //TODO split into applications
  }else if(isSDF(id)){
  return sdfApps->memCons(id);
  }else{
  return -1;
  }
  }*/

//get the period
int Applications::getPeriod(size_t id){
  if(isIPT(id)){
    return iptApps->getPeriod(id-n_sdfActors); 
  }/*else if(isSDF(id)){
     int app = getSDFGraph(id);
     return sdfApps[app]->period();
     }*/
  return -1;
}

//get the phase
int Applications::getPhase(size_t id){
  if(isIPT(id)){
    return iptApps->getPhase(id-n_sdfActors); 
  }else if(isSDF(id)){
    return 0;
  }
  return -1;
}

//get the deadline
int Applications::getDeadline(size_t id){
  if(isIPT(id)){
    return iptApps->getDeadline(id-n_sdfActors); 
  }else if(isSDF(id)){
    return 0;
  }
  return -1;
}

//task preemptable?
bool Applications::isPreemtable(size_t id){
  if(isIPT(id)){
    return iptApps->isPreemtable(id-n_sdfActors); 
  }else if(isSDF(id)){
    return false;
  }
  return false;
}

//get all possible hyperperiods
vector<int> Applications::getHyperperiods(){
  if(n_iptTasks>0){
    return iptApps->getHyperperiods();
  }else{
    vector<int> h_periods;
    return h_periods;
  }
}

//get maximum hyperperiod
int Applications::getMaxHyperperiod(){
  if(n_iptTasks>0)
    return iptApps->getMaxHyperperiod();
  return 0;
}

//get maximum number of instances of task id
int Applications::getMaxNumberOfIPTInstances(size_t id){
  if(n_iptTasks>0 && isIPT(id)){
    return getMaxHyperperiod()/getPeriod(id);
  }
  return 0;
} 

//get maximum number of ipt instances
int Applications::getMaxNumberOfIPTInstances(){
  if(n_iptTasks>0)
    return iptApps->getMaxNumberOfInstances();
  return 0;
}

int Applications::getPeriodBound(size_t g_id){
  if(g_id<sdfApps.size()){
    return desContr[g_id]->getPeriodBound();
  }
  return -1;
}

SolutionMode Applications::getConstrType(size_t g_id){
  if(g_id>=sdfApps.size())
    THROW_EXCEPTION(RuntimeException, "g_id >= sdfApps.size()" );

  return desContr[g_id]->getSolutionMode();

}

//get index of the SDFG that actor id belongs to
size_t Applications::getSDFGraph(size_t id){
  if(isSDF(id)){
    for (size_t i=0; i<offsets.size()-1; i++){
      if(id<offsets[i+1]) return i;
    }
    if(id<offsets.back()+sdfApps.back()->n_actors()) return offsets.size()-1;
  }
  return -1;
}

//get the actor from which id originates (if SDF actor)
int Applications::getParentActor(size_t id){
  if(isIPT(id)){
    return n_sdfParentActors+(id-n_sdfActors);
  }else if(isSDF(id)){
    int app = getSDFGraph(id);
    return sdfApps[app]->getParentId(id-offsets[app]);
  }else{
    return -1;
  }
}

//get the instance of parentActor id with the highest index (the "last" one)
int Applications::getLastFiring(size_t id){
  if(id>=n_sdfParentActors){
    return id;
  }else{ // id < n_sdfParentActors
    size_t app=0;
    size_t p_actors=0;
    for (size_t i=0; i<sdfApps.size(); i++){
      if(id<p_actors+sdfApps[i]->n_parentActors()){
        app = i;
      }else{
        p_actors += sdfApps[i]->n_parentActors();  
      }
    }
    return sdfApps[app]->lastActorInstanceId(id-p_actors);
  }
}

//get a list of predecessors of program entity id
vector<int> Applications::getPredecessors(size_t id){
  vector<int> preds;
  if(isSDF(id)){
    int app = getSDFGraph(id);
    vector<SDFActor*> _preds = sdfApps[app]->getPredecessors(id-offsets[app]);
    for (size_t i=0; i<_preds.size(); i++){
      preds.push_back(_preds[i]->id+offsets[app]);
    }
  }
  return preds;
}

//get a list of successors of program entity id
vector<int> Applications::getSuccessors(size_t id){
  vector<int> succs;
  if(isSDF(id)){
    int app = getSDFGraph(id);
    vector<SDFActor*> _succs = sdfApps[app]->getSuccessors(id-offsets[app]);
    for (size_t i=0; i<_succs.size(); i++){
      succs.push_back(_succs[i]->id+offsets[app]);
    }
  }
  return succs;
}

//get the list of channels in the program 
vector<SDFChannel*> Applications::getChannels(int appId){
  vector<SDFChannel*> channels;
  if(n_SDFApps()>0){
    channels = sdfApps[appId]->getChannels();
  }
  return channels;
}

//get the list of channels in all programs
vector<SDFChannel*> Applications::getChannels(){
  vector<SDFChannel*> channels;
  int newId=0;
  for (size_t i=0; i<sdfApps.size(); i++){
    vector<SDFChannel*> channelsToAdd = sdfApps[i]->getChannels();
    for (size_t j=0; j<channelsToAdd.size(); j++){
      channels.push_back(new SDFChannel());
      channels[newId]->id = newId;
      channels[newId]->name = channelsToAdd[j]->name; 
      channels[newId]->source = channelsToAdd[j]->source+offsets[i]; 
      channels[newId]->src_name = channelsToAdd[j]->src_name; 
      channels[newId]->prod = channelsToAdd[j]->prod; 
      channels[newId]->destination = channelsToAdd[j]->destination+offsets[i]; 
      channels[newId]->dst_name = channelsToAdd[j]->dst_name; 
      channels[newId]->cons = channelsToAdd[j]->cons; 
      channels[newId]->initTokens = channelsToAdd[j]->initTokens; 
      channels[newId]->tokenSize = channelsToAdd[j]->tokenSize; 
      channels[newId]->messageSize = channelsToAdd[j]->messageSize; 
      channels[newId]->oldIds = channelsToAdd[j]->oldIds; 
      newId++;
    }
  }
  return channels;
}

//get token size on channel id
size_t Applications::getTokenSize(size_t id) {
  int tokens=-1;
  vector<SDFChannel*> channels = getChannels();
  if(id<n_SDFchannels()){
    tokens = channels[id]->messageSize;
  }
  for (size_t i=0; i<channels.size(); i++){
    delete channels[i];  
  }
  return tokens;
}

//get the number of initial tokens on a channel from src to dst
int Applications::getTokensOnChannel(int src, int dst) {
  if(isSDF(src)&&isSDF(dst)){
    int srcApp = getSDFGraph(src);
    int dstApp = getSDFGraph(dst);
    if(srcApp == dstApp){
      return sdfApps[srcApp]->tokensOnChannel(src-offsets[srcApp], dst-offsets[dstApp]);
    }else{
      return 0;	
    }
  }
  return 0;
}

//does a channel from src to destination exist?
bool Applications::channelExists(int src, int dst) {
  if(isSDF(src)&&isSDF(dst)){
    int srcApp = getSDFGraph(src);
    int dstApp = getSDFGraph(dst);
    if(srcApp == dstApp){
      return sdfApps[srcApp]->channelExists(src-offsets[srcApp], dst-offsets[dstApp]);
    }else{
      return false;
    }
  }
  return false;
}

//get a topological ordering of the program entities
/*vector<int> Applications::getTopologicalOrdering(){
  vector<int> topOrd;
  if(n_SDFApps()>0){
  topOrd = sdfApps->topologicalOrderingDependency();
  }
  for (size_t i=0; i<n_iptTasks; i++){
  topOrd.push_back(i+n_sdfActors);
  }
  cout << "Ordering: ";
  for (size_t i=0; i<n_sdfActors+n_iptTasks; i++){
  cout << getName(topOrd[i]) << " ";
  }
  cout << endl;
  return topOrd;
  }*/

string Applications::getTaskName(int entityID) {
  //Subtracting number of actors because task ids start from zero
  return iptApps->getTask(entityID - n_sdfActors)->name; 
}
int Applications::getTaskPeriod(int entityID) {
  //Subtracting number of actors because task ids start from zero
  return iptApps->getTask(entityID - n_sdfActors)->period; 
}
int Applications::getTaskDeadline(int entityID) {
  //Subtracting number of actors because task ids start from zero
  return iptApps->getTask(entityID - n_sdfActors)->deadline; 
}
int Applications::getTaskPriority(int entityID) {
  //Subtracting number of actors because task ids start from zero
  return iptApps->getTask(entityID - n_sdfActors)->priority; 
}
string Applications::getTaskType(int entityID) {
  //Subtracting number of actors because task ids start from zero
  return iptApps->getTask(entityID - n_sdfActors)->type; 
}
void Applications::setRMPriorities() {
  iptApps->SetRMPriorities();
}

int Applications::getPeriodConstraint(size_t id) {
  if(isSDF(id))
    return sdfApps[id]->getPeriodConstraint();
  else {
    THROW_EXCEPTION(InvalidArgumentException, string("app ") + tools::toString(id) + " is not sdf");
    return 0;
  }
}

int Applications::getLatencyConstraint(size_t id) {
  if(isSDF(id))
    return sdfApps[id]->getLatencyConstraint();
  else {
    THROW_EXCEPTION(InvalidArgumentException,string("app ") + tools::toString(id) + " is not sdf");
    return 0;
  }
}

void Applications::swapPrTasks(int i, int j) {
  iptApps->SwapTasks(i - n_sdfActors, j - n_sdfActors);
}

std::ostream& operator<< (std::ostream &out, const Applications &apps) {
  out                  << apps.n_sdfParentActors << " sdf parents, " <<
    apps.n_sdfActors   << " actors "             <<
    apps.n_sdfChannels << " channels "           <<
    apps.n_iptTasks    << " pr tasks "           << 
    endl;
  return out;
}
