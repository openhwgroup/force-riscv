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
#ifndef Force_VmFactory_H
#define Force_VmFactory_H

#include <vector>

#include "Enums.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class VmControlBlock;
  class VmDirectMapControlBlock;
  class AddressTagging;
  class VmRegime;
  class PteAttribute;
  class VmasControlBlock;
  class ConstraintSet;
  class VmContext;
  class PageTableConstraint;
  class VmAddressSpace;
  class VmPaMapper;
  class Generator;

  /*!
    \class VmFactory
    \brief Class for constructing virtual memory management objects.
  */
  class VmFactory {
  public:
    explicit VmFactory(const EVmRegimeType vmRegimeType) : mVmRegimeType(vmRegimeType) { } //!< Constructor with regime type parameter.
    virtual ~VmFactory() = default; //!< Destructor
    VmFactory& operator=(const VmFactory& rOther) = delete; //!< Copy assignment operator not currently needed.

    virtual VmDirectMapControlBlock* CreateVmDirectMapControlBlock(const EMemBankType memBankType) const = 0; //!< Create an appropriate VmDirectMapControlBlock instance.
    virtual VmasControlBlock* CreateVmasControlBlock(const EMemBankType memBankType) const = 0; //!< Create an appropriate VmasControlBlock instance.
    virtual AddressTagging* CreateAddressTagging(const VmControlBlock& rVmControlBlock) const = 0; //!< Create an appropriate AddressTagging instance.
    virtual VmRegime* VmRegimeInstance() const = 0; //!< Return a VmRegime object based on the regime enum type.
    virtual PteAttribute* PteAttributeInstance(EPteAttributeType attrType) const = 0; //!< Return an appropriate PteAttribute instance.
    virtual void SetupVmConstraints(std::vector<ConstraintSet* >& rVmConstraints) const = 0; //!< Setup VM Constraints.
    virtual VmContext* CreateVmContext() const = 0; //!< Setup VmContext.
    virtual void CreatePageTableConstraints(std::vector<PageTableConstraint* >& rPageTableConstraints) const = 0; //!< Setup Page Table constraints.
    virtual VmPaMapper* VmPaMapperInstance(VmAddressSpace* pAddressSpace) const = 0; //!< Create an appropriate VmPaMapper instance.
    virtual EPrivilegeLevelType GetPrivilegeLevel() const = 0; //!< Return privilege level of Regime
    virtual void GetRegisterContext(std::vector<std::string>& rRegNames, bool pagingEnabled) const = 0; //!< Update rRegNames with list of context register names
    virtual void GetDelayedRegisterContext(std::vector<std::string>& rRegNames) const = 0; //!< Update rRegNames with list of delayed init context register names
    virtual bool PagingEnabled(Generator& rGen) const = 0; //!< Return bool indicating whether or not paging is enabled
  protected:
    VmFactory() : mVmRegimeType(EVmRegimeType(0))  { } //!< Default constructor.
    VmFactory(const VmFactory& rOther) = delete; //!< Copy constructor not currently needed.
  protected:
    const EVmRegimeType mVmRegimeType; //!< Regime type used to resolve the appropriate subclasses to create.
  };

}

#endif  // Force_VmFactory_H
