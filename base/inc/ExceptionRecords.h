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
#ifndef Force_ExceptionRecords_H
#define Force_ExceptionRecords_H

#include <map>
#include <vector>

#include "Defines.h"
#include "Enums.h"
#include "Object.h"
#include ARCH_ENUM_HEADER

namespace Force {

   /*!
    \class ExceptionRecord
    \brief A single record that contains information about the exeption event. Managed by ExceptionRecordManager.
  */
  struct ExceptionRecord {
    // initialize with invalid value, so that we can say which field is turned on in combination search
    uint32  exception_code; //!< Exception Class.
    uint64  pc_value; //!< Program Counter.
    uint32  src_exception_level; //!< Source Exception Level
    uint32  tgt_exception_level; //!< Target Exception Level
    uint32  dfsc_ifsc_code; //!< Fault Status Code
  };

   /*!
    \class ExceptionRecordManager
    \brief Manager that records the occurance of any exception events.
  */
  class ExceptionRecordManager : public Object {
  public:
    Object* Clone() const override; //!< Clone a ExceptionRecordManager object.
    const std::string ToString() const override; //!< Return a string describing the current state of the ExceptionRecordManager object.
    const char* Type() const override { return "ExceptionRecordManager"; } //!< Return object type in a C string.

    ExceptionRecordManager(); //!< Default constructor.
    ~ExceptionRecordManager(); //!< Destructor.

    void ReportNewExceptionRecord(const ExceptionRecord& exception_record); //!< Used by the exception space to record any new exceptions.
    void GetExceptionHistoryByEC(EExceptionClassType exceptionType, std::vector<ExceptionRecord> &output); //!< Returns a list of exception events, sorted in chronological order, in the provided output vector
    void GetExceptionHistoryByArgs(const std::map<std::string, std::string>& inputArgs, std::vector<ExceptionRecord> &output); //!< Returns a list of exception events, sorted in chronological order, in the provided output vector
  protected:
    ExceptionRecordManager(const ExceptionRecordManager& rOther); //!< Copy constructor.
  protected:
    std::vector<ExceptionRecord> mRecords; //!< List maintaining the chronological history of all exceptions.
  };

}

#endif


