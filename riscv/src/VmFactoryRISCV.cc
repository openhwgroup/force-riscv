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
#include "VmFactoryRISCV.h"

#include "AddressTagging.h"
#include "Constraint.h"
#include "Generator.h"
#include "Log.h"
#include "PageTableConstraint.h"
#include "UtilityFunctions.h"
#include "VmDirectMapControlBlockRISCV.h"
#include "VmInfoRISCV.h"
#include "VmMapper.h"
#include "VmPaMapper.h"
#include "VmasControlBlockRISCV.h"

using namespace std;

/*!
  \file VmFactoryRISCV.cc
  \brief Code supporting construction of RISC-specific virtual memory management objects.
*/

namespace Force {

  VmRegime* VmFactoryRISCV::VmRegimeInstance() const
  {
    switch (mVmRegimeType)
    {
      case EVmRegimeType::M:
        return new VmPagingRegime(this, mVmRegimeType, EPrivilegeLevelType::M, EMemBankType::Default);
      case EVmRegimeType::S:
        return new VmPagingRegime(this, mVmRegimeType, EPrivilegeLevelType::S, EMemBankType::Default);
      case EVmRegimeType::HS:
        return new VmPagingRegime(this, mVmRegimeType, EPrivilegeLevelType::S, EMemBankType::Default);
      case EVmRegimeType::VS:
        return new VmPagingRegime(this, mVmRegimeType, EPrivilegeLevelType::S, EMemBankType::Default);
      default:
        LOG(fail) << "{VmFactoryRISCV::VmRegimeInstance} invalid regime type: " << EVmRegimeType_to_string(mVmRegimeType) << endl;
        FAIL("invalid_regime_type_vm_regime_instance");
    }

    return nullptr;
  }

  VmDirectMapControlBlock* VmFactoryRISCV::CreateVmDirectMapControlBlock(const EMemBankType memBankType) const
  {
    switch (mVmRegimeType)
    {
      case EVmRegimeType::M:
        return new VmDirectMapControlBlockRISCV(EPrivilegeLevelType::M, memBankType);
      case EVmRegimeType::S:
      case EVmRegimeType::HS:
      case EVmRegimeType::VS:
        return new VmDirectMapControlBlockRISCV(EPrivilegeLevelType::S, memBankType);
      default:
        LOG(fail) << "{VmFactoryRISCV::CreateVmDirectMapControlBlock} Unsupported EVmRegimeType enum value: " << EVmRegimeType_to_string(mVmRegimeType) << endl;
        FAIL("invalid_regime_type_vm_dmcb");
    }

    return nullptr;
  }

  VmasControlBlock* VmFactoryRISCV::CreateVmasControlBlock(const EMemBankType memBankType) const
  {
    switch (mVmRegimeType)
    {
      case EVmRegimeType::M:
        return new VmasControlBlockRISCV(EPrivilegeLevelType::M, memBankType);
      case EVmRegimeType::S:
      case EVmRegimeType::HS:
      case EVmRegimeType::VS:
        return new VmasControlBlockRISCV(EPrivilegeLevelType::S, memBankType);
      default:
        LOG(fail) << "{VmFactoryRISCV::CreateVmasControlBlock} Unsupported EVmRegimeType enum value: " << EVmRegimeType_to_string(mVmRegimeType) << endl;
        FAIL("invalid_regime_type_vmas_cb");
    }
    return nullptr;
  }


  AddressTagging* VmFactoryRISCV::CreateAddressTagging(const VmControlBlock& rVmControlBlock) const
  {
    return new AddressTagging();
  }

  PteAttribute* VmFactoryRISCV::PteAttributeInstance(EPteAttributeType attrType) const
  {
    LOG(fail) << "{VmFactoryRISCV::PteAttributeInstance} not yet implemented." << endl;
    FAIL("not-yet-implemented");

    return nullptr;
  }

  void VmFactoryRISCV::SetupVmConstraints(std::vector<ConstraintSet* >& rVmConstraints) const
  {
    check_enum_size(EVmConstraintTypeSize);
    for (EVmConstraintTypeBaseType i = 0; i < EVmConstraintTypeSize; ++ i) {
      LOG(info) << "{VmFactoryRISCV::SetupVmConstraints} instantiating " << EVmConstraintType_to_string(EVmConstraintType(i)) << " constraint." << endl;
      rVmConstraints.push_back(new ConstraintSet());
    }
  }

  EPrivilegeLevelType VmFactoryRISCV::GetPrivilegeLevel() const
  {
    switch (mVmRegimeType)
    {
      case EVmRegimeType::M:
        return EPrivilegeLevelType::M;
      case EVmRegimeType::S:
      case EVmRegimeType::HS:
      case EVmRegimeType::VS:
        return EPrivilegeLevelType::S;
      default:
        LOG(fail) << "{VmFactoryRISCV::GetPrivilegeLevel} Unsupported EVmRegimeType enum value: " << EVmRegimeType_to_string(mVmRegimeType) << endl;
        FAIL("invalid_regime_type_get_priv");
    }

    return EPrivilegeLevelType::M;
  }

  void VmFactoryRISCV::GetRegisterContext(std::vector<std::string>& rRegNames, bool pagingEnabled) const
  {
    rRegNames.push_back("mstatus");
    rRegNames.push_back("misa");
    rRegNames.push_back("privilege");

    switch (mVmRegimeType)
    {
      case EVmRegimeType::M:
        break;
      case EVmRegimeType::S:
        rRegNames.push_back("sstatus");
        if (pagingEnabled) rRegNames.push_back("satp");
        break;
      case EVmRegimeType::HS:
        break;
        //hgatp, hstatus
      case EVmRegimeType::VS:
        break;
        //vsstatus, vsatp
      default:
        LOG(fail) << "{VmFactoryRISCV::GetRegisterContext} Unsupported EVmRegimeType enum value: " << EVmRegimeType_to_string(mVmRegimeType) << endl;
        FAIL("invalid_regime_type_get_reg_context");
    }
  }

  void VmFactoryRISCV::GetDelayedRegisterContext(std::vector<std::string>& rRegNames) const
  {
  }

  bool VmFactoryRISCV::PagingEnabled(Generator& rGen) const
  {
    VmInfoRISCV vm_info;
    vm_info.SetPrivilegeLevelType(GetPrivilegeLevel());
    vm_info.GetOtherStates(rGen);
    return vm_info.PagingEnabled();
  }

  VmContext* VmFactoryRISCV::CreateVmContext() const
  {
    LOG(fail) << "{VmFactoryRISCV::CreateVmContext} not yet implemented." << endl;
    FAIL("not-yet-implemented");

    return nullptr;
  }

  void VmFactoryRISCV::CreatePageTableConstraints(std::vector<PageTableConstraint* >& rPageTableConstraints) const
  {
    for (EMemBankTypeBaseType i = 0; i < EMemBankTypeSize; ++ i)
    {
      LOG(info) << "{VmFactoryRISCV::CreatePageTableConstraints} instantiating " << EMemBankType_to_string(EMemBankType(i)) << " constraint." << endl;
      rPageTableConstraints.push_back(new PageTableConstraint(EMemBankType(i)));
    }
  }

  VmPaMapper* VmFactoryRISCV::VmPaMapperInstance(VmAddressSpace* pAddressSpace) const
  {
    return new VmPaMapper(pAddressSpace);
  }

}
