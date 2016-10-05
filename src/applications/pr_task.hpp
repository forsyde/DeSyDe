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
#include <math.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <boost/math/common_factor.hpp>

using namespace std;

class PeriodicTask{
public:
    int phase;
    int period;
    int deadline;
    int codeSize;
    int priority;
    string name; /*!< Name of the task. */
    string type; /*!< Type of the task. */
    int id;      /*!< Id of the task. */
    
    int memCons;

    bool preemtable;


    PeriodicTask(int _phase, int _period, int _deadline, int _mem, int _cS, int _priority, string _name, string _type, int _id);

    PeriodicTask(int _period, int _mem, int _cS, string _name, string _type, int _id);
    
    /**
     * constructs a periodic task using vectors of elements and values
     * to be used for reading tasks from xml files
     */ 
    PeriodicTask(vector<char*> elements, vector<char*> values, int number);

    friend std::ostream& operator<< (std::ostream &out, const PeriodicTask &task);
    
};
