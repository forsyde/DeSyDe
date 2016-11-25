#ifndef __SDF_PR_ONLINE_MODEL__
#define __SDF_PR_ONLINE_MODEL__

/**
 * Copyright (c) 2013-2016, Nima Khalilzad   <nkhal@kth.se>
 *                             Kathrin Rosvall  <krosvall@kth.se>
 *                          
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
#include "schedulability.hpp"
#include "../settings/dse_settings.hpp"

using namespace Gecode;

/**
 * Gecode space containing the scheduling model based on the paper.
 */
class SDFPROnlineModel : public Space 
{
  
private:
    Applications*             apps;         /**< Pointer to the application object. */  
    Platform*                 platform;    /**< Pointer to the platform object. */
    Mapping*                 mapping;    /**< Pointer to the mapping object. */
    DesignDecisions*         desDec;        /**< Pointer to the design decition object. */
    //DSESettings*             settings;    /**< Pointer to the setting object. */
    Config*                  cfg;    /**< Pointer to the config object. */

    IntVarArray             next;        /**< static schedule of firings. */
    IntVarArray             rank;
    IntVarArray             proc;        /**< mapping of firings onto processors. */
    IntVarArray             proc_mode;    /**< processor modes: economy, regular and performance. */
    IntVarArray                tdmaAlloc;    /**< allocation of TDMA slots to processors.__PRESOLVER__ */

    IntVarArray             sendNext;    /**< a schedule for sent messages on interconnect. */
    IntVarArray             recNext;    /**< a schedule for sent messages on interconnect. */
    IntVarArray             sendbufferSz;/**< receiving buffers size. */
    IntVarArray             recbufferSz;/**< receiving buffers size. */


    /**
    * SECONDARY VARIABLES
    */   
    IntVarArray             period;                    /**< period of all applications (inverse of throughput). */
    IntVarArray             proc_period;            /**< period of the processors. */
    IntVarArray             latency;                /**< initial latency of all applications. */
    IntVar                     procsUsed;                /**< number of processors used in mapping. */
    IntVarArray             utilization;            /**< utilization of processing nodes. */
    IntVarArray             proc_util_sum;            /**< sum utilization of processing nodes. */
    IntVar                     sys_utilization;        /**< utilization of all procs. */
    IntVar                     procsUsed_utilization;    /**< utilization of all used procs */
    IntVarArray             proc_power;                /**< long-run consumption of each proc. */
    IntVar                     sys_power;                /**< long-run consumption of system. */
    IntVarArray             proc_area;                /**< area cost of each proc. */
    IntVar                     sys_area;                /**< area cost of all procs combined. */
    IntVarArray             proc_cost;                /**< monetary cost of each proc. */
    IntVar                     sys_cost;                /**< monetary cost of all procs combined. */
    IntVarArray             wcct_b;                    /**< communication delay, block (pre-send-wait). */
    IntVarArray             wcct_s;                    /**< communication delay, send. */
    IntVarArray             wcct_r;                    /**< coummunication delay, receive */
    

    int                        least_power_est;        /**< estimated least power consumption. */
  
public:

    SDFPROnlineModel(Mapping* p_mapping, Config* _cfg);

    SDFPROnlineModel(bool share, SDFPROnlineModel& s);

    virtual Space* copy(bool share);
    
    /**
     * This function is used for creating a csv file for the MOST tool,
     * it is copied from the old DSE source code
     */ 
    Mapping* extractResult();
  
  /**
   * This function extracts the order-based schedules of actors for all processing
   * nodes. The actors are identified by their integer id.
   */
  std::vector<std::vector<int>> extractSchedules() const;

  /**
   * This function extracts the order-based schedules of channels from all processing
   * nodes. The actors are identified by their integer id.
   */
  std::vector<std::vector<int>> extractCommSchedules() const;
    
    /**
    * Prints the variables
    */ 
    void print(std::ostream& out) const;
    /**
    * Prints variables for csv output
    */ 
    void printCSV(std::ostream& out) const;
    /**
    * Prints mappings and modes for csv output
    */ 
    void printMappingCSV(std::ostream& out) const;
    /**
     * function for imposing new constraints when using branch-and-bound
     */ 
    virtual void constrain(const Space& _b)
    {
        const SDFPROnlineModel& b = static_cast<const SDFPROnlineModel&>(_b);

        switch(cfg->settings().criteria[0]) //creates the model based on the first criterion
        {
            case(Config::POWER):
                rel(*this, sys_power < b.sys_power);
                break;
            case(Config::THROUGHPUT):
                for(size_t i=0;i<apps->n_SDFApps();i++)
                {
                    if(apps->getPeriodConstraint(i) == -1)
                    cout << "BAB app: " << i << "period=" << b.period[i] << endl;
                        {    rel(*this, period[i] < b.period[i]);break;}
                }
                break;
            case(Config::LATENCY):
                for(size_t i=0;i<apps->n_SDFApps();i++)
                {
                    if(apps->getLatencyConstraint(i) == -1)
                        {rel(*this, latency[i] < b.latency[i]); break;} 
                }                
                break;
            default:
                cout << "unknown optimization criterion !!!\n";
                throw 42;
                break;
        }   
    }
  
    vector<int> getPeriodResults();
    
    /**
    * Returns the processor number which task i has to be allocated.
    */ 
    static int valueProc(const Space& home, IntVar x, int i);
    typedef int (*IntBranchVal)(const Space& home, IntVar x, int i);    
};

#endif
