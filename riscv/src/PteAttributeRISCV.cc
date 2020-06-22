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
#include <PteAttributeRISCV.h>
#include <GenRequest.h>
#include <Constraint.h>
#include <VmAddressSpace.h>
#include <VmasControlBlockRISCV.h>
#include <Page.h>
#include <ChoicesFilter.h>
#include <Random.h>
#include <PagingChoicesAdapter.h>
#include <Generator.h>
#include <Register.h>
#include <PteStructure.h>
#include <Choices.h>
#include <UtilityFunctions.h>
#include <Log.h>

#include <memory>
#include <algorithm>
#include <sstream>

using namespace std;

namespace Force
{

  void ValidPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    uint32 level = rPte.ParentTableLevel();
    mValue = mpStructure->Value();

    // check for constraint passed along from the page-request parameter, apply choices filter if necessary.
    auto req_constr = rPagingReq.GenAttributeConstraint(EPageGenAttributeType::Invalid);
    if (nullptr != req_constr)
    {
      uint32 req_invalid = req_constr->ChooseValue();
      LOG(notice) << "{ValidPteAttributeRISCV::Generate} requesting invalid? " << req_invalid << endl;
      if (req_invalid == 0) {
        return; // request valid PTE.
      }
    }

    auto choices_raw = rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Invalid Descriptor", level);
    std::unique_ptr<ChoiceTree> choices_tree(choices_raw);
    auto chosen_ptr = choices_tree->Choose();
    auto chosen_value = chosen_ptr->Value();
    if (chosen_value != 1) // page translation fault is selected
    {
      rPte.SetPageGenAttribute(EPageGenAttributeType::Invalid, 1);
      mValue = chosen_value;
      LOG(notice) << "{ValidPteAttributeRISCV::Generate} " << "Level= " << level << " Choice value=" << chosen_value << " Original Correct Value=" << mpStructure->Value() << " Modified Value=" << mValue << endl;
    }
  }

  void AddressPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    //uint32 level = rPte.ParentTableLevel();
    LOG(trace) << "{AddressPteAttributeRISCV::Generate} phys_lower=0x" << hex << rPte.PhysicalLower() << " mask=0x" << mpStructure->Mask() << " lsb=0x" << mpStructure->Lsb() << endl;
    mValue = (rPte.PhysicalLower() >> (mpStructure->Lsb() + 2)) & mpStructure->Mask(); //TODO get hardcoded shift programatically
   
    //TODO rework into address error range fault, (i.e. top VA bits not all equal)
    //uint32 addr_fault_value = 0;
    
    /*auto req_constr = rPagingReq.GenAttributeConstraint(EPageGenAttributeType::AddrSizeFault);
    if (nullptr != req_constr) // has addr size fault constraint in page request
    {
      uint32 req_fault = req_constr->ChooseValue();
      if (req_fault != 0) addr_fault_value = 1;
    }
    else // choose from paging choices
    {
      auto choices_raw = rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Address size fault", level);
      std::unique_ptr<ChoiceTree> choices_tree(choices_raw);
      auto chosen_ptr = choices_tree->Choose();
      addr_fault_value = chosen_ptr->Value();
    }*/

    //rPte.SetPageGenAttribute(EPageGenAttributeType::AddrSizeFault, addr_fault_value);

    /*if (addr_fault_value != 0)
    {
      LOG(notice) << "{AddressPteAttributeRISCV::Generate} requesting address size fault level=" << level << endl;
      uint64 random_value = Random::Instance()->Random64(rVmas.GetControlBlock()->MaxPhysicalAddress() + 1, MAX_UINT64);
      random_value &= ~rVmas.GetControlBlock()->MaxPhysicalAddress();
      mValue = (((random_value | rPte.PhysicalLower()) >> (mpStructure->Lsb() + 2)) & mpStructure->Mask()); //TODO get hardcoded shift programatically
    }
    else
    {*/
    //}
  }

  void DAPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    //TODO this class is only assigned for leaf PTE attributes, D/A bits are res0 for table entries.
    //If we can get category of PteStructure here, can programatically assign for all pte categories
    auto mem_access = rPagingReq.MemoryAccessType();

    switch (mem_access)
    {
      case EMemAccessType::Read:
        mValue = 0x1;
        break;
      case EMemAccessType::ReadWrite:
      case EMemAccessType::Write:
        mValue = 0x3; //set dirty for writeable pages (anything with store access).
        break;
      case EMemAccessType::Unknown:
        mValue = 0x1;
        LOG(info) << "{DAPteAttributeRISCV::Generate} unknown mem access type, using default val of D=0, A=1" << endl;
        break;
      default:
        LOG(fail) << "{DAPteAttributeRISCV::Generate} invalid or unsupported mem access type=" << EMemAccessType_to_string(mem_access) << endl;
        FAIL("da_pte_gen_invalid_mem_access");
        break;
    }
  }

  void XPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    auto mem_access = rPagingReq.MemoryAccessType();
    //bool user_access = (rVmas.GetControlBlock()->PrivilegeLevel() == EPrivilegeLevelType::U);
    bool user_access = rPagingReq.PrivilegeLevelSpecified() and (rPagingReq.PrivilegeLevel() == EPrivilegeLevelType::U);
    uint32 iap = 0;

    LOG(trace) << "{RPteAttributeRISCV::Generate} user_access=" << user_access << endl;

    switch (mem_access)
    {
      case EMemAccessType::Read:
        mValue = 1;
        iap = user_access ? uint32(EInstrAccessPermissionType::PrivilegedNoExecute) : uint32(EInstrAccessPermissionType::Execute);
        rPte.SetPageGenAttribute(EPageGenAttributeType::InstrAccessPermission, iap);
        break;
      case EMemAccessType::ReadWrite:
      case EMemAccessType::Write:
        mValue = 0;
        rPte.SetPageGenAttribute(EPageGenAttributeType::InstrAccessPermission, uint32(EInstrAccessPermissionType::NoExecute));
        break;
      case EMemAccessType::Unknown:
        mValue = 1;
        iap = user_access ? uint32(EInstrAccessPermissionType::PrivilegedNoExecute) : uint32(EInstrAccessPermissionType::Execute);
        rPte.SetPageGenAttribute(EPageGenAttributeType::InstrAccessPermission, iap);
        LOG(info) << "{XPteAttributeRISCV::Generate} unknown mem access type, using default val of 1" << endl;
        break;
      default:
        LOG(fail) << "{XPteAttributeRISCV::Generate} invalid or unsupported mem access type=" << EMemAccessType_to_string(mem_access) << endl;
        FAIL("x_pte_gen_invalid_mem_access");
        break;
    }
  }

  void WPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    auto mem_access = rPagingReq.MemoryAccessType();
    //bool user_access = (rVmas.GetControlBlock()->PrivilegeLevel() == EPrivilegeLevelType::U);
    bool user_access = rPagingReq.PrivilegeLevelSpecified() and (rPagingReq.PrivilegeLevel() == EPrivilegeLevelType::U);
    uint32 dap = 0;

    LOG(trace) << "{RPteAttributeRISCV::Generate} user_access=" << user_access << endl;

    switch (mem_access)
    {
      case EMemAccessType::ReadWrite:
      case EMemAccessType::Write:
        mValue = 1;
        dap = user_access ? uint32(EDataAccessPermissionType::ReadWriteUserOnly) : uint32(EDataAccessPermissionType::ReadWriteNoUser);
        rPte.SetPageGenAttribute(EPageGenAttributeType::DataAccessPermission, dap);
        break;
      case EMemAccessType::Read:
        mValue = 0;
        dap = user_access ? uint32(EDataAccessPermissionType::ReadOnlyUserOnly) : uint32(EDataAccessPermissionType::ReadOnlyNoUser);
        rPte.SetPageGenAttribute(EPageGenAttributeType::DataAccessPermission, dap);
        break;
      case EMemAccessType::Unknown:
        mValue = 1;
        dap = user_access ? uint32(EDataAccessPermissionType::ReadWriteUserOnly) : uint32(EDataAccessPermissionType::ReadWriteNoUser);
        rPte.SetPageGenAttribute(EPageGenAttributeType::DataAccessPermission, dap);
        LOG(info) << "{WPteAttributeRISCV::Generate} unknown mem access type, using default val of 1" << endl;
        break;
      default:
        LOG(fail) << "{WPteAttributeRISCV::Generate} invalid or unsupported mem access type=" << EMemAccessType_to_string(mem_access) << endl;
        FAIL("w_pte_gen_invalid_mem_access");
        break;
    }
  }

  void RPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    auto mem_access = rPagingReq.MemoryAccessType();
    //bool user_access = rPagingReq.PrivilegeLevelSpecified() and (rPagingReq.PrivilegeLevel() == EPrivilegeLevelType::U);
    bool user_access = (rVmas.GetControlBlock()->PrivilegeLevel() == EPrivilegeLevelType::U);
    uint32 dap = 0;

    LOG(trace) << "{RPteAttributeRISCV::Generate} user_access=" << user_access << endl;

    switch (mem_access)
    {
      case EMemAccessType::ReadWrite:
      case EMemAccessType::Write:
        mValue = 1;
        dap = user_access ? uint32(EDataAccessPermissionType::ReadWriteUserOnly) : uint32(EDataAccessPermissionType::ReadWriteNoUser);
        rPte.SetPageGenAttribute(EPageGenAttributeType::DataAccessPermission, dap);
        break;
      case EMemAccessType::Read:
        mValue = 1;
        dap = user_access ? uint32(EDataAccessPermissionType::ReadOnlyUserOnly) : uint32(EDataAccessPermissionType::ReadOnlyNoUser);
        rPte.SetPageGenAttribute(EPageGenAttributeType::DataAccessPermission, dap);
        break;
      case EMemAccessType::Unknown:
        mValue = 1;
        dap = user_access ? uint32(EDataAccessPermissionType::ReadWriteUserOnly) : uint32(EDataAccessPermissionType::ReadWriteNoUser);
        rPte.SetPageGenAttribute(EPageGenAttributeType::DataAccessPermission, dap);
        LOG(info) << "{RPteAttributeRISCV::Generate} unknown mem access type, using default val of 1" << endl;
        break;
      default:
        LOG(fail) << "{RPteAttributeRISCV::Generate} invalid or unsupported mem access type=" << EMemAccessType_to_string(mem_access) << endl;
        FAIL("r_pte_gen_invalid_mem_access");
        break;
    }
  }

  void UPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    //std::unique_ptr<ChoiceTree> u_choice_tree(rVmas.GetControlBlock()->GetChoicesAdapter()->GetPagingChoiceTree("U"));
    //mValue = u_choice_tree.get()->Choose()->Value();

    bool user_access = rPagingReq.PrivilegeLevelSpecified() and (rPagingReq.PrivilegeLevel() == EPrivilegeLevelType::U);

    mValue = user_access ? 1 : 0;

    LOG(info) << "{UPteAttributeRISCV::Generate} generated U=" << mValue << endl;
  }

}
