//
// Copyright (C) [2020] Futurewei Technologies, Inc.
//
// FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
// FIT FOR A PARTICULAR PURPOSE.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "Log.h"

#include <iostream>

#include "Dump.h"

using namespace std;

namespace Force {

  Logger* gLog = nullptr;
  char* gSmallBuffer = nullptr;

  /*!
    \class Logger
  */
  Logger::Logger(ostream& stream, ostream& errorStream, ostream& testStream)
    : mOStream(stream), mErrorStream(errorStream), mTestStream(testStream), mLogLevel(LL::error)
  {
  }

  void Logger::Initialize()
  {
    if (nullptr == gLog) {
      gLog = new Logger(cout, cerr, cout);
    }
    if (nullptr == gSmallBuffer) {
      gSmallBuffer = new char [MAX_SMALL_BUFFER_SIZE];
    }
  }

  void Logger::Destroy()
  {
    delete gLog;
    gLog = nullptr;
    delete gSmallBuffer;
    gSmallBuffer = nullptr;
  }

  static inline const char * ll_to_string(LL logLevel)
  {
    switch (logLevel) {
    case LL::trace:
      return "[trace]";
    case LL::debug:
      return "[debug]";
    case LL::info:
      return "[info]";
    case LL::warn:
      return "[warn]";
    case LL::error:
      return "[error]";
    case LL::fail:
      return "[fail]";
    case LL::notice:
      return "[notice]";
    default:
      return "[UNKNOWN]";
    }
  }

  static inline LL string_to_ll(const char *logLevel)
  {
    if (string("trace") == logLevel) {
      return LL::trace;
    } else if (string("debug") == logLevel) {
      return LL::debug;
    } else if (string("info") == logLevel) {
      return LL::info;
    } else if (string("warn") == logLevel) {
      return LL::warn;
    } else if (string("error") == logLevel) {
      return LL::error;
    } else if (string("fail") == logLevel) {
      return LL::fail;
    } else if (string("notice") == logLevel) {
      return LL::notice;
    } else {
      LOG(fail) << "Unknown log level \'" << logLevel << "\', supported log levels are \'trace, debug, info, warn, error, fail, notice\'." << endl;
      FAIL("unknown-log-level");
    }
    return LL::notice;
  }

  ostream& Logger::Stream(LL logLevel)
  {
    const char * heading = ll_to_string(logLevel);
    switch (logLevel)
    {
      case LL::fail:
      case LL::error:
        mErrorStream << heading;
        return mErrorStream;
        break;
      default:
        mOStream << heading;
        return mOStream;
    }
  }

  void Logger::Fail(const char* msg, const char* fileName, int lineNo, const char* funcName)
  {
#ifndef UNIT_TEST
    mErrorStream << "[FAIL]{" << msg << "} in file \'" << fileName << "\' line " << dec << lineNo << " func \'" << funcName << "\'." << endl;
    if (mLogLevel < LL::error) {
      ::abort();
    } else {
      ::exit(1);
    }
#else
    // Throw exception if doing unit testing
    throw std::runtime_error(msg);
#endif
  }

  void Logger::DumpFail(const char* msg, const char* fileName, int lineNo, const char* funcName)
  {
#ifndef UNIT_TEST
    Dump::Instance()->DumpInfo();
#endif
    Fail(msg, fileName, lineNo, funcName);
  }

  void Logger::SetLevel(const char* logLevel)
  {
    LL log_level = string_to_ll(logLevel);
    mLogLevel = log_level;
  }

}
