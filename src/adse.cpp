/**
 * Copyright (c) 2013-2016, Katrhin Rosvall  <krosvall@kth.se>
 *  						Nima Khalilzad   <nkhal@kth.se>
 * 							George Ungureanu <ugeorge@kth.se>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
  
 /** ! \file adse.cpp
 \brief The file with the main function.

 Does all the housekeeping for the DSE tool.
 */

#include <vector>

#include "applications/sdfgraph.hpp"
#include "platform/platform.hpp"
#include "system/mapping.hpp"
//#include "cp_model/model.hpp"
#include "cp_model/sdf_pr_online_model.hpp"
#include "presolving/oneProcMappings.hpp"
#include "execution/execution.cpp"
#include "presolving/presolver.cpp"
#include "settings/input_reader.hpp"
#include "cp_model/schedulability.hpp"
#include "validation/validation.hpp"

#include "xml/xmldoc.hpp"
#include "settings/config.hpp"
#include "exceptions/exception.h"

using namespace Gecode;
using namespace Int;


int main(int argc, const char* argv[]) {

  cout << "DeSyDe - Analytical Design Space Exploration Tool\n";
  int exit_status = 0;

  Config cfg;

  try {
    if (cfg.parse(argc, argv))
      return 0;
  } catch (DeSyDe::Exception& ex) {
    cout << ex.toString() << endl;
    return 1;
  }

  try {
	  
	  TaskSet* taskset;
	  Platform* platform;
	  string WCET_path;
	  string desConst_path;
	  string mappingRules_path;
	  for (const auto& path : cfg.settings().inputs_paths) {
       /// Reading taskset
       size_t found_taskset=path.find("taskset");
       if(found_taskset != string::npos){
			XMLdoc xml(path);
			LOG_INFO("Parsing taskset XML files...");
			xml.read(false);
			taskset =  new TaskSet(xml);
			if (taskset->getNumberOfTasks() > 0) {
			  taskset->SetRMPriorities();
			  LOG_INFO(tools::toString(*taskset));
			} else {
			  LOG_INFO("did not import any periodic tasks!");
			}
	   }
	   /// Reading platform
       size_t found_platform=path.find("platform");
       if(found_platform != string::npos){
			XMLdoc xml(path);
			LOG_INFO("Parsing platform XML files...");
			xml.read(false);
			platform =  new Platform(xml);
			LOG_INFO(tools::toString(*platform));
	   }
	   /// Storing WCET xml path
       size_t found_wcet=path.find("WCETs");
       if(found_wcet != string::npos){
		   WCET_path = path;
			LOG_INFO("Storing WCET XML file...");
	   }
	   /// Storing design constraints xml path
       size_t found_desConst=path.find("desConst");
       if(found_desConst != string::npos){
		   desConst_path = path;
			LOG_INFO("Storing desConst XML file...");
	   }
	   /// Storing mapping rules xml path
       size_t found_mappingRules=path.find("mappingRules");
       if(found_mappingRules != string::npos){
		   mappingRules_path = path;
			 LOG_INFO("Storing mappingRules XML file...");
	   }
     }
	
    
    vector<SDFGraph*> sdfs;
    for (const auto& path : cfg.settings().inputs_paths) {
		 
       if(path.find("/sdfs/") != string::npos){		
           XMLdoc xml(path);
           LOG_INFO("Parsing SDF3 graphs...");
           xml.readXSD("sdf3", "noNamespaceSchemaLocation");
           sdfs.push_back(new SDFGraph(xml));
       }
     }
    /*for(auto &i : sdfXMLs)
     sdfs.push_back(new SDFGraph(i));*/

    
	XMLdoc xml_const(desConst_path);
	xml_const.read(false);
    LOG_INFO("Creating an application object ... ");
    Applications* appset = new Applications(sdfs, taskset, xml_const);
    LOG_INFO(tools::toString(*appset));

	LOG_INFO("Creating a mapping object ... " );
    Mapping* map;
    XMLdoc xml_wcet(WCET_path);
    xml_wcet.read(false);
    if(mappingRules_path != ""){
      XMLdoc xml_mapRules(mappingRules_path);
      xml_mapRules.read(false);
      map = new Mapping(appset, platform, xml_wcet, xml_mapRules);
    }else{
      map = new Mapping(appset, platform, xml_wcet);
    }
    
    LOG_INFO("Sorting pr tasks based on utilization ... ");
    //map->PrintWCETs();
    map->SortTasksUtilization();
    LOG_INFO(tools::toString(*taskset));

    //PRESOLVING +++
    
    SDFPROnlineModel* model;
    
    if(cfg.doPresolve()){

      LOG_INFO("Creating PRESOLVING execution object ... ");
      Presolver presolver(cfg);

      LOG_INFO("Running PRESOLVING model object ... ");
      model = (SDFPROnlineModel*)presolver.presolve(map);

      vector<vector<tuple<int,int>>> mappings = presolver.getMappingResults();
      LOG_INFO("Presolver found " + tools::toString(mappings.size()) + " isolated mappings.");
      
    }else{
      LOG_INFO("No PRESOLVER specified.");
      model = new SDFPROnlineModel(map, &cfg);
    }


//    cout << messageStart + "Creating a constraint model object ... " << endl;
//    SDFPROnlineModel* model = new SDFPROnlineModel(map, dseSettings);
    //LOG_INFO("Creating a constraint model object ... ");
    //SDFPROnlineModel* model = new SDFPROnlineModel(map, &cfg);
//
    LOG_INFO("Creating an execution object ... ");
    Execution<SDFPROnlineModel> execObj(model, cfg);
    
    LOG_INFO("Running the model object ... ");
    execObj.Execute();

//    Validation* val = new Validation(map, cfg);
//    val->Validate();

    return exit_status;
  } catch (DeSyDe::Exception& ex) {
    cout << ex.toString() << endl;
    return 1;
  }

}

