#include "dse_settings.hpp"

using namespace std;

DSESettings::DSESettings(Config& cfg) : cfg(cfg) {
  model = cfg.settings().model;
  search = cfg.settings().search;
  criterion = cfg.settings().criteria[0];
  inputsPath = cfg.settings().output_path;//inputs_paths[0];
  outputsPath = cfg.settings().output_path + "out/";
  debug = 0;
  luby_scale = cfg.settings().luby_scale;
  timeout = cfg.settings().timeout_all;
}

DSESettings::~DSESettings() { }

string DSESettings::getCPModelString() const {
  string modelStr = "";
  switch (model) {
  case Config::SDF_PR_ONLINE:
    modelStr = "SDF_PR_ONLINE";
    break;
  case Config::SDF:
    modelStr = "SDF";
    break;
  default:
    cout << "unknown model type !!!";
    break;
  }
  return modelStr;
}

int DSESettings::getCPModel() const {
  return model;
}

string DSESettings::getSearchTypeString() const {
  string searchStr = "";
  switch (search) {
  case Config::FIRST:
    searchStr = "FIRST";
    break;
  case Config::ALL:
    searchStr = "ALL";
    break;
  case Config::OPTIMIZE:
    searchStr = "OPTIMIZE";
    break;
  case Config::OPTIMIZE_IT:
    searchStr = "OPTIMIZE_IT";
    break;
  case Config::GIST_ALL:
    searchStr = "GIST_ALL";
    break;
  case Config::GIST_OPT:
    searchStr = "GIST_OPT";
    break;
  default:
    cout << "unknown search type !!!";
    break;
  }
  return searchStr;
}

int DSESettings::getSearchType() const {
  return search;
}

string DSESettings::getOptCriterionString() const {
  string criterionStr = "";
  switch (criterion) {
  case Config::POWER:
    criterionStr = "POWER";
    break;
  case Config::THROUGHPUT:
    criterionStr = "THROUGHPUT";
    break;
  case Config::LATENCY:
    criterionStr = "LATENCY";
    break;
  default:
    criterionStr = "unknown";
    break;
  }
  return criterionStr;
}

int DSESettings::getOptCriterion() const {
  return criterion;
}

bool DSESettings::doOptimize() const {
  if (search == Config::OPTIMIZE || search == Config::OPTIMIZE_IT || search == Config::GIST_OPT) {
    return true;
  }
  return false;
}

string DSESettings::getInputsPath() const {
  return inputsPath;
}

void DSESettings::setInputsPath(string _inputsPath) {
  inputsPath = _inputsPath;
}

string DSESettings::getOutputsPath(string fileType) const {
  string outPutsPath = outputsPath;
  outPutsPath += getCPModelString();
  outPutsPath += "_";
  outPutsPath += getSearchTypeString();
  if (search == Config::OPTIMIZE || search == Config::OPTIMIZE_IT) {
    outPutsPath += "_";
    outPutsPath += getOptCriterionString();
  }
  outPutsPath += "_results";
  outPutsPath += fileType;
  return outPutsPath;
}


int DSESettings::IsDebug() {
  return debug;
}

unsigned long int DSESettings::getLubyScale() {
  return luby_scale;
}
unsigned long int DSESettings::getTimeout() {
  return timeout;
}
std::ostream& operator<<(std::ostream &out, const DSESettings &dseSettings) {
  out << "model:" << dseSettings.getCPModelString() << endl;
  out << "search:" << dseSettings.getSearchTypeString() << endl;
  out << "criterion:" << dseSettings.getOptCriterionString() << endl;
  out << "inputsPath:" << dseSettings.getInputsPath() << endl;

  return out;
}

