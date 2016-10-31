/**
 * Copyright (c) 2013-2016, Nima Khalilzad   <nkhal@kth.se>
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
#pragma once

#include <math.h>
#include <iostream>
#include <vector>
#include <boost/math/common_factor.hpp>
#include <cstring>
#include <exception>

#include "config.hpp"

using namespace std;
using namespace DeSyDe;

struct PresolverResults{
  size_t it_mapping; /**< Informs the CP model how to use oneProcMappings: <.size(): Enforce mapping, >=.size() Forbid all. */
  vector<vector<tuple<int,int>>> oneProcMappings;
  vector<vector<size_t>> periods;
  vector<size_t> sys_powers;
};

class DSESettings
{
public:

    DSESettings(Config& cfg);
    ~DSESettings();
    
    string     getCPModelString()                     const;
    int     getCPModel()                         const;
    
    string     getSearchTypeString()                 const;
    int     getSearchType()                     const;
    
    string     getOptCriterionString()             const;
    int     getOptCriterion()                     const;
    
    /**
     * Determines whether optimization is used.
     */
    bool    doOptimize()                        const;

    /**
     * Builds the name of the output path based on the current settings
     */ 
    string     getOutputsPath(string fileType)     const;
    
    string     getInputsPath()                     const;
    void     setInputsPath(string _inputsPath);
    
    /**
     * returns 1 if the mode is debug
     */ 
    int IsDebug();
    /**
     * Returns the luby scale
     */
    unsigned long int getLubyScale();     
    /**
     * Returns the timeout for search
     */
    unsigned long int getTimeout();  
    friend std::ostream& operator<< (std::ostream &out, const DSESettings &dseSettings);
    
    void setPresolverResults(shared_ptr<PresolverResults> _p);
    shared_ptr<PresolverResults> getPresolverResults();

private:
    
    Config& cfg;
    shared_ptr<PresolverResults> pre_results;
    enum     Config::CPModels         model;
    enum     Config::SearchTypes     search;
    enum     Config::OptCriterion    criterion;
    string     inputsPath;
    string     outputsPath;
    int     debug;
    unsigned long int luby_scale;
    unsigned long int timeout;            /**< Search timeout. */

};
