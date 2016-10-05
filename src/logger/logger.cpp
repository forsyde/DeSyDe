// Implementation of a multithread safe singleton logger class
#include <stdexcept>
#include "logger.hpp"

using namespace std;

Logger* Logger::pInstance = nullptr;

mutex Logger::sMutex;

const size_t Logger::kLogEntryLineWidthLimit = 100;

Logger& Logger::instance() throw (IOException) {
  static Cleanup cleanup;

  lock_guard<mutex> guard(sMutex);
  if (pInstance == nullptr)
    pInstance = new Logger();
  return *pInstance;
}

Logger& Logger::instance(const string& log_file) throw (IOException) {
  static Cleanup cleanup;

  lock_guard<mutex> guard(sMutex);
  if (pInstance == nullptr)
    pInstance = new Logger(log_file);
  return *pInstance;
}

Logger::Cleanup::~Cleanup() {
  lock_guard<mutex> guard(Logger::sMutex);
  delete Logger::pInstance;
  Logger::pInstance = nullptr;
}

Logger::~Logger() {
  mOutputStream.close();
}

Logger::Logger() throw (IOException)
     : level_stdout_(INFO), level_log_(DEBUG) {
  path_ = "log.out";
  mOutputStream.open(path_, std::ios::out | std::ios::trunc);
  if (!mOutputStream.good()) {
    THROW_EXCEPTION(IOException,path_,"Unable to initialize the logger!");
  }
}

Logger::Logger(const string& file) throw (IOException)
         : level_stdout_(INFO), level_log_(DEBUG) {
  path_ = file;
  mOutputStream.open(path_.c_str(), std::ios::out | std::ios::trunc);
  if (!mOutputStream.good()) {
    THROW_EXCEPTION(IOException, path_,"Unable to initialize the logger!");
  }
}

void Logger::log(const string& inMessage, LogLevel inLogLevel) throw (IOException) {
  lock_guard<mutex> guard(sMutex);
  logHelper(inMessage, inLogLevel);
}

void Logger::log(const vector<string>& inMessages, LogLevel inLogLevel) throw (IOException) {
  lock_guard<mutex> guard(sMutex);
  for (size_t i = 0; i < inMessages.size(); i++) {
    logHelper(inMessages[i], inLogLevel);
  }
}

void Logger::setLogLevel(const LogLevel level_stdout, const LogLevel level_log) throw () {
  level_stdout_ = level_stdout;
  level_log_    = level_log;
}

pair<Logger::LogLevel,Logger::LogLevel> Logger::getLogLevel() const throw () {
  return make_pair(level_stdout_, level_log_);
}

const string& Logger::getPath() const throw () {
  return path_;
}

string Logger::logLevelToString(LogLevel level) throw () {
  switch (level) {
  case DEBUG: {
    return "DEBUG";
  }
  case INFO: {
    return "INFO";
  }
  case WARNING: {
    return "WARNING";
  }
  case ERROR: {
    return "ERROR";
  }
  case CRITICAL: {
    return "CRITICAL";
  }
  default: {
    // Should never be reached
    return "???";
  }
  }
}

Logger::LogLevel Logger::stringToLogLevel(string str)
    throw (InvalidArgumentException) {
  if (str == "DEBUG") {
    return Logger::DEBUG;
  } else if (str == "INFO") {
    return Logger::INFO;
  } else if (str == "WARNING") {
    return Logger::WARNING;
  } else if (str == "ERROR") {
    return Logger::ERROR;
  } else if (str == "CRTICAL") {
    return Logger::CRITICAL;
  } else {
    THROW_EXCEPTION(InvalidArgumentException, "Unrecognized log level");
  }
}

void formatLogEntry(string& entry, int indent_length) {
  // Align the log message over linebreaks
  string new_linebreak(indent_length, ' ');
  new_linebreak.insert(0, 1, '\n');
  tools::searchReplace(entry, "\n", new_linebreak);
  tools::breakLongLines(entry, 0, indent_length);
}

void Logger::logHelper(const std::string& message, LogLevel level)  throw (IOException) {
  if (level < level_log_) return;

  // Generate log entry
  string entry;
  entry += tools::getCurrentTimestamp();
  entry += " [";
  entry += Logger::logLevelToString(level);
  entry += "] - ";
  int indent_length = entry.length();
  string formatted_message(message);
  tools::trim(formatted_message);
  entry += formatted_message;
  formatLogEntry(entry, indent_length);
  entry += '\n';
  try {
    mOutputStream << entry << std::flush;
  } catch (ofstream::failure&) {
    THROW_EXCEPTION(IOException, path_);
  }

  if (level < level_stdout_) return;

  // Generate console output
  string prompt_output(" * ");
  prompt_output += Logger::logLevelToString(level) + ": ";
  indent_length = prompt_output.length();
  prompt_output += message;
  formatLogEntry(prompt_output, indent_length);
  cout << prompt_output << std::endl;

}
