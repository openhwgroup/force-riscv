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
#include <GenRequest.h>
#include <Log.h>
#include <Constraint.h>

#include <pybind11/pybind11.h>

/*!
  \file GenRequestResults
  \brief Module specifically designated to host GenRequest::GetResults method.
*/

using namespace std;

namespace Force {

  void GenPhysicalRegionRequest::GetResults(py::object& rPyObject) const
  {
    rPyObject = py::int_(mBaseAddress);
  }

  void GenVmContextRequest::GetResults(py::object& rPyObject) const
  {
    rPyObject = py::int_(mId);
  }


  void GenSystemCall::GetResults(py::object& rPyObject) const
  {
    py::dict resDict;
    for (auto &item : mSysCallResults)
    {
      resDict[py::cast(item.first)] = py::int_(item.second);
    }
    rPyObject = resDict;
  }

  void GenUpdateHandlerInfo::GetResults(py::object& rPyObject) const
  {
    rPyObject = py::int_(mUpdateHandlerResult);
  }

  void GenLoopRequest::GetResults(py::object& rPyObject) const
  {
    py::tuple result_tuple = py::tuple(2);
    result_tuple[0] = py::int_(mLoopId);
    result_tuple[1] = py::int_(mLoopBackAddress);
    rPyObject = result_tuple;
  }

  void GenLinearBlockRequest::GetResults(py::object& rPyObject) const
  {
    if (mActionType == EGenStateActionType::Push) {
      rPyObject = py::int_(mBlockId);
    }
    else {
      py::tuple result_tuple = py::tuple(2);
      result_tuple[0] = py::int_(mValue);
      result_tuple[1] = py::int_(mEmpty);
      rPyObject = result_tuple;
    }
  }

  void GenBntHookRequest::GetResults(py::object& rPyObject) const
  {
    rPyObject = py::int_(mBntId);
  }

  void GenFreePageRequest::GetResults(py::object& rPyObject) const
  {
    auto tuple_num = 3 + mPageNum;
    py::tuple result_tuple = py::tuple(tuple_num);

    result_tuple[0] = mValid;
    result_tuple[1] = mStartAddr;
    result_tuple[2] = mpResolvedRanges->ToSimpleString();
    for (auto i = 0u; i < mPageNum; i ++ )
      result_tuple[3 + i] = mResolvedPageSizes[i];

    rPyObject = result_tuple;
  }

  void GenUpdateVmRequest::GetResults(py::object& rPyObject) const
  {
    rPyObject = py::none();
  }

}
