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
#ifndef __PLATFORM__
#define __PLATFORM__

#include<iostream>
#include <limits>
#include <vector>
#include <string>
#include "math.h"
#include <stdio.h>
#include <string.h>
#include "../xml/xmldoc.hpp"

#include "../exceptions/runtimeexception.h"

using namespace std;

enum InterconnectType { TDMA_BUS, TDN_NOC, NOC, UNASSIGNED };

/**
 * Trivial class to represent a processing element.
 */
class PE {
    
public:
  std::string name;
  std::string type;
  std::string model;
  int n_types;
  vector<double> cycle_length;
  vector<int> memorySize;
  vector<int> powerCons;
  vector<int> areaCost;
  vector<int> monetaryCost;
  int NI_bufferSize;

  PE() {};

  PE(std::string p_name, std::string p_type, vector<double> p_cycle,
     vector<int> p_memSize, vector<int> _power, vector<int> _area, 
     vector<int> _money, int p_buffer){
    name          = p_name; 
    type          = p_type; 
    n_types       = p_cycle.size();
    cycle_length  = p_cycle; 
    memorySize    = p_memSize; 
    powerCons     = _power;
    areaCost      = _area;
    monetaryCost  = _money;
    NI_bufferSize = p_buffer;
  }
   PE(std::string p_model, int id){
    n_types       = 0;
    name          = p_model + "_" + tools::toString(id); 
    model         = p_model;
  }  
  PE(vector<char*> elements, vector<char*> values, int number)
    : n_types(0),
      NI_bufferSize(0) {
    
    for(unsigned int i=0;i< elements.size();i++) {
      try {                     
        if(strcmp(elements[i], "NI_bufferSize") == 0)
          NI_bufferSize = atoi(values[i]);
                                        
        if(strcmp(elements[i], "n_types") == 0)
          n_types = atoi(values[i]);

        if(strcmp(elements[i], "type") == 0)
          type = string(values[i]);

        if(strcmp(elements[i], "model") == 0) {
          model = string(values[i]);
          name = string(values[i]) + to_string(number);
        }        
      }                             
      catch(std::exception const & e) {
        cout << "reading taskset xml file error : " << e.what() << endl;
      }
    }
  };
    
  void AddMode(vector<char*> elements, vector<char*> values)
  {
    /**
     * need to initialize in case some are not specified in the XML
     */          
    double _cycle_length        = std::numeric_limits<double>::max(); 
    int _powerCons                      = std::numeric_limits<int>::max() - 1; 
    int _areaCost                       = std::numeric_limits<int>::max() - 1; 
    int _monetaryCost           = std::numeric_limits<int>::max() - 1; 
    int _memorySize                     = 0;
                
    for(unsigned int i=0;i< elements.size();i++) {
      try {   
        if(strcmp(elements[i], "cycle_length") == 0) {
          _cycle_length = atof(values[i]);
          n_types++; ///each cycle_length is one operational mode
        }
                                
        if(strcmp(elements[i], "powerCons") == 0)
          _powerCons = atoi(values[i]);
                                
        if(strcmp(elements[i], "areaCost") == 0)
          _areaCost = atoi(values[i]);
                                
        if(strcmp(elements[i], "monetaryCost") == 0)
          _monetaryCost = atoi(values[i]);
                                        
        if(strcmp(elements[i], "memorySize") == 0)
          _memorySize = atoi(values[i]);    
      }                             
      catch(std::exception const & e) {
        cout << "reading taskset xml file error : " << e.what() << endl;
      }
    }
    AddMode(_cycle_length, _memorySize, _powerCons, _areaCost, _monetaryCost);
  };

  friend std::ostream& operator<< (std::ostream &out, const PE &pe) {
    out << "PE:" << pe.name << "[model=" << pe.model << "], no_types=" << pe.n_types << ", speeds(";
    for(unsigned int i=0;i<pe.cycle_length.size();i++)
      {
        i!=0 ? out << ", ": out << "" ;
        out << pe.cycle_length[i];
      }
    out << ")";
    return out;
  }

  void AddMode(double _cycle_length, int _memorySize, int _powerCons, int _areaCost, int _monetaryCost) {
      n_types++;///Increase the number of modes
    cycle_length.push_back(_cycle_length);
    memorySize.push_back(_memorySize);
    powerCons.push_back(_powerCons);
    areaCost.push_back(_areaCost);
    monetaryCost.push_back(_monetaryCost);
  }    
};


/**
 * Trivial class to represent the interconnect.
 */
class Interconnect {
  
public:
  enum InterconnectType type;
  int dataPerSlot;
  int dataPerRound;
  int tdmaSlots;
  int roundLength;
  int columns;
  int rows;
  int flitSize;
  int tdnCycles;
  int cycleLength;
  

  Interconnect() {
    type = UNASSIGNED;
  };

  Interconnect(InterconnectType p_type, int p_dps, int p_tdma, int p_roundLength, 
               int p_col, int p_row, int p_fs, int p_tdnC, int p_cl){
    type         = p_type;
    dataPerSlot  = p_dps;
    tdmaSlots    = p_tdma;
    roundLength  = p_roundLength;
    columns      = p_col;
    rows         = p_row;
    dataPerRound = dataPerSlot * tdmaSlots;
    flitSize = p_fs;
    tdnCycles = p_tdnC;
    cycleLength = p_cl;
  }
};


//! Struct to capture the links of the TDN NoC.
/*! Provides information on which link on the NoC, and at which TDN cycle. */
struct tdn_link{
  int from; /*!< Value -1: from NI to switch at NoC-node \ref tdn_link.to */
  int to; /*!< Value -1: from switch to NI at NoC-node \ref tdn_link.from */
  int cycle; /*!< TDN cycle */
};

//! Struct to capture a route through the TDN NoC.
/*! Combines the destination processor with a path of nodes in the TDN graph.
 * The \ref tnd_route.tdn_nodePath combines information about location with time (TND cycle). */
struct tdn_route{
  int dstProc; /*!< Id of the destination processor / NoC node. */
  vector<int> tdn_nodePath; /*!< The sequence of node Ids of the TDN-graph nodes, starting with the root node, ending with the node corresponding to \ref tdn_route.dstProc. */
};

//! Struct to capture a node in the TDN graph.
/*!  */
struct tdn_graphNode{
    set<int> passingProcs; /*!< All processors whose messages can pass this link. */
    tdn_link link; /*!< The link of the NoC that this node represents. */
    vector<shared_ptr<tdn_route>> tdn_routes; /*!< All routes that go through this node. */
};


/**
 * This class specifies a platform.
 * TODO: This could be specified in an XML file or something.
 */
class Platform {
  
protected:

  std::vector<PE*> compNodes;
  Interconnect interconnect;
  vector<tdn_graphNode> tdn_graph;
  
  void createTDNGraph() throw (InvalidArgumentException);

public:

  Platform() {};
  
  Platform(XMLdoc& doc) throw (InvalidArgumentException);

  Platform(size_t p_nodes, int p_cycle, size_t p_memSize, int p_buffer, enum InterconnectType p_type, int p_dps, int p_tdma, int p_roundLength);
  
  Platform(std::vector<PE*> p_nodes, InterconnectType p_type, int p_dps, int p_tdma, int p_roundLength);

  Platform(std::vector<PE*> p_nodes, InterconnectType p_type, int p_dps, int p_tdma, int p_roundLength, int p_col, int p_row);
  
  /**
   * Destructor.
   */
  ~Platform();

   void load_xml(XMLdoc& xml) throw (InvalidArgumentException);
   
  // Gives the number of nodes
  size_t nodes() const;
  
  // Gives the type of interconnect
  InterconnectType getInterconnectType() const;
  
  // Gives the TDN Graph / Table
  vector<tdn_graphNode> getTDNGraph() const;

  // Gives the number of tdma slots of the interconnect
  int tdmaSlots() const;

  // Gives the dataPerSlot of the interconnect
  int dataPerSlot() const;

  // Gives the dataPerRound of the interconnect
  int dataPerRound() const;

  // Gives the cycle-length of the specified node
  double speedUp(int node, int mode) const;

  // Gives the memory size of the specified node
  size_t memorySize(int node, int mode) const;
  
  size_t getModes(int node)const;
  
  size_t getMaxModes()const;
  
  vector<int> getMemorySize(int node)const;
  vector<int> getPowerCons(int node)const;
  vector<int> getAreaCost(int node)const;
  vector<int> getMonetaryCost(int node)const;

  // Gives the NI buffer size of the specified node
  int bufferSize(int node) const;

  //maximum communication time (=blocking+sending) on the TDM bus for a token of size tokSize
  //for different TDM slot allocations (index of vector = number of slots
  //for use with the element constraint in the model
  const vector<int> maxCommTimes(int tokSize) const;
  
  //maximum blocking time on the TDM bus for a token
  //for different TDM slot allocations (index of vector = number of slots
  //for use with the element constraint in the model
  const vector<int> maxBlockingTimes() const;
  
  //maximum transer time on the TDM bus for a token of size tokSize
  //for different TDM slot allocations (index of vector = number of slots
  //for use with the element constraint in the model
  const vector<int> maxTransferTimes(int tokSize) const;

  // True, if none of the procs has alternative modes
  bool isFixed() const;

  // True if the nodes are homogeneous
  bool homogeneousNodes(int node0, int node1) const;
  
  // True if the nodes with modes are homogeneous
  bool homogeneousModeNodes(int node0, int node1) const;
  
  // True if the platform is homogeneous
  bool homogeneous() const;
  
  bool allProcsFixed() const;
  
  string getProcModel(size_t id);
  
  friend std::ostream& operator<< (std::ostream &out, const Platform &p);
};
#endif
