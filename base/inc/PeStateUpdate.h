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
#ifndef Force_PeStateUpdate_H
#define Force_PeStateUpdate_H

#include <vector>

#include "Defines.h"
#include "Object.h"

namespace Force {

  class GenRequest;

  /*!
    \class PeStateUpdate
    \brief maintain a list of requests (in order) to update current state later
   */

  class PeStateUpdate : public Object {
  public:
    Object* Clone() const override;  //!< Return a cloned PeStateUpdate object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the PeStateUpdate object.
    const char* Type() const override { return "PeStateUpdate"; } //!< Return the type of the object

    PeStateUpdate(); //!< Constructor.
    ~PeStateUpdate(); //!< Destructor.

    std::vector<GenRequest*>& GetRequests() { return mRequestList; }

    void UpdateState(const std::string& stateName, const std::string& actionName, uint64 value); //!< request to generate a state update request
    void UpdateRegisterField(const std::string& regName, const std::string& fieldName, uint64 value); //!< request to generate a register field update request

  protected:
    void AddRequest(GenRequest* request);  //!< add given request to the request list

  private:
    PeStateUpdate(const PeStateUpdate& rOther); //!< Copy constructor hidden.
    std::vector<GenRequest*> mRequestList;  //!< a list of requests to be processed
  };
}

#endif
