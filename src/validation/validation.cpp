#include "validation.hpp"

using namespace std;
using namespace Gecode;


Validation::Validation(Mapping* _map, DSESettings* settings)
  : map(_map) {

  cout << "validation\n";

  InputReader* iReader = new InputReader(settings->getInputsPath());
  mapping_csv          = iReader->ReadCSV(settings->getOutputsPath("_mapping.csv"));
  Applications* app    = map->getApplications();
  no_ipts              = app->n_IPTTasks();
  no_actors            = app->n_SDFActors();
	
  Platform* plat       = map->getPlatform();
  no_procs	       = plat->nodes();
	
  cout 	<< "no_ipts: "	  << no_ipts
	<< ", actors: "	  << no_actors
	<< ", no_procs: " << no_procs
	<< endl;
};

void Validation::Validate() {
  if(no_ipts <= 0) {
    cout << "No periodic tasks: validation is not performed\n";
    return;
  }
  if(mapping_csv.size() <= 0) {
    cout << "No solutions in the csv file: validation is not performed\n";
    return;
  }
  for (size_t i=0;i<mapping_csv.size();i++) {
    vector<string> row	= mapping_csv[i];		
    vector<int> mapping = getMapping(row);		
    vector<int> mode    = getMode(row);
		
    map->setMappingMode(mapping, mode);
    if(map->FPSchedulable()) {
      //cout << "Schedulable";
    }
    else{
      cout <<	i	<< ": ";	
      for (size_t j=0;j<row.size();j++)
	cout <<	row[j]	<< ", ";
      cout << endl;			 
      cout << "NOT Schedulable!!!";
      cout << endl;
      return;
    }
  }
  cout << "Validation completed \n";
}

vector<int> Validation::getMapping(vector<string> solution) {
  vector<int> my_mapping;
  for (size_t j=0;j<no_actors+no_ipts;j++)
    my_mapping.push_back(atoi(solution[j].c_str()));		
  return my_mapping;
}

vector<int> Validation::getMode(vector<string> solution) {
  vector<int> my_modes;
  for (size_t j=no_actors+no_ipts;j<no_actors+no_ipts+no_procs;j++)
    my_modes.push_back(atoi(solution[j].c_str()));		
  return my_modes;
}
