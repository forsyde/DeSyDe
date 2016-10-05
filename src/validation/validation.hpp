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
#include <boost/math/common_factor.hpp>
#include <cstring>
#include <gecode/gist.hh>
#include "../settings/dse_settings.hpp"
#include "../settings/input_reader.hpp"
#include "../system/mapping.hpp"

using namespace std;
using namespace Gecode;

//template <class CPModelTemplate>
class Validation {
public:
  Validation(Mapping* _map, DSESettings* settings);
    
  /**
   * Loops through all solutions and test the schedulability
   */ 
  void Validate();

private:

  Mapping* map;
  size_t no_actors;    /**< Number of actors. */  
  size_t no_ipts;    /**< Number of periodic tasks. */  
  size_t no_procs;    /**< Number of processors. */  
  vector<vector<string>> mapping_csv;

  /**
   * gets a row of the solution csv and
   * return the mappings
   */ 
  vector<int> getMapping(vector<string> solution);

  /**
   * gets a row of the solution csv and
   * return the processor modes
   */ 
  vector<int> getMode(vector<string> solution);

};

