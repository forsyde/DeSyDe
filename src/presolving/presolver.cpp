#ifndef __PRESOLVER__
#define __PRESOLVER__

/* Copyright (c) 2013-2016, Nima Khalilzad   <nkhal@kth.se>
 *              Katrhin Rosvall  <krosvall@kth.se>
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

/**
 * This class is a simple contraint model Presolver engine
 * It receives a CP model and a setting object
 */
#include <math.h>
#include <iostream>
#include <vector>
#include <boost/math/common_factor.hpp>
#include <cstring>
#include <gecode/gist.hh>
#include "../settings/dse_settings.hpp"
#include "../system/mapping.hpp"
#include "../cp_model/sdf_pr_online_model.hpp"
#include "oneProcMappings.hpp"
#include <chrono>
#include <fstream>

using namespace std;
using namespace Gecode;

class Presolver {
public:
  Presolver(Config& _cfg) : settings(_cfg) {
    geSearchOptions.threads = 0.0;
    results = make_shared<Config::PresolverResults>();
  }
  ;
  ~Presolver() {
  }
  /**
   * This function executes the presolving CP model.
   * The CP model has to implement the following functions:
   * (i)  print()
   */
  Space* presolve(Mapping* map) {

    OneProcModel* pre_model = new OneProcModel(map, settings);

    switch (settings.settings().pre_search) {
    case (Config::GIST_ALL): {
      Gist::Print<OneProcModel> p("Print solution");
      Gist::Options options;
      options.inspect.click(&p);
      Gist::dfs(pre_model, options);
      break;
    }
    case (Config::GIST_OPT): {
      Gist::Print<OneProcModel> p("Print solution");
      Gist::Options options;
      options.inspect.click(&p);
      Gist::bab(pre_model, options);
      break;
    }
    case (Config::FIRST):
    case (Config::ALL): {
      cout << "DFS engine ...\n";
      DFS<OneProcModel> e(pre_model, geSearchOptions);
      loopSolutions<DFS<OneProcModel>>(&e, map);
      break;
    }
    case (Config::OPTIMIZE): {
      cout << "BAB engine, optimizing ... \n";
      BAB<OneProcModel> e(pre_model, geSearchOptions);
      loopSolutions<BAB<OneProcModel>>(&e, map);
      break;
    }
//    case (Config::OPTIMIZE_IT): {
//      cout << "BAB engine, optimizing iteratively ... \n";
//      Search::Cutoff* cut = Search::Cutoff::luby(settings->getLubyScale());
//      geSearchOptions.cutoff = cut;
//      RBS<BAB, Space> e(model, geSearchOptions);
//      loopSolutions<RBS<BAB, Space>>(&e);
//      break;
//    }
    default:
      cout << "unknown search type !!!" << endl;
      throw 42;
      break;
    }
//    cout << "Output file name: " << settings.settings().output_path << " end of exploration." << endl;
    return full_model;
  }
  ;
  vector<vector<tuple<int,int>>> getMappingResults() const{
    return results->oneProcMappings;
  }
  ;

private:
  vector<Space*> pre_models; /**< Vector of presolver constraint models. */
  Space* full_model; /**< Pointer to the problem constraint model. */
  Config& settings; /**< pointer to the Config object. */
  int nodes; /**< Number of nodes. */
  Search::Options geSearchOptions; /**< Gecode search option object. */
  ofstream out, outFull; /**< Output file streams: .txt and .csv. */
  typedef std::chrono::high_resolution_clock runTimer; /**< Timer type. */
  runTimer::time_point t_start, t_endAll; /**< Timer objects for start and end of experiment. */
  shared_ptr<Config::PresolverResults> results;

  /**
   * Prints the solutions in the ofstream (out)
   */
  template<class SearchEngine> void printSolution(SearchEngine *e, OneProcModel* s) {

    out << "Solution " << nodes << ":" << endl;
    out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    s->print(out);
    out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";

  }
  ;

  /**
   * Loops through the solutions and prints them using the input search engine
   */
  template<class SearchEngine> void loopSolutions(SearchEngine *e, Mapping* map) {
    DSESettings* dseSettings = new DSESettings(settings);
    nodes = 0;
    int fullNodes = 0;
    out.open(settings.settings().output_path + "/out/" + "presolver_results.txt");
    outFull.open(dseSettings->getOutputsPath(".txt"));
    outFull << "~~~~~ *** BEGIN OF PRESOLVER SOLUTIONS *** ~~~~~" << endl;
//    cout << "start searching for " << settings.settings().pre_search << " solutions \n";
    t_start = runTimer::now();
    while(Space * s = e->next()){
      nodes++;
      if(nodes == 1){
        if(settings.settings().pre_search == Config::FIRST){
          t_endAll = runTimer::now();
          printSolution(e, (OneProcModel*)s);
          cout << "returning" << endl;
          return;
        }
      }
      t_endAll = runTimer::now();

      if(nodes % 10 == 0){
        cout << ".";
        if(nodes % 50 == 0)
          cout.flush();
        if(nodes % 1000 == 0)
          cout << endl;
      }

      printSolution(e, (OneProcModel*)s);
      results->it_mapping = nodes-1;
      results->oneProcMappings.push_back(((OneProcModel*)s)->getResult());
      delete s;

      settings.setPresolverResults(results);
      SDFPROnlineModel* full_model = new SDFPROnlineModel(map, &settings);
      DFS<SDFPROnlineModel> ef(full_model, geSearchOptions);
      if(SDFPROnlineModel * sf = ef.next()){
        fullNodes++;
        outFull << "Pre-solution " << nodes << "----------" << endl;
        sf->print(outFull);
        outFull << "------------------------------" << endl << endl;
        Mapping* mapRes = sf->extractResult();
        settings.getPresolverResults()->periods.push_back(mapRes->getPeriods());
        settings.getPresolverResults()->sys_energys.push_back(mapRes->getSysEnergy());
        delete sf;
      }else{
        outFull << "------------------------------" << endl << endl;
        outFull << "Presolver mapping " << nodes << " does not give a solution." << endl;
        outFull << "------------------------------" << endl << endl;
      }
      delete full_model;
    }
    outFull << "~~~~~ *** END OF PRESOLVER SOLUTIONS *** ~~~~~" << endl;
    cout << endl;
    auto durAll = runTimer::now() - t_start;
    auto durAll_s = std::chrono::duration_cast<std::chrono::seconds>(durAll).count();
    auto durAll_ms = std::chrono::duration_cast<std::chrono::milliseconds>(durAll).count();
    out << "===== search ended after: " << durAll_s << " s (" << durAll_ms << " ms)";
    if(e->stopped()){
      out << " due to time-out!";
    }
    out << " =====\n" << nodes << " solutions found\n" << "search nodes: " << e->statistics().node << ", fail: " << e->statistics().fail << ", propagate: "
        << e->statistics().propagate << ", depth: " << e->statistics().depth << ", nogoods: " << e->statistics().nogood << " ***\n";


    outFull << "===== search ended after: " << durAll_s << " s (" << durAll_ms << " ms)";
    if(e->stopped()){
      outFull << " due to time-out!";
    }
    outFull << " =====\n" << fullNodes << " solutions found\n" << "search nodes: " << e->statistics().node << ", fail: " << e->statistics().fail << ", propagate: "
        << e->statistics().propagate << ", depth: " << e->statistics().depth << ", nogoods: " << e->statistics().nogood << " ***\n";

    out.close();
    outFull.close();
    // +++ Now: Create the full model
    results->it_mapping = results->oneProcMappings.size();
    settings.setPresolverResults(results);

    full_model = new SDFPROnlineModel(map, &settings);
    delete dseSettings;
  }

};

#endif

