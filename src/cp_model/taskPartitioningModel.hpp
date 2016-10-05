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
/* - This class includes the CP model corresponding to ONLY periodic tasks */

#include <math.h>
#include <vector>

#include <gecode/int.hh>
#include <gecode/set.hh>
#include <gecode/gist.hh>
#include <gecode/minimodel.hh>

#include "../applications/applications.hpp"
#include "../platform/platform.hpp"
#include "../system/mapping.hpp"
#include "../systemDesign/designDecisions.hpp"
#include "schedulability.hpp"

using namespace Gecode;

/**
 * Gecode space containing the scheduling model based on the paper.
 */
class taskPartitioningModel : public Space {
  
private:
  Applications* apps;
  Platform* platform;
  Mapping* mapping;
  DesignDecisions* desDec;
  
  //DECISION VARIABLES
  //mapping of firings onto processors
  IntVarArray proc;
  //for "flexible" procs with modes: economy, regular and performance
  IntVarArray proc_mode;
  
  
  //number of processors used in mapping
  IntVar procsUsed;
  //utilization of processing nodes
  IntVarArray utilization;
  //utilization of all procs
  IntVar sys_utilization;
  //utilization of all used procs
  IntVar procsUsed_utilization;
  
  //long-run consumption of each proc
  IntVarArray proc_power;
  //long-run consumption of system
  IntVar sys_power;
  //area cost of each proc
  IntVarArray proc_area;
  //area cost of all procs combined
  IntVar sys_area;
  //monetary cost of each proc
  IntVarArray proc_cost;
  //monetary cost of all procs combined
  IntVar sys_cost;

  
public:

  taskPartitioningModel(Mapping* p_mapping);
  
  taskPartitioningModel(bool share, taskPartitioningModel& s);
  
  // Copies the space
  virtual Space* copy(bool share);
  
  // Prints the variables
  void print(std::ostream& out) const;

  virtual void constrain(const Space& _b)
  {
    const taskPartitioningModel& b = static_cast<const taskPartitioningModel&>(_b);
    
    vector<int> optApps;
    for(int a=0; a<apps->n_SDFApps(); a++){
      if(apps->getConstrType(a) == FIRSTOPT){
        optApps.push_back(a);
      }
    }
    if(optApps.size()>0)
    {
      {
        IntVar new_sys_power(sys_power);
        int cur_sys_power = b.sys_power.min();
        if(mapping->getFirstMapping().size() == 0)
          rel(*this, (new_sys_power < cur_sys_power));
      }
    }
    else
    {
      IntVar new_sys_power(sys_power);
      int cur_sys_power = b.sys_power.min();
      rel(*this, new_sys_power < cur_sys_power);
    }
  }
  
  vector<int> getPeriodResults();
  
  
};

