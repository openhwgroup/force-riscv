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
#ifndef Force_Log_H
#define Force_Log_H

#include <ostream>
#include <cassert>

namespace Force {

  /*
    LL Loging level
   */
  enum class LL : unsigned char {
    trace  = 0,
    debug  = 1,
    info   = 2,
    warn   = 3,
    error  = 4,
    fail   = 5,
    notice = 6
  };

  class Logger {
  public:
    ~Logger() { } //!< Destructor.
    inline bool Log(LL logLevel) const { return (logLevel >= mLogLevel); } //!< Return whether logging is enable at the specified log-leve
    void DumpFail(const char* msg, const char* fileName, int lineNo, const char* funcName);
    void SetLevel(const char* logLevel);
    std::ostream& Stream(LL logLvel);
    std::ostream& TestStream() { return mTestStream; }

    static void Initialize();
    static void Destroy();
  private:
    Logger(std::ostream& stream, std::ostream& errorStream, std::ostream& testStream);
    void Fail(const char* msg, const char* fileName, int lineNo, const char* funcName);
  private:
    std::ostream& mOStream;
    std::ostream& mErrorStream;
    std::ostream& mTestStream;
    LL mLogLevel;
  };

  extern Logger* gLog;
  extern char* gSmallBuffer;

#define LOG(LEVEL) if (gLog->Log(LL::LEVEL)) gLog->Stream(LL::LEVEL)
#define FAIL(msg) gLog->DumpFail(msg, __FILE__,__LINE__,__func__)
#define SET_LOG_LEVEL(level) gLog->SetLevel(level)
#define TEST_INFO gLog->TestStream() << "[TEST_INFO]"
#define MAX_SMALL_BUFFER_SIZE 256

  /*!
    A function similar to C lib snprintf that return a string for convenient use.
   */
  template< typename... Args >
  std::string string_snprintf(unsigned int length, const char* pFormat, Args... args)
  {
    assert(length <= MAX_SMALL_BUFFER_SIZE);
    std::snprintf(gSmallBuffer, length, pFormat, args...);
    return std::string(gSmallBuffer);
  }

}

#endif
