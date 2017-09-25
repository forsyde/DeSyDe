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

template<class PresolverCPTemplate, class CPModelTemplate>
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
  CPModelTemplate* presolve(Mapping* map) {
    size_t step = 0;
    settings.setOptimizationStep(step);
    if(settings.doPresolve()){
      PresolverCPTemplate* pre_model = new PresolverCPTemplate(map, settings);

      switch (settings.settings().pre_search) {
      case (Config::GIST_ALL): {
        Gist::Print<PresolverCPTemplate> p("Print solution");
        Gist::Options options;
        options.inspect.click(&p);
        Gist::dfs(pre_model, options);
        break;
      }
      case (Config::GIST_OPT): {
        Gist::Print<PresolverCPTemplate> p("Print solution");
        Gist::Options options;
        options.inspect.click(&p);
        Gist::bab(pre_model, options);
        break;
      }
      case (Config::FIRST):
      case (Config::ALL): {
        cout << "DFS engine ...\n";
        DFS<PresolverCPTemplate> e(pre_model, geSearchOptions);
        loopSolutions<DFS<PresolverCPTemplate>>(&e, map);
        break;
      }
      case (Config::OPTIMIZE): {
        cout << "BAB engine, optimizing ... \n";
        BAB<PresolverCPTemplate> e(pre_model, geSearchOptions);
        loopSolutions<BAB<PresolverCPTemplate>>(&e, map);
        break;
      }
      case (Config::OPTIMIZE_IT): {
        cout << "BAB engine, optimizing iteratively ... \n";
        Search::Cutoff* cut = Search::Cutoff::luby(settings.settings().luby_scale);
        geSearchOptions.cutoff = cut;
        RBS<BAB, PresolverCPTemplate> e(pre_model, geSearchOptions);
        loopSolutions<RBS<BAB, PresolverCPTemplate>>(&e, map);
        break;
      }
      default:
        THROW_EXCEPTION(RuntimeException, "unknown search type for presolver.");
        break;
      }
    }
    if(settings.doMultiStep()){
      LOG_DEBUG("  MULTISTEP SOLVING");
      
      for(auto i: settings.settings().pre_heuristics){
        switch(i){
        case (Config::TODAES): {
          LOG_DEBUG("    using TODAES heuristic");
          useHeuristic_TODAES(map);
          full_model = new CPModelTemplate(map, &settings);
          break;
        }
        default:
          THROW_EXCEPTION(RuntimeException, "unknown heuristic for multi-step solving");
          break;
        }
        
        geSearchOptions.threads = settings.settings().threads;
        if(settings.settings().timeout_first > 0){
          Search::TimeStop* stop = new Search::TimeStop(settings.settings().timeout_first);
          geSearchOptions.stop = stop;
        }
        
        switch (settings.settings().pre_multi_step_search) {
        case (Config::GIST_ALL): {
          Gist::Print<CPModelTemplate> p("Print solution");
          Gist::Options options;
          options.inspect.click(&p);
          Gist::dfs(full_model, options);
          break;
        }
        case (Config::GIST_OPT): {
          Gist::Print<CPModelTemplate> p("Print solution");
          Gist::Options options;
          options.inspect.click(&p);
          Gist::bab(full_model, options);
          break;
        }
        case (Config::FIRST):
        case (Config::ALL): {
          LOG_DEBUG("    DFS engine ...");
          DFS<CPModelTemplate> e(full_model, geSearchOptions);
          loopSolutions<DFS<CPModelTemplate>>(&e);
          break;
        }
        case (Config::OPTIMIZE): {
          LOG_DEBUG("    BAB engine, optimizing ...");
          BAB<CPModelTemplate> e(full_model, geSearchOptions);
          loopSolutions<BAB<CPModelTemplate>>(&e);
          break;
        }
        case (Config::OPTIMIZE_IT): {
          LOG_DEBUG("    BAB engine, optimizing iteratively ...");
          Search::Cutoff* cut = Search::Cutoff::luby(settings.settings().luby_scale);
          geSearchOptions.cutoff = cut;
          RBS<BAB, CPModelTemplate> e(full_model, geSearchOptions);
          loopSolutions<RBS<BAB, CPModelTemplate>>(&e);
          break;
        }
        default:
          THROW_EXCEPTION(RuntimeException, "unknown search type for multi-step solving.");
          break;
        }
        
        LOG_DEBUG("  final step optimizing for " + tools::toString(settings.settings().criteria.back()));
        
        // +++ Now: Create the full model
        LOG_INFO("Creating full CP model from multi-step solver, step " + tools::toString(step+1));
        map->resetFirstMapping();
        full_model = new CPModelTemplate(map, &settings);
        step++;
        settings.setOptimizationStep(step);
      }
    }
    
    return full_model;
  }
  ;
  vector<tuple<int, vector<tuple<int,int>>>> getMappingResults() const{
    return results->oneProcMappings;
  }
  ;

private:
  vector<PresolverCPTemplate*> pre_models; /**< Vector of presolver constraint models. */
  CPModelTemplate* full_model; /**< Pointer to the problem constraint model. */
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
  template<class SearchEngine> void printSolution(SearchEngine *e, PresolverCPTemplate* s) {

    out << "Solution " << nodes << ":" << endl;
    out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    s->print(out);
    out << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";

  }
  ;
  
  /**
   * Prints the solutions in the ofstreams (out and outCSV)
   */
  template<class SearchEngine> void printSolution(SearchEngine *e, CPModelTemplate* s) {
    LOG_INFO(tools::toString(nodes) +" solution found in a search tree with " + tools::toString(e->statistics().node) + " nodes so far");
    auto durAll = t_endAll - t_start;
    auto durAll_ms = std::chrono::duration_cast<std::chrono::milliseconds>(durAll).count();
    out << "*** Solution number: " << nodes << ", after " << durAll_ms << " ms" << ", search nodes: " << e->statistics().node << ", fail: " << e->statistics().fail << ", propagate: "
        << e->statistics().propagate << ", depth: " << e->statistics().depth << ", nogoods: " << e->statistics().nogood << ", restarts: " << e->statistics().restart << " ***\n";
    s->print(out);
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
          printSolution(e, (PresolverCPTemplate*)s);
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

      printSolution(e, (PresolverCPTemplate*)s);
      results->it_mapping = nodes-1;
      results->oneProcMappings.push_back(((PresolverCPTemplate*)s)->getResult());
      delete s;

      settings.setPresolverResults(results);
      CPModelTemplate* full_model = new CPModelTemplate(map, &settings);
      DFS<CPModelTemplate> ef(full_model, geSearchOptions);
      if(CPModelTemplate * sf = ef.next()){
        fullNodes++;
        outFull << "Pre-solution " << nodes << "----------" << endl;
        sf->print(outFull);
        outFull << "------------------------------" << endl << endl;
        //Mapping* mapRes = sf->extractResult();
        //settings.getPresolverResults()->periods.push_back(mapRes->getPeriods());
        //settings.getPresolverResults()->sys_energys.push_back(mapRes->getSysEnergy());
        t_endAll = runTimer::now();
        settings.getPresolverResults()->optResults.push_back(Config::SolutionValues{t_endAll-t_start, sf->getOptimizationValues()});
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
    settings.getPresolverResults()->presolver_delay = durAll;
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
    LOG_INFO("Creating full CP model from Presolver...");
    results->it_mapping = results->oneProcMappings.size();
    settings.setPresolverResults(results);

    full_model = new CPModelTemplate(map, &settings);
    delete dseSettings;
  }
  
  /**
   * Loops through the solutions for the multi-step solving and prints them using the input search engine
   */
  template<class SearchEngine> void loopSolutions(SearchEngine *e) {
    nodes = 0;
    size_t timerResets = 0;
    out.open(settings.settings().output_path + "/out/" + "out_step"+tools::toString(settings.settings().optimizationStep)+"_results.txt");
    //outFull.open(cfg.settings().output_path+"out/out.txt");
    out << "~~~~~ *** BEGIN OF MULTI-STEP (STEP "+tools::toString(settings.settings().optimizationStep)+") SOLUTIONS *** ~~~~~" << endl;
    
    std::chrono::high_resolution_clock::duration presolver_delay(0);
    if(settings.doPresolve() && settings.is_presolved()){
      presolver_delay = settings.getPresolverResults()->presolver_delay;
    }else{
      settings.setPresolverResults(results);
    }
    
    CPModelTemplate* prev_sol = nullptr;
    t_start = runTimer::now();
    while(Space * s = e->next()){
      nodes++;
      if(nodes == 1){
        if(settings.settings().pre_multi_step_search == Config::FIRST){
          t_endAll = runTimer::now();
          printSolution(e, (CPModelTemplate*)s);
          return;
        }
        if(settings.settings().out_print_freq == Config::FIRSTandLAST){
          t_endAll = runTimer::now();
          printSolution(e, (CPModelTemplate*)s);
        }
      }
      t_endAll = runTimer::now();
      
      //cout << nodes << " solutions found." << endl;
      settings.getPresolverResults()->optResults.push_back(Config::SolutionValues{t_endAll-t_start+presolver_delay, ((CPModelTemplate*)s)->getOptimizationValues()});
      //cout << nodes << " solutions found." << endl;

      if(settings.settings().out_print_freq == Config::ALL_SOL){
          printSolution(e, (CPModelTemplate*)s);          
      }
     /// We want to keep the last solution in case we only print the last one
      if(prev_sol != nullptr)
        delete prev_sol;
      prev_sol = (CPModelTemplate*)s;

      if(settings.settings().out_print_freq == Config::FIRSTandLAST ||
         settings.settings().out_print_freq == Config::LAST){
        if(nodes % 100 == 0){
          cout << ".";
          if(nodes % 2000 == 0)
            cout.flush();
          if(nodes % 10000 == 0)
            cout << endl;
        }
      }
      
      if(settings.settings().timeout_all){
        ((Search::TimeStop*)geSearchOptions.stop)->reset();
        ((Search::TimeStop*)geSearchOptions.stop)->limit(settings.settings().timeout_all);
        timerResets++;
      }
    }
    
    
    auto durAll = runTimer::now() - t_start;
    settings.getPresolverResults()->presolver_delay += durAll;
    auto durAll_s = std::chrono::duration_cast<std::chrono::seconds>(durAll).count();
    auto durAll_ms = std::chrono::duration_cast<std::chrono::milliseconds>(durAll).count();
    
    if(settings.settings().out_print_freq == Config::LAST && nodes > 0){
      printSolution(e, prev_sol);
    }else if(settings.settings().out_print_freq == Config::LAST && nodes == 0){
      out << "No (better) solution found." << endl;
    }
    
    if(settings.settings().out_print_freq == Config::FIRSTandLAST && nodes > 1){
      printSolution(e, prev_sol);
    }else if(settings.settings().out_print_freq == Config::FIRSTandLAST && nodes == 1){
      out << "No better solution found." << endl;
    }
    delete prev_sol;
    
    out << "===== search ended after: " << durAll_s << " s (" << durAll_ms << " ms)";
    if(e->stopped()){
      out << " due to time-out!";
    }
    if(settings.settings().timeout_all){
      out << " (with " << timerResets << " incremental timer reset(s).)";
    }
    out << " =====\n" << nodes << " solutions found\n" << "search nodes: " << e->statistics().node << ", fail: " << e->statistics().fail << ", propagate: "
        << e->statistics().propagate << ", depth: " << e->statistics().depth << ", nogoods: " << e->statistics().nogood << ", restarts: " << e->statistics().restart << " ***\n";
        
        
    out << "\n~~~~~ *** END OF MULTI-STEP (STEP "+tools::toString(settings.settings().optimizationStep)+") SOLUTIONS *** ~~~~~" << endl;

    out.close();
  }
  
  void useHeuristic_TODAES(Mapping* map){
  //vector<div_t> determineFirstMapping(vector<SDFGraph*>& sdfApps, Applications* apps, Platform* target, Mapping* mapping){
    vector<div_t> proc(map->getApplications()->n_SDFApps());
    
    //Step 1: each app gets 1 proc
    vector<int> share(map->getApplications()->n_SDFApps(), 1);
    //Step 2: check how many procs are left to be distributed
    int n_extraProcs = map->getPlatform()->nodes() - map->getApplications()->n_SDFApps();
    if(n_extraProcs>0){
      n_extraProcs = map->getPlatform()->nodes();
      //Step 3: try to give all apps with throughput constraints as many as needed
      for(unsigned int i=0; i<map->getApplications()->n_SDFApps(); i++){
        if(map->getApplications()->getPeriodConstraint(i) > 0){
          int sumMinWCETs = 0;
          for(size_t j=0; j<map->getApplications()->n_programEntities(); j++){
            if(map->getApplications()->getSDFGraph(j)==i){
              vector<int> wcets;
              
              for(size_t p=0; p<map->getPlatform()->nodes(); p++){
                vector<int> wcets_proc = map->getValidWCETs(j, p);
                if(wcets_proc.size()>0)
                  wcets.push_back(*min_element(wcets_proc.begin(),wcets_proc.end()));
              }
              sumMinWCETs += *min_element(wcets.begin(),wcets.end()); 
            }
          }
          share[i] = ceil((double)sumMinWCETs/map->getApplications()->getPeriodConstraint(i));
          n_extraProcs -= share[i];
        }
      }
    }

    if(n_extraProcs>0){
      int optApps = 0;
      int remainder;
      //Step 4: give remaining procs to the graph with optimization
      for(unsigned int i=0; i<map->getApplications()->n_SDFApps(); i++){
        if(map->getApplications()->getPeriodConstraint(i) == -1){
          optApps++;
        }
      }
      remainder = n_extraProcs%optApps;
      for(unsigned int i=0; i<map->getApplications()->n_SDFApps(); i++){
        if(map->getApplications()->getPeriodConstraint(i) == -1){
          share[i] = n_extraProcs/optApps;
          if(remainder>0){
            share[i]+=remainder;
            remainder = 0;
          }
          share[i] = min((int)map->getApplications()->n_SDFActorsOfApp(i), share[i]);
          n_extraProcs -= share[i];
          optApps--;
        }
      }
    }
    
  //  for(unsigned int i=0; i<sdfApps.size(); i++){
    //  share[i] += nearbyint((double)sdfApps[i]->n_actors()*n_extraProcs/map->getApplications()->n_SDFActors());
    //}
    
    int minProc=0;
    for(unsigned int i=0; i<map->getApplications()->n_SDFApps(); i++){
      proc[i].quot = minProc;
      proc[i].rem = minProc + share[i] -1;
      LOG_INFO("  First mapping in multi-step solving for app " + tools::toString(i)
               + ": "+ tools::toString(proc[i].quot) + " - " + tools::toString(proc[i].rem) );
      minProc = proc[i].rem +1;
    }
    //return proc;
    map->setFirstMapping(proc);
  //}
    
  }
  
  
};

#endif

