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
#include <chrono>
#include <fstream>

using namespace std;
using namespace Gecode;

template<class CPModelTemplate>
class Presolver {
public:
  Presolver(CPModelTemplate* _model, Config& _cfg) :
      model(_model), settings(_cfg) {
    geSearchOptions.threads = 0.0;
  }
  ;
  ~Presolver() {
  }
  /**
   * This function executes the presolving CP model.
   * The CP model has to implement the following functions:
   * (i)  print()
   * (ii) printCSV()
   */
  int presolve() {

    switch (settings.settings().pre_search) {
    case (Config::GIST_ALL): {
      Gist::Print<CPModelTemplate> p("Print solution");
      Gist::Options options;
      options.inspect.click(&p);
      Gist::dfs(model, options);
      break;
    }
    case (Config::GIST_OPT): {
      Gist::Print<CPModelTemplate> p("Print solution");
      Gist::Options options;
      options.inspect.click(&p);
      Gist::bab(model, options);
      break;
    }
    case (Config::FIRST):
    case (Config::ALL): {
      cout << "DFS engine ...\n";
      DFS<CPModelTemplate> e(model, geSearchOptions);
      loopSolutions<DFS<CPModelTemplate>>(&e);
      break;
    }
    case (Config::OPTIMIZE): {
      cout << "BAB engine, optimizing ... \n";
      BAB<CPModelTemplate> e(model, geSearchOptions);
      loopSolutions<BAB<CPModelTemplate>>(&e);
      break;
    }
//    case (Config::OPTIMIZE_IT): {
//      cout << "BAB engine, optimizing iteratively ... \n";
//      Search::Cutoff* cut = Search::Cutoff::luby(settings->getLubyScale());
//      geSearchOptions.cutoff = cut;
//      RBS<BAB, CPModelTemplate> e(model, geSearchOptions);
//      loopSolutions<RBS<BAB, CPModelTemplate>>(&e);
//      break;
//    }
    default:
      cout << "unknown search type !!!" << endl;
      throw 42;
      break;
    }
//    cout << "Output file name: " << settings.settings().output_path << " end of exploration." << endl;
    return 1;
  }
  ;
  vector<vector<tuple<int,int>>> getMappingResults() const{
    return mappings;
  }
  ;

private:
  CPModelTemplate* model; /**< Pointer to the constraint model class. */
  Config& settings; /**< pointer to the DSESetting class. */
  int nodes; /**< Number of nodes. */
  Search::Options geSearchOptions; /**< Gecode search option object. */
  ofstream out; /**< Output file streams: .txt and .csv. */
  typedef std::chrono::high_resolution_clock runTimer; /**< Timer type. */
  runTimer::time_point t_start, t_endAll; /**< Timer objects for start and end of experiment. */
  vector<vector<tuple<int,int>>> mappings;

  /**
   * Prints the solutions in the ofstreams (out and outCSV)
   */
  template<class SearchEngine> void printSolution(SearchEngine *e, CPModelTemplate* s) {

    out << "Solution " << nodes << ":" << endl;
    out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    s->printSolution(out);
    out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";

  }
  ;

  /**
   * Loops through the solutions and prints them using the input search engine
   */
  template<class SearchEngine> void loopSolutions(SearchEngine *e) {
    nodes = 0;
    out.open(settings.settings().output_path + "/out/" + "presolver_results.txt");
//    cout << "start searching for " << settings.settings().pre_search << " solutions \n";
    t_start = runTimer::now();
    while(CPModelTemplate * s = e->next()){
      nodes++;
      if(nodes == 1){
        if(settings.settings().pre_search == Config::FIRST){
          t_endAll = runTimer::now();
          printSolution(e, s);
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

      printSolution(e, s);
      mappings.push_back(s->getResult());

      delete s;

    }
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

    out.close();
  }

};

