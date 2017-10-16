/**
 * Copyright (c) 2013-2016, Nima Khalilzad   <nkhal@kth.se>
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
#pragma once
#include <gecode/int.hh>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <sstream>
#include <fstream>


using namespace Gecode;
using namespace Int;
using namespace std;

class Schedulability : public Propagator {

protected:
  ViewArray<IntView> wcet; /*!< current WCETs. */
  ViewArray<IntView> proc; /*!< current mapping of actors and task. */
  ViewArray<IntView> proc_mode; /*!< current proc_modes. */
  IntArgs periods;
  IntArgs priorities;
  IntArgs deadlines;
  const size_t n_actors;
  const size_t n_tasks;
  //for evaluation purposes
  size_t calls;
  int total_time;
  bool printDebug;
  /**
   * Returns the level-i workload given the current mapping
   * NOTE that the task has to be assiged to a processor before calling this
   * i.e. proc[entityID] == ture;
   * @param entity id
   * @param time
   * @return level-i workload of task with matching entity id
   */ 
  int leveliWorkload(int entityID, int t);  
  /**
   * @return true if tasks assigned to procid are schedulable based on their utilization bound test
   */ 
  bool utilizationBound(int procid);
  /**
   * @param int taskid
   * @param int procid
   * @return true if (1) task is assigned to a proc with (2) procid
   */ 
  bool isOnProc(int taskid, int procid);
  /**
   * @return true if tasks assigned to procid are schedulable based on the time demand test
   */ 
  bool timedemand(int procid);
        
public:
  Schedulability( Space& home, 
                  ViewArray<IntView> _wcet,
                  ViewArray<IntView> _proc,
                  ViewArray<IntView> _proc_mode,
                  IntArgs _periods,
                  IntArgs _priorities,
                  IntArgs _deadlines, 
                  int _n_actors
                  );

  static ExecStatus post(       Space& home, 
                                ViewArray<IntView> _wcet,
                                ViewArray<IntView> _proc,
                                ViewArray<IntView> _proc_mode,
                                IntArgs _periods,
                                IntArgs _priorities,
                                IntArgs _deadlines,
                                int _n_actors){
    (void) new (home) Schedulability(home, _wcet, _proc, _proc_mode, _periods, _priorities, _deadlines, _n_actors);
    return ES_OK;
  }

  virtual size_t dispose(Space& home);

  Schedulability(Space& home, bool share, Schedulability& p);

  virtual Propagator* copy(Space& home, bool share);

  //TODO: do this right
  virtual PropCost cost(const Space& home, const ModEventDelta& med) const;

  virtual void reschedule(Space& home);

  virtual ExecStatus propagate(Space& home, const ModEventDelta&);
  /**
   * Returns true if the taskset is schedulable given 
   * (1) current mapping and (2) partitioned FP algorithm
   * with rate monotonic priorities
   * @return true if schedulable, otherwise false
   */ 
  bool FPSchedulable();
        
};

extern void  Schedulability( Space& home, 
                             const IntVarArgs& _wcet,
                             const IntVarArgs& _proc,
                             const IntVarArgs& _proc_mode,
                             const IntArgs& _periods,
                             const IntArgs& _priorities,
                             const IntArgs& _deadlines, 
                             const int& _n_actors
                             );
extern void Schedulability();
