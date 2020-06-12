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
#include <ArchInfoRISCV.h>
#include <GeneratorRISCV.h>

#include <GenQueryAgent.h>
#include <Register.h>
#include <GenExceptionAgentRISCV.h>
#include <GenInstructionAgent.h>
#include <ExceptionRecords.h>
#include <GenCondition.h>
#include <AddressFilteringRegulator.h>
#include <AddressSolutionFilter.h>

//#include <GenQueryAgentRISCV.h>
//#include <GenInstructionAgentRISCV.h>
//#include <ExceptionRecordsRISCV.h>
//#include <GenConditionRISCV.h>
#include <AddressFilteringRegulatorRISCV.h>

#include <GenSequenceAgentRISCV.h>
#include <BootOrderRISCV.h>
#include <RegisterRISCV.h>
#include <VmManagerRISCV.h>
#include <AddressTableManagerRISCV.h>
#include <PhysicalPageManagerRISCV.h>
#include <PageRequestRegulatorRISCV.h>
#include <Log.h>

using namespace std;

namespace Force {

  ArchInfoRISCV::ArchInfoRISCV(const std::string& name)
    : ArchInfoBase(name)
  {
  }

  void ArchInfoRISCV::Setup()
  {
    mMemoryBanks.push_back(EMemBankType::Default);
  }

  Generator* ArchInfoRISCV::InstantiateGenerator() const
  {
    return new GeneratorRISCV();
  }

  std::list<EVmRegimeType> ArchInfoRISCV::VmRegimeTypes() const
  {
    list<EVmRegimeType> regime_types;
    regime_types.push_back(EVmRegimeType::M);
    regime_types.push_back(EVmRegimeType::S);
    regime_types.push_back(EVmRegimeType::HS);
    regime_types.push_back(EVmRegimeType::VS);

    return regime_types;
  }

  VmManager* ArchInfoRISCV::InstantiateVmManager() const
  {
    return new VmManagerRISCV();
  }

  GenAgent* ArchInfoRISCV::InstantiateGenAgent(EGenAgentType agentType) const
  {
    switch (agentType) {
    case EGenAgentType::GenExceptionAgent:
      return new GenExceptionAgentRISCV();
    case EGenAgentType::GenSequenceAgent:
      return new GenSequenceAgentRISCV();
      /*
    case EGenAgentType::GenQueryAgent:
      return new GenQueryAgentRISCV();
    case EGenAgentType::GenInstructionAgent:
      return new GenInstructionAgentRISCV();
      */
    default:
      return ArchInfoBase::InstantiateGenAgent(agentType);
    }

    return nullptr;
  }

  BootOrder* ArchInfoRISCV::InstantiateBootOrder() const
  {
    return new BootOrderRISCV();
  }

  RegisterFile* ArchInfoRISCV::InstantiateRegisterFile() const
  {
    return new RegisterFileRISCV();
  }

  ExceptionRecordManager* ArchInfoRISCV::InstantiateExceptionRecordManager() const
  {
    return new ExceptionRecordManager();
    // return new ExceptionRecordManagerRISCV();
  }

  GenConditionSet* ArchInfoRISCV::InstantiateGenConditionSet() const
  {
    return new GenConditionSet();
    // return new GenConditionSetRISCV();
  }

  PhysicalPageManager* ArchInfoRISCV::InstantiatePhysicalPageManager(EMemBankType bankType) const
  {
    return new PhysicalPageManagerRISCV(bankType);
  }

  PageRequestRegulator* ArchInfoRISCV::InstantiatePageRequestRegulator() const
  {
    return new PageRequestRegulatorRISCV();
  }

  AddressFilteringRegulator* ArchInfoRISCV::InstantiateAddressFilteringRegulator() const
  {
    return new AddressFilteringRegulatorRISCV();
  }

  AddressTableManager* ArchInfoRISCV::InstantiateAddressTableManager() const
  {
    return new AddressTableManagerRISCV();
  }

  AddressSolutionFilter* ArchInfoRISCV::InstantiateAddressSolutionFilter(EAddressSolutionFilterType filterType) const
  {
    switch (filterType) {
    case EAddressSolutionFilterType::BaseDependency:
      return new BaseDependencyFilter();
    case EAddressSolutionFilterType::IndexDependency:
      return new IndexDependencyFilter();
    case EAddressSolutionFilterType::SpAlignment:
      return new SpAlignmentFilter();
    default:
      LOG(fail) << "{ArchInfoRISCV::InstantiateAddressSolutionFilter} unsupported AddressSolutionFilter type: " << EAddressSolutionFilterType_to_string(filterType) << endl;
      FAIL("unsupported-address-solution-filter-type");
    }

    return nullptr;
  }
}
