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
#ifndef Force_GenStateTransitionAgent_H
#define Force_GenStateTransitionAgent_H

#include "Defines.h"
#include "Enums.h"
#include "GenAgent.h"

namespace Force {

  class GenStateTransitionRequest;

  /*!
    \class GenStateTransitionAgent
    \brief A generator agent class for StateTransitions.
  */
  class GenStateTransitionAgent : public GenAgent {
  public:
    GenStateTransitionAgent();
    SUBCLASS_DESTRUCTOR_DEFAULT(GenStateTransitionAgent);
    ASSIGNMENT_OPERATOR_ABSENT(GenStateTransitionAgent);

    Object* Clone() const override { return new GenStateTransitionAgent(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "GenStateTransitionAgent"; } //!< Return a string describing the actual type of the Object.

    EGenAgentType GenAgentType() const override { return EGenAgentType::GenStateTransitionAgent; } //!< Return type of the generator agent.
    void SetGenRequest(GenRequest* genRequest) override; //!< Set pointer to GenRequest object.
  protected:
    GenStateTransitionAgent(const GenStateTransitionAgent& rOther);
    void HandleRequest() override; //!< Handle GenStateTransitionRequest transaction.
    void CleanUpRequest() override; //!< Clean up request item.
  private:
    GenStateTransitionRequest* mpStateTransReq;
  };

}

#endif
