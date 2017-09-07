/*
 * Copyright (c) 2011-2013
 *     George Ungureanu <ugeorge@kth.se>
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


#include <vector>
#include <boost/bind.hpp>

#include "config.hpp"
#include "../tools/tools.hpp"
#include "../logger/logger.hpp"


using std::string;
using std::vector;
using std::list;
using namespace DeSyDe;


const string help_intro = string()
    + "DeSyDe - Analytical Design Space Exploration tool\n"
    + "Developers: Kathrin Rosvall  <danmann@kth.se>\n"
    + "            Nima Khalilzad   <nkhal@kth.se>\n"
    + "            George Ungureanu <ugeorge@kth.se>\n"
    + "KTH - Royal Institute of Technology, Stockholm, Sweden\n"
    + "Copyright (c) 2013-2016\n"
    + "This tool is part of the ForSyDe framework <https://github.com/forsyde>\n";


Config::Config() throw (){}

Config::~Config() throw() {}

int Config::parse(int argc, const char** argv) throw (IOException, InvalidArgumentException,
    InvalidFormatException) {

  po::positional_options_description p;
  p.add("inputs", -1);

  po::options_description hidden;
  hidden.add_options()
      ("inputs,i",
          po::value<vector<string>>()->multitoken()->required()
          ->notifier(boost::bind(&Config::setInputPaths, this, _1)),
          "input file or path. Multiple paths allowed.");

  po::options_description generic("Generic options");
  generic.add_options()
      ("help,h",
          "prints help menu.")
      ("version,v",
          "prints version.")
      ("config,c",
          po::value<string>()->default_value("config.cfg"),
          "input configuration file.")
      ("dump-cfg",
          po::value<string>()->implicit_value("config.cfg"),
          "creates a default configuration file and exits the program.")
      ("output,o",
          po::value<string>()->default_value(".")->notifier(
              boost::bind(&Config::setOutputPaths, this, _1)),
          "output path.")
      ("log-file",
          po::value<string>()->default_value(string("output.log"))->notifier(
              boost::bind(&Config::setLogPaths, this, _1)),
          "path to log file.")
      ("log-level",
          po::value<vector<string>>()->multitoken()->default_value({"INFO", "DEBUG"},
              "INFO DEBUG")->notifier(boost::bind(&Config::setLogLevel, this, _1)),
          "log level. If two options provided, the first one is for stdout, the second for log file.\n"
          "Valid options are CRITICAL, ERROR, WARNING, INFO, and DEBUG.")
      ("output-file-type",
          po::value<string>()->default_value(string("ALL_OUT"))->notifier(
              boost::bind(&Config::setOutputFileType, this, _1)),
          "Output file type.\n"
          "Valid options ALL, CSV, TXT, XML. ")
      ("output-print-frequency",
          po::value<string>()->default_value(string("ALL_SOL"))->notifier(
              boost::bind(&Config::setOutputPrintFrequency, this, _1)),
          "Frequency of printing output.\n"
          "Valid options ALL, LAST, Every_n, FIRSTandLAST. ");        

  po::options_description dse("DSE options");
  dse.add_options()
      ("dse.model",
          po::value<string>()->default_value(string("NONECP"))->notifier(
              boost::bind(&Config::setModel, this, _1)),
          "Constraint programming model.\n"
          "Valid options NONECP, SDF, SDF_PR_ONLINE. ")
      ("dse.search",
          po::value<string>()->default_value(string("NONESEARCH"))->notifier(
              boost::bind(&Config::setSearch, this, _1)),
          "Search type.\n"
          "Valid options NONESEARCH, FIRST, ALL, OPTIMIZE, OPTIMIZE_IT, GIST_ALL, GIST_OPT. ")
      ("dse.criteria",
          po::value<vector<string>>()->multitoken()->default_value({"NONE",""},
              "NONE ")->notifier(boost::bind(&Config::setCriteria, this, _1)),
          "Optimization criteria.\n"
          "Valid options NONE, POWER, THROUGHPUT, LATENCY.")
      ("dse.timeout",
          po::value<vector<unsigned long int>>()->multitoken()->default_value({0,0},
              "0 0")->notifier(boost::bind(&Config::setTimeout, this, _1)),
          "search timeout. 0 means infinite. If two values are provided, the first one specifies "
          "the timeout for the first solution, and the second one for all solutions.")
      ("dse.luby_scale",
          po::value<unsigned long int>()->default_value(0)->notifier(
              boost::bind(&Config::setLubyScale, this, _1)),
          "Luby scale")      
      ("dse.th_prop",
          po::value<string>()->default_value(string("SSE"))->notifier(
              boost::bind(&Config::setThPropagator, this, _1)),
          "Throughput propagator type.\n"
          "Valid options SSE, MCR. ");

  po::options_description presolver("Presolver options");
  presolver.add_options()
    ("presolver.model",
              po::value<vector<string>>()->multitoken()->default_value({"NONE",""},
                  "NONE ")->notifier(boost::bind(&Config::setPresolverModel, this, _1)),
              "Constraint pre-solving model.\n"
              "Valid options NONE, ONE_PROC_MAPPINGS.")
    ("presolver.search",
               po::value<string>()->default_value(string("ALL"))->notifier(
                  boost::bind(&Config::setPresolverSearch, this, _1)),
             "Search type.\n"
             "Valid options NONESEARCH, FIRST, ALL, OPTIMIZE, OPTIMIZE_IT, GIST_ALL, GIST_OPT. ");

  po::variables_map vm;
  po::options_description visible_options, all_options;

  visible_options.add(generic).add(presolver).add(dse);
  all_options.add(hidden).add(generic).add(presolver).add(dse);


  auto cli_options = po::command_line_parser(argc, argv).options(all_options).positional(p).run();
  po::store(cli_options, vm);

  if (vm.count("help")) {
    std::cout << help_intro << std::endl;
    std::cout << "Usage: adse [options] input-path|input-file.xml..." << std::endl;
    std::cout << visible_options << std::endl;
    return 1;
  }

  if (vm.count("version")) {
    std::cout << "Version: " << VERSION << std::endl;
    return 1;
  }


  if (vm.count("dump-cfg")) {
    string conf_path = vm["dump-cfg"].as<string>();
    std::cout << conf_path << std::endl;
    dumpConfigFile(conf_path, all_options);
    return 1;
  }

  bool loaded_cfg_file;
  string conf_path = vm["config"].as<string>();
  if (!conf_path.size()) conf_path = "config.cfg";
  ifstream ifs(conf_path.c_str());
  if (ifs) {
    auto cfg_file = po::parse_config_file(ifs, all_options);
    po::store(cfg_file, vm);
    po::store(cli_options, vm);
    loaded_cfg_file = true;
  } else {
    loaded_cfg_file = false;
  }

  try {
    po::notify(vm);
  } catch (po::error& ex) {
    THROW_EXCEPTION(IOException,"CLI",ex.what());
  }

  if (loaded_cfg_file)
    LOG_DEBUG("Loaded initial configuration from \'" + conf_path + "\'");
  else
    LOG_WARNING("Could not open \'" + conf_path
        + "\'. Loading default (hardcoded) configuration.");

  LOG_INFO(string () + "Started logging into \'"
      + Logger::instance().getPath() + "\'");
  LOG_DEBUG("Configuration parsed successfully.");
  LOG_DEBUG(printSettings());

  return 0;

}

const Config::Settings& Config::settings() const throw() {
  return settings_;
}

string Config::printSettings() {
  return string()
      + "\n* inputs_paths : " + tools::toString(settings_.inputs_paths)
      + "\n* output_path : " + settings_.output_path
      + "\n* logger : " + Logger::instance().getPath()
      + "\n* log level : " + Logger::logLevelToString(Logger::instance().getLogLevel().first)
      + " | " +  Logger::logLevelToString(Logger::instance().getLogLevel().second)
      + "\n* model : " + tools::toString(settings_.model)
      + "\n* search : " + tools::toString(settings_.search)
      + "\n* propagator : " + tools::toString(settings_.th_prop)
      + "\n* criteria : " + tools::toString(settings_.criteria)
      + "\n* timeout : " + tools::toString(settings_.timeout_first)
      + " | " + tools::toString(settings_.timeout_all)
      + "\n* luby_scale : " + tools::toString(settings_.luby_scale);
}

void Config::dumpConfigFile(string path, po::options_description opts) throw (IOException){
  map<string,vector<pair<vector<string>,string>>> config;
  for (const auto& o : opts.options()) {
    if (o->long_name() == "help")     continue;
    if (o->long_name() == "version")  continue;
    if (o->long_name() == "config")   continue;
    if (o->long_name() == "dump-cfg") continue;
    //std::cout << o->format_parameter() << ":" << o->description() << "\n";
    string section, description;
    vector<string> variables;

    vector<string> full_name = tools::split(o->long_name(),'.');
    string variable;
    if (full_name.size() > 1) {
      section=full_name[0];
      variable=full_name[1];
    } else {
      section="_general";
      variable=full_name[0];
    }

    string args = o->format_parameter().substr(3,o->format_parameter().size()-1);
    args = tools::trim(args);
    if (args.length() == 0) {
      variables.push_back(variable+"=");
    } else {
      args = args.substr(2,args.size()-3);
      for (string value : tools::split(args, ' ')) {
        value = tools::trim(value);
        variables.push_back(variable+"="+value);
      }
    }

    description = o->description();
    tools::breakLongLines(description, 70, 0);
    description="# " + description;
    description=tools::searchReplace(description,"\n","\n# ");

    config[section].push_back(make_pair(variables,description));
  }

  ofstream cfg_file;
  cfg_file.open(path.c_str(), ios::out | ios::trunc);
  if (!cfg_file.is_open())
    THROW_EXCEPTION(IOException,path,"cannot write config file");
  for (auto sec : config) {
    if (sec.first != "_general") cfg_file << "\n[" << sec.first << "]\n";
    for (auto var : sec.second) {
      cfg_file << "\n" << var.second << "\n";
      for (auto v : var.first) {
        cfg_file << v << "\n";
      }
    }
  }
  cfg_file.close();

}


// SETTERS

void Config::setInputPaths(const vector<string> & paths) throw (IOException) {
  for (auto p : paths) {
    tools::append(settings_.inputs_paths, tools::getFileNames(p, ".xml"));
  }
}

void Config::setOutputPaths(const string &path) throw (IOException) {
  if (!tools::isAccessible(path))
    THROW_EXCEPTION(IOException,path,"output path is not accessible");
  settings_.output_path = path;
}

void Config::setLogPaths(const string &path) throw (IOException) {
  if (!tools::isValidFilePath(path))
      THROW_EXCEPTION(IOException,path,"cannot write log file");
  Logger::instance(path);
}

void Config::setLogLevel(const vector<string> &levels) throw (IllegalStateException, InvalidFormatException){
  string           level_stdout,   level_logger;
  Logger::LogLevel level_stdout_l, level_logger_l;
  try {
    level_stdout = (levels.size() < 1) ? "INFO"  : levels[0];
    level_logger = (levels.size() < 2) ? "DEBUG" : levels[1];
  } catch (std::exception& e) {
    THROW_EXCEPTION(IllegalStateException, "internal error");
  }
  try {
    level_stdout_l = Logger::stringToLogLevel(level_stdout);
  } catch (InvalidArgumentException& ex) {
    THROW_EXCEPTION(InvalidFormatException, level_stdout, "invalid option");
  }
  try {
    level_logger_l = Logger::stringToLogLevel(level_logger);
  } catch (InvalidArgumentException& ex) {
    THROW_EXCEPTION(InvalidFormatException, level_logger, "invalid option");
  }
  Logger::instance().setLogLevel(level_stdout_l,level_logger_l);
}

Config::CPModels stringToModel(const string &str) throw (InvalidFormatException) {
  if (str == "NONECP")             return Config::NONECP;
  else if (str == "SDF")           return Config::SDF;
  else if (str == "SDF_PR_ONLINE") return Config::SDF_PR_ONLINE;
  else THROW_EXCEPTION(InvalidFormatException, str, "invalid option");
}

void Config::setModel(const string &str) throw (InvalidFormatException) {
  settings_.model = stringToModel(str);
}

Config::SearchTypes stringToSearch (const string &str) throw (InvalidFormatException) {
  if (str == "NONESEARCH")       return Config::NONESEARCH;
  else if (str == "FIRST")       return Config::FIRST;
  else if (str == "ALL")         return Config::ALL;
  else if (str == "OPTIMIZE")    return Config::OPTIMIZE;
  else if (str == "OPTIMIZE_IT") return Config::OPTIMIZE_IT;
  else if (str == "GIST_ALL")    return Config::GIST_ALL;
  else if (str == "GIST_OPT")    return Config::GIST_OPT;
  else THROW_EXCEPTION(InvalidFormatException, str, "invalid option");
}

void Config::setSearch(const string &str) throw (InvalidFormatException) {
  settings_.search = stringToSearch(str);
}

Config::ThroughputPropagator stringToPropagator(const string &str) throw (InvalidFormatException) {
  if (str == "SSE")            return Config::SSE;
  else if (str == "MCR")      return Config::MCR;
  else THROW_EXCEPTION(InvalidFormatException, str, "invalid option");
}

void Config::setThPropagator(const string &str) throw (InvalidFormatException) {
  settings_.th_prop = stringToPropagator(str);
}
Config::OptCriterion stringToCriterion(const string &str) throw (InvalidFormatException) {
  if (str == "NONE")            return Config::NONE;
  else if (str == "POWER")      return Config::POWER;
  else if (str == "THROUGHPUT") return Config::THROUGHPUT;
  else if (str == "LATENCY")    return Config::LATENCY;
  else THROW_EXCEPTION(InvalidFormatException, str, "invalid option");
}



Config::OutputFileType stringToOutputFileType(const string &str) throw (InvalidFormatException) {
  if (str == "ALL_OUT")              return Config::ALL_OUT;
  else if (str == "CSV")         return Config::CSV;
  else if (str == "CSV_MOST")    return Config::CSV_MOST;
  else if (str == "TXT")         return Config::TXT;
  else if (str == "XML")         return Config::XML;
  else THROW_EXCEPTION(InvalidFormatException, str, "invalid option");
}
void Config::setOutputFileType(const string &str) throw (InvalidFormatException) {
  settings_.out_file_type = stringToOutputFileType(str);
}

Config::OutputPrintFrequency stringToPrintFrequency(const string &str) throw (InvalidFormatException) {
  if (str == "ALL_SOL")            return Config::ALL_SOL;
  else if (str == "LAST")      return Config::LAST;
  else if (str == "EVERY_n") return Config::EVERY_n;
  else if (str == "FIRSTandLAST") return Config::FIRSTandLAST;
  else THROW_EXCEPTION(InvalidFormatException, str, "invalid option");
}

string printFrequencyToString(Config::OutputPrintFrequency freq) throw (InvalidFormatException) {
  if (freq == Config::ALL_SOL)     return "ALL_SOL";
  else if (freq == Config::LAST)    return "LAST";
  else if (freq == Config::EVERY_n) return "EVERY_n";
  else if (freq == Config::FIRSTandLAST) return "FIRSTandLAST";
  else THROW_EXCEPTION(InvalidFormatException, "ouput frequency", "invalid option");
}
string Config::get_out_freq() const {
    return printFrequencyToString(settings_.out_print_freq);
}

string searchTypeToString(Config::SearchTypes freq) throw (InvalidFormatException) {
  if (freq == Config::NONESEARCH)       return "NONESEARCH";
  else if (freq == Config::FIRST)       return "FIRST";
  else if (freq == Config::ALL)         return "ALL";
  else if (freq == Config::OPTIMIZE)    return "OPTIMIZE";
  else if (freq == Config::OPTIMIZE_IT) return "OPTIMIZE_IT";
  else if (freq == Config::GIST_ALL)    return "GIST_ALL";
  else if (freq == Config::GIST_OPT)    return "GIST_OPT";
  else THROW_EXCEPTION(InvalidFormatException, "searchTypeToString", "invalid option");
}
string Config::get_search_type() const {
    return searchTypeToString(settings_.search);
}

void Config::setOutputPrintFrequency(const string &str) throw (InvalidFormatException) {
  settings_.out_print_freq = stringToPrintFrequency(str);
}

Config::PresolverModels stringToPresolverModel(const string &str) throw (InvalidFormatException) {
  if (str == "NONE")            return Config::NO_PRE;
  else if (str == "ONE_PROC_MAPPINGS")      return Config::ONE_PROC_MAPPINGS;
  else THROW_EXCEPTION(InvalidFormatException, str, "invalid option");
}


void Config::setCriteria(const vector<string> &str) throw (InvalidFormatException) {
  for (string s : str)
    if (s.length() != 0)
      settings_.criteria.push_back(stringToCriterion(tools::trim(s)));
}

void Config::setTimeout(const vector<unsigned long int> &touts) throw (IllegalStateException) {
  try {
    settings_.timeout_first = (touts.size() < 1) ? 0 : touts[0];
    settings_.timeout_all   = (touts.size() < 2) ? 0 : touts[1];
  } catch (std::exception& e) {
    THROW_EXCEPTION(IllegalStateException, "internal error");
  }
}

void Config::setLubyScale(unsigned long int scale) throw () {
  settings_.luby_scale = scale;
}

void Config::setPresolverModel(const vector<string> &str) throw (InvalidFormatException) {
  for (string s : str)
    if (s.length() != 0)
      settings_.pre_models.push_back(stringToPresolverModel(tools::trim(s)));
}

void Config::setPresolverSearch(const string &str) throw (InvalidFormatException) {
  settings_.pre_search = stringToSearch(str);
}

void Config::setPresolverResults(shared_ptr<Config::PresolverResults> _p){
  pre_results = _p;
}

shared_ptr<Config::PresolverResults> Config::getPresolverResults(){
  if(!pre_results)  
      THROW_EXCEPTION(RuntimeException, "no presolver results exist");
  return pre_results;
}
bool Config::doOptimize() const {
  if (settings().search == Config::OPTIMIZE || settings().search == Config::OPTIMIZE_IT || settings().search == Config::GIST_OPT) {
    return true;
  }
  return false;
}

bool Config::doOptimizeThput() const{
  for(auto i: settings_.criteria)
    if(i==THROUGHPUT) return true;
  
  return false;
}

bool Config::doOptimizePower() const{
  for(auto i: settings_.criteria)
    if(i==POWER) return true;
  
  return false;
  
}

bool Config::doPresolve() const{
  for(auto i : settings_.pre_models){
    if(i > NO_PRE) return true;
  }
  return false;
}

bool Config::is_presolved()
{
  if(doPresolve()){
    if(pre_results->oneProcMappings.size() > 0)
        return true;
    else
        return false;
  }
  return false;
}
