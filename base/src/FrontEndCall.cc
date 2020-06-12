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
#include <FrontEndCall.h>
#include <PyInterface.h>

/*!
  \ file FrontEndCall.cc
  \ brief call for call functions on front end
*/

namespace Force {

  FrontEndCall* FrontEndCall::mspFrontEnd = nullptr;

  void FrontEndCall::Initialize()
  {
    if (nullptr == mspFrontEnd) {
      mspFrontEnd = new FrontEndCall();
    }
  }

  void FrontEndCall::Destroy()
  {
    delete mspFrontEnd;
    mspFrontEnd = nullptr;
  }

  uint32 FrontEndCall::CallBackTemplate(uint32 threadId, ECallBackTemplateType callBackType,  const std::string& primaryValue, const std::map<std::string, uint64>& callBackValues )
  {
    return mpPyInterface->CallBackTemplate(threadId, callBackType, primaryValue, callBackValues);
  }

}
