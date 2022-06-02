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
#ifndef Force_GenCallBackAgent_H
#define Force_GenCallBackAgent_H

#include <map>

#include "Defines.h"
#include "Enums.h"
#include "GenAgent.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class Generator;
  class GenCallBackRequest;
  class BntNode;

  /*!
    \class GenInstructionAgent
    \brief A generator agent class to handle instruction generation.
  */
  class GenCallBackAgent : public GenAgent {
  public:
    explicit GenCallBackAgent(Generator* gen) : GenAgent(gen), mpCallBackRequest(nullptr), mLastSequenceName() { }  //!< Constructor with Generator pointer parameter.
    GenCallBackAgent() : GenAgent(), mpCallBackRequest(nullptr), mLastSequenceName() {} //!< default constructor

    Object* Clone() const override;  //!< Return a cloned GenCallBackAgent object of the same type and content.
    const char* Type() const override { return "GenCallBackAgent"; } //!< Return type of the GenCallBackAgent object.

    ASSIGNMENT_OPERATOR_ABSENT(GenCallBackAgent);
    EGenAgentType GenAgentType() const override { return EGenAgentType::GenCallBackAgent; } //!< Return type of the generator agent.
    void SetGenRequest(GenRequest* genRequest) override; //!< Set pointer to GenRequest object.
  protected:
    GenCallBackAgent(const GenCallBackAgent& rOther); //!< Copy constructor.
    void HandleRequest() override; //!< Handle GenRequest transaction.

    void ProcessAccurateBntNode(); //!< process accurate Bnt Node
    void ProcessEretPreambleSequence(); //!< process eret preamble sequence
    uint32 CallBackTemplate(ECallBackTemplateType callBackType, const std::string& primaryValue, const std::map<std::string, uint64>& callBackValues = std::map<std::string, uint64>()); //!< call back some function on template

    GenCallBackRequest* mpCallBackRequest; //!< pointer to callback request
    std::string mLastSequenceName; //!< last sequenceName to improve performance
  };
}
#endif
