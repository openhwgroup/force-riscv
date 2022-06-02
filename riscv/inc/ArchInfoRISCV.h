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
#ifndef Force_ArchInfoRISCV_H
#define Force_ArchInfoRISCV_H

#include "ArchInfoBase.h"

namespace Force {

  /*!
    \class ArchInfoRISCV
    \brief ArchInfo object for RISC-V architecture
  */

  class ArchInfoRISCV : public ArchInfoBase {
  public:
    explicit ArchInfoRISCV(const std::string& name); //!< Constructor
    std::list<EVmRegimeType> VmRegimeTypes() const override; //!< Return all applicable virtual memory regime types.
    ~ArchInfoRISCV() { } //!< Destructor.
    void Setup() override; //!< Setup necessary details RISCV architecture details for the ArchInfo object.
    uint32 ElfMachineType() const override { return 0xF3; } //!< Return RISC-V ELF machine type.
    const char* DefaultConfigFile() const override { return "config/riscv_rv64.config"; } //!< Return the default config file name.
    PhysicalPageManager* InstantiatePhysicalPageManager(EMemBankType bankType, MemoryTraitsManager* pMemTraitsManager) const override; //!< Instantiate a RISC-V architecture PhysicalPageManager object.
    MemoryTraitsRegistry* InstantiateMemoryTraitsRegistry() const override; //!< Instantiate a RISC-V architecture MemoryTraitsRegistry object.
  protected:
    Generator* InstantiateGenerator() const override; //!< Instantiate a RISC-V architecture Generator object.
    VmManager* InstantiateVmManager() const override; //!< Instantiate a RISC-V architecture VmManager object.
    GenAgent* InstantiateGenAgent(EGenAgentType agentType) const override; //!< Instantiate a GenAgent object based on the ArchInfo type and the passed in agent-type parameter.
    BootOrder* InstantiateBootOrder() const override;  //<! Instantiate a RISC-V architecture BootOrder object.
    RegisterFile* InstantiateRegisterFile() const override; //!< Instantiate a RISC-V architecture RegisterFile.
    ExceptionRecordManager* InstantiateExceptionRecordManager() const override; //!< Instantiate a RISC-V architecture exception records object.
    GenConditionSet* InstantiateGenConditionSet() const override; //!< Instantiate a RISC-V architecture GenConditionSet object.
    PageRequestRegulator* InstantiatePageRequestRegulator() const override; //!< Instantiate a RISC-V architecture PageRequestRegulator object.
    AddressFilteringRegulator* InstantiateAddressFilteringRegulator() const override; //!< Instantiate a RISC-V architecture AddressFilteringRegulator object.
    AddressTableManager* InstantiateAddressTableManager() const override; //!< Instantiate a AddressTableManager object based on the ArchInfo type.
    AddressSolutionFilter* InstantiateAddressSolutionFilter(EAddressSolutionFilterType filterType) const override; //!< Instantiate a AddressSolutionFilter object based on the ArchInfo type and the passed in filterType parameter.
 private:
  };

}

#endif
