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
#ifndef DESYDE_SDFGRAPH__
#define DESYDE_SDFGRAPH__


#include <unordered_map>

#include <boost/graph/transitive_closure.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/rational.hpp>


#include "../xml/xmldoc.hpp"
#include "../platform/platform.hpp"

using namespace std;
using boost::rational;


/**
 * Trivial class to encapsulate an actor.
 */
class SDFActor { //TODO! protected member and friends
public:
  size_t id;          /*!< Id of the actor. */
  string name;        /*!< Name of the actor. */
  size_t parent_id;   /*!< Id of the actor in the SDF from which this actor was generated. */
  string parent_name; /*!< Name of the actor in the SDF from which this actor was generated. */
  size_t codeSize;    /*!< Code size of the actor. */
  size_t dataSize;    /*!< Data size of the actor. */
};

/**
 * Trivial class to encapsulate a channel.
 */
class SDFChannel {
public:
  size_t id;          /*!< Id of the channel. */
  string name;        /*!< Name of the channel. */
  int source;         /*!< Id of source actor of the channel. */
  string src_name;    /*!< Name of source actor of the channel. */
  int prod;           /*!< Production rate of source actor onto the channel. */
  int destination;    /*!< Id of destination actor of the channel. */
  string dst_name;    /*!< Name of destination actor of the channel. */
  int cons;           /*!< Consumption rate of destination actor from the channel. */
  int initTokens;     /*!< Number of initial tokens on the channel. */
  size_t tokenSize;   /*!< Size of one token on the channel. */
  size_t messageSize; /*!< Total message size of this channel (prod*tokenSize). */
  int cost;           /*!< Cost of the channel in a specific mapping. */
  vector<int> oldIds; /*!< List of ids of channels in the input graph,
                         that the channel originated from. */
};

/**
 * Trivial class to combine number of initial tokens with the existence of a path.
 */
class simplePath {
public:
  bool exists;     /*!< Existence of the path. */
  bool initTokens; /*!< Numbers of initial tokens on the path. */
  simplePath():exists(0),initTokens(0) {};
};


/**
 * This class represents the input SDF graph.
 * It transforms the input graph in SDF3-format into the internal,
 * parallel format and provides functions for the CP model.
 * In order to allow for fairly easy exchange of the input format,
 * only the 3 transform* functions rely on the SDF3 library. Hence,
 * in order to add a new input format, a new constructor, transform,
 * transformFromSDF and transformFromHSDF need to be added.
 */
class SDFGraph {
  
protected:

  /// Intermediate container for dictionaries used during the
  /// construction of a SDFGraph object. Destroyed once the object
  /// is constructed.
  struct dictionaries {
    map<string,map<string,int>> rate;     //!< \c rate[actor][port] dictionary
    map<string,map<string,string>> channel;  //!< \c channel[actor][port] dictionary
    map<string, int>            actor_id; //!< \c id[actor] dictionary
    map<string, int>            actor_sz; //!< \c state_size[actor] dictionary
    map<string, int>            chan_sz;  //!< \c size[channel] dictionary
    map<string, int>            init_tok; //!< \c initial_tokens[channel] dictionary
    map<string, vector<string>> chan_con; //!< \c connections[channel]={src_act,src_prt,dst_act,dst_prt} dictionary
    dictionaries() {};
    ~dictionaries() {};
  };

  dictionaries* _d;

  XMLdoc& xml;
  
  //! Name of the graph.
  string graphName;

  //! Vector with all actors of the graph.
  vector<SDFActor*> actors;

  //! Vector with all channels of the graph.
  vector<SDFChannel*> channels;

  //! Number of "parent actors" (actors of the SDF, as opposed to
  //! HSDF).
  size_t parentActors;
  
  //! Matrix containing all existing paths in the graph
  //! (i.e. transitive closure of the graph).
  vector<simplePath> pathMatrix;
  
  int period_constraint;  /**< max period constraint for the sdf graph. */
  int latency_constraint; /**< max latency constraint for the sdf graph. */

  void buildDictionaries();

  void calculateRepetitionVector(vector<rational<int>>& firing, string a);

  /**
   * The transform function takes an input application graph g
   * and transforms it into the internal graph format (HSDF without
   * parallel edges). It first identifies the type of graph in the 
   * XML file (SDF or HSDF) and then initiates the transformation.
   */
  void transform();
  
  /**
   * The transformFromSDF function creates the internal SDF graph by
   * transforming it into a graph with as many copies of each actor
   * as given by the repetition vector (i.e. just as in an HSDF) and 
   * the minimal amount of channels giving the precedence constraints
   * from the input graph. The resulting channels have the same number
   * of produced and consumed tokens, not necessarily - but possibly -
   * equal to one (as in HSDF).
   */
  void transformFromSDF(const std::vector<int>& rp);
  
  /**
   * The transformFromHSDF function creates the internal SDF graph by
   * copying all actors from the input graph and combining possible
   * parallel channels (i.e. same source and destination actor) into 
   * one channel. If the channels have the same token size, the token 
   * rates are increased by 1. Otherwise the token size is adjusted
   * by adding the two sizes.
   */
  void transformFromHSDF();
  
  /** 
   * Creates a matrix (in form of a vector) with all existing paths in the graph
   * (transitive closure of the graph)
   */
  void createPathMatrix();
  
  /** 
   * Prints the path matrix on the screen, for debugging purposes.
   * The output means:
   * - "-" (dash): no path from actor with id #row to actor with id #column exists
   * - a number: a path from actor with id #row to actor with id #column exits and
   *             the number of initial tokens along the path corresponds to the printed number
   */
  std::string printPathMatrix() const;

  
public:

  /**
   * Constructor. Creates an SDFGraph from the SDF3 representation of the graph.
   */
  SDFGraph(XMLdoc& doc);
  
  /**
   * Constructor. Creates an SDFGraph for TDN config of the platform model.
   */
  SDFGraph(Platform* platform, XMLdoc& doc);

  /**
   * Destructor.
   */
  ~SDFGraph();

   /**
   * Gives the period constraint
   * @return 0: no constraint, -1: optimizie, >0: constraint
   */ 
  int getPeriodConstraint() const;
    
  /**
   * Sets the period constraint
   */ 
  void setPeriodConstraint(int _period_constraint);
    
  /**
   * Gives the latency constraint
   * @return 0: no constraint, -1: optimizie, >0: constraint
   */  
  int getLatencyConstraint() const;    

  /**
   * Sets the latency constraint
   */ 
  void setLatencyConstraint(int _latency_constraint);    

  /**
   * parses the two input vectors into design constraints
   * @param elements xml node names
   * @param values    xml node values
   */ 
  void setDesignConstraints(vector<char*> elements, vector<char*> values);

  /**
   * Gives the number of actors in the graph, i.e. the graph
   * converted to the internal HSDF-like format.
   * @return Number of actors.
   */ 
  int n_actors() const;
  
  /**
   * Gives the number of channels in the graph, i.e. the graph
   * converted to the internal HSDF-like format.
   * @returns Number of channels.
   */
  int n_channels() const;

  /**
   * Gives the number of parent actors of the graph, i.e. the number
   * of actors in the original SDFG / the number of different actor
   * functionalities.
   * @returns Number of parent actors.
   */
  size_t n_parentActors() const;

  /**
   * Gives the name of the graph.
   * @returns Name of the graph.
   */
  string getName() const;

  /**
   * Gives the name of the actor.
   * @param p_actor Id of the actor.
   * @returns Name of the specified actor.
   */
  string getActorName(size_t p_actor) const;

  /**
   * Gives the name of the parent actor.
   * @param p_actor Id of the (child) actor.
   * @returns Name of the parent actor of the specified actor.
   */
  string getParentName(size_t p_actor) const;

  /**
   * Gives the id of the parent actor.
   * @param p_actor Id of the (child) actor.
   * @returns Id of the parent actor of the specified actor.
   */
  int getParentId(size_t p_actor) const;

  /**
   * Gives the code size of the actor.
   * @param p_actor Id of the actor.
   * @returns Code size of the specified actor.
   */
  size_t getCodeSize(size_t p_actor) const;

  /**
   * Gives the data size of the actor.
   * @param p_actor Id of the actor.
   * @returns Data size of the specified actor.
   */
  size_t getDataSize(size_t p_actor) const;

  /**
   * Gives the largest code size over all actors in the graph.
   * @returns Maximum code size over all actors in the graph.
   */
  size_t getMaxCodeSize() const;

  /**
   * Gives the token size of the channel.
   * @param ch_id Id of the channel.
   * @returns Token size of the specified channel.
   */
  size_t getTokenSize(size_t ch_id) const;

  /**
   * Gives the actor that has the highest id of all actors
   * originating from the specified parent actor.
   * @param p_parentActor Id of the parent actor.
   * @returns The Id of the requested actor.
   */
  int lastActorInstanceId(size_t p_parentActor) const;
  
  /**
   * Gives the list of all channels in the graph.
   * @returns List of channels.
   */
  vector<SDFChannel*> getChannels() const;
  
  /**
   * Gives a list of all direct predecessor of the specified actor.
   * A direct predecessor is the source of a channel connected to 
   * the actor with no initial tokens.
   * @param p_actor The id of the actor whose predecessors are requested.
   * @returns List of predecessors.
   */
  vector<SDFActor*> getPredecessors(int p_actor) const;
  
  /**
   * Gives a list of all direct successors of the specified actor.
   * A direct successor is the destination of a channel connected to 
   * the actor with no initial tokens.
   * @param p_actor The id of the actor whose successors are requested.
   * @returns List of successors.
   */
  vector<SDFActor*> getSuccessors(int p_actor) const;
  
  /**
   * Determines whether a channel (not path!), with or without initial tokens,
   * exists from actor with id p_src to actor with id p_dst.
   * @param p_src Id of the source actor of the requested channel.
   * @param p_dst Id of the destination actor of the requested channel.
   * @returns True, if such a channel exists. False otherwise.
   */
  bool channelExists(int p_src, int p_dst) const;
  
  /**
   * Gives the number of initial tokens on channel from from actor 
   * with id p_src to actor with id p_dst.
   * @param p_src Id of the source actor of the requested channel.
   * @param p_dst Id of the destination actor of the requested channel.
   * @returns The number of initial tokens, if such a channel exists. -1 otherwise.
   */
  int tokensOnChannel(int p_src, int p_dst) const;

  /**
   * Determines whether a path, with or without initial tokens,
   * exists from actor with id p_src to actor with id p_dst.
   * @param p_src Id of the source actor of the requested path.
   * @param p_dst Id of the destination actor of the requested path.
   * @returns \c true, if such a path exists. \c false otherwise.
   */
  bool pathExists(int p_src, int p_dst) const;

  /**
   * Determines whether there are initial tokens on the path
   * from actor with id p_src to actor with id p_dst.
   * @param p_src Id of the source actor of the requested path.
   * @param p_dst Id of the destination actor of the requested path.
   * @returns \c true, if such a path exists and there are initial tokens along the path. \c false otherwise.
   */
  bool tokensOnPath(int p_src, int p_dst) const;

  /**
   * Determines whether the actor with id p_actorI precedes the actor
   * with id p_actorJ in the graph. I.e. it checks whether there is a
   * path from actorI to actorJ with no initial tokens.
   * @param p_actorI Id of the potentially preceding actor.
   * @param p_actorJ Id of the potentially preceded actor.
   * @returns \c true, if such a path exists (i.e. actorI precedes actorJ in the graph). \c false otherwise.
   */
  bool precedes(int p_actorI, int p_actorJ) const;

  /**
   * Determines whether the actor with id p_actorI and the actor
   * with id p_actorJ are independent of each other in the graph. I.e.
   * there is neither a path with no initial tokens from actorI to 
   * actorJ, nor from actorJ to actorI.
   * @param p_actorI Id of the first actor.
   * @param p_actorJ Id of the second actor.
   * @returns \c true, if the actors are independent. \c false otherwise.
   */
  bool independent(int p_actorI, int p_actorJ) const;
  
  /**
   * This function outputs the graph, transformed into the internal,
   * HSDF-like version, as dot file. The file will have the same name
   * as the graph, with the extension ".dot". It will be put into the
   * specified directory. If the macro \c _PRINT_ALL_DEBUG 
   * (see settings.h) is defined, the function will also issue the 
   * command to produce a pdf file from the dot representation (using
   * the graphviz tool. Only tested on Ubuntu).
   * @param dir path to the directory where the file should be placed.
   */
  void outputGraphAsDot(const string &dir) const;

  std::string getString() const;

};

#endif
