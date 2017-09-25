/**
 * Copyright (c) 2013-2016, Kathrin Rosvall  <krosvall@kth.se>
 *                          Nima Khalilzad   <nkhal@kth.se>
 *                          George Ungureanu <ugeorge@kth.se>
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
#ifndef __APPLICATIONS__
#define __APPLICATIONS__

#include <math.h>
#include <iostream>
#include <vector>

#include "sdfgraph.hpp" 
#include "pr_taskset.hpp" 
#include "../systemDesign/designConstraints.hpp"

#include "../exceptions/runtimeexception.h"

using namespace std;

class Applications {

protected:
  vector<SDFGraph*> sdfApps;
  vector<DesignConstraints*> desContr;
  //keep track of difference between local id and global id of sdf actors
  vector<size_t> offsets;
  TaskSet* iptApps;

  size_t n_sdfActors;
  size_t n_sdfParentActors;
  size_t n_sdfChannels;
  size_t n_iptTasks;

public:
  Applications();
  ~Applications();
  Applications(vector<SDFGraph*> _sdfApps, TaskSet* _iptApps);
  Applications(vector<SDFGraph*> _sdfApps, TaskSet* _iptApps, XMLdoc& xml);
  Applications(vector<SDFGraph*> _sdfApps, vector<DesignConstraints*> _desContr, TaskSet* _iptApps);
  Applications(vector<SDFGraph*> _sdfApps, vector<DesignConstraints*> _desContr);
  Applications(TaskSet* _iptApps);

  //Does id belong to an IPT task?
  bool isIPT(size_t id);
  //Does id belong to an SDF firing?
  bool isSDF(size_t id);
  //Is id1 dependent on id0?
  bool dependsOn(size_t id0, size_t id1);
  // Gives the name of the program entity id
  std::string getName(size_t id);
  // Gives the type of the program entity id
  std::string getType(size_t id);
  // Gives the name of the parent actor
  std::string getParentActorName(size_t id);
  // Gives the name of the graph
  std::string getGraphName(size_t g_id);

  //get number of SDF applications
  size_t n_SDFApps();
  //get number of SDF actors
  size_t n_SDFParentActors();
  //get number of SDF firings
  size_t n_SDFActors();
  //get number of SDF actors of specified graph
  size_t n_SDFActorsOfApp(size_t app);
  //get number of SDF channels
  size_t n_SDFchannels();
  //get number of IPT tasks
  size_t n_IPTTasks();
  //get total number of program entities (firings + tasks)
  size_t n_programEntities();
  //get total number of program channels
  size_t n_programChannels();
  //get the id of the last channel of application app
  size_t getMaxChannelId(size_t app) const;
  //get id of the application that channel ch belongs to
  size_t getAppIdForChannel(size_t ch) const;
  //Maximum code size of all program entities (firings + tasks)
  size_t getMaxCodeSize();

  //get code size of program entity id
  size_t getCodeSize(size_t id);
  //get code size of actor of program entity id
  size_t getCodeSizeActor(size_t id);
  //get code size of program entity id
  size_t getDataSize(size_t id);
  //get memory consumption of program entity id
  int getMemCons(size_t id);
  //get the period
  int getPeriod(size_t id);
  //get the phase
  int getPhase(size_t id);
  //get the deadline
  int getDeadline(size_t id);
  //task preemptable?
  bool isPreemtable(size_t id);
  //get all possible hyperperiods
  vector<int> getHyperperiods();
  //get maximum hyperperiod
  int getMaxHyperperiod();
  //get maximum number of instances of task id
  int getMaxNumberOfIPTInstances(size_t id);
  //get maximum number of ipt instances
  int getMaxNumberOfIPTInstances();
  //get worst case execution cost (-time depends on mapping)
  vector<int> getWCEC(size_t id);
  //get max worst case execution cost (-time depends on mapping)
  int getMaxWCET(size_t id);
  //get min worst case execution cost (-time depends on mapping)
  int getMinWCET(size_t id);
  //get worst case execution cost of ipt tasks(-time depends on mapping)
  int getMaxWCET_IPT();
  //get bound on period for graph g_id from design constraints
  int getPeriodBound(size_t g_id);
  //get constraint type on period, sat or opt
  SolutionMode getConstrType(size_t g_id);

  //get index of the SDFG that actor id belongs to
  size_t getSDFGraph(size_t id);
  //get the actor from which id originates (if SDF firing)
  int getParentActor(size_t id);  
  //get the instance of actor id with the highest index (the "last" 
  int getLastFiring(size_t id);
  //get a list of predecessors of program entity id
  vector<int> getPredecessors(size_t id);
  //get a list of successors of program entity id
  vector<int> getSuccessors(size_t id);

  //get the list of channels in the application appId 
  vector<SDFChannel*> getChannels(int appId);
  //get the list of channels in all applications 
  vector<SDFChannel*> getChannels();
  //get the token size on channel id
  size_t getTokenSize(size_t id);
  //get the number of initial tokens on a channel from src to dst
  int getTokensOnChannel(int src, int dst);
  //does a channel from src to destination exist?
  bool channelExists(int src, int dst);

  //get a topological ordering of the program entities
  vector<int> getTopologicalOrdering();
  /**
   * Returns the period of the task which has the input entity ID
   * If entity id does not exist, it returns empty string
   * @param id of the entity
   * @return name of the task
   */
  int getTaskPeriod(int entityID);
  /**
   * Returns the name of the task which has the input entity ID
   * If entity id does not exist, it returns empty string
   * @param id of the entity
   * @return name of the task
   */
  string getTaskName(int entityID); 
  /**
   * Returns the priority of the task which has the input entity ID
   * If entity id does not exist, it returns empty string
   * @param id of the entity
   * @return name of the task
   */
  int getTaskPriority(int entityID);
  /**
   * Returns the deadline of the task which has the input entity ID
   * If entity id does not exist, it returns empty string
   * @param id of the entity
   * @return deadline of the task
   */
  int getTaskDeadline(int entityID);
  /**
   * Calls the SetRMPriority method in the taskset class
   */ 
  void setRMPriorities();
  /**
   * returns the period constraint of the input sdf id
   */ 
  int getPeriodConstraint(size_t id);
  /**
   * returns the latency constraint of the input sdf id
   */ 
  int getLatencyConstraint(size_t id);
  /**
   * returns the task type
   */ 
  string getTaskType(int entityID);
  /**
   * Swaps pr task_i with pr task_j
   */ 
  void swapPrTasks(int i, int j);
  /**
   * Loads the design constraints from an xml input
   */ 
  void load_const(XMLdoc&);
  /**
   * assinging period and latency constraints for an sdf application
   */ 
  void set_const(string app_name, int period_const, int latency_const);
  /**
   * Prints applications
   */ 
  friend std::ostream& operator<< (std::ostream &out, const Applications &apps); 
};
#endif
