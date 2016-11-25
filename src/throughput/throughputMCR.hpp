/**
 * Copyright (c) 2013-2016, Kathrin Rosvall  <krosvall@kth.se>
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
#ifndef __THROUGHPUTMCR__
#define __THROUGHPUTMCR__


#include "../tools/tools.hpp"

#include <gecode/int.hh>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <chrono>
#include <sstream>
#include <fstream>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/howard_cycle_ratio.hpp>


using namespace Gecode;
using namespace Int;
using namespace std;
//using namespace boost;
namespace b = boost;

//! alias for actor ID property (check BGL documentation)
//<http://www.boost.org/doc/libs/1_53_0/libs/graph/doc/using_adjacency_list.html#sec:adjacency-list-properties>
enum vertex_actorid_t { vertex_actorid };
namespace boost {
  BOOST_INSTALL_PROPERTY(vertex, actorid);
}

using actor_prop = b::property<vertex_actorid_t, int>;
using chan_prop  = b::property<b::edge_weight_t, int, b::property<b::edge_weight2_t, int> >;
using boost_msag = b::adjacency_list<b::vecS, b::vecS, b::directedS, actor_prop, chan_prop>;




class ThroughputMCR : public Propagator {
private: 
  class SuccessorNode{
    public:
      int successor_key;
      int min_tok;
      int max_tok;
      int delay;
      int channel; //in case of sending/receiving node
      int recOrder; //in case of receiving node
      int destination; //in case of sending node

      SuccessorNode():successor_key(-1){};

  };
protected:
  ViewArray<IntView> latency; //resulting initial latency
  ViewArray<IntView> period; //resulting period
  ViewArray<IntView> iterations; //resulting min/max iterations for actors
  ViewArray<IntView> iterationsCh; //resulting min/max iterations for channels
  ViewArray<IntView> sendbufferSz; //resulting min/max buffer sizes in items (send)
  ViewArray<IntView> recbufferSz; //resulting min/max buffer sizes in items (receive) 
  ViewArray<IntView> next; //current schedule
  ViewArray<IntView> wcet; //current possible execution times of actors
  ViewArray<IntView> sendingTime; //current sending time for channels
  ViewArray<IntView> sendingLatency; //current sending latency for channels
  ViewArray<IntView> sendingNext; //current sending schedule for channels
  ViewArray<IntView> receivingTime; //current receiving time for channels (atm 0)
  ViewArray<IntView> receivingNext; //current receiving schedule (Note: sending schedule is included/respected in sending latency)
  IntArgs ch_src; //source actors for all channels 
  IntArgs ch_dst; //destination actors for all channels
  IntArgs tok; //initial token distribution of application graph
  IntArgs apps; //apps[i] is index of last actor of application i
  IntArgs minIndices; //for each entity and channel, its first index in the time-based schedule
  IntArgs maxIndices; //for each entity and channel, its last index in the time-based schedule

  int n_actors; //number of actors in the graph
  int n_channels; //number of channels in the graph
  int n_procs;
  int n_msagActors; //number of actors in msag
  int n_msagChannels; //number of channels
  int channel_count; //number of messages on interconnect
  
  //for construction of the mapping and scheduling aware graph
  unordered_map<int,vector<SuccessorNode>> msaGraph;
  //MSAG representation for boost
  boost_msag b_msag;
  //MSAG representation for boost
  vector<boost_msag*> b_msags;
  //for mapping from msag send/rec actors to appG-channels
  vector<int> channelMapping;
  //receivingActors: for storing/finding the first receiving actor for each dst
  vector<int> receivingActors;

  //MCR results
  vector<vector<int>> wc_latency; 
  vector<int> wc_period;
  vector<vector<int>> max_start; //self-timed schedule generated by MCR
  vector<vector<int>> max_end; //self-timed schedule generated by MCR
  vector<vector<int>> min_start; //minimal schedule with same latency & period
  vector<vector<int>> min_end; //minimal schedule with same latency & period
  vector<int> start_pp; //start times for periodic phase
  vector<int> end_pp; //end times for periodic phase
  vector<int> min_iterations; //min iterations of actors for wc latency and period
  vector<int> max_iterations; //max iterations of actors for wc latency and period
  vector<int> min_send_buffer; //min buffer size of all appG-channels
  vector<int> max_send_buffer; //max buffer size of all appG-channels
  vector<int> min_rec_buffer; //min buffer size of all appG-channels
  vector<int> max_rec_buffer; //max buffer size of all appG-channels

  //for evaluation purposes
  bool printDebug;
  
  //builds the msaGraph based on the current state of the solution
  void constructMSAG();
  //builds the msaGraph based on the current state of the solution
  //the coMapped vector specifies for each application, which MSAG it is part of
  void constructMSAG(vector<int> &msagMap);
  int getBlockActor(int ch_id) const;
  int getSendActor(int ch_id) const;
  int getRecActor(int ch_id) const;
  int getApp(int msagActor_id) const;
  void printThroughputGraph() const;
  void printThroughputGraphAsDot(const string &dir) const;


public:
ThroughputMCR(Space& home, ViewArray<IntView> p_latency,
                         ViewArray<IntView> p_period,  
                         ViewArray<IntView> p_iterations, 
                         ViewArray<IntView> p_iterationsCh,  
                         ViewArray<IntView> p_sendbufferSz, 
                         ViewArray<IntView> p_recbufferSz,  
                         ViewArray<IntView> p_next,  
                         ViewArray<IntView> p_wcet, 
                         ViewArray<IntView> p_sendingTime, 
                         ViewArray<IntView> p_sendingLatency, 
                         ViewArray<IntView> p_sendingNext, 
                         ViewArray<IntView> p_receivingTime,  
                         ViewArray<IntView> p_receivingNext,
                         IntArgs p_ch_src, 
                         IntArgs p_ch_dst,
                         IntArgs p_tok, 
                         IntArgs p_apps, 
                         IntArgs p_minIndices, 
                         IntArgs p_maxIndices);

static ExecStatus post(Space& home, ViewArray<IntView> p_latency,
                       ViewArray<IntView> p_period,  
                       ViewArray<IntView> p_iterations, 
                       ViewArray<IntView> p_iterationsCh,  
                       ViewArray<IntView> p_sendbufferSz, 
                       ViewArray<IntView> p_recbufferSz,  
                       ViewArray<IntView> p_next,  
                       ViewArray<IntView> p_wcet, 
                       ViewArray<IntView> p_sendingTime, 
                       ViewArray<IntView> p_sendingLatency, 
                       ViewArray<IntView> p_sendingNext, 
                       ViewArray<IntView> p_receivingTime,  
                       ViewArray<IntView> p_receivingNext,
                       IntArgs p_ch_src, 
                       IntArgs p_ch_dst,
                       IntArgs p_tok, 
                       IntArgs p_apps, 
                       IntArgs p_minIndices, 
                       IntArgs p_maxIndices){
  (void) new (home) ThroughputMCR(home, p_latency, p_period, p_iterations, p_iterationsCh, 
                                  p_sendbufferSz, p_recbufferSz, p_next,
                                  p_wcet, p_sendingTime, p_sendingLatency, p_sendingNext,
                                  p_receivingTime, p_receivingNext,p_ch_src, p_ch_dst,
                                  p_tok, p_apps, p_minIndices, p_maxIndices);
  return ES_OK;
}

virtual size_t dispose(Space& home);

ThroughputMCR(Space& home, bool share, ThroughputMCR& p);

virtual Propagator* copy(Space& home, bool share);

//TODO: do this right
virtual PropCost cost(const Space& home, const ModEventDelta& med) const;

virtual ExecStatus propagate(Space& home, const ModEventDelta&);

};


//throughput constraint for one app without propagation on time-based schedule
extern void throughputMCR(Space& home, const IntVar latency,
                             const IntVar period,
                             const IntVarArgs& iterations, //min/max
                             const IntVarArgs& iterationsCh, //min/max
                             const IntVarArgs& sendbufferSz,
                             const IntVarArgs& recbufferSz, 
                             const IntVarArgs& next,   
                             const IntVarArgs& wcet,
                             const IntVarArgs& sendingTime,
                             const IntVarArgs& sendingLatency,
                             const IntVarArgs& sendingNext,
                             const IntVarArgs& receivingTime,
                             const IntVarArgs& receivingNext,
                             const IntArgs& ch_src,
                             const IntArgs& ch_dst, 
                             const IntArgs& tok);
                             
//throughput constraint for multiple apps without propagation on time-based schedule
extern void throughputMCR(Space& home, const IntVarArgs& latency,
                             const IntVarArgs& period,
                             const IntVarArgs& iterations, //min/max
                             const IntVarArgs& iterationsCh, //min/max
                             const IntVarArgs& sendbufferSz,
                             const IntVarArgs& recbufferSz, 
                             const IntVarArgs& next,   
                             const IntVarArgs& wcet,
                             const IntVarArgs& sendingTime,
                             const IntVarArgs& sendingLatency,
                             const IntVarArgs& sendingNext,
                             const IntVarArgs& receivingTime,
                             const IntVarArgs& receivingNext,
                             const IntArgs& ch_src,
                             const IntArgs& ch_dst, 
                             const IntArgs& tok, 
                             const IntArgs& apps);
#endif

