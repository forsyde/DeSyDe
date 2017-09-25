#include "mapping.hpp"

using namespace std;
Mapping::Mapping(Applications* p_program, Platform* p_target, XMLdoc& p_xml_wcet) {
  program = p_program;
  target = p_target;
  n_apps = program->n_SDFApps() + program->n_IPTTasks();
  period.assign(n_apps, 0);
  initLatency.assign(n_apps, 0);
  proc_modes.assign(p_target->nodes(), 0);
  proc_period.assign(p_target->nodes(), 0);
  proc_utilization.assign(p_target->nodes(), 0);
  proc_energy.assign(p_target->nodes(), 0);
  proc_area.assign(p_target->nodes(), 0);
  proc_cost.assign(p_target->nodes(), 0);
  memLoad.assign(p_target->nodes(), 0);
  procsUsed_utilization = 0;
  sys_utilization = 0;
  sys_energy = 0;
  sys_cost = 0;

  //initialize vector of vectors of vectors for WCETs (combination of entity, proc & mode)
  wcets.insert(wcets.end(), program->n_programEntities(), //[Nima] n_SDFActors-->n_programEntities
               vector<vector<int>>(p_target->nodes(),
                                   vector<int>(p_target->getMaxModes(), std::numeric_limits<int>::max() - 1)));
                                   
  mappingRules_do.insert(mappingRules_do.end(), program->n_programEntities(), -1);
  mappingRules_doNot.insert(mappingRules_doNot.end(), program->n_programEntities(), vector<int>());
  
  //prepare WCETs
  for (size_t i=0; i<program->n_programEntities(); i++){ //all entities
    for (size_t j=0; j<target->nodes(); j++){  //all processors
      wcets[i][j].resize(target->getModes(j));
    }
  }

  commSched.insert(commSched.end(), p_target->nodes(), vector<int>());
  //initialize buffers
  send_buff.assign(program->n_programChannels(), 0);
  rec_buff.assign(program->n_programChannels(), 0);
  comm_delay.assign(program->n_programChannels(), div_t());



  maxIterationsTransPhEntity.assign(p_program->n_programEntities(), 1);
  maxIterationsTransPhChannel.assign(p_program->n_programChannels(), 1);
  
  ///Load WCETs
  load_wcets(p_xml_wcet);
  sysConstr = SystemConstraints{0, 0, 0, 0, 0};
}

Mapping::Mapping(Applications* p_program, Platform* p_target, XMLdoc& p_xml_wcet, XMLdoc& p_xml2) {
  program = p_program;
  target = p_target;
  n_apps = program->n_SDFApps() + program->n_IPTTasks();
  period.assign(n_apps, 0);
  initLatency.assign(n_apps, 0);
  proc_modes.assign(p_target->nodes(), 0);
  proc_period.assign(p_target->nodes(), 0);
  proc_utilization.assign(p_target->nodes(), 0);
  proc_energy.assign(p_target->nodes(), 0);
  proc_area.assign(p_target->nodes(), 0);
  proc_cost.assign(p_target->nodes(), 0);
  memLoad.assign(p_target->nodes(), 0);
  procsUsed_utilization = 0;
  sys_utilization = 0;
  sys_energy = 0;
  sys_cost = 0;

  //initialize vector of vectors of vectors for WCETs (combination of entity, proc & mode)
  wcets.insert(wcets.end(), program->n_programEntities(), //[Nima] n_SDFActors-->n_programEntities
               vector<vector<int>>(p_target->nodes(),
                                   vector<int>(p_target->getMaxModes(), std::numeric_limits<int>::max() - 1)));
                                   
  mappingRules_do.insert(mappingRules_do.end(), program->n_programEntities(), -1);
  mappingRules_doNot.insert(mappingRules_doNot.end(), program->n_programEntities(), vector<int>());
  
  //prepare WCETs
  for (size_t i=0; i<program->n_programEntities(); i++){ //all entities
    for (size_t j=0; j<target->nodes(); j++){  //all processors
      wcets[i][j].resize(target->getModes(j));
    }
  }

  commSched.insert(commSched.end(), p_target->nodes(), vector<int>());
  //initialize buffers
  send_buff.assign(program->n_programChannels(), 0);
  rec_buff.assign(program->n_programChannels(), 0);
  comm_delay.assign(program->n_programChannels(), div_t());



  maxIterationsTransPhEntity.assign(p_program->n_programEntities(), 1);
  maxIterationsTransPhChannel.assign(p_program->n_programChannels(), 1);
  
  ///Load WCETs
  load_wcets(p_xml_wcet);
  if(isDesignConstraints(p_xml2)){
    load_designConstraints(p_xml2);
  }else if(isMappingRules(p_xml2)){
    load_mappingRules(p_xml2);
    sysConstr = SystemConstraints{0, 0, 0, 0, 0};
  }
}

Mapping::Mapping(Applications* p_program, Platform* p_target, XMLdoc& p_xml_wcet, XMLdoc& p_des_constr, XMLdoc& p_xml_mapRules) {
  program = p_program;
  target = p_target;
  n_apps = program->n_SDFApps() + program->n_IPTTasks();
  period.assign(n_apps, 0);
  initLatency.assign(n_apps, 0);
  proc_modes.assign(p_target->nodes(), 0);
  proc_period.assign(p_target->nodes(), 0);
  proc_utilization.assign(p_target->nodes(), 0);
  proc_energy.assign(p_target->nodes(), 0);
  proc_area.assign(p_target->nodes(), 0);
  proc_cost.assign(p_target->nodes(), 0);
  memLoad.assign(p_target->nodes(), 0);
  procsUsed_utilization = 0;
  sys_utilization = 0;
  sys_energy = 0;
  sys_cost = 0;

  //initialize vector of vectors of vectors for WCETs (combination of entity, proc & mode)
  wcets.insert(wcets.end(), program->n_programEntities(), //[Nima] n_SDFActors-->n_programEntities
               vector<vector<int>>(p_target->nodes(),
                                   vector<int>(p_target->getMaxModes(), std::numeric_limits<int>::max() - 1)));
                                   
  mappingRules_do.insert(mappingRules_do.end(), program->n_programEntities(), -1);
  mappingRules_doNot.insert(mappingRules_doNot.end(), program->n_programEntities(), vector<int>());
  
  //prepare WCETs
  for (size_t i=0; i<program->n_programEntities(); i++){ //all entities
    for (size_t j=0; j<target->nodes(); j++){  //all processors
      wcets[i][j].resize(target->getModes(j));
    }
  }

  commSched.insert(commSched.end(), p_target->nodes(), vector<int>());
  //initialize buffers
  send_buff.assign(program->n_programChannels(), 0);
  rec_buff.assign(program->n_programChannels(), 0);
  comm_delay.assign(program->n_programChannels(), div_t());



  maxIterationsTransPhEntity.assign(p_program->n_programEntities(), 1);
  maxIterationsTransPhChannel.assign(p_program->n_programChannels(), 1);
  
  ///Load WCETs
  load_wcets(p_xml_wcet);
  load_designConstraints(p_des_constr);
  load_mappingRules(p_xml_mapRules);
}

/*Mapping::Mapping(Applications* p_program, Platform* p_target, 
  vector<vector<int>>& p_mappingSched, vector<int>& p_slots,
  vector<int>& p_memLoad, vector<vector<SDFChannel*>>& p_msgOrder){
  program = p_program;
  target = p_target;
  n_apps = program->n_SDFApps() + program->n_IPTTasks();
  wcets.insert(wcets.end(), program->n_programEntities(), vector<int>(p_target->nodes(), std::numeric_limits<int>::max() - 1));
  mappingSched = p_mappingSched;
  tdma_slots = p_slots;
  memLoad = p_memLoad;
  msgOrder = p_msgOrder;
  period.assign(n_apps,0);
  throughput.assign(n_apps,-1.0);
  initLatency.assign(n_apps, 0);
  proc_utilization.assign(p_target->nodes(), 0);
  }*/

Mapping::~Mapping() {
  delete program;
  delete target;
}

void Mapping::load_wcets(XMLdoc& xml){
  //ofstream out; 
  //out.open("./WCETsTODAES_tradeoff.xml");
  //double scale[] = {1.6, 1.2, 1.0, 0.8, 0.4};
  //string scalename[] = {"mode1", "mode2", "mode3", "mode4", "mode5"}; 
  
  //out << "<WCET_table>" << endl;
  const char* my_xpathString = "///WCET_table/mapping";
	LOG_DEBUG("running xpathString  " + tools::toString(my_xpathString) + " on WCET file ...");
	auto xml_mappings = xml.xpathNodes(my_xpathString);
	for (const auto& map : xml_mappings)
	{
		string task_type = xml.getProp(map, "task_type");
    LOG_DEBUG("Reading mapping for task type: " + task_type + "...");
    
    //out << "\t<mapping task_type=\"" + task_type << "\">" << endl;
    
    /// Parsing the modes
    string query = "///WCET_table/mapping[@task_type=\'" + task_type + "\']/wcet";
    auto   proc_mappings = xml.xpathNodes(query.c_str());
    for (auto p : proc_mappings) {
      
      string proc_type = xml.getProp(p, "processor");
      string proc_mode = xml.getProp(p, "mode");
      string task_wcet = xml.getProp(p, "wcet");
      
      setWCETs(task_type, proc_type, proc_mode, atoi(task_wcet.c_str()));
        
      //if(proc_type == "small" && proc_mode == "default"){
        //for(size_t i=0; i<(sizeof(scale)/sizeof(double)); i++){
          //out << "\t\t<wcet processor=\"" << "default";
          //out << "\" mode=\"" << scalename[i];
          //out << "\" wcet=\"" << ceil(scale[i]*atoi(task_wcet.c_str()));
          //out << "\"/>" << endl;
        //}
      //}
        
      LOG_DEBUG("Proc: " + proc_type + ", proc_mode: " + proc_mode 
                 + ", WCET: " + tools::toString(atoi(task_wcet.c_str())));		
    }
    
    //out << "\t</mapping>" << endl;
    
	}	
    for (size_t i=0; i < wcets.size(); i++)
    {
      bool wcetSet = false;
        for (size_t j=0; j<wcets[i].size(); j++)
        {
            for (size_t k=0; k<wcets[i][j].size(); k++)
            {
                if(wcets[i][j][k] >= std::numeric_limits<int>::max() - 1){ 
                  LOG_INFO("Note: no WCET is specified for task "+program->getName(i)
                           + " on proc type " + target->getProcModel(j)
                           + " in mode " + target->getProcModelMode(j,k));
                }else{
                  wcetSet = true;
                }
            }
        }
        if(!wcetSet){
          THROW_EXCEPTION(InvalidArgumentException,"wcet is not specified for task "+program->getName(i)+"\n");
        }
    }
  
  //out << "</WCET_table>";
  //out.close();
}

bool Mapping::isMappingRules(XMLdoc& xml){
  const char* my_xpathString = "///mappingRules/mapping";
	auto xml_mappings = xml.xpathNodes(my_xpathString);
  if(xml_mappings.size()>0) return true; else return false;
  
}

void Mapping::load_mappingRules(XMLdoc& xml){
  const char* my_xpathString = "///mappingRules/mapping";
	LOG_DEBUG("running xpathString  " + tools::toString(my_xpathString) + " on mapping rules file ...");
	auto xml_mappings = xml.xpathNodes(my_xpathString);
	for (const auto& map : xml_mappings)
	{
		string task_type = xml.getProp(map, "task_type");
    string mapOn;
    if(xml.hasProp(map, "mapOn"))
      mapOn = xml.getProp(map, "mapOn");
    vector<string> notMapOn_s;
    vector<int> notMapOn;
    if(xml.hasProp(map, "notMapOn")){
      notMapOn_s = tools::split(xml.getProp(map, "notMapOn"), ',');
      for(auto i : notMapOn_s){
        notMapOn.push_back(atoi(i.c_str()));
      }
    }
    
    LOG_DEBUG("Reading mapping rules for task type: " + task_type 
               + " - map on: " + mapOn 
               + ", do not map on: " + tools::toString(notMapOn));
    
    int _mapOn = -1;
    if(mapOn != "") _mapOn = atoi(mapOn.c_str());
    setMappingRules(task_type, _mapOn , notMapOn);
        
	}	
    
}

bool Mapping::isDesignConstraints(XMLdoc& xml){
  const char* my_xpathString = "///designConstraints/designConstraints";
	auto xml_constraints = xml.xpathNodes(my_xpathString);
  if(xml_constraints.size()>0) return true; else return false;
  
}

void Mapping::load_designConstraints(XMLdoc& xml)
{
  const char* my_xpathString = "///designConstraints/constraint";
	LOG_DEBUG("running xpathString  " + tools::toString(my_xpathString) + " on desConst file ...");
	auto xml_constraints = xml.xpathNodes(my_xpathString);
	for (const auto& cons : xml_constraints)
	{
    string power_constr;
		string util_constr;
		string area_constr;
		string money_constr;
		string procsUsed_constr;
    
    if(xml.hasProp(cons, "power"))
      power_constr = xml.getProp(cons, "power");
    if(xml.hasProp(cons, "utilization"))
      util_constr = xml.getProp(cons, "utilization");
    if(xml.hasProp(cons, "area"))
      area_constr = xml.getProp(cons, "area");
    if(xml.hasProp(cons, "money"))
      money_constr = xml.getProp(cons, "money");
    if(xml.hasProp(cons, "procsUsed"))
      procsUsed_constr = xml.getProp(cons, "procsUsed");
      
    int power;
    int util;
    int area;
    int money;
    int procsUsed;
      
    if(power_constr.empty()) power = -1; else power = atoi(power_constr.c_str());
    if(util_constr.empty()) util = -1; else util = atoi(util_constr.c_str())*(max_utilization/100);
    if(area_constr.empty()) area = -1; else area = atoi(area_constr.c_str());
    if(money_constr.empty()) money = -1; else money = atoi(money_constr.c_str());
    if(procsUsed_constr.empty()) procsUsed = -1; else procsUsed = atoi(procsUsed_constr.c_str());
        
		LOG_DEBUG("Reading system constraints: " + tools::toString(power) + ", "
              + tools::toString(util) + ", "
              + tools::toString(area) + ", "
              + tools::toString(money) + ", "
              + tools::toString(procsUsed) );		
    
    setSystemConstraints(power, util, area, money, procsUsed);
		
	}	
}

void Mapping::setMappingRules(string taskType, int _mapOn, vector<int> _notMapOn){
  for (size_t i = 0; i < program->n_programEntities(); i++) {      
    if (taskType.compare(program->getType(i)) == 0) {
      if (_mapOn != -1 && (wcets[i].size() <= _mapOn)) {
        THROW_EXCEPTION(InvalidArgumentException,"proc id for doMap of "+ taskType +" out of bound\n");
      }
      mappingRules_do[i] = _mapOn;
      for(j: _notMapOn){
        if (wcets[i].size() <= j) {
          THROW_EXCEPTION(InvalidArgumentException,"proc id for doNotMap of "+ taskType +" out of bound\n");
        }
        mappingRules_doNot[i].push_back(j);
      }
      
    }
  }
}

void Mapping::setSystemConstraints(int _pow, int _util, int _area, int _money, int _pu){
  sysConstr = SystemConstraints{_pow, _util, _area, _money, _pu};
}

Applications* Mapping::getApplications() const {
  return program;
}

Platform* Mapping::getPlatform() const {
  return target;
}

void Mapping::setMappingScheds(vector<vector<int>>& p_mapping) {
  mappingSched = p_mapping;
}

vector<vector<int>> Mapping::getMappingSched() const {
  return mappingSched;
}

int Mapping::getNumberOfApps() const {
  return n_apps;
}

bool Mapping::homogeneousPlatform() {
  if (!target->homogeneous())
    return false;

  for (size_t i = 0; i < program->n_SDFActors(); i++) {
    vector<int> wcets_i = getWCETsModes(i); //fixed
    for (unsigned j = 1; j < wcets_i.size(); j++) {
      if (wcets_i[j - 1] != wcets_i[j])
        return false;
    }
  }
  return true;
}
bool Mapping::homogeneousNodes(int nodeI, int nodeJ) {
  if (!target->homogeneousNodes(nodeI, nodeJ)) {
    return false;
  }
  for (size_t i = 0; i < program->n_SDFActors(); i++) {
    vector<int> wcets_i = getWCETsModes(i);//fixed
    if (wcets_i[nodeI] != wcets_i[nodeJ])
      return false;
  }
  return true;
}

bool Mapping::homogeneousModeNodes(int nodeI, int nodeJ) {
    //cout << "~~~~~~~~~~~~~~~~" << endl;
    //cout << "Mapping::homogeneousModeNodes("<< nodeI << ", " << nodeJ << ")" << endl;
    if(!target->homogeneousModeNodes(nodeI, nodeJ)){
        //cout << "  target->homogeneousModeNodes("<< nodeI << ", " << nodeJ << ") = ";
        //cout << target->homogeneousModeNodes(nodeI, nodeJ) << endl;

        //cout << "~~~~~~~~~~~~~~~~" << endl;
        return false;
    }
    for(size_t i = 0; i < program->n_SDFActors(); i++){
        vector<vector<int>> wcets_i = getWCETs(i); //getWCETsModes(i);//fixed

        /*cout << "  Actor " << i << endl;
        cout << "    ";
        for(size_t k = 0; k < wcets_i[nodeI].size(); k++){
            cout << wcets_i[nodeI][k] << " ";
        }
        cout << endl;
        cout << "    ";
        for(size_t k = 0; k < wcets_i[nodeJ].size(); k++){
            cout << wcets_i[nodeJ][k] << " ";
        }
        cout << endl;*/

        if(wcets_i[nodeI].size() != wcets_i[nodeJ].size()){
            //cout << "~~~~~~~~~~~~~~~~" << endl;
            return false;
        }else{ //same number of modes
            for(size_t j = 0; j < wcets_i[nodeI].size(); j++){
                if(wcets_i[nodeI][j] != wcets_i[nodeJ][j]){
                    //cout << "~~~~~~~~~~~~~~~~" << endl;
                    return false;
                }
            }
        }
    }
    //cout << "~~~~~~~~~~~~~~~~" << endl;
    return true;
}

//void Mapping::setWCETs(string name, vector<int> _wcets) {
//	//[Nima] n_SDFActors changed to n_programEntities()
//	for (size_t i = 0; i < program->n_programEntities(); i++) {
//		if (name.compare(program->getName(i)) == 0) {
//			wcets[i] = _wcets;
//		}
//	}
//}

//fixed
void Mapping::setWCETs(string taskType, string procModel, string procMode, int _wcet) {
  for (size_t i = 0; i < program->n_programEntities(); i++) {      
    if (taskType.compare(program->getType(i)) == 0) {
      for (size_t j = 0; j < target->nodes(); j++) {
        if (wcets[i].size() <= j) {
          THROW_EXCEPTION(InvalidArgumentException,"wcet out of bound\n");
        }
        if (procModel.compare(target->getProcModel(j)) == 0) {
          for (size_t k=0; k<target->getModes(j); k++){           
            if (wcets[i][j].size() <= k) {
              THROW_EXCEPTION(InvalidArgumentException,"wcet out of bound\n");
            } 
            if (procMode.compare(target->getProcModelMode(j,k)) == 0) {
              wcets[i][j][k] = _wcet;
            }
          }
        }
      }
    }
  }
}

void Mapping::setWCETs(vector<char*> elements, vector<char*> values) {
  string taskType, procModel, procMode;
  int _wcet;
  for (unsigned int i = 0; i < elements.size(); i++) {
    try {
      if (strcmp(elements[i], "wcet") == 0)
        _wcet = atoi(values[i]);

      if (strcmp(elements[i], "taskType") == 0)
        taskType = string(values[i]);

      if (strcmp(elements[i], "procModel") == 0)
        procModel = string(values[i]);
        
      if (strcmp(elements[i], "mode") == 0)
        procMode = string(values[i]);
    } catch (std::exception const & e) {
      cout << "parsing WCETs xml file error : " << e.what() << endl;
    }
  }
  setWCETs(taskType, procModel, procMode, _wcet);
}

//fixed
vector<int> Mapping::getWCETsModes(unsigned actorId) const {
  //flatten the vector of vectors (wcets[actorId]) into one vector
  vector<int> _wcets;
  for(const auto &v: wcets[actorId])
  {
    _wcets.insert(_wcets.end(), v.begin(), v.end());
  }
  return _wcets;
}

//fixed
vector<int> Mapping::getWCETsSingleMode(unsigned actorId) const {
  vector<int> _wcets;
  for (size_t i = 0; i < target->nodes(); i++) {
    _wcets.push_back(wcets[actorId][i][0]);
  }
  return _wcets;
}

//fixed
vector<vector<int>> Mapping::getWCETs(unsigned actorId) const {
  return wcets[actorId];
}

//fixed
vector<int> Mapping::getWCETs(unsigned actorId, unsigned proc) const {
  return wcets[actorId][proc];
}

//checked
int Mapping::getMinWCET(unsigned actorId) const {
  int wcet = getValidWCETs(actorId, 0)[0];
  for (size_t i = 0; i < target->nodes(); i++) {
    vector<int> tmp_wcets = getValidWCETs(actorId, i);
    if (wcet > *min_element(tmp_wcets.begin(), tmp_wcets.end()))
      wcet = *min_element(tmp_wcets.begin(), tmp_wcets.end());
  }

  return wcet;
}

//checked
int Mapping::getMaxWCET(unsigned actorId) const {
  int wcet = 0;
  for (size_t i = 0; i < target->nodes(); i++) {
    vector<int> tmp_wcets = getValidWCETs(actorId, i);
    if (wcet < *max_element(tmp_wcets.begin(), tmp_wcets.end()))
      wcet = *max_element(tmp_wcets.begin(), tmp_wcets.end());
  }

  return wcet;
}

//fixed
vector<int> Mapping::getValidWCETs(unsigned actorId, unsigned proc) const {
  vector<int> _wcets;
  for (size_t i = 0; i<wcets[actorId][proc].size(); i++){
    if (wcets[actorId][proc][i] > 0)
      _wcets.push_back(wcets[actorId][proc][i]);
  }

  return _wcets;
}

bool compareDIVT(div_t i, div_t j) {
  if (i.quot != j.quot) {
    return i.quot > j.quot;
  } else {
    return i.rem > j.rem;
  }
  return true;
}

//fixed
vector<int> Mapping::sortedByWCETs(size_t app) const {
  vector<div_t> tmp;
  for (size_t i = 0; i < program->n_SDFActors(); i++) {
    if (program->getSDFGraph(i) == app) {
      div_t tmpI;
      vector<int> wcets = getWCETsModes(i);
      tmpI.quot = *max_element(wcets.begin(), wcets.end());
      for (unsigned w = 0; w < wcets.size(); w++) {
        if (wcets[w] != -1) {
          if (tmpI.quot > wcets[w])
            tmpI.quot = wcets[w];
        }
      }
      tmpI.rem = i;
      tmp.push_back(tmpI);
    }
  }
  sort(tmp.begin(), tmp.end(), compareDIVT);

  vector<int> result;
  for (unsigned i = 0; i < tmp.size(); i++) {
    result.push_back(tmp[i].rem);
  }
  return result;
}

//fixed
vector<int> Mapping::sortedByWCETs() {
  vector<div_t> tmp;
  for (size_t i = 0; i < program->n_SDFActors(); i++) {
    div_t tmpI;
    vector<int> wcets = getWCETsModes(i);
    tmpI.quot = *max_element(wcets.begin(), wcets.end());
    for (unsigned w = 0; w < wcets.size(); w++) {
      if (wcets[w] != -1) {
        if (tmpI.quot > wcets[w])
          tmpI.quot = wcets[w];
      }
    }
    tmpI.rem = i;
    tmp.push_back(tmpI);
  }
  sort(tmp.begin(), tmp.end(), compareDIVT);

  vector<int> result;
  for (size_t i = 0; i < program->n_SDFActors(); i++) {
    result.push_back(tmp[i].rem);
  }
  return result;
}

/** Gets the designer-specified rules for mapping. */
vector<int> Mapping::getMappingRules_do() const{
  return mappingRules_do;
}

/** Gets the designer-specified rules for mapping. */
vector<vector<int>> Mapping::getMappingRules_doNot() const{
  return mappingRules_doNot;
}

/** Gets the designer-specified system constraints. */
SystemConstraints Mapping::getSystemConstraints() const{
  return sysConstr;
}

void Mapping::setFirstMapping(vector<div_t>& _firstMapping) {
  firstMapping = _firstMapping;
}

vector<div_t> Mapping::getFirstMapping() const {
  return firstMapping;
}

void Mapping::resetFirstMapping() {
  firstMapping.clear();
}

// Sets the maximum cycle mean (inverse of throughput)
void Mapping::setPeriod(int app, int p_period) {
  if (app < n_apps) {
    period[app] = p_period;
  }
}

int Mapping::getPeriod(int app) const {
  if (app < n_apps) {
    return period[app];
  } else {
    return -1;
  }
}

// Sets the initial latency
void Mapping::setInitLatency(int app, int p_initLatency) {
  if (app < n_apps) {
    initLatency[app] = p_initLatency;
  }
}

// Gets the initial latency
int Mapping::getInitLatency(int app) const {
  if (app < n_apps) {
    return initLatency[app];
  } else {
    return -1;
  }
}
/*
  void Mapping::setUtilization(int node, float p_utilization){
  if(node < target->nodes()){
  utilization[node] = p_utilization;
  }
  }

  float Mapping::getUtilization(int node){
  if(node < target->nodes()){
  return utilization[node];
  }else{
  return -1.0;
  }
  }

  float Mapping::getThroughput(int app) const{
  if(app<n_apps){
  return throughput[app];
  }else{
  return -1.0;
  }
  }

  void Mapping::setTDMAslots(vector<int>& p_slots){
  tdma_slots = p_slots;
  }

  vector<int> Mapping::getTDMAslots() const{
  return tdma_slots;
  }

  void Mapping::setMemoryLoad(vector<int>& p_memLoad){
  memLoad = p_memLoad;
  }

  vector<int> Mapping::getMemoryLoad() const{
  return memLoad;
  }

  void Mapping::setMessageOrder(vector<vector<SDFChannel*>>& p_msgOrder){
  msgOrder = p_msgOrder;
  }

  vector<vector<SDFChannel*>> Mapping::getMessageOrder() const{
  return msgOrder;
  }
*/

// Gives the memory consumption of the specified actor on the specified node
//TODO: get this from a file
int Mapping::memConsCode(int actor, int node) {
  return program->getCodeSize(actor);
}

// Gives the memory consumptions of the specified actor on all nodes
const vector<int> Mapping::memConsCode(int actor) {
  vector<int> mem;
  for (size_t j = 0; j < target->nodes(); j++) {
    mem.push_back(memConsCode(actor, j));
  }
  return mem;
}
// Gives the memory consumption of the specified actor on the specified node
//TODO: get this from a file
int Mapping::memConsData(int actor, int node) {
  return program->getDataSize(actor);
}

// Gives the memory consumptions of the specified actor on all nodes
const vector<int> Mapping::memConsData(int actor) {
  vector<int> mem;
  for (size_t j = 0; j < target->nodes(); j++) {
    mem.push_back(memConsData(actor, j));
  }
  return mem;
}

// Gives the maximum communication time for the channel
// i.e. if the processor gets only one TDMA slot
int Mapping::wcCommTime(int channel) {
  /*int maxTime=program->getTokenSize(channel)/target->bandwidthPerSlot().quot;
    if((program->getTokenSize(channel)%target->bandwidthPerSlot().quot)>0) maxTime++;
    return maxTime;*/
  return target->maxCommTimes(program->getTokenSize(channel))[1];
}

// Gives a vector with communication times for the channel, depending on allocated TDMA slots
const vector<int> Mapping::wcCommTimes(int channel) {
  return target->maxCommTimes(program->getTokenSize(channel));
}

// Gives a vector with blocking times for the channel, depending on allocated TDMA slots
// (waiting time from release of a message until the first available slot)
const vector<int> Mapping::wcBlockingTimes() {
  return target->maxBlockingTimes();
}

// Gives a vector with communication times for the channel, depending on allocated TDMA slots
const vector<int> Mapping::wcTransferTimes(int channel) {
  return target->maxTransferTimes(program->getTokenSize(channel));
}

// Gives the sum of maximum communication times over all channels
int Mapping::sumWcetCommTimes() {
  int sum = 0;
  for (size_t i = 0; i < program->n_SDFchannels(); i++) {
    sum += wcCommTime(i);
  }
  return sum;
}

void Mapping::setMaxScheduleLength(int p_time) {
  maxScheduleLength = p_time;
}

// Gives the maximum length of the time-based static schedule, consisting
// of transient and periodic phase
int Mapping::getMaxScheduleLength() {
  return maxScheduleLength;
}

// Gives the maximum number of iterations for program entities (ipt or
// sdf actor) and channels in the fully static time-based schedule (transient phase)
void Mapping::maxIterations() {
  /*vector<int> its(program->n_programEntities(), 0);
    vector<int> actors;
    for (size_t id=program->n_programEntities()-1; id>=0; id--){
    actors.push_back(id);
    }
    while(actors.size()>0){
    int id = actors.back();
    actors.pop_back();
    if(program->isIPT(id)){
    its[id]=program->getMaxNumberOfIPTInstances(id);
    }else if(program->isSDF(id)){
    if(target->nodes()>1){
    if(program->getPredecessors(id).size() == 0){
    its[id]=(sumMaxExecTimes()+sumWcetCommTimes())/minExecTime(id);
    }else{
    vector<int> preds = program->getPredecessors(id);
    vector<int> tmp_its(preds.size(), -1);
    for (size_t i=0; i<preds.size(); i++){
    if(its[preds[i]]>0) tmp_its[i]=its[preds[i]];
    }
    if(find(tmp_its.begin(), tmp_its.end(), -1)==tmp_its.end()){
    its[id]=*min_element(tmp_its.begin(),tmp_its.end());
    }else{
    actors.insert(actors.begin(), id);
    }
    }
    }else{
    its[id]=1;
    }
    }
    }
    maxIterationsTransPhEntity=its;

    vector<int> its_ch(program->n_SDFchannels(), 0);
    vector<SDFChannel*> channels = program->getChannels();
    for (size_t i=0; i<program->n_SDFchannels(); i++){
    its_ch[i]=maxIterationsTransPhEntity[channels[i]->source];
    }
    maxIterationsTransPhChannel=its_ch;*/
}

void Mapping::setIndecesTimedSchedule() {
  int index = 0;
  firstIndexTransPhEntity.clear();
  lastIndexTransPhEntity.clear();
  for (size_t id = 0; id < program->n_programEntities(); id++) {
    firstIndexTransPhEntity.push_back(index);
    index += maxIterationsTransPhEntity[id];
    lastIndexTransPhEntity.push_back(index - 1);
    /*cout << id << ": ";
      cout << firstIndexTransPhEntity[id] << ", ";
      cout << lastIndexTransPhEntity[id] << ", ";
      cout << endl;*/
  }

  index = 0;
  firstIndexTransPhChannel.clear();
  lastIndexTransPhChannel.clear();
  for (size_t id = 0; id < program->n_SDFchannels(); id++) {
    firstIndexTransPhChannel.push_back(index);
    index += maxIterationsTransPhChannel[id];
    lastIndexTransPhChannel.push_back(index - 1);
    /*cout << id << ": ";
      cout << firstIndexTransPhChannel[id] << ", ";
      cout << lastIndexTransPhChannel[id] << ", ";
      cout << endl;*/
  }
}

// Sets the maximum number of iterations for program entity (ipt or
// sdf actor) id in the fully static time-based schedule (transient phase)
void Mapping::setMaxIterationsEntity(int id, int its) {
  maxIterationsTransPhEntity[id] = its;
}

// Gives the maximum number of iterations for program entity (ipt or
// sdf actor) id in the fully static time-based schedule (transient phase)
int Mapping::getMaxIterationsEntity(int id) {
  return maxIterationsTransPhEntity[id];
}

// Gives the sum of iterations for program entities (ipt or
// sdf actor) in the fully static time-based schedule
int Mapping::sumIterationsEntity() {
  int sum = 0;
  for (size_t id = 0; id < program->n_programEntities(); id++) {
    sum += maxIterationsTransPhEntity[id];
  }
  return sum;
}

// Sets the maximum number of iterations for channel (src,dst)
// in the fully static time-based schedule (transient phase)
void Mapping::setMaxIterationsChannel(int id, int its) {
  maxIterationsTransPhChannel[id] = its;
}

// Gives the maximum number of iterations for channel (src,dst)
// in the fully static time-based schedule (transient phase)
int Mapping::getMaxIterationsChannel(int id) {
  return maxIterationsTransPhChannel[id];
}

// Gives the sum of iterations for channels 
// in the fully static time-based schedule
int Mapping::sumIterationsChannel() {
  int sum = 0;
  for (size_t id = 0; id < program->n_SDFchannels(); id++) {
    sum += (maxIterationsTransPhChannel[id]);
  }
  return sum;
}

// returns the first index of program entity id in the time-based static schedule of the transient phase
int Mapping::firstIndexTransPhaseEntity(int id) const {
  return firstIndexTransPhEntity[id];
}
// returns the last index of program entity id in the time-based static schedule of the transient phase
int Mapping::lastIndexTransPhaseEntity(int id) const {
  return lastIndexTransPhEntity[id];
}

// returns the first index of channel id in the time-based static schedule of the transient phase
int Mapping::firstIndexTransPhaseChannel(int id) const {
  return firstIndexTransPhChannel[id];
}
// returns the last index of channel id in the time-based static schedule of the transient phase
int Mapping::lastIndexTransPhaseChannel(int id) const {
  return lastIndexTransPhChannel[id];
}

// Determines the minimum number of processors needed to satisfy the IPT tasks
// (based on utilization)
int Mapping::minProcs_IPT() {
  /*int sumWCET = 0;
    int hyperperiod = program->getMaxHyperperiod();
    int numProcs = 0;
    for (size_t i=0; i<program->n_IPTTasks(); i++){
    sumWCET += program->getMaxNumberOfIPTInstances(program->n_SDFActors()+i)*minExecTime(program->n_SDFActors()+i);
    }
    if(hyperperiod>0){
    numProcs = sumWCET/hyperperiod;
    if(sumWCET%hyperperiod!=0) numProcs++;
    }
    return numProcs;*/
  return 0;
}

// Print all parameters for MiniZinc model
void Mapping::generateDZNfile(const string &dir) const {
  ofstream out;
  string outputFile = dir;
  string filename;
  for (size_t i = 0; i < program->n_SDFApps(); i++) {
    filename += program->getGraphName(i);
    if (i < program->n_SDFApps() - 1)
      filename += "_";
  }
  outputFile +=
    (outputFile.back() == '/') ?
    (filename + ".dzn") : ("/" + filename + ".dzn");
  out.open(outputFile.c_str());

  out << "% ***Platform parameters***" << endl;
  out << "p = " << target->nodes() << ";" << endl;
  out << endl;

  out << "% ***Application parameters (SDFGs)***" << endl;
  out << "% number of SDF actors" << endl;
  out << "nActors = " << program->n_SDFActors() << ";" << endl;
  out
    << "% actorGraph: which application graph does the actor originate from?"
    << endl;
  out << "actorGraph = [";
  for (size_t i = 0; i < program->n_SDFActors(); i++) {
    out << program->getSDFGraph(i);
    if (i < program->n_SDFActors() - 1)
      out << ", ";
  }
  out << "];" << endl;
  out << "% parentActors: the parent actor of an actor in the actor's graph"
      << endl;
  out << "parentActors = [";
  for (size_t i = 0; i < program->n_SDFActors(); i++) {
    out << program->getParentActor(i);
    if (i < program->n_SDFActors() - 1)
      out << ", ";
  }
  out << "];" << endl;
  out << "% or alternatively, actorGraph and parentActors combined" << endl;
  out << "parentActors2 = [";
  for (size_t i = 0; i < program->n_SDFActors(); i++) {
    if (program->getSDFGraph(i) > 0)
      out << program->getSDFGraph(i);
    out << program->getParentActor(i);
    if (i < program->n_SDFActors() - 1)
      out << ", ";
  }
  out << "];" << endl;

  out << "% precedence matrix nActors x nActors" << endl;
  out << "% (row i, col j)=1 means i precedes j" << endl;
  out << "precedence = [| ";
  for (size_t i = 0; i < program->n_SDFActors(); i++) {
    if (i > 0)
      out << "              | ";
    for (size_t j = 0; j < program->n_SDFActors(); j++) {
      out << program->dependsOn(i, j);
      if (i < program->n_SDFActors() - 1
          || j < program->n_SDFActors() - 1)
        out << ", ";
    }
    if (i < program->n_SDFActors() - 1)
      out << endl;
    else
      out << " |];" << endl;
  }
  out << endl;
  out << "% SDF channels" << endl;
  out << "% number of SDF channels" << endl;
  out << "nChannels = " << program->n_SDFchannels() << ";" << endl;
  out
    << "% channel properties: source actor, destination actor and initial tokens"
    << endl;
  vector<SDFChannel*> channels = program->getChannels();
  out << "ch_sources = [";
  for (size_t i = 0; i < program->n_SDFchannels(); i++) {
    out << channels[i]->source;
    if (i < program->n_SDFchannels() - 1)
      out << ", ";
  }
  out << "];" << endl;
  out << "ch_destinations = [";
  for (size_t i = 0; i < program->n_SDFchannels(); i++) {
    out << channels[i]->destination;
    if (i < program->n_SDFchannels() - 1)
      out << ", ";
  }
  out << "];" << endl;
  out << "initTok = [";
  for (size_t i = 0; i < program->n_SDFchannels(); i++) {
    out << channels[i]->initTokens;
    if (i < program->n_SDFchannels() - 1)
      out << ", ";
  }
  out << "];" << endl;

  out << endl;
  out << "% Actor names" << endl;
  for (size_t i = 0; i < program->n_SDFActors(); i++) {
    out << "% " << i << ": " << program->getName(i) << endl;
  }
  out.close();

  for (unsigned int i = 0; i < channels.size(); i++) {
    delete channels[i];
  }
}

//fixed
void Mapping::PrintWCETs() const{
  for (unsigned i = 0; i < program->n_programEntities(); i++) {
    cout << "\nWCETs[" << i << "]: " << endl;
    for (unsigned j = 0; j < wcets[i].size(); j++) {
      cout << "  p" << j << ": ";
      for(unsigned k = 0; k<wcets[i][j].size(); k++){
        cout << wcets[i][j][k] << ", ";
      }
    }
  }
  cout << endl;
}
void Mapping::PrintUtilizations() {
  for (size_t i = program->n_SDFActors(); i < program->n_programEntities();
       i++) {
    for (size_t k = 0; k < target->nodes(); k++) {
      cout << "\nUtilizations[" << i << ", " << k << "]: ";
      vector<int> utils_ik = getUtilizationModeVector(i, k);
      for (unsigned j = 0; j < utils_ik.size(); j++) {
        cout << utils_ik[j] << ", ";
      }
    }
  }
  cout << endl;
}
//fixed
double Mapping::getTaskUtilization(int entityID, unsigned procID) {
  vector<int> _wcets = getWCETsSingleMode(entityID); ///should use function because it does not factor in speedupds
  if (program->getTaskPeriod(entityID) <= 0) {
    THROW_EXCEPTION(InvalidArgumentException,string("period of entity: ") + tools::toString(entityID) +
        " : " + tools::toString(program->getTaskPeriod(entityID)));
  }
  return (double) _wcets[procID] / program->getTaskPeriod(entityID);
}

int Mapping::LeveliWorkload(int entityID, int t) {
  auto procID = current_mapping[entityID];
  int W = getWCET(entityID, current_mapping[entityID], current_modes[procID]);
  for (size_t i = program->n_SDFActors(); i < program->n_programEntities();
       i++) {
    /**
     * If task is mapped on my processor and it has higher priority
     */
    if (procID == current_mapping[i]
        && program->getTaskPriority(entityID)
        < program->getTaskPriority(i)) {
      W += ceil((double) t / program->getTaskPeriod(i))
        * getWCET(i, current_mapping[i],
                  current_modes[current_mapping[i]]);
    }
  }
  return W;
}
bool Mapping::FPSchedulable() {
  const clock_t begin_time = clock();
  bool taski;
  //program->SetRMPriorities();

  for (size_t k = 0; k < target->nodes(); k++) {
    double util = 0;
    int task_on_k = 0;
    for (size_t i = program->n_SDFActors(); i < program->n_programEntities();
         i++) {
      if ((size_t)current_mapping[i] == k) {
        task_on_k++;
        util += getWCET(i, current_mapping[i],
                        current_modes[current_mapping[i]])
          / (double) program->getTaskDeadline(i);
      }
    }
    double bound = task_on_k * (pow(2, (1.0 / task_on_k)) - 1);
    //cout << "util[" << k << "] = " << util << " bound = " << bound << endl;
    if (task_on_k == 0 || util <= bound) {
      // cout << "proc " << k << " is schedulable based on utilization test\n";
    } else {
      for (size_t i = program->n_SDFActors();
           i < program->n_programEntities(); i++) {
        if ((size_t)current_mapping[i] == k) {
          taski = false;
          for (auto t = 1; t <= program->getTaskDeadline(i); t++) //This can be improved by Bini and Buttazzo's approach
            {
              if (LeveliWorkload(i, t) <= t) {
                taski = true;
                break;
              }
            }
          if (!taski) {
            schedulabilityTime = clock() - begin_time;
            cout << "processor " << k << " is not schedulable \n";
            return false;
          }
        }
      }
      schedulabilityTime = clock() - begin_time;
    }
  }

  return true;
}
int Mapping::getProcessorID(int entityID) {
  vector<int> myMapping = mappingSched[entityID];
  for (unsigned i = 0; i < myMapping.size(); i++) {
    if (myMapping[i] == 1)
      return i;
  }
  return -1; /*!< If not found >*/
}

//fixed
int Mapping::getMaxWCET_IPT() {
  int maxWCET = 0;
  for (size_t i = program->n_SDFActors() + 1; i < program->n_programEntities();
       i++){
    for (size_t j = 0; j < target->nodes(); j++){
      for (size_t k = 0; k<target->getModes(j); k++){
        if (wcets[i][j][k] > maxWCET)
          maxWCET = wcets[i][j][k];
      }
    }
  }

  return maxWCET;

}

void Mapping::PrintMapping() {
  for (size_t i = 0; i < program->n_programEntities(); i++) {
    cout << program->getName(i) << ": " << getProcessorID(i) << ", ";
  }
  cout << endl;
}

vector<int> Mapping::getUtilizationVector(int entityID) {
  vector<int> utils;
  for (size_t i = 0; i < target->nodes(); i++) {
    utils.push_back(getTaskUtilization(entityID, i) * max_utilization);
  }
  return utils;
}
//fixed
vector<int> Mapping::getUtilizationModeVector(int entityID, unsigned proc) {
  vector<int> _utils, tmp_wcets;
  tmp_wcets = getWCETs(entityID, proc);
  for (size_t i = 0; i < target->getModes(proc); i++) {
    if (wcets[entityID][proc][i] != -1)
      _utils.push_back(
                       ceil(
                            (tmp_wcets[i] * max_utilization)
                            / program->getTaskPeriod(entityID)));
    else
      _utils.push_back(-1);
  }
  //if there are less than three modes for proc, fill-up with -1
  //TODO: take this away
  for (unsigned int i = _utils.size(); i < 3; i++) {
    _utils.push_back(-1);
  }

  return _utils;
}
int Mapping::getLeastTotalUtilization() {
  int least_util = 0;
  /**
   * first calculating period task utils
   */
  for (size_t i = program->n_SDFActors(); i < program->n_programEntities();
       i++) {
    vector<int> entityUtils = getUtilizationVector(i);
    auto maxUtil = std::min_element(std::begin(entityUtils),
                                    std::end(entityUtils));
    least_util += *maxUtil;
  }
  /**
   * Now considering SDF apps
   */
  for (size_t i = 0; i < program->n_SDFApps(); i++) {
    least_util += getLeastSDFUtil(i);
  }
  return least_util;
}
int Mapping::getLeastPowerConsumption() {
  int least_proc_power = INT_MAX;
  for (size_t i = 0; i < target->nodes(); i++) {
    vector<int> powers = target->getDynPowerCons(i);
    auto minPower = std::min_element(std::begin(powers), std::end(powers));
    if (*minPower < least_proc_power)
      least_proc_power = *minPower;
  }
  cout << "least_proc_power: " << least_proc_power << endl;

  int least_power = 0;
  /**
   * First periodic tasks
   */
  for (size_t i = program->n_SDFActors(); i < program->n_programEntities();
       i++) {
    least_power += getLeastPowerForTask(i);
  }
  /**
   * Now, SDF apps
   */
  for (size_t i = 0; i < program->n_SDFApps(); i++) {
    least_power += getLeastSDFUtil(i) * least_proc_power;
    cout << "least util for sdf: " << i << " is " << getLeastSDFUtil(i)
         << endl;
  }
  cout << "least_power: " << least_power << endl;

  return least_power;
}

int Mapping::getLeastPowerForTask(int task) {
  int least_power = INT_MAX;
  for (size_t k = 0; k < target->nodes(); k++) {
    vector<int> utils = getUtilizationModeVector(task, k);
    vector<int> powers = target->getDynPowerCons(k);
    vector<int> powers_utils(powers.size(), 0);
    int size_diff = utils.size() - powers.size();
    std::transform(utils.begin(), utils.end() - size_diff, powers.begin(),
                   powers_utils.begin(), std::multiplies<int>());
    auto minPower = std::min_element(std::begin(powers_utils),
                                     std::end(powers_utils));
    if (*minPower < least_power)
      least_power = *minPower;
  }
  return least_power;
}
void Mapping::setRMPriorities() {
  program->setRMPriorities();
}

string Mapping::getAppName(int i) {
  return program->getName(i);
}

int Mapping::getLeastSDFUtil(int sdf) {
  int least_util = 0;
  int SDF_toltal_WCET = 0;
  for (size_t i = 0; i < program->n_SDFActors(); i++) {
    if (program->getPeriodConstraint(sdf) > 0)
      SDF_toltal_WCET += getMinWCET(i);
  }
  /**
   * if period is constraint, then we derive utilization using that period
   * otherwise we assume that period is equal to total WCET, i.e., sdf requires a full processor
   */
  if (program->getPeriodConstraint(sdf) > 0)
    least_util += (max_utilization * SDF_toltal_WCET)
      / program->getPeriodConstraint(sdf);
  else
    least_util += max_utilization;

  return least_util;
}

void Mapping::SortTasksUtilization() {
  for (size_t j = program->n_SDFActors(); j < program->n_programEntities(); j++)
    for (size_t i = program->n_SDFActors();
         i < program->n_programEntities() - 1; i++) {
      auto util_i = getUtilizationVector(i);
      auto util_ip1 = getUtilizationVector(i + 1);
      if (util_i[0] < util_ip1[0]) {
        program->swapPrTasks(i, i + 1);
        using std::swap;
        swap(wcets[i], wcets[i + 1]);
      }
    }
}

void Mapping::setMappingMode(vector<int>& _mapping, vector<int>& _modes) {
  current_mapping = _mapping;
  current_modes = _modes;
}
//fixed
int Mapping::getWCET(unsigned actorId, unsigned proc, unsigned mode) const {
  return wcets[actorId][proc][mode];
}
int Mapping::getMemorySize(unsigned proc) {
  return target->memorySize(proc, current_modes[proc]);
}

void Mapping::setMappingSched(vector<int>& p_sched, int p) {
  mappingSched[p] = p_sched;
}

vector<int> Mapping::getMappingSched(int p) const {
  return mappingSched[p];
}

void Mapping::setCommScheds(vector<vector<int>>& p_sched) {
  commSched = p_sched;
}

void Mapping::setCommSched(vector<int>& p_sched, int p) {
  commSched[p] = p_sched;
}

vector<vector<int> > Mapping::getCommSched() const {
  return commSched;
}

vector<int> Mapping::getCommSched(int p) const {
  return commSched[p];
}

void Mapping::setPeriods(vector<int>& p_periods) {
  period = p_periods;
}

vector<int> Mapping::getPeriods() const {
  return period;
}

void Mapping::setInitLatencys(vector<int>& p_latency) {
  initLatency = p_latency;
}

vector<int> Mapping::getInitLatencys() const {
  return initLatency;
}

void Mapping::setProcPeriod(int proc, int p_period) {
  proc_period[proc] = p_period;
}

void Mapping::setProcPeriods(vector<int>& p_periods) {
  proc_period = p_periods;
}

int Mapping::getProcPeriod(int proc) const {
  return proc_period[proc];
}

vector<int> Mapping::getProcPeriods() const {
  return proc_period;
}

void Mapping::setProcsUsed(int p_procs) {
  procsUsed = p_procs;
}

int Mapping::getProcsUsed() const {
  return procsUsed;
}

void Mapping::setProcMode(int proc, int p_mode) {
  proc_modes[proc] = p_mode;
}

void Mapping::setProcModes(vector<int>& p_modes) {
  proc_modes = p_modes;
}

int Mapping::getProcMode(int proc) const {
  return proc_modes[proc];
}

vector<int> Mapping::getProcModes() const {
  return proc_modes;
}

void Mapping::setTDMAslots(int proc, int p_slots) {
  tdma_slots[proc] = p_slots;
}

void Mapping::setTDMAslots(vector<int>& p_slots) {
  tdma_slots = p_slots;
}

int Mapping::getTDMAslots(int proc) const {
  return tdma_slots[proc];
}

vector<int> Mapping::getTDMAslots() const {
  return tdma_slots;
}

void Mapping::setSendBuff(int ch, int p_size) {
  send_buff[ch] = p_size;
}

void Mapping::setSendBuffs(vector<int>& p_sizes) {
  send_buff = p_sizes;
}

int Mapping::getSendBuff(int ch) const {
  return send_buff[ch];
}

vector<int> Mapping::getSendBuffs() const {
  return send_buff;
}

void Mapping::setRecBuff(int ch, int p_size) {
  rec_buff[ch] = p_size;
}

void Mapping::setRecBuffs(vector<int>& p_sizes) {
  rec_buff = p_sizes;
}

int Mapping::getRecBuff(int ch) const {
  return rec_buff[ch];
}

vector<int> Mapping::getRecBuffs() const {
  return rec_buff;
}

void Mapping::setFixedWCET(int id, int p_wcet) {
  fixed_wcets[id] = p_wcet;
}

void Mapping::setFixedWCETs(vector<int>& p_wcets) {
  fixed_wcets = p_wcets;
}

int Mapping::getFixedWCET(int id) const {
  return fixed_wcets[id];
}

vector<int> Mapping::getFixedWCETs() const {
  return fixed_wcets;
}

void Mapping::setCommDelay(int id, div_t p_wcct) {
  comm_delay[id] = p_wcct;
}

void Mapping::setCommDelays(vector<div_t>& p_wccts) {
  comm_delay = p_wccts;
}

div_t Mapping::getCommDelay(int id) const {
  return comm_delay[id];
}

vector<div_t> Mapping::getCommDelays() const {
  return comm_delay;
}

void Mapping::setMemLoad(unsigned proc, int p_mem) {
  memLoad[proc] = p_mem;
}

void Mapping::setMemLoads(vector<int>& p_mems) {
  memLoad = p_mems;
}

int Mapping::getMemLoad(unsigned proc) const {
  return memLoad[proc];
}

vector<int> Mapping::getMemLoads() const {
  return memLoad;
}

void Mapping::setSysUtilization(int p_util) {
  sys_utilization = p_util;
}

int Mapping::getSysUtilization() const {
  return sys_utilization;
}

void Mapping::setProcsUsedUtilization(int p_util) {
  procsUsed_utilization = p_util;
}

int Mapping::getProcsUsedUtilization() const {
  return procsUsed_utilization;
}

void Mapping::setProcUtilization(unsigned proc, int p_util) {
  proc_utilization[proc] = p_util;
}

void Mapping::setProcUtilizations(vector<int>& p_utils) {
  proc_utilization = p_utils;
}

int Mapping::getProcUtilization(unsigned proc) const {
  return proc_utilization[proc];
}

vector<int> Mapping::getProcUtilizations() const {
  return proc_utilization;
}

void Mapping::setSysEnergy(int p_nrg) {
  sys_energy = p_nrg;
}

int Mapping::getSysEnergy() const {
  return sys_energy;
}

void Mapping::setProcEnergy(unsigned proc, int p_nrg) {
  proc_energy[proc] = p_nrg;
}

void Mapping::setProcEnergys(vector<int>& p_nrgs) {
  proc_energy = p_nrgs;
}

int Mapping::getProcEnergy(unsigned proc) const {
  return proc_energy[proc];
}

vector<int> Mapping::getProcEnergys() const {
  return proc_energy;
}

void Mapping::setSysArea(int p_area) {
  sys_area = p_area;
}

int Mapping::getSysArea() const {
  return sys_area;
}

void Mapping::setProcArea(unsigned proc, int p_area) {
  proc_area[proc] = p_area;
}

void Mapping::setProcAreas(vector<int>& p_areas) {
  proc_area = p_areas;
}

int Mapping::getProcArea(unsigned proc) const {
  return proc_area[proc];
}

vector<int> Mapping::getProcAreas() const {
  return proc_area;
}

void Mapping::setSysCost(int p_cost) {
  sys_cost = p_cost;
}

int Mapping::getSysCost() const {
  return sys_cost;
}

void Mapping::setProcCost(unsigned proc, int p_cost) {
  proc_cost[proc] = p_cost;
}

void Mapping::setProcCosts(vector<int>& p_costs) {
  proc_cost = p_costs;
}

int Mapping::getProcCost(unsigned proc) const {
  return proc_cost[proc];
}

vector<int> Mapping::getProcCosts() const {
  return proc_cost;
}

//vector<div_t> determineFirstMapping(vector<SDFGraph*>& sdfApps, Applications* apps, Platform* target, Mapping* mapping){
  //vector<div_t> proc(apps->n_SDFApps());
  ////Step 1: each app gets 1 proc
  //vector<int> share(apps->n_SDFApps(), 1);
  ////Step 2: check how many procs are left to be distributed
  //int n_extraProcs = target->nodes() - apps->n_SDFApps();
  //if(n_extraProcs>0){
    //n_extraProcs = target->nodes();
    ////Step 3: try to give all apps with throughput constraints as many as needed
    //for(unsigned int i=0; i<sdfApps.size(); i++){

      //if(apps->getPeriodBound(i) > 0){
        //int sumMinWCETs = 0;
        //for(int j=0; j<apps->n_programEntities(); j++){
          //if(apps->getSDFGraph(j)==i){
            //vector<int> wcets;
            //for(int p=0; p<target->nodes(); p++){
              //vector<int> wcets_proc = mapping->getValidWCETs(j, p);
              //if(wcets_proc.size()>0)
                //wcets.push_back(*min_element(wcets_proc.begin(),wcets_proc.end()));
            //}
            //sumMinWCETs += *min_element(wcets.begin(),wcets.end()); 
          //}
        //}
        //share[i] = ceil((double)sumMinWCETs/apps->getPeriodBound(i));
        //n_extraProcs -= share[i];
      //}
    //}
  //}

  //if(n_extraProcs>0){
    //int optApps = 0;
    //int remainder;
    ////Step 4: give remaining procs to the graph with optimization
    //for(unsigned int i=0; i<sdfApps.size(); i++){
      //if(apps->getPeriodBound(i) == -1){
        //optApps++;
      //}
    //}
    //remainder = n_extraProcs%optApps;
    //for(unsigned int i=0; i<sdfApps.size(); i++){
      //if(apps->getPeriodBound(i) == -1){
        //share[i] = n_extraProcs/optApps;
        //if(remainder>0){
          //share[i]+=remainder;
          //remainder = 0;
        //}
        //share[i] = min(sdfApps[i]->n_actors(), share[i]);
        //n_extraProcs -= share[i];
        //optApps--;
      //}
    //}
  //}
  
////  for(unsigned int i=0; i<sdfApps.size(); i++){
  ////  share[i] += nearbyint((double)sdfApps[i]->n_actors()*n_extraProcs/apps->n_SDFActors());
  ////}
  
  //int minProc=0;
  //for(unsigned int i=0; i<sdfApps.size(); i++){
    //cout << "First mapping app " << i << ": ";
    //proc[i].quot = minProc;
    //proc[i].rem = minProc + share[i] -1;
    //cout << proc[i].quot << " - " << proc[i].rem << endl;
    //minProc = proc[i].rem +1;
  //}
  //return proc;
//}

