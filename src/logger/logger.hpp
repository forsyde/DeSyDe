/**
 * Copyright (c) 2016, George Ungureanu <ugeorge@kth.se>
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
/*
 * Author: George Ungureanu <ugeorge@kth.se>
 *
 * Code inspired from Professional C++ 2nd edition.
 */

#ifndef DESYDE_LOGGER_LOGGER_H_
#define DESYDE_LOGGER_LOGGER_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <mutex>

#include "../tools/stringtools.hpp"
#include "../tools/systools.hpp"
#include "../exceptions/ioexception.h"
#include "../exceptions/runtimeexception.h"

using namespace DeSyDe;

// Definition of a multithread safe singleton logger class
class Logger {
public:

  static const size_t kLogEntryLineWidthLimit;

  enum LogLevel {
    DEBUG,   /*!< For debugging. */
    INFO,    /*!< For informative messages. */
    WARNING, /*!< For warnings which do not affect behavior if ignored. */
    ERROR,   /*!< For errors which are caused by invalid input. */
    CRITICAL /*!< For errors which should just never happen. */
  };

  // Returns a reference to the singleton Logger object
  static Logger& instance() throw (IOException);
  static Logger& instance(const std::string& log_file) throw (IOException);

  // Logs a single message at the given log level
  void log(const std::string& inMessage, LogLevel level) throw (IOException);

  // Logs a vector of messages at the given log level
  void log(const std::vector<std::string>& inMessages, LogLevel level) throw (IOException);


  void setLogLevel(const LogLevel level_stdout, const LogLevel level_log) throw ();
  std::pair<LogLevel,LogLevel> getLogLevel() const throw ();
  const std::string& getPath() const throw ();

  static LogLevel stringToLogLevel(std::string str) throw (InvalidArgumentException);
  static std::string logLevelToString(LogLevel level) throw ();

protected:
  // Static variable for the one-and-only instance
  static Logger* pInstance;


  // Data member for the output stream
  std::ofstream mOutputStream;

  LogLevel level_stdout_, level_log_;
  std::string path_;

  // Embedded class to make sure the single Logger
  // instance gets deleted on program shutdown.
  friend class Cleanup;
  class Cleanup {
  public:
    ~Cleanup();
  };

  // Logs message. The thread should own a lock on sMutex
  // before calling this function.
  void logHelper(const std::string& message, LogLevel level)  throw (IOException);

private:
  Logger() throw (IOException);
  Logger(const std::string& file) throw (IOException);
  virtual ~Logger();
  Logger(const Logger&);
  Logger& operator=(const Logger&);
  static std::mutex sMutex;
};

template<typename T>
void LOG_DEBUG(const T& msg) {
  Logger::instance().log(msg, Logger::DEBUG);
}

template<typename T>
void LOG_INFO(const T& msg) {
  Logger::instance().log(msg, Logger::INFO);
}
template<typename T>
void LOG_WARNING(const T& msg) {
  Logger::instance().log(msg, Logger::WARNING);
}
template<typename T>
void LOG_ERROR(const T& msg) {
  Logger::instance().log(msg, Logger::ERROR);
}
template<typename T>
void LOG_CRITICAL(const T& msg) {
  Logger::instance().log(msg, Logger::CRITICAL);
}

#endif
