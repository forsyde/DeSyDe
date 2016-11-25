/**
 * Copyright (c) 2013-2016, Nima Khalilzad   <nkhal@kth.se> *                             
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
#include "pr_task.hpp"
#include "../xml/xmldoc.hpp"

using namespace std;

class TaskSet {

protected:
    vector<PeriodicTask*> tasks;
    clock_t schedulabilityTime;
    /**
     * Loads the taskset from an input xml file
     */
     void load_xml(XMLdoc& xml);

public:
    enum SchedulingAlgorith {FP, EDF};
    TaskSet();
    ~TaskSet();
    TaskSet(vector<PeriodicTask*> _tasks);
    TaskSet(XMLdoc& doc);
    SchedulingAlgorith Scheduler;

    int getNumberOfTasks();
    int getPhase(int id);
    int getPeriod(int id);
    vector<int> getHyperperiods();
    int getDeadline(int id);
    int getMemCons(int id); 
    int getCodeSize(int id);
    bool isPreemtable(int id);

    int getMaxCodeSize();
    int getMaxHyperperiod();
    int getMinPeriod();
    int getMaxPeriod();
    int getMaxNumberOfInstances();
    string getTaskType(int id);
    /**
     * Returns the pointer to the periodic task in taskset with the matching id.
     * @param id of task
     * @return periodic task
     */ 
    PeriodicTask* getTask(int id);

    /**
    * Sort tasks based on their periods
    */       
    void sortTasksPeriod(); 
    /**
    * Compares two tasks based on their periods
    */ 
    bool comparePeriods(PeriodicTask* lhs, PeriodicTask* rhs);     
    /**
    * Prints the taskset
    */ 
    friend std::ostream& operator<< (std::ostream &out, const TaskSet &taskset);
    /**
    * Set periods based on the rate monotonic algorithm
    */
    void SetRMPriorities(); 
    
    void PrintSchedulabilityElapsedTime();
    /**
     * Swaps task_i with task_j
     */ 
    void SwapTasks(int i, int j);       
    
};
