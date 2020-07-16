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
#include <RandomUtils.h>
#include <PagingChoicesAdapter.h>
#include <Generator.h>
#include <Register.h>
#include <PteStructure.h>
#include <Choices.h>
#include <UtilityFunctions.h>
#include <UtilityFunctionsRISCV.h>
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
      //LOG(notice) << "{ValidPteAttributeRISCV::Generate} requesting invalid? " << req_invalid << endl;
      if (req_invalid == 0) {
        return; // request valid PTE.
      }
    }

    std::unique_ptr<ChoiceTree> choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Invalid Descriptor", level));
    auto chosen_value = choices_tree->Choose()->Value();
    if (chosen_value != 1) // page translation fault is selected
    {
      rPte.SetPageGenAttribute(EPageGenAttributeType::Invalid, 1);
      mValue = chosen_value;
      LOG(notice) << "{ValidPteAttributeRISCV::Generate} " << "Level= " << level << " Choice value=" << chosen_value << " Original Correct Value=" << mpStructure->Value() << " Modified Value=" << mValue << endl;
    }
  }

  void AddressPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    uint32 level = rPte.ParentTableLevel();
    EPteCategoryType descr_type = rPte.PteCategory();
    uint32 addr_fault_value = 0;

    if (level >= 1 && level <= 3 && descr_type == EPteCategoryType::Page)
    {
      std::unique_ptr<ChoiceTree> choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Misaligned Superpage", level));
      addr_fault_value = choices_tree->Choose()->Value();
      rPte.SetPageGenAttribute(EPageGenAttributeType::AddrSizeFault, addr_fault_value); //TODO update pagegenattr type to 'AddrFault' instead of address size fault
    }

    if (addr_fault_value != 0)
    {
      LOG(info) << "{AddressPteAttributeRISCV::Generate} requesting misaligned superpage fault level=" << level << endl;
      uint32 level_bits = level * 9;
      uint64 error_val = random_value64(0x1ull, (0x1 << level_bits) - 1) << 12; 
      LOG(trace) << "{AddressPteAttributeRISCV::Generate} error_val=0x" << hex << error_val 
                 << " phys_lower=0x" << rPte.PhysicalLower() << " mask=0x" << mpStructure->Mask() 
                 << " lsb=0x" << mpStructure->Lsb() << endl;
      mValue = ((error_val | rPte.PhysicalLower()) >> (mpStructure->Lsb() + 2)) & mpStructure->Mask(); //TODO get hardcoded shift programatically*/
    }
    else
    {
      LOG(trace) << "{AddressPteAttributeRISCV::Generate} phys_lower=0x" << hex << rPte.PhysicalLower() 
                 << " mask=0x" << mpStructure->Mask() << " lsb=0x" << mpStructure->Lsb() << endl;
      mValue = (rPte.PhysicalLower() >> (mpStructure->Lsb() + 2)) & mpStructure->Mask(); //TODO get hardcoded shift programatically
    }
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
    uint32 level = rPte.ParentTableLevel();
    uint32 last_level_ptr_fault = 0;
    if (level == 0)
    {
      std::unique_ptr<ChoiceTree> choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Last Level Pointer", level));
      last_level_ptr_fault = choices_tree->Choose()->Value();
    }

    auto mem_access = rPagingReq.MemoryAccessType();
    //bool user_access = (rVmas.GetControlBlock()->PrivilegeLevel() == EPrivilegeLevelType::U);
    bool user_access = rPagingReq.PrivilegeLevelSpecified() and (rPagingReq.PrivilegeLevel() == EPrivilegeLevelType::U);
    uint32 iap = 0;

    LOG(trace) << "{XPteAttributeRISCV::Generate} user_access=" << user_access << endl;

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

    if (last_level_ptr_fault != 0)
    {
      mValue = 0;
      LOG(info) << "{XPteAttributeRISCV::Generate} overriding x to 0 for last level ptr fault" << endl;
    }
  }

  void WPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    uint32 level = rPte.ParentTableLevel();
    uint32 last_level_ptr_fault = 0;
    if (level == 0)
    {
      std::unique_ptr<ChoiceTree> choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Last Level Pointer", level));
      last_level_ptr_fault = choices_tree->Choose()->Value();
    }

    auto mem_access = rPagingReq.MemoryAccessType();
    //bool user_access = (rVmas.GetControlBlock()->PrivilegeLevel() == EPrivilegeLevelType::U);
    bool user_access = rPagingReq.PrivilegeLevelSpecified() and (rPagingReq.PrivilegeLevel() == EPrivilegeLevelType::U);
    uint32 dap = 0;

    LOG(trace) << "{WPteAttributeRISCV::Generate} user_access=" << user_access << endl;

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
    
    if (last_level_ptr_fault != 0)
    {
      mValue = 0;
      LOG(info) << "{WPteAttributeRISCV::Generate} overriding w to 0 for last level ptr fault" << endl;
    }
  }

  void RPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    uint32 level = rPte.ParentTableLevel();
    uint32 last_level_ptr_fault = 0;
    if (level == 0)
    {
      std::unique_ptr<ChoiceTree> choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Last Level Pointer", level));
      last_level_ptr_fault = choices_tree->Choose()->Value();
    }

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
    
    if (last_level_ptr_fault != 0)
    {
      mValue = 0;
      LOG(info) << "{RPteAttributeRISCV::Generate} overriding r to 0 for last level ptr fault" << endl;
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
