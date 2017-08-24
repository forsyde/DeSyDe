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
  vector<int> dynPowerCons;
  vector<int> staticPowerCons;
  vector<int> areaCost;
  vector<int> monetaryCost;
  int NI_bufferSize;

  PE() {};

  PE(std::string p_name, std::string p_type, vector<double> p_cycle,
     vector<int> p_memSize, vector<int> _dynPower, vector<int> _staticPower, vector<int> _area, 
     vector<int> _money, int p_buffer){
    name          = p_name; 
    type          = p_type; 
    n_types       = p_cycle.size();
    cycle_length  = p_cycle; 
    memorySize    = p_memSize; 
    dynPowerCons     = _dynPower;
    staticPowerCons = _staticPower;
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
    int _dynPowerCons                      = std::numeric_limits<int>::max() - 1; 
    int _staticPowerCons                      = std::numeric_limits<int>::max() - 1; 
    int _areaCost                       = std::numeric_limits<int>::max() - 1; 
    int _monetaryCost           = std::numeric_limits<int>::max() - 1; 
    int _memorySize                     = 0;
                
    for(unsigned int i=0;i< elements.size();i++) {
      try {   
        if(strcmp(elements[i], "cycle_length") == 0) {
          _cycle_length = atof(values[i]);
          n_types++; ///each cycle_length is one operational mode
        }
                                
        if(strcmp(elements[i], "dynPowerCons") == 0)
          _dynPowerCons = atoi(values[i]);
          
        if(strcmp(elements[i], "staticPowerCons") == 0)
          _staticPowerCons = atoi(values[i]);
                                
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
    AddMode(_cycle_length, _memorySize, _dynPowerCons, _staticPowerCons, _areaCost, _monetaryCost);
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

  void AddMode(double _cycle_length, int _memorySize, int _dynPowerCons, int _staticPowerCons, int _areaCost, int _monetaryCost) {
      n_types++;///Increase the number of modes
    cycle_length.push_back(_cycle_length);
    memorySize.push_back(_memorySize);
    dynPowerCons.push_back(_dynPowerCons);
    staticPowerCons.push_back(_staticPowerCons);
    areaCost.push_back(_areaCost);
    monetaryCost.push_back(_monetaryCost);
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
  int srcProc;
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
 * Trivial struct to represent the modes of the Interconnect.
 */
struct InterconnectMode {
  string name;
  int cycleLength;
  int dynPower_link;
  int dynPower_NI;
  int dynPower_switch;
  int staticPow_link;
  int staticPow_NI;
  int staticPow_switch;
  int area_link;
  int area_NI;
  int area_switch;
  int monetary_link;
  int monetary_NI;
  int monetary_switch;
  
};


/**
 * Trivial class to represent the interconnect.
 */
class Interconnect {
  
public:
  enum InterconnectType type;
  string name;
  int dataPerSlot;
  int dataPerRound;
  int tdmaSlots;
  int roundLength;
  int columns;
  int rows;
  int flitSize;
  int tdnCycles;
  vector<InterconnectMode> modes;
  vector<tdn_route> all_routes; //all routes, without time (TDN cycles) - unlink in the tdn graph
  

  Interconnect() {
    type = UNASSIGNED;
  };

  Interconnect(InterconnectType p_type, string p_name, int p_dps, int p_tdma, int p_roundLength, 
               int p_col, int p_row, int p_fs, int p_tdnC){
    type         = p_type;
    name         = p_name;
    dataPerSlot  = p_dps;
    tdmaSlots    = p_tdma;
    roundLength  = p_roundLength;
    columns      = p_col;
    rows         = p_row;
    dataPerRound = dataPerSlot * tdmaSlots;
    flitSize     = p_fs;
    tdnCycles    = p_tdnC;
  }
  
  void addMode(string _name,
               int _cycleLength,
               int _dynPower_link,
               int _dynPower_NI,
               int _dynPower_switch,
               int _staticPow_link,
               int _staticPow_NI,
               int _staticPow_switch,
               int _area_link,
               int _area_NI,
               int _area_switch,
               int _monetary_link,
               int _monetary_NI,
               int _monetary_switch){
    modes.push_back(InterconnectMode{_name, _cycleLength, 
                      _dynPower_link, _dynPower_NI, _dynPower_switch,
                      _staticPow_link, _staticPow_NI, _staticPow_switch, 
                      _area_link, _area_NI, _area_switch, 
                      _monetary_link, _monetary_NI, _monetary_switch});
  }
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
  void createRouteTable();

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
  
  int getTDNCycles() const;
  
  size_t getInterconnectModes() const;
  
  int getTDNCycleLength() const;
  
  /*! Gets the cycle length, depending on the NoC mode. */
  vector<int> getTDNCycleLengths() const;
  
  /*! Gets the dynamic power consumption of a link for each mode. */
  vector<int> getDynPowerCons_link() const;
  
  /*! Gets the dynamic power consumption of an NI for each mode. */
  vector<int> getDynPowerCons_NI() const;
  
  /*! Gets the dynamic power consumption of a switch for each mode. */
  vector<int> getDynPowerCons_switch() const;
  
  /*! Gets the static power consumption of the entire NoC for each mode. */
  vector<int> getStaticPowerCons() const;
  
  /*! Gets the dynamic power consumption of the link at node node for each mode. */
  vector<int> getStaticPowerCons_link(size_t node) const;
  
  /*! Gets the dynamic power consumption of a NI for each mode. */
  vector<int> getStaticPowerCons_NI() const;
  
  /*! Gets the dynamic power consumption of a switch for each mode. */
  vector<int> getStaticPowerCons_switch() const;
  
  /*! Gets the area cost of the NoC, depending on the mode. */
  vector<int> interconnectAreaCost() const;
  
  /*! Gets the area cost of the links at node node for each mode. */
  vector<int> interconnectAreaCost_link(size_t node) const;
  
  /*! Gets the area cost of an NI for each mode. */
  vector<int> interconnectAreaCost_NI() const;
  
  /*! Gets the area cost of a switch for each mode. */
  vector<int> interconnectAreaCost_switch() const;
  
  /*! Gets the monetary cost of the NoC, depending on the mode. */
  vector<int> interconnectMonetaryCost() const;
  
  /*! Gets the monetary cost of the links at node node for each mode. */
  vector<int> interconnectMonetaryCost_link(size_t node) const;
  
  /*! Gets the monetary cost of a NI for each mode. */
  vector<int> interconnectMonetaryCost_NI() const;
  
  /*! Gets the monetary cost of a switch for each mode. */
  vector<int> interconnectMonetaryCost_switch() const;
  
  int getMaxNoCHops() const;
  
  int getFlitSize() const;

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
