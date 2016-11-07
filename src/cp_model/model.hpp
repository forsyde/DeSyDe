#ifndef __MODEL__
#define __MODEL__

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
#include "../throughput/throughputSSE.hpp"
#include "../throughput/throughputMCR.hpp"

using namespace Gecode;

/**
 * Gecode space containing the scheduling model based on the paper.
 */
class ADSEModel : public Space {
  
private:
  Applications* apps;
  Platform* platform;
  Mapping* mapping;
  Config* cfg;
  DesignDecisions* desDec;
  
  //DECISION VARIABLES
  //static schedule of firings
  IntVarArray next;
  //mapping of firings onto processors
  IntVarArray proc;
  //for "flexible" procs with modes: economy, regular and performance
  IntVarArray proc_mode;
  //allocation of TDMA slots to processors
  IntVarArray tdmaAlloc;
  
  //needed for prediction of communication time
  //a schedule for sent messages on interconnect
  IntVarArray sendNext;
  //a schedule for sent messages on interconnect
  IntVarArray recNext;
  //receiving buffers size
  IntVarArray recbufferSz;
  
  //for independent periodic tasks
  IntVarArray hyperPeriod;
  IntVarArray start;
  IntVarArray end;
  //for preemtable tasks
  SetVarArray exec;
  
  //SECONDARY VARIABLES
  //period of all applications (inverse of throughput)
  IntVarArray period;
  IntVarArray proc_period;
  //initial latency of all applications
  IntVarArray latency;
  
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

  //communication delay, block (pre-send-wait)
  IntVarArray wcct_b;
  //communication delay, send
  IntVarArray wcct_s;
  //coummunication delay, receive
  IntVarArray wcct_r;
  
  
public:

  ADSEModel(Mapping* p_mapping, Config* _cfg);
  
  ADSEModel(bool share, ADSEModel& s);
  
  // Copies the space
  virtual Space* copy(bool share);
  
  // Prints the variables
  void print(std::ostream& out) const;

  virtual void constrain(const Space& _b){
    const ADSEModel& b = static_cast<const ADSEModel&>(_b);
    
    vector<int> optApps;
    for(size_t a=0; a<apps->n_SDFApps(); a++){
      if(apps->getConstrType(a) == FIRSTOPT){
        optApps.push_back(a);
      }
    }
    if(optApps.size()>0){
      IntVarArgs newPeriods;
      int sumPeriods = 0;
      for(size_t a=0; a<optApps.size(); a++){
        newPeriods << period[optApps[a]];
        sumPeriods += b.period[optApps[a]].min();
      }
      
      if(platform->homogeneous()){
        rel(*this, sum(newPeriods) < sumPeriods);
      }else{
        IntVar new_sys_power(sys_power);
        int cur_sys_power = b.sys_power.min();
        if(mapping->getFirstMapping().size() == 0)
          rel(*this, (sum(newPeriods) < sumPeriods) ||
                     (sum(newPeriods) == sumPeriods && 
                      (new_sys_power < cur_sys_power)));
        else
          rel(*this, sum(newPeriods) < sumPeriods);
      }
    }else{
      IntVar new_sys_power(sys_power);
      int cur_sys_power = b.sys_power.min();
      rel(*this, new_sys_power < cur_sys_power);
    }
  }
  
  vector<int> getPeriodResults();
  
  
};

#endif
