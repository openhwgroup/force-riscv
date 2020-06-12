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
#include <ExceptionRecords.h>
#include <Constraint.h>
#include <Log.h>

#include <algorithm>

using namespace std;

namespace Force {

  ExceptionRecordManager::ExceptionRecordManager()
    : Object(), mRecords()
  {
  }

  ExceptionRecordManager::ExceptionRecordManager(const ExceptionRecordManager& rOther)
    : Object(rOther), mRecords()
  {

  }

  ExceptionRecordManager::~ExceptionRecordManager()
  {
  }

  const string ExceptionRecordManager::ToString() const
  {
    return Type();
  }

  Object* ExceptionRecordManager::Clone() const
  {
    return new ExceptionRecordManager(*this);
  }

  void ExceptionRecordManager::ReportNewExceptionRecord(const ExceptionRecord& exception_record)
  {
    LOG(notice) << "New Exception Recorded: " << EExceptionClassType_to_string(EExceptionClassType(exception_record.exception_code)) << ". Exception taken to level " << exception_record.tgt_exception_level <<"." << endl;

    /*
     * The idea is that the user will call into the simpler API (the one that returns just an integer) more often,
     * so it makes sense to maintain the same data in two structures to allow for more optimized lookup. The map provides
     * the faster look up time for specific exception counts, and the list allows us to retrieve the chronological history of
     * the exception events, if we need to look that up.
     */
    mRecords.push_back(exception_record);
    return;
  }

  // find Exception history by Exception Code ( exception code )
  void ExceptionRecordManager::GetExceptionHistoryByEC(EExceptionClassType exceptionClass, vector<ExceptionRecord> &output)
  {
    /* See if this exception event is in our history */
    copy_if(mRecords.cbegin(), mRecords.cend(), back_inserter(output),
      [exceptionClass](const ExceptionRecord& rExceptionRecord) { return (rExceptionRecord.exception_code == EExceptionClassTypeBaseType(exceptionClass)); });
  }

  // This is the combination seach by arguments, if a field in the sample is not explicitly set, the field will not be searched.
  void ExceptionRecordManager::GetExceptionHistoryByArgs(const map<string, string>& inputArgs, vector<ExceptionRecord> &output)
  {
    /* See if this sample exception event field(s) is in our history */
    if (inputArgs.find("Last") != inputArgs.end() && inputArgs.at("Last") == "True")
    {
      for (auto itr = mRecords.rbegin(); itr != mRecords.rend(); ++itr )
      {
        if (inputArgs.find("EC") != inputArgs.end() && !ConstraintSet(inputArgs.at("EC")).ContainsValue((*itr).exception_code))
          continue;
        if (inputArgs.find("PC") != inputArgs.end() && !ConstraintSet(inputArgs.at("PC")).ContainsValue((*itr).pc_value))
          continue;
        if (inputArgs.find("SRC_PRIVLEV") != inputArgs.end() && !ConstraintSet(inputArgs.at("SRC_PRIVLEV")).ContainsValue((*itr).src_exception_level))
          continue;
        if (inputArgs.find("TGT_PRIVLEV") != inputArgs.end() && !ConstraintSet(inputArgs.at("TGT_PRIVLEV")).ContainsValue((*itr).tgt_exception_level))
          continue;
        if (inputArgs.find("FSC") != inputArgs.end() && !ConstraintSet(inputArgs.at("FSC")).ContainsValue((*itr).dfsc_ifsc_code))
          continue;
        output.push_back(*itr);
        break;
      }
    }
    else
    {
      for (auto exception_record : mRecords)
      {
        if (inputArgs.find("EC") != inputArgs.end() && !ConstraintSet(inputArgs.at("EC")).ContainsValue(exception_record.exception_code))
          continue;
        if (inputArgs.find("PC") != inputArgs.end() && !ConstraintSet(inputArgs.at("PC")).ContainsValue(exception_record.pc_value))
          continue;
        if (inputArgs.find("SRC_PRIVLEV") != inputArgs.end() && !ConstraintSet(inputArgs.at("SRC_PRIVLEV")).ContainsValue(exception_record.src_exception_level))
          continue;
        if (inputArgs.find("TGT_PRIVLEV") != inputArgs.end() && !ConstraintSet(inputArgs.at("TGT_PRIVLEV")).ContainsValue(exception_record.tgt_exception_level))
          continue;
        if (inputArgs.find("FSC") != inputArgs.end() && !ConstraintSet(inputArgs.at("FSC")).ContainsValue(exception_record.dfsc_ifsc_code))
          continue;
        output.push_back(exception_record);
      }
    }
  }


}

