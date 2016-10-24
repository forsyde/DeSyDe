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
#include "cp_model/model.hpp"
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
using namespace std;

int main(int argc, const char* argv[]) {

  cout << "DeSyDe - Analytical Design Space Exploration Tool\n";

  Config cfg;

  try {
    if (cfg.parse(argc, argv))
      return 0;
  } catch (DeSyDe::Exception& ex) {
    cout << ex.toString() << endl;
    return 1;
  }

  try {

    /**
     * name of the input files
     */
    const string tasksetFile = "taskset.xml";
    const string platformFile = "platform.xml";
    const string wcetsFile = "WCETs.xml";
    const string messageStart = "==========";
    int exit_status = 0;

    cout << "\n###START###\n\n";

    cout << messageStart + "Reading settings ..." << endl;
    DSESettings* dseSettings = new DSESettings(cfg);
    cout << *dseSettings;

    cout << messageStart + "Creating InputReader object ..." << endl;
    InputReader* iReader = new InputReader(dseSettings->getInputsPath());
    cout << *iReader;

    cout << messageStart + "Reading periodic tasks and setting priorities ..."
        << endl;
    vector<PeriodicTask*> tasks = iReader->ReadTaskset(tasksetFile);
    TaskSet* inputTaskset = new TaskSet(tasks);
    if (tasks.size() > 0) {
      inputTaskset->SetRMPriorities();
      cout << *inputTaskset;
    } else {
      cout << "did not import any periodic tasks!" << endl;
    }

    cout << messageStart + "Reading processors ..." << endl;
    vector<PE*> procs = iReader->ReadPlatform(platformFile);
    for (size_t i = 0; i < procs.size(); i++)
      cout << *procs[i] << endl;

    cout << messageStart + "Creating a platform object ... " << endl;
    Platform* plat = new Platform(procs, TDMA_BUS, 32, (int) procs.size(), 1);

    cout << messageStart + "Reading SDF appications ..." << endl;
    vector<SDFGraph*> sdfs;

    for (const auto& path : cfg.settings().inputs_paths) {
       XMLdoc xml(path);
       LOG_INFO("Parsing SDF3 graphs...");
       xml.readXSD("sdf3", "noNamespaceSchemaLocation");
       sdfs.push_back(new SDFGraph(xml));
     }
    /*for(auto &i : sdfXMLs)
     sdfs.push_back(new SDFGraph(i));*/

    cout << messageStart + "Reading design constraints ... " << endl;
    iReader->ReadConstraints("desConst.xml", &sdfs);
    for (size_t i = 0; i < sdfs.size(); i++)
      cout << "App " << sdfs[i]->getName() << " period const: "
          << sdfs[i]->getPeriodConstraint() << " latency const: "
          << sdfs[i]->getLatencyConstraint() << endl;

    cout << messageStart + "Creating an application object ... " << endl;
    Applications* appset = new Applications(sdfs, inputTaskset);

    cout << *appset;

    cout << messageStart + "Creating a mapping object ... " << endl;
    Mapping* map = new Mapping(appset, plat);

    cout << messageStart + "Reading the WCET mapping file ... " << endl;
    iReader->ReadWCETS(wcetsFile, map);
    cout << messageStart + "Sorting pr tasks based on utilization ... " << endl;
    map->SortTasksUtilization();
    cout << *inputTaskset;

    //PRESOLVING +++

    cout << messageStart + "Creating PRESOLVING constraint model object ... " << endl;
    OneProcModel* pre_model = new OneProcModel(map, cfg);

    cout << messageStart + "Creating PRESOLVING execution object ... " << endl;
    Presolver<OneProcModel> presolver(pre_model, cfg);

    cout << messageStart + "Running PRESOLVING model object ... " << endl;
    presolver.presolve();

    vector<vector<tuple<int,int>>> mappings = presolver.getMappingResults();
    cout << "Presolver found " << mappings.size() << " isolated mappings." << endl;

//    cout << messageStart + "Creating a constraint model object ... " << endl;
//    SDFPROnlineModel* model = new SDFPROnlineModel(map, dseSettings);
//
//    cout << messageStart + "Creating an execution object ... " << endl;
//    Execution<SDFPROnlineModel> execObj(model, dseSettings);
//
//    cout << messageStart + "Running the model object ... " << endl;
//    execObj.Execute();

    Validation* val = new Validation(map, dseSettings);
    val->Validate();

    return exit_status;
  } catch (DeSyDe::Exception& ex) {
    cout << ex.toString() << endl;
    return 1;
  }

}

