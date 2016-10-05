/**
 * Copyright (c) 2013-2016, Kathrin Rosvall  <krosvall@kth.se>
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
#ifndef __FLEXPERIODICTASKS__
#define __FLEXPERIODICTASKS__

#include <math.h>
#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <boost/math/common_factor.hpp>

using namespace std;

class FlexPeriodicTask{
public:
  int phase;
  vector<int> period;
  vector<int> wcec;
  int deadline;
  vector<int> memCons;
  int codeSize;
  bool preemtable;

  FlexPeriodicTask(int _phase, vector<int> _period, vector<int> _wcec, int _deadline, vector<int> _mem, int _cS):
    phase(_phase),
    period(_period),
    wcec(_wcec),
    deadline(_deadline),
    memCons(_mem),
    codeSize(_cS),
    preemtable(true)  {
      sort(period.begin(), period.end());  
    };

  FlexPeriodicTask(vector<int> _period, vector<int> _wcec, vector<int> _mem, int _cS):
    phase(0),
    period(_period),
    wcec(_wcec),
    deadline(-1), //TODO: fix this in a meaningful way
    memCons(_mem),
    codeSize(_cS),
    preemtable(true)  {
      sort(period.begin(), period.end());  
    };

};

class FlexPeriodicTasks {

protected:
  vector<FlexPeriodicTask> tasks;

public:
  FlexPeriodicTasks();
  FlexPeriodicTasks(vector<FlexPeriodicTask> _tasks);

  int getNumberOfTasks();
  vector<int> getWCEC(int id); //processor dependend
  int getMinWCEC(int id);
  int getMaxWCEC(int id);
  int getWCEC(int id, int node);
  int getPhase(int id);
  vector<int> getPeriod(int id);
  vector<vector<int>> getHyperperiods();
  vector<vector<vector<int>>> getHyperperiodsPerTask();
  int getNumberOfPeriodCombinations();
  int getDeadline(int id);
  vector<int> getMemCons(int id); //processor dependend
  int getCodeSize(int id);
  bool isPreemtable(int id);
  
  int getMaxCodeSize();
  //int getMaxHyperperiod();
  int getMinWCEC();
  int getMaxWCEC();
  //int getMinPeriod();
  //int getMaxPeriod();
  vector<vector<vector<int>>> getInstancesPerTask();
  vector<vector<int>> getMaxInstancesPerPeriod();
  vector<int> getMaxNumberOfInstances();

  int areHomogeneous(int nodeI, int nodeJ);

};
#endif
