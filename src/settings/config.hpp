/*
 * Copyright (c) 2016  George Ungureanu <ugeorge@kth.se>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OF THIS SOFTWARE NOR THE
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef F2CC_SOURCE_CONFIG_CONFIG_H_
#define F2CC_SOURCE_CONFIG_CONFIG_H_

/**
 * @file
 * @author  George Ungureanu <ugeorge@kth.se>
 * @version 0.1
 *
 * @brief Defines a class for containing program settings.
 */

#include "../exceptions/ioexception.h"
#include "../exceptions/runtimeexception.h"
#include <string>
#include <list>
#include <boost/program_options.hpp>
#include <vector>


namespace po = boost::program_options;
using namespace DeSyDe;
using namespace std;
/**
 * @brief Defines a class for containing program settings.
 *
 * The \c Config class provides methods for accessing the program-related
 * settings. The settings are usually given through the command-line and there
 * is a special constructor for parsing the command-line into a \c Config
 * object.
 */
class Config {
public:
  enum CPModels {
    NONECP,
    SDF,
    SDF_PR_ONLINE
  };
  enum PresolverModels {
    NO_PRE,
    ONE_PROC_MAPPINGS
  };
  enum SearchTypes {
    NONESEARCH,
    FIRST,
    ALL,
    OPTIMIZE,
    OPTIMIZE_IT,
    GIST_ALL,
    GIST_OPT
  };
  enum OptCriterion {
    NONE,
    POWER,
    THROUGHPUT,
    LATENCY
  };
  struct Settings {
    std::vector<std::string> inputs_paths;
    std::string              output_path;

    CPModels                  model;
    std::vector<PresolverModels> pre_models;
    SearchTypes               search;
    SearchTypes               pre_search;
    std::vector<OptCriterion> criteria;
    unsigned long int         timeout_first;
    unsigned long int         timeout_all;

    unsigned long int luby_scale;
  };
  struct PresolverResults{
  size_t it_mapping; /**< Informs the CP model how to use oneProcMappings: <.size(): Enforce mapping, >=.size() Forbid all. */
  vector<vector<tuple<int,int>>> oneProcMappings;
  vector<vector<size_t>> periods;
  vector<size_t> sys_powers;
  };

public:

  /**
   * Creates a configuration using settings from the command
   * line. Non-specified settings use default values.
   *
   * @param argc
   *        Number of arguments.
   * @param argv
   *        Array of char arrays.
   * @throws InvalidArgumentException
   *         When \c argc is less than 1 or when \c argv is NULL.
   * @throws InvalidFormatException
   *         When the command line contains errors.
   */
   Config() throw ();
  /**
   * Destroys this configuration.
   */
  ~Config() throw ();

  int parse(int argc, const char** argv) throw (IOException, InvalidArgumentException,
      InvalidFormatException);

  const Settings& settings() const throw();

  std::string printSettings();

    void setPresolverResults(shared_ptr<PresolverResults> _p);
    shared_ptr<PresolverResults> getPresolverResults();
    /**
     * Determines whether optimization is used.
     */
    bool doOptimize() const;
    bool is_presolved();
private:
  Settings settings_;
  shared_ptr<PresolverResults> pre_results;

private:
  void dumpConfigFile(std::string path, po::options_description opts) throw (IOException);

  void setInputPaths(const std::vector<std::string> &) throw (IOException);
  void setOutputPaths(const std::string &) throw (IOException);
  void setLogPaths(const std::string &) throw (IOException);
  void setLogLevel(const std::vector<std::string> &) throw (IllegalStateException, InvalidFormatException);
  void setModel(const std::string &) throw (InvalidFormatException);
  void setSearch(const std::string &) throw (InvalidFormatException);
  void setCriteria(const std::vector<std::string> &) throw (InvalidFormatException);
  void setTimeout(const std::vector<unsigned long int> &) throw (IllegalStateException);
  void setLubyScale(unsigned long int) throw ();
  void setPresolverModel(const std::vector<std::string> &) throw (InvalidFormatException);
  void setPresolverSearch(const std::string &) throw (InvalidFormatException);


};


#endif
