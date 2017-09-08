#ifndef __EXECUTION__
#define __EXECUTION__

/**
 * Copyright (c) 2013-2016, Nima Khalilzad   <nkhal@kth.se>
 * 							Katrhin Rosvall  <krosvall@kth.se>
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
 * This class is a simple contraint model execution engine
 * It receives a CP model and a setting object
 */
#include <math.h>
#include <iostream>
#include <vector>
#include <boost/math/common_factor.hpp>
#include <cstring>
#include <gecode/gist.hh>
#include "../settings/config.hpp"
#include "../system/mapping.hpp"
#include <chrono>
#include <fstream> 

using namespace std;
using namespace Gecode;

struct SolutionValues{
  std::chrono::high_resolution_clock::duration time;
  vector<int> values;
};

template<class CPModelTemplate>
class Execution {
public:
  Execution(CPModelTemplate* _model, Config& _cfg) :
      model(_model), cfg(_cfg) {
    geSearchOptions.threads = 0.0;
    if(cfg.settings().timeout_first > 0){
      Search::TimeStop* stop = new Search::TimeStop(cfg.settings().timeout_first);
      geSearchOptions.stop = stop;
    }
  }
  ;
  ~Execution() {
    delete geSearchOptions.stop;
  }
  /**
   * This funtion executes the CP model.
   * The CP model has to implement the following functions:
   * (i) 	print()
   * (ii) printCSV()
   */
  int Execute() {
    switch (cfg.settings().search) {
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
    case (Config::OPTIMIZE_IT): {
      cout << "BAB engine, optimizing iteratively ... \n";
      Search::Cutoff* cut = Search::Cutoff::luby(cfg.settings().luby_scale);
      geSearchOptions.cutoff = cut;
      RBS<BAB, CPModelTemplate> e(model, geSearchOptions);
      loopSolutions<RBS<BAB, CPModelTemplate>>(&e);
      break;
    }
    default:
      cout << "unknown search type !!!" << endl;
      throw 42;
      break;
    }
    cout << "Output file name: " << cfg.settings().output_path + ".txt" << " end of exploration." << endl;
    return 1;
  }
  ;

private:
  CPModelTemplate* model; /**< Pointer to the constraint model class. */
  Config& cfg; /**< pointer to the config class. */
  int nodes; /**< Number of nodes. */
  Search::Options geSearchOptions; /**< Gecode search option object. */
  ofstream out, outCSV, outMOSTCSV, outMappingCSV; /**< Output file streams: .txt and .csv. */
  typedef std::chrono::high_resolution_clock runTimer; /**< Timer type. */
  runTimer::time_point t_start, t_endAll; /**< Timer objects for start and end of experiment. */
  vector<SolutionValues> solutionData;
  

  void printMOSTCSV(Mapping* solution, int n, int split) {
    //N_TASKS;N_EDGES;N_PES;N_SLOTS;N_SCHEDS;MAP_PE1;MAP_PE2;FREQ_PE1;FREQ_PE2;MEM_PE1;MEM_PE2;SLOTS_PE1;SLOTS_PE2;MAP_T1;MAP_T2;MAP_T3;TASK_SCHED;COMM_SCHED;cluster;
    Applications* program = solution->getApplications();
    const Platform* target = solution->getPlatform();

    //print first row (column names)
    if((split != -1 && n % split == 1) || (split == -1 && n == 1)){
      outMOSTCSV << "N_TASKS;N_EDGES;N_PES;N_SLOTS;N_SCHEDS;";
      for(size_t j = 1; j <= target->nodes(); j++){
        outMOSTCSV << "MAP_PE" << j << ";";
      }
      for(size_t j = 1; j <= target->nodes(); j++){
        outMOSTCSV << "FREQ_PE" << j << ";";
      }
      for(size_t j = 1; j <= target->nodes(); j++){
        outMOSTCSV << "MEM_PE" << j << ";";
      }
      for(size_t j = 1; j <= target->nodes(); j++){
        outMOSTCSV << "SLOTS_PE" << j << ";";
      }
      for(size_t i = 1; i <= program->n_programEntities(); i++){
        outMOSTCSV << "MAP_T" << i << ";";
      }
      outMOSTCSV << "TASK_SCHED;COMM_SCHED;"; //COMM_ENTRIES";
      for(size_t j = 1; j <= target->nodes(); j++){
        outMOSTCSV << "MEMCONS_PE" << j << ";";
      }
      for(size_t j = 1; j <= target->nodes(); j++){
        outMOSTCSV << "U_PE" << j << ";";
      }
      for(size_t a = 1; a <= solution->getNumberOfApps(); a++){
        outMOSTCSV << "PERIOD_APP" << a << ";";
      }
      for(size_t a = 1; a <= solution->getNumberOfApps(); a++){
        outMOSTCSV << "THROUGHPUT_APP" << a << ";";
      }
      for(size_t a = 1; a <= solution->getNumberOfApps(); a++){
        outMOSTCSV << "LATENCY_APP" << a << ";";
      }
      outMOSTCSV << "cluster;" << endl;
      outMOSTCSV << endl;
    }

    // "N_TASKS;N_EDGES;N_PES;N_SLOTS;N_SCHEDS;";
    outMOSTCSV << "\"" << program->n_programEntities() << "\";";
    outMOSTCSV << "\"" << program->n_SDFchannels() << "\";";
    outMOSTCSV << "\"" << target->nodes() << "\";";
    outMOSTCSV << "\"" << target->tdmaSlots() << "\";";
    outMOSTCSV << "\"" << target->nodes() << "\";";
    //"MAP_PE"
    for(size_t ji = 1; ji <= target->nodes(); ji++){
      outMOSTCSV << "\"(";
      for(size_t jj = 1; jj <= target->nodes(); jj++){
        if((ji == jj) && jj < target->nodes())
          outMOSTCSV << "1-";
        if((ji == jj) && jj == target->nodes())
          outMOSTCSV << "1";
        if((ji != jj) && jj < target->nodes())
          outMOSTCSV << "0-";
        if((ji != jj) && jj == target->nodes())
          outMOSTCSV << "0";
      }
      outMOSTCSV << ")\";";
    }
    //"FREQ_PE"
    for(size_t j = 1; j <= target->nodes(); j++){
      outMOSTCSV << "\"50\";";
    }
    //"MEM_PE"
    for(size_t j = 0; j < target->nodes(); j++){
      outMOSTCSV << "\"" << solution->getMemorySize(j) << "\";";
    }
    //"SLOTS_PE"
    vector<int> tdmaSlots = solution->getTDMAslots();
    for(size_t j = 0; j < tdmaSlots.size(); j++){
      outMOSTCSV << "\"" << tdmaSlots[j] << "\";";
    }
    //"MAP_T"
    vector<vector<int>> mapping = solution->getMappingSched();
    for(size_t i = 0; i < program->n_SDFActors(); i++){
      outMOSTCSV << "\"(";
      vector<int>::iterator it;
      for(size_t jj = 0; jj < target->nodes(); jj++){
        vector<int> proc = mapping[jj];
        it = find(proc.begin(), proc.end(), i);
        if((it != proc.end()) && jj < target->nodes() - 1)
          outMOSTCSV << "1-";
        if((it != proc.end()) && jj == target->nodes() - 1)
          outMOSTCSV << "1";
        if((it == proc.end()) && jj < target->nodes() - 1)
          outMOSTCSV << "0-";
        if((it == proc.end()) && jj == target->nodes() - 1)
          outMOSTCSV << "0";
      }
      outMOSTCSV << ")\";";
    }
    //"TASK_SCHED"
    outMOSTCSV << "\"@";
    for(size_t ji = 0; ji < target->nodes(); ji++){
      vector<int> proc = mapping[ji];
      for(size_t jj = 0; jj < proc.size(); jj++){
        outMOSTCSV << proc[jj] + 1;
        if(jj < proc.size() - 1 || (ji < target->nodes() - 1 && !mapping[ji + 1].empty()))
          outMOSTCSV << "-";
      }
    }
    outMOSTCSV << "@\";";
    //"COMM_SCHED"
    vector<int> chIds;
    for(size_t k = 0; k < program->n_SDFchannels(); k++){
      chIds.push_back(k);
    }
    int commEntries = 0;
    outMOSTCSV << "\"@";
    //vector<vector<SDFChannel*>> msgOrder = solution->getMessageOrder();
    vector<vector<int>> msgOrder = solution->getCommSched();
    for(size_t k = 0; k < msgOrder.size(); k++){
      //vector<SDFChannel*> sched = msgOrder[k];
      vector<int> sched = msgOrder[k];
      for(size_t i = 0; i < sched.size(); i++){
        //outMOSTCSV << sched[i]->newId; //newId does not exist anymore
        //outMOSTCSV << sched[i]->id;
        outMOSTCSV << sched[i];
        commEntries++;
        //chIds.erase(find(chIds.begin(), chIds.end(), sched[i]->newId));//newId does not exist anymore
        //chIds.erase(find(chIds.begin(), chIds.end(), sched[i]->id));
        chIds.erase(find(chIds.begin(), chIds.end(), sched[i]));
        if(i < sched.size() - 1 || (k < target->nodes() - 1 && !msgOrder[k + 1].empty()) || !chIds.empty())
          outMOSTCSV << "-";
      }
    }
    for(size_t k = 0; k < chIds.size(); k++){
      outMOSTCSV << chIds[k];
      if(k < chIds.size() - 1)
        outMOSTCSV << "-";
    }
    outMOSTCSV << "@\";";
    //"COMM_ENTRIES"
    //outMOSTCSV << "\"" << commEntries << "\";";
    //"MEMCONS_PE"
    vector<int> memLoad = solution->getMemLoads();
    for(size_t j = 0; j < memLoad.size(); j++){
      outMOSTCSV << "\"" << memLoad[j] << "\";";
    }
    //"U_PE"
    for(size_t j = 0; j < target->nodes(); j++){
      outMOSTCSV << "\"" << solution->getProcUtilization(j) << "\";";
    }
    //"PERIOD_APP"
    for(size_t a = 0; a < solution->getNumberOfApps(); a++){
      outMOSTCSV << "\"" << solution->getPeriod(a) << "\";";
    }
    //"THROUGHPUT_APP"
    for(size_t a = 0; a < solution->getNumberOfApps(); a++){
      outMOSTCSV << "\"" << 1.0 / ((double) solution->getPeriod(a)) << "\";";
    }
    //"LATENCY_APP"
    for(size_t a = 0; a < solution->getNumberOfApps(); a++){
      outMOSTCSV << "\"" << solution->getInitLatency(a) << "\";";
    }
    outMOSTCSV << 0 << ";";
    outMOSTCSV << endl;
  }

  /**
   * Prints the solutions in the ofstreams (out and outCSV)
   */
  template<class SearchEngine> void printSolution(SearchEngine *e, CPModelTemplate* s) {
    cout << nodes << " solution found in a search tree with " << e->statistics().node << " nodes so far" << endl;
    auto durAll = t_endAll - t_start;
    auto durAll_ms = std::chrono::duration_cast<std::chrono::milliseconds>(durAll).count();
    out << "*** Solution number: " << nodes << ", after " << durAll_ms << " ms" << ", search nodes: " << e->statistics().node << ", fail: "
        << e->statistics().fail << ", propagate: " << e->statistics().propagate << ", depth: " << e->statistics().depth << ", nogoods: "
        << e->statistics().nogood << " ***\n";
    s->print(out);
    /// Printing CSV format output
    /*if(cfg.settings().out_file_type == Config::ALL_OUT ||
       cfg.settings().out_file_type == Config::CSV){    
        outCSV << nodes << "," << durAll_ms << ",";
        s->printCSV(outCSV);
        s->printMappingCSV(outMappingCSV);
    }*/
    /**
     * Calling printcsv for the MOST tool
     */
    /*if(cfg.settings().out_file_type == Config::ALL_OUT ||
       cfg.settings().out_file_type == Config::CSV_MOST){
        Mapping* mapping = s->extractResult();
        const int split = -1;
        printMOSTCSV(mapping, nodes, split);
    }*/
  }
  ;

  /**
   * Loops through the solutions and prints them using the input search engine
   */
  template<class SearchEngine> void loopSolutions(SearchEngine *e) {
    nodes = 0;
    out.open(cfg.settings().output_path+"out/out.txt");
    LOG_INFO("Opened file for printing results: " +cfg.settings().output_path+"out/out.txt");
    outCSV.open(cfg.settings().output_path+"out/out.csv");
    outMOSTCSV.open(cfg.settings().output_path+"out/out-MOST.csv");    
    outMappingCSV.open(cfg.settings().output_path+"out/out_mapping.csv");
    LOG_INFO("started searching for " + cfg.get_search_type() + " solutions ");
    LOG_INFO("Printing frequency: " + cfg.get_out_freq());
    out << "\n \n*** \n";    
    
    CPModelTemplate * prev_sol = nullptr;
    t_start = runTimer::now();
    while(CPModelTemplate * s = e->next()){
      nodes++;
      if(nodes == 1){
        if(cfg.settings().search == Config::FIRST){
          t_endAll = runTimer::now();
          printSolution(e, s);
          return;
        }
        if(cfg.settings().out_print_freq == Config::FIRSTandLAST){
          t_endAll = runTimer::now();
          printSolution(e, s);
        }
      }
      t_endAll = runTimer::now();
      
      //cout << nodes << " solutions found." << endl;
      solutionData.push_back(SolutionValues{t_endAll-t_start, s->getOptimizationValues()});
      //cout << nodes << " solutions found." << endl;

      if(cfg.settings().out_print_freq == Config::ALL_SOL){
          cout << nodes << " solutions found so far." << endl;
          printSolution(e, s);          
      }
     /// We want to keep the last solution in case we only print the last one
      if(prev_sol != nullptr)
        delete prev_sol;
      prev_sol = s;

      if(cfg.settings().out_print_freq == Config::FIRSTandLAST ||
         cfg.settings().out_print_freq == Config::LAST){
        if(nodes % 100 == 0){
          cout << ".";
          if(nodes % 2000 == 0)
            cout.flush();
          if(nodes % 10000 == 0)
            cout << endl;
        }
      }

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

    if(cfg.settings().out_print_freq == Config::LAST && nodes > 0){
      printSolution(e, prev_sol);
    }else if(cfg.settings().out_print_freq == Config::LAST && nodes == 0){
      out << "No (better) solution found." << endl;
    }
    
    if(cfg.settings().out_print_freq == Config::FIRSTandLAST && nodes > 1){
      printSolution(e, prev_sol);
    }else if(cfg.settings().out_print_freq == Config::FIRSTandLAST && nodes == 1){
      out << "No better solution found." << endl;
    }
    delete prev_sol;
    
    for(auto i: solutionData){
      outCSV << std::chrono::duration_cast<std::chrono::milliseconds>(i.time).count() << " ";
      for(auto j: i.values){
        outCSV << j << " ";
      }
      outCSV << endl;
    }

    out.close();
    outCSV.close();
    outMOSTCSV.close();
    outMappingCSV.close();
  }

};

#endif
