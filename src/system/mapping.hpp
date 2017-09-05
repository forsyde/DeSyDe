/**
 * Copyright (c) 2013-2016, Kathrin Rosvall  <krosvall@kth.se>
 *                          Nima Khalilzad   <nkhal@kth.se>
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
#ifndef __MAPPING__
#define __MAPPING__

#include <vector>
#include <algorithm>

#include "../applications/applications.hpp"
#include "../platform/platform.hpp"

#include "../exceptions/runtimeexception.h"


using namespace std;


/**
 * This class specifies a mapping.
 * TODO: This could be specified in an XML file.
 */
class Mapping {
protected:

  Applications* program;            /*!< Reference to the applications. */
  Platform* target;                 /*!< Reference to the target platform. */
  int n_apps;                       /*!< Number of applications in the mapping (=SDF apps + no. of tasks). */
  vector<vector<int>> mappingSched; /*!< The mapping, in form of a schedule of actors, for each processing element. */
  vector<vector<int>> commSched;    /*!< The order-based schedules for communication channels from each processor. */
  vector<int> period;               /*!< The period for each application. */
  vector<int> initLatency;          /*!< The length of the initial transient phase for each application. */
  vector<int> proc_period;          /*!< The period for each processor. */
  int procsUsed;                    /*!< Number of processors used in the mapping. */
  vector<int> proc_modes;           /*!< Processing mode of each processor. */
  vector<int> tdma_slots;           /*!< Number of TMDA slots allocated to each processor. */
  vector<int> send_buff;            /*!< Buffer sizes for receiving buffer of all channels. */
  vector<int> rec_buff;             /*!< Buffer sizes for receiving buffer of all channels. */

  vector<vector<vector<int>>> wcets; /*!< WCET of all actors and all periodic tasks on each processor in all modes. */
  vector<int> fixed_wcets;   /*!< WCET of all actors and all periodic tasks on its allocated processor 
                               in a fixed mapping. */
  vector<div_t> comm_delay;   /*!< Communication delay for all channels (sending).
                                div_t.quot: blocking time
                                div_t.rem: sending time (transfer on bus) */

  vector<int> memLoad;          /*!< Amount of memory used on each processor. */
  int sys_utilization;          /*!< Utilization of the system. */
  int procsUsed_utilization;    /*!< Utilization of the system, only considering the used processors. */
  vector<int> proc_utilization; /*!< Utilization of each processor. */
  int sys_energy;               /*!< Energy consumption of the system. */
  vector<int> proc_energy;      /*!< Energy consumption of each processor. */
  int sys_area;                 /*!< Area consumption of the system. */
  vector<int> proc_area;        /*!< Area consumption of each processor. */
  int sys_cost;                 /*!< Cost consumption of the system. */
  vector<int> proc_cost;        /*!< Cost consumption of each processor. */
  
  /** Captures a first almost-fixed mapping for two-step optimization. 
      It determines for each application which processing nodes it is allowed
      to use in the first, heuristically-restricted step.
      div_t.quot: index of the first proc for the application.
      div_t.rem: index of the last proc for the application. */
  vector<div_t> firstMapping;
 
  //for maximum schedule length (transient + periodic phase)
  int maxScheduleLength;
  //for saving first index of each program entity in the time-based static schedule (transient phase)
  vector<int> firstIndexTransPhEntity;
  //for saving last index of each program entity in the time-based static schedule (transient phase)
  vector<int> lastIndexTransPhEntity;
  //for saving the max number of instantiations of each entity in the time-based static schedule (transient phase)
  vector<int> maxIterationsTransPhEntity;

  //for saving first index of each channel in the time-based static schedule (transient phase)
  vector<int> firstIndexTransPhChannel;
  //for saving last index of each channel in the time-based static schedule (transient phase)
  vector<int> lastIndexTransPhChannel;
  //for saving the max number of instantiations of each channel in the time-based static schedule (transient phase)
  vector<int> maxIterationsTransPhChannel;

  // Gives the maximum number of iterations for program entities (ipt or
  // sdf actor) in the fully static time-based schedule (transient phase)
  void maxIterations();
  
  //Time spent in schedulability analysis
  clock_t schedulabilityTime; 

  vector<int>  current_mapping; /*!< Used by validation class. */  
  vector<int>  current_modes;   /*!< Used by validation class. */  

public:

  int const max_utilization = 100; //TODO: add to configuration

  Mapping() {};
  Mapping(Applications*, Platform*, XMLdoc&);
  //Mapping(Applications*, Platform*, vector<vector<int>>&, vector<int>&, vector<int>&, vector<vector<SDFChannel*>>&);

  ~Mapping();
  void load_wcets(XMLdoc& xml);
  Applications* getApplications() const;

  Platform* getPlatform() const;

 int getNumberOfApps() const;                       /*!< Gets the number of applications in the mapping (=SDF apps + no. of tasks). */

 void setMappingScheds(vector<vector<int>>&);       /*!< Sets the actor mapping and schedules. */
 
 void setMappingSched(vector<int>& p_sched, int p); /*!< Sets the actor mapping and schedule for processor p. */
 
 vector<vector<int>> getMappingSched() const;       /*!< Gets the actor mapping and schedules. */
 
 vector<int> getMappingSched(int p) const;          /*!< Gets the actor mapping and schedule of processor p. */
 
 void setCommScheds(vector<vector<int>>&);          /*!< Sets the channel schedules for each processor. */
 
 void setCommSched(vector<int>&, int p);            /*!< Sets the channel schedule for processor p. */
 
 vector<vector<int>> getCommSched() const;          /*!< Gets the channel schedules for all processors. */
 
 vector<int> getCommSched(int p) const;             /*!< Gets the channel schedule of processor p. */
 
 void setPeriod(int app, int p_period);             /*!< Sets the period (inverse of throughput). */
 
 void setPeriods(vector<int>& p_periods);           /*!< Sets the periods of all apps. */

 int getPeriod(int app) const;                      /*!< Gets the period (inverse of throughput). */

 vector<int> getPeriods() const;                    /*!< Gets the periods of all apps (inverse of throughput). */

 void setInitLatency(int app, int p_latency);       /*!< Sets the initial latency (inverse of throughput). */
 
 void setInitLatencys(vector<int>& p_latency);      /*!< Sets the initial latencys of all apps. */

 int getInitLatency(int app) const;                 /*!< Gets the initial latency (inverse of throughput). */

 vector<int> getInitLatencys() const;               /*!< Gets the initial latencys of all apps (inverse of throughput). */
 
 void setProcPeriod(int proc, int p_period);        /*!< Sets the period of a processor. */
 
 void setProcPeriods(vector<int>& p_periods);       /*!< Sets the periods of all procs. */

 int getProcPeriod(int proc) const;                 /*!< Gets the period of a processor. */

 vector<int> getProcPeriods() const;                /*!< Gets the periods of all procs. */
 
 void setProcsUsed(int p_procs);                    /*!< Sets the number of processors used in the mapping. */
 
 int getProcsUsed() const;                          /*!< Gets the number of processors used in the mapping. */
 
 void setProcMode(int proc, int p_mode);            /*!< Sets the mode of a processor. */
 
 void setProcModes(vector<int>& p_modes);           /*!< Sets the modes of all procs. */

 int getProcMode(int proc) const;                   /*!< Gets the mode of a processor. */

 vector<int> getProcModes() const;                  /*!< Gets the modes of all procs. */
 
 void setTDMAslots(int proc, int p_slots);          /*!< Sets the number of TMDA slots allocated to the processor. */
 
 void setTDMAslots(vector<int>& p_slots);           /*!< Sets the number of TMDA slots allocated to each processor. */

 int getTDMAslots(int proc) const;                  /*!< Gets the number of TMDA slots allocated to the processor. */

 vector<int> getTDMAslots() const;                  /*!< Gets the number of TMDA slots allocated to each processor. */
 
 void setSendBuff(int ch, int p_size);              /*!< Sets the size of the sending buffer of the channel. */
 
 void setSendBuffs(vector<int>& p_sizes);           /*!< Sets the size of the sending buffer of all channels. */

 int getSendBuff(int ch) const;                     /*!< Gets the size of the sending buffer of the channel. */

 vector<int> getSendBuffs() const;                  /*!< Gets the size of the sending buffer of all channels. */
 
 void setRecBuff(int ch, int p_size);               /*!< Sets the size of the receiving buffer of the channel. */
 
 void setRecBuffs(vector<int>& p_sizes);            /*!< Sets the size of the receiving buffer of all channels. */

 int getRecBuff(int ch) const;                      /*!< Gets the size of the receiving buffer of the channel. */

 vector<int> getRecBuffs() const;                   /*!< Gets the size of the receiving buffer of all channels. */
  
  /** The function goes through all actors and pr tasks 
    to find the corresponding task and it assignes the WCET set.
    @param type of task.
    @param model of the proccessor
    @param wcets of tasktype on procType. */ 
  void setWCETs(string taskType, string procModel, string procMode, int _wcet);
  /** The function parses the vectors read from the xml file and
    calls setWCETs(string taskType, string procModel, int _wcet); */ 
  void setWCETs(vector<char*> elements, vector<char*> values);

  /** @return vector of all WCETs of an actor on all processors
    factoring in speedups */
  vector<int> getWCETsModes(unsigned actorId) const;

  /** @return vector of all WCETs of an actor on all processors
    without processing mode */
  vector<int> getWCETsSingleMode(unsigned actorId) const;

  /** @return vector of vectors of all WCETs of an actor on all processors
  in all modes */
  vector<vector<int>> getWCETs(unsigned actorId) const;

  /** @return WCETs of an actor on a particular processor in different modes */ 
  vector<int> getWCETs(unsigned actorId, unsigned proc) const;
  
  /** @return WCET of an entity on a particular processor in a given mode */ 
  int getWCET(unsigned actorId, unsigned proc, unsigned mode) const;
  
  /** @param id of the actor/task.
    @return the minimal WCET of an actor/task. */
  int getMinWCET(unsigned actorId) const;

  /** @param id of the actor/task.
    @return the maximal WCET of an actor/task. */
  int getMaxWCET(unsigned actorId) const;

  /** 
      Gets only all possible wcets, no -1 "mapping not possible" values. */
  vector<int> getValidWCETs(unsigned actorId, unsigned proc) const;

  /** Returns the actor ids of graph app in decending order of max WCETs (heaviest first).
    @param The application graph to consider. */
  vector<int> sortedByWCETs(size_t app) const;

  /** 
      Returns the actor ids in decending order of max WCETs (heaviest first). */
  vector<int> sortedByWCETs();
  
  void setFixedWCET(int id, int p_wcet);              /*!< Sets the WCET of the actor/task. */
  
  void setFixedWCETs(vector<int>& p_wcets);           /*!< Sets the WCET of all actors/tasks. */

  int getFixedWCET(int id) const;                     /*!< Gets the WCET of the actor/task. */

  vector<int> getFixedWCETs() const;                  /*!< Gets the WCET of all actors/tasks. */
  
  void setCommDelay(int id, div_t p_wcct);            /*!< Sets the communication delay of the channel. */
  
  void setCommDelays(vector<div_t>& p_wccts);         /*!< Sets the communication delay of the channels. */

  div_t getCommDelay(int id) const;                   /*!< Gets the communication delay of the channel. */

  vector<div_t> getCommDelays() const;                /*!< Gets the communication delay of the channels. */
  
  void setMemLoad(unsigned proc, int p_mem);          /*!< Sets the memory load on the processor. */
  
  void setMemLoads(vector<int>& p_mems);              /*!< Sets the memory load on the processors. */
 
  int getMemLoad(unsigned proc) const;                /*!< Gets the memory load on the processor. */

  vector<int> getMemLoads() const;                    /*!< Gets the memory load on the processors. */
   
  void setSysUtilization(int p_util);                 /*!< Sets the system utilization. */
  
  int getSysUtilization() const;                      /*!< Gets the system utilization. */
  
  void setProcsUsedUtilization(int p_util);           /*!< Sets the system utilization, only considering the used processors. */
  
  int getProcsUsedUtilization() const;                /*!< Gets the system utilization, only considering the used processors. */

  void setProcUtilization(unsigned proc, int p_util); /*!< Sets the utilization on the processor. */
  
  void setProcUtilizations(vector<int>& p_utils);     /*!< Sets the utilization on the processors. */

  int getProcUtilization(unsigned proc) const;        /*!< Gets the utilization on the processor. */

  vector<int> getProcUtilizations() const;            /*!< Gets the utilization on the processors. */
  
  void setSysEnergy(int p_nrg);                       /*!< Sets the system energy consumption. */
  
  int getSysEnergy() const;                           /*!< Gets the system energy consumption. */
  
  void setProcEnergy(unsigned proc, int p_nrg);       /*!< Sets the energy consumption on the processor. */
  
  void setProcEnergys(vector<int>& p_nrgs);           /*!< Sets the energy consumption on the processors. */

  int getProcEnergy(unsigned proc) const;             /*!< Gets the energy consumption on the processor. */

  vector<int> getProcEnergys() const;                 /*!< Gets the energy consumption on the processors. */
  
  void setSysArea(int p_area);                        /*!< Sets the system area consumption. */
  
  int getSysArea() const;                             /*!< Gets the system area consumption. */
  
  void setProcArea(unsigned proc, int p_area);        /*!< Sets the area consumption of the processor. */
  
  void setProcAreas(vector<int>& p_areas);            /*!< Sets the area consumption of the processors. */

  int getProcArea(unsigned proc) const;               /*!< Gets the area consumption of the processor. */

  vector<int> getProcAreas() const;                   /*!< Gets the area consumption of the processors. */
  
  void setSysCost(int p_cost);                        /*!< Sets the system cost. */
  
  int getSysCost() const;                             /*!< Gets the system cost. */
  
  void setProcCost(unsigned proc, int p_cost);        /*!< Sets the cost of the processor. */
  
  void setProcCosts(vector<int>& p_costs);            /*!< Sets the cost of the processors. */

  int getProcCost(unsigned proc) const;               /*!< Gets the cost of the processor. */

  vector<int> getProcCosts() const;                   /*!< Gets the cost of the processors. */
  
  bool homogeneousPlatform(); /*!< Determines whether the platform is homogenous. */
  bool homogeneousNodes(int nodeI, int nodeJ); /*!< Determines whether nodes nodeI and nodeJ are homogenous. */
  bool homogeneousModeNodes(int nodeI, int nodeJ); /*!< Determines whether nodes nodeI and nodeJ have the same set of modes. */

  void setFirstMapping(vector<div_t>& _firstMapping);

  vector<div_t> getFirstMapping() const;

  void resetFirstMapping();
  
  // Sets the utilization
  //void setUtilization(int node, float utilization);

  // Gets the utilization
  //float getUtilization(int node);

  // Gets the throughput value of the mapping
  //float getThroughput(int app) const;

  // Sets the allocated tdma slots per processor
  //void setTDMAslots(vector<int>& p_slots);

  // Gets the allocated tdma slots per processor
  //vector<int> getTDMAslots() const;

  // Sets the allocated tdma slots per processor
  //void setMemoryLoad(vector<int>& p_memLoad);

  // Gets the allocated tdma slots per processor
  //vector<int> getMemoryLoad() const;

  // Sets the allocated tdma slots per processor
  //void setMessageOrder(vector<vector<SDFChannel*>>& p_msgOrder);

  // Gets the allocated tdma slots per processor
  //vector<vector<SDFChannel*>> getMessageOrder() const;

  // Gives the memory consumption of the specified actor on the specified node
  int memConsCode(int actor, int node);

  // Gives the memory consumptions of the specified actor on all nodes
  const vector<int> memConsCode(int actor);

  // Gives the memory consumption of the specified actor on the specified node
  int memConsData(int actor, int node);

  // Gives the memory consumptions of the specified actor on all nodes
  const vector<int> memConsData(int actor);

  // Gives the maximum communication time for the channel
  int wcCommTime(int channel);

  // Gives a vector with communication times for the channel, depending on allocated TDMA slots
  const vector<int> wcCommTimes(int channel);

  // Gives a vector with blocking times for the channel, depending on allocated TDMA slots
  // (waiting time from release of a message until the first available slot)
  const vector<int> wcBlockingTimes();

  // Gives a vector with communication times for the channel, depending on allocated TDMA slots
  const vector<int> wcTransferTimes(int channel);

  // Gives the sum of maximum communication times over all channels
  int sumWcetCommTimes();

  // Sets the maximum length of the time-based static schedule, consisting
  // of transient and periodic phase
  void setMaxScheduleLength(int p_time);

  // Gives the maximum length of the time-based static schedule, consisting
  // of transient and periodic phase
  int getMaxScheduleLength();

  // Sets the maximum number of iterations for program entity (ipt or
  // sdf actor) id in the fully static time-based schedule
  void setMaxIterationsEntity(int id, int its);

  // Gives the maximum number of iterations for program entity (ipt or
  // sdf actor) id in the fully static time-based schedule
  int getMaxIterationsEntity(int id);

  // Gives the sum of iterations for program entities (ipt or
  // sdf actor) in the fully static time-based schedule
  int sumIterationsEntity();

  // Sets the maximum number of iterations for channel (src, dst)
  // in the fully static time-based schedule
  void setMaxIterationsChannel(int id, int its);

  // Gives the maximum number of iterations for channel (src, dst)
  // in the fully static time-based schedule
  int getMaxIterationsChannel(int id);

  // Gives the sum of iterations for channels 
  // in the fully static time-based schedule
  int sumIterationsChannel();

  //Determines the indeces (first & last) for each entity and channel in the
  //time-based schedule
  void setIndecesTimedSchedule();

  // returns the first index of program entity id in the time-based static schedule of the transient phase
  int firstIndexTransPhaseEntity(int id) const;
  // returns the last index of program entity id in the time-based static schedule of the transient phase
  int lastIndexTransPhaseEntity(int id) const;

  // returns the first index of channel id in the time-based static schedule of the transient phase
  int firstIndexTransPhaseChannel(int id) const;
  // returns the last index of channel id in the time-based static schedule of the transient phase
  int lastIndexTransPhaseChannel(int id) const;

  // Determines the minimum number of processors needed to satisfy the IPT tasks
  int minProcs_IPT();

  // Generates an XML file with a description of the mapping
  void generateXML() const;

  // Print all parameters for MiniZinc model
  void generateDZNfile(const string &dir) const;

  void PrintWCETs() const; /*!< Printing the WCET table */

  void PrintUtilizations();  /*!< Printing the Utilization table */

  /*!< Returns the utilization of the pr_task
    @param entity id: note that this is NOT task id
    @param processor id
    @return utilization of the pr_task */ 
  double getTaskUtilization(int entityID, unsigned procID);

  /*!< Returns true if the taskset is schedulable given 
    (1) current mapping and (2) partitioned FP algorithm
    with rate monotonic priorities
    @return true if schedulable, otherwise false */ 
  bool FPSchedulable();

  /*!< Returns the processor id of the input entity id,
    returns -1 if did not find
    @param entity id */
  int getProcessorID(int entityID); 

  /*!< Returns the level-i workload given the current mapping
    @param entity id
    @param time
    @return level-i workload of task with matching entity id */ 
  int LeveliWorkload(int entityID, int t);

  /*!< @return Returns the maximum WCET of the periodic tasks */ 
  int getMaxWCET_IPT();

  void PrintMapping(); /*!< Prints the mapping for all actors and tasks */

  /*!< @param int entityID of pr task
    @return an array of integer utilizations */ 
  vector<int> getUtilizationVector(int entityID);

  /*!< Retruns a vector of utilization for given processor and entityID
    @param entityID has to be the id of pr task
    @param proc is the id of processor
    @return int vector of utilizations */
  vector<int> getUtilizationModeVector(int entityID, unsigned proc); 

  void setRMPriorities();  /*!< Set RM priorities for tasks */

  /*!< @return name of the app i */ 
  string getAppName(int i);

  /*!< calculates the minimum required utilization by periodic tasks */ 
  int getLeastTotalUtilization();

  /*!< calculates the minimum required power by periodic tasks
    TODO: It should be extended to consider sdf apps as well */ 
  int getLeastPowerConsumption();

  /*!< Finds a proc and a proc mode in which task uses min power
    @return the corresponding min power */
  int getLeastPowerForTask(int task); 

  /*!< Derives the minimum utilization for input SDF app
    In case throughput is not bounded it assumes full processor (may be optimistic) */ 
  int getLeastSDFUtil(int sdf);

  /*!< Sorts the taskset based on the utilization of tasks.
    It only considers the utilization of tasks on node 0 */
  void SortTasksUtilization(); 

  void setMappingMode(vector<int>& _mapping, vector<int>& _modes); /*!< sets the current mapping and modes */

  /*!< returns the memory size of proc based on the current mappings */ 
  int getMemorySize(unsigned proc);
                
        
};
#endif
