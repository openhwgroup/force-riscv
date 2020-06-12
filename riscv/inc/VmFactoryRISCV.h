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
#ifndef Force_VmFactoryRISCV_H
#define Force_VmFactoryRISCV_H

#include <VmFactory.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

namespace Force {

  /*!
    \class VmFactoryRISCV
    \brief Class for constructing RISC-specific virtual memory management objects.
  */
  class VmFactoryRISCV : public VmFactory {
  public:
    explicit VmFactoryRISCV(const EVmRegimeType vmRegimeType) : VmFactory(vmRegimeType) { } //!< Constructor
    ~VmFactoryRISCV() override = default; //!< Destructor
    VmFactoryRISCV& operator=(const VmFactoryRISCV& rOther) = delete; //!< Copy assignment operator not currently needed.

    VmDirectMapControlBlock* CreateVmDirectMapControlBlock(const EMemBankType memBankType) const override; //!< Create an appropriate VmDirectMapControlBlock instance.
    VmasControlBlock* CreateVmasControlBlock(const EMemBankType memBankType) const override; //!< Create an appropriate VmasControlBlock instance.
    AddressTagging* CreateAddressTagging(const VmControlBlock& rVmControlBlock) const override; //!< Create an appropriate AddressTagging instance.
    VmRegime* VmRegimeInstance() const override; //!< Return an RISCV VmRegime object based on the regime enum type.
    PteAttribute* PteAttributeInstance(EPteAttributeType attrType) const override; //!< Return an appropriate PteAttribute instance.
    void SetupVmConstraints(std::vector<ConstraintSet* >& rVmConstraints) const override; //!< Setup RISCV VM Constraints.
    VmContext* CreateVmContext() const override; //!< Setup VmContext.
    void CreatePageTableConstraints(std::vector<PageTableConstraint* >& rPageTableConstraints) const override; //!< Setup Page Table constraints.
    VmPaMapper* VmPaMapperInstance(VmAddressSpace* pAddressSpace) const override; //!< Return an RISCV VmPaMapper instance.
    EPrivilegeLevelType GetPrivilegeLevel() const override; //!< Return privilege level of Regime
    void GetRegisterContext(std::vector<std::string>& rRegNames, bool pagingEnabled) const override; //!< Update rRegNames with list of context register names
    void GetDelayedRegisterContext(std::vector<std::string>& rRegNames) const override; //!< Update rRegNames with list of delayed init context register names
    bool PagingEnabled(Generator& rGen) const override; //!< Return bool indicating whether or not paging is enabled
  protected:
    VmFactoryRISCV(const VmFactoryRISCV& rOther) = delete; //!< Copy constructor not currently needed.
    VmFactoryRISCV() = default; //!< Default constructor.
  };

}

#endif  // Force_VmFactoryRISCV_H
