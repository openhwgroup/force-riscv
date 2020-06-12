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
#ifndef Force_ArchInfoBase_H
#define Force_ArchInfoBase_H

#include <Architectures.h>

namespace Force {

  /*!
    \class ArchInfoBase
    \brief More concrete class implementing some common behaviors in the base layer.
  */
  class ArchInfoBase : public ArchInfo {
  public:
    explicit ArchInfoBase(const std::string& name); //!< Constructor.
    ~ArchInfoBase(); //!< Destructor.
    Generator* CreateGenerator(uint32 threadId) const override; //!< Base implementation of CreateGenerator.
    std::list<EMemBankType> MemoryBankTypes() const override; //!< Return all memory bank types.
    std::list<EVmRegimeType> VmRegimeTypes() const override; //!< Return all applicable virtual memory regime types.
    void SetupSimAPIs() override; //!< Setup simulator APIs.
  protected:
    GenAgent* InstantiateGenAgent(EGenAgentType agentType) const override; //!< Instantiate a GenAgent object based on the ArchInfo type and the passed in agentType parameter.
    void AssignGenAgents(Generator* pGen) const override; //!< Assign GenAgents to the Generator.
    void AssignAddressSolutionFilters(Generator* pGen) const override; //!< Assign AddressSolutionFilters to the Generator.
    AddressSolutionFilter* InstantiateAddressSolutionFilter(EAddressSolutionFilterType filterType) const override; //!< Instantiate a AddressSolutionFilter object based on the ArchInfo type and the passed in filterType parameter.
  };

}

#endif
