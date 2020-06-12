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
#ifndef Force_GenStateAgent_H
#define Force_GenStateAgent_H

#include <Defines.h>
#include <GenAgent.h>

namespace Force {

  class GenStateRequest;

  /*!
    \class GenStateAgent
    \brief A generator agent class to handle generator state modifications.
  */
  class GenStateAgent : public GenAgent {
  public:
    explicit GenStateAgent(Generator* gen) : GenAgent(gen), mpStateRequest(nullptr) { } //!< Constructor with Generator pointer parameter.
    GenStateAgent() : GenAgent(), mpStateRequest(nullptr) { } //!< Constructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenStateAgent);
    Object* Clone() const override;  //!< Return a cloned GenStateAgent object of the same type and content.
    const char* Type() const override { return "GenStateAgent"; } //!< Return type of the GenStateAgent object.

    EGenAgentType GenAgentType() const override { return EGenAgentType::GenStateAgent; } //!< Return type of the generator agent.
    void SetGenRequest(GenRequest* genRequest) override; //!< Set pointer to GenRequest object.
  protected:
    GenStateAgent(const GenStateAgent& rOther) : GenAgent(rOther), mpStateRequest(nullptr) { } //!< Copy constructor, do not copy the request pointer.
    void HandleRequest() override; //!< Handle GenStateRequest transaction.
    void CleanUpRequest() override; //!< Clean up request item.
    void ResetRequest() override; //!< Reset request item.

    void PushState(); //!< Push a generator state.
    void PopState(); //!< Pop a generator state.
    void SetState(); //!< Set a generator state.
    void EnableState(); //!< Enable a generator state.
    void DisableState(); //!< Disable a generator state.
    void BeginLoop(); //!< Begin loop processing.
    void EndLoop(); //!< End loop processing.
    void BeginLinearBlock(); //!< Begin linear block processing.
    void EndLinearBlock(); //!< End linear block processing.
    void UpdatePcOnISS(uint64 pc); //!< update pc value on ISS
    void PushBntHook();  //!< Push bnt state
    void RevertBntHook(); //!< revert bnt state to last one
    bool IsRequestSpeculative() const; //!< whether the request is speculative or not
  protected:
    GenStateRequest* mpStateRequest; //!< Pointer to GenStateRequest object.
  };

}

#endif
