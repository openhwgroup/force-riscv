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
      auto req_constr = rPagingReq.GenAttributeConstraint(EPageGenAttributeType::Invalid);
      uint32 req_invalid = 1;
      if (nullptr != req_constr)
      {
        req_invalid = req_constr->ChooseValue();
      }
      if (req_invalid != 0)
      {
        std::unique_ptr<ChoiceTree> choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Misaligned Superpage", level));
        addr_fault_value = choices_tree->Choose()->Value();
      }
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

    uint32 level = rPte.ParentTableLevel();
    uint32 fault_value = 1;

    std::unique_ptr<ChoiceTree> choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Invalid DA", level));
    fault_value = choices_tree->Choose()->Value();

    if (fault_value != 1)
    {
      LOG(info) << "{DAPteAttributeRISCV::Generate} invalid DA bit fault generated value=0x" << hex << fault_value << endl;
      mValue = fault_value;
    }
  }

  void XPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    uint32 level = rPte.ParentTableLevel();
    uint32 last_level_ptr_fault = 0;
    uint32 xwr_fault = 0;

    if (level == 0)
    {
      std::unique_ptr<ChoiceTree> ll_choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Last Level Pointer", level));
      last_level_ptr_fault = ll_choices_tree->Choose()->Value();
    }

    std::unique_ptr<ChoiceTree> xwr_choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Invalid XWR", level));
    xwr_fault = xwr_choices_tree->Choose()->Value();

    auto mem_access = rPagingReq.MemoryAccessType();
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

    if (xwr_fault == 0)
    {
      mValue = mValue ? 0 : 1;
      LOG(info) << "{XPteAttributeRISCV::Generate} inverting for fault, val=" << mValue << endl;
    }

    if (last_level_ptr_fault != 0)
    {
      mValue = 0;
      LOG(info) << "{XPteAttributeRISCV::Generate} overriding x to 0 for last level ptr fault" << endl;
    }
  }

  EPagingExceptionType WRPteAttributeRISCV::GetExceptionType(const GenPageRequest& rPagingReq) const
  {
    bool is_instr = rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr);
    if (is_instr)
    {
      return EPagingExceptionType::InstructionPageFault;
    }
    else
    {
      auto mem_access = rPagingReq.MemoryAccessType();
      switch (mem_access)
      {
        case EMemAccessType::Write:
        case EMemAccessType::ReadWrite:
          return EPagingExceptionType::LoadPageFault;
          break;
        default:
          return EPagingExceptionType::StoreAmoPageFault;
          break;
      }
    }

    return EPagingExceptionType::LoadPageFault; //TODO need to handle fallthrough
  }

  void WRPteAttributeRISCV::ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, ConstraintSet& rTriggerConstr) const
  {
    //TODO - may not be necessary since W=1 R=0 can cause instruction page fault currently
    /*bool is_instr = rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr);
    if (is_instr)
    {
      GetDefaultConstraint(rTriggerConstr);
    }*/

    auto mem_access = rPagingReq.MemoryAccessType();
    rTriggerConstr.AddValue(0x2); // W=1 R=0 always invalid (reserved by specification)
    switch (mem_access)
    {
      case EMemAccessType::Read:
      case EMemAccessType::ReadWrite:
        rTriggerConstr.AddValue(0x0); // W=0 R=0 no data access
        break;
      case EMemAccessType::Write:
        rTriggerConstr.AddRange(0x0, 0x1); // W=0 R=1 no write access, W=0 R=0 no data access
        break;
      case EMemAccessType::Unknown:
        LOG(info) << "{WRPteAttributeRISCV::ExceptionTriggeringConstraint} unknown mem access type, can't guarantee fault generation." << endl;
        break;
      default:
        LOG(fail) << "{WRPteAttributeRISCV::ExceptionTriggeringConstraint} invalid or unsupported mem access type=" << EMemAccessType_to_string(mem_access) << endl;
        FAIL("wr_pte_invalid_mem_access");
        break;
    }
  }

  void WRPteAttributeRISCV::ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, ConstraintSet& rPreventConstr) const
  {
    bool is_instr = rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr);
    if (is_instr)
    {
      GetDefaultConstraint(rPreventConstr); //TODO might need to sub val of 2 (write set w/o read)
    }
    else
    {
      auto mem_access = rPagingReq.MemoryAccessType();
      rPreventConstr.AddValue(0x3); // W=1 R=1 read/write access
      switch (mem_access)
      {
        case EMemAccessType::Read:
          rPreventConstr.AddValue(0x1); // W=0 R=1 read access
        case EMemAccessType::ReadWrite:
        case EMemAccessType::Write:
          break;
        case EMemAccessType::Unknown:
          LOG(info) << "{WRPteAttributeRISCV::ExceptionPreventingConstraint} unknown mem access type, can't guarantee fault prevention." << endl;
          break;
        default:
          LOG(fail) << "{WRPteAttributeRISCV::ExceptionPreventingConstraint} invalid or unsupported mem access type=" << EMemAccessType_to_string(mem_access) << endl;
          FAIL("wr_pte_invalid_mem_access");
          break;
      }
    }
  }

  void WRPteAttributeRISCV::SetPteGenAttribute(const GenPageRequest& rPagingReq, PageTableEntry& rPte) const
  {
    uint32 dap = uint32(EDataAccessPermissionType::NoAccess);
    bool user_access = rPagingReq.PrivilegeLevelSpecified() and (rPagingReq.PrivilegeLevel() == EPrivilegeLevelType::U);
    switch (mValue)
    {
      case 0:
        LOG(warn) << "{WRPteAttributeRISCV::SetPteGenAttribute} no data access, leaving default NoAccess value" << endl;
        break;
      case 1:
        dap = user_access ? uint32(EDataAccessPermissionType::ReadOnlyUserOnly) : uint32(EDataAccessPermissionType::ReadOnlyNoUser);
        break;
      case 2:
      case 3:
        dap = user_access ? uint32(EDataAccessPermissionType::ReadWriteUserOnly) : uint32(EDataAccessPermissionType::ReadWriteNoUser);
        break;
      default:
        LOG(fail) << "{WRPteAttributeRISCV::SetPteGenAttribute} invalid generated value, can't set pte attribute" << endl;
        FAIL("wr_pte_invalid_mem_access");
        break;
    }
    rPte.SetPageGenAttribute(EPageGenAttributeType::DataAccessPermission, dap);
  }

  /*TODO - integrate individual page fault type choices into ExceptionConstraintClass
    uint32 level = rPte.ParentTableLevel();
    uint32 last_level_ptr_fault = 0;
    uint32 xwr_fault = 0;

    if (level == 0)
    {
      std::unique_ptr<ChoiceTree> ll_choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Last Level Pointer", level));
      last_level_ptr_fault = ll_choices_tree->Choose()->Value();
    }

    std::unique_ptr<ChoiceTree> xwr_choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Invalid XWR", level));
    xwr_fault = xwr_choices_tree->Choose()->Value();

    if (xwr_fault == 0) mValue = mValue ? 0 : 1;
    if (last_level_ptr_fault != 0) mValue = 0;
    */

  void UPteAttributeRISCV::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte)
  {
    uint32 level = rPte.ParentTableLevel();
    uint32 sum_value = 0;
    auto mem_access = rPagingReq.MemoryAccessType();
    bool user_access = rPagingReq.PrivilegeLevelSpecified() and (rPagingReq.PrivilegeLevel() == EPrivilegeLevelType::U);

    if (rVmas.GetControlBlock()->HasContextParam(EVmContextParamType::SUM))
    {
      sum_value = rVmas.GetControlBlock()->ContextParamValue(EVmContextParamType::SUM);
    }

    bool sum_mem_access = (mem_access == EMemAccessType::ReadWrite || mem_access == EMemAccessType::Write);

    std::unique_ptr<ChoiceTree> choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Invalid U", level));
    uint32 u_bit_fault = choices_tree->Choose()->Value();

    mValue = user_access ? 1 : 0;

    LOG(info) << "{UPteAttributeRISCV::Generate} generated U=" << mValue << endl;

    if (u_bit_fault == 0)
    {
      if (!user_access && sum_value == 1 && sum_mem_access)
      {
        LOG(info) << "{UPteAttributeRISCV::Generate} can't cause U bit fault in S mode while SUM is enabled." << endl;
      }
      else
      {
        mValue = mValue ? 0 : 1;
        LOG(info) << "{UPteAttributeRISCV::Generate} inverting U bit to cause fault, val=" << mValue << endl;
      }
    }
  }

}
