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
#ifndef Force_FrontEndCall_H
#define Force_FrontEndCall_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <map>

namespace Force {

  class PyInterface;

  /*
    \ class FrontEndCall
    \ brief class to call functions on front-end
  */

  class FrontEndCall {
  public:
    static void Initialize();  //!< Initialization interface.
    static void Destroy();     //!< Destruction clean up interface.
    inline static FrontEndCall* Instance() { return mspFrontEnd; } //!< Access instance.
    ASSIGNMENT_OPERATOR_ABSENT(FrontEndCall);
    COPY_CONSTRUCTOR_DEFAULT(FrontEndCall);
    inline void SetInterface(PyInterface* interface) { mpPyInterface = interface; } //!< set interface
    uint32 CallBackTemplate(uint32 threadId, ECallBackTemplateType callBackType,  const std::string& primaryValue, const std::map<std::string, uint64>& callBackValues); //!< call back some function on front-end
  private:
    FrontEndCall() : mpPyInterface(nullptr) { } //!< constructor
    virtual ~FrontEndCall()  { } //!< destructor

    static FrontEndCall* mspFrontEnd;  //!< Pointer to singleton FrontEndCall object.
    PyInterface* mpPyInterface; //!< Pointer to the Python interface instance.
  };
}

#endif
