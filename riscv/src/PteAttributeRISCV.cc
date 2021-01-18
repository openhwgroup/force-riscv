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

    if (addr_fault_value == 1)
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

  EPagingExceptionType DAPteAttributeRISCV::GetExceptionType(const GenPageRequest& rPagingReq) const
  {
    return get_exception_type_from_access_type(rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr), rPagingReq.MemoryAccessType());
  }

  bool DAPteAttributeRISCV::EvaluateArchFaultChoice(const VmAddressSpace& rVmas, PageTableEntry& rPte, bool& rHardFaultChoice) const
  {
    uint32 level = rPte.ParentTableLevel();
    uint32 da_fault = 0;
    bool da_only_choice = false;

    std::unique_ptr<ChoiceTree> da_choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Invalid DA", level));
    da_only_choice = da_choices_tree->OnlyChoice();
    da_fault = da_choices_tree->Choose()->Value();

    if (da_fault and da_only_choice)
    {
      rHardFaultChoice = true;
      return true;
    }

    if (da_fault)
    {
      return true;
    }

    return false;
  }

  void DAPteAttributeRISCV::ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, cuint32 pteLevel, ConstraintSet& rTriggerConstr) const
  {
    GetDefaultConstraint(rTriggerConstr);
    auto mem_access = rPagingReq.MemoryAccessType();
    if (mem_access == EMemAccessType::ReadWrite or mem_access == EMemAccessType::Write)
    {
      rTriggerConstr.SubValue(0x3);
    }
    else
    {
      rTriggerConstr.SubValue(0x1);
    }
  }

  void DAPteAttributeRISCV::ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rPreventConstr) const
  {
    auto mem_access = rPagingReq.MemoryAccessType();
    if (mem_access == EMemAccessType::ReadWrite or mem_access == EMemAccessType::Write)
    {
      rPreventConstr.AddValue(0x3);
    }
    else
    {
      rPreventConstr.AddValue(0x1);
    }
  }

  void DAPteAttributeRISCV::SetPteGenAttribute(const GenPageRequest& rPagingReq, PageTableEntry& rPte) const
  {
    switch(mValue) {
    case 0:
      break;
    case 1:
      rPte.SetPageGenAttribute(EPageGenAttributeType::Accessed, 1);
      break;
    case 2:
      rPte.SetPageGenAttribute(EPageGenAttributeType::Dirty, 1);
      break;
    case 3:
      rPte.SetPageGenAttribute(EPageGenAttributeType::Accessed, 1);
      rPte.SetPageGenAttribute(EPageGenAttributeType::Dirty, 1);
      break;
    default:
        LOG(fail) << "{DAPteAttributeRISCV::SetPteGenAttribute} invalid generated value, can't set pte attribute" << endl;
        FAIL("da-pte-invalid");
    }
  }

  bool GPteAttributeRISCV::GetValueConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte, ConstraintSet& rValueConstr) const
  {
    GetDefaultConstraint(rValueConstr);
    return false;
  }

  EPagingExceptionType UPteAttributeRISCV::GetExceptionType(const GenPageRequest& rPagingReq) const
  {
    return get_exception_type_from_access_type(rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr), rPagingReq.MemoryAccessType());
  }

  //TODO can maybe make this more generic, impl is similar (1 choice to eval) for most bits
  bool UPteAttributeRISCV::EvaluateArchFaultChoice(const VmAddressSpace& rVmas, PageTableEntry& rPte, bool& rHardFaultChoice) const
  {
    uint32 level = rPte.ParentTableLevel();
    uint32 u_fault = 0;
    std::unique_ptr<ChoiceTree> u_choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Invalid U", level));
    bool u_only_choice = u_choices_tree->OnlyChoice();
    u_fault = u_choices_tree->Choose()->Value();

    if (u_fault and u_only_choice)
    {
      rHardFaultChoice = true;
      return true;
    }

    if (u_fault)
    {
      return true;
    }

    return false;
  }

  void UPteAttributeRISCV::ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, cuint32 pteLevel, ConstraintSet& rTriggerConstr) const
  {
    uint32 sum_value = 0;
    bool is_instr = rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr);
    bool user_access = rPagingReq.PrivilegeLevelSpecified() and (rPagingReq.PrivilegeLevel() == EPrivilegeLevelType::U);
    auto mem_access = rPagingReq.MemoryAccessType();
    bool sum_mem_access = (mem_access == EMemAccessType::ReadWrite || mem_access == EMemAccessType::Write);

    if (rVmas.GetControlBlock()->HasContextParam(EVmContextParamType::SUM))
    {
      sum_value = rVmas.GetControlBlock()->ContextParamValue(EVmContextParamType::SUM);
    }

    if (user_access or (sum_value == 1 and sum_mem_access and (not is_instr)))
    {
      rTriggerConstr.AddValue(0x0); // no user access
    }
    else
    {
      rTriggerConstr.AddValue(0x1); // user access
    }
  }

  void UPteAttributeRISCV::ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rPreventConstr) const
  {
    uint32 sum_value = 0;
    bool is_instr = rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr);
    bool user_access = rPagingReq.PrivilegeLevelSpecified() and (rPagingReq.PrivilegeLevel() == EPrivilegeLevelType::U);
    auto mem_access = rPagingReq.MemoryAccessType();
    bool sum_mem_access = (mem_access == EMemAccessType::ReadWrite || mem_access == EMemAccessType::Write);

    if (rVmas.GetControlBlock()->HasContextParam(EVmContextParamType::SUM))
    {
      sum_value = rVmas.GetControlBlock()->ContextParamValue(EVmContextParamType::SUM);
    }

    if (user_access or ((sum_value == 1) and (sum_mem_access) and (not is_instr)))
    {
      rPreventConstr.AddValue(0x1); // user access
    }
    else
    {
      rPreventConstr.AddValue(0x0); // no user access
    }
  }

  void UPteAttributeRISCV::SetPteGenAttribute(const GenPageRequest& rPagingReq, PageTableEntry& rPte) const
  {
    //TODO determine if any necessary pte attrs need to be set by U
  }

  EPagingExceptionType XPteAttributeRISCV::GetExceptionType(const GenPageRequest& rPagingReq) const
  {
    return EPagingExceptionType::InstructionPageFault;
  }

  bool XPteAttributeRISCV::EvaluateArchFaultChoice(const VmAddressSpace& rVmas, PageTableEntry& rPte, bool& rHardFaultChoice) const
  {
    uint32 level = rPte.ParentTableLevel();
    uint32 x_fault = 0;
    std::unique_ptr<ChoiceTree> x_choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Invalid X", level));
    bool x_only_choice = x_choices_tree->OnlyChoice();
    x_fault = x_choices_tree->Choose()->Value();

    if (x_fault and x_only_choice)
    {
      rHardFaultChoice = true;
      return true;
    }

    if (x_fault)
    {
      return true;
    }

    return false;
  }

  void XPteAttributeRISCV::ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, cuint32 pteLevel, ConstraintSet& rTriggerConstr) const
  {
    bool is_instr = rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr);
    if (is_instr)
    {
      rTriggerConstr.AddValue(0x0);
    }
    else
    {
      GetDefaultConstraint(rTriggerConstr);
    }
  }

  void XPteAttributeRISCV::ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rPreventConstr) const
  {
    bool is_instr = rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr);
    if (is_instr)
    {
      rPreventConstr.AddValue(0x1);
    }
    else
    {
      GetDefaultConstraint(rPreventConstr);
    }
  }

  void XPteAttributeRISCV::SetPteGenAttribute(const GenPageRequest& rPagingReq, PageTableEntry& rPte) const
  {
    bool is_instr = rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr);
    bool user_access = rPagingReq.PrivilegeLevelSpecified() and (rPagingReq.PrivilegeLevel() == EPrivilegeLevelType::U);

    if (is_instr)
    {
      uint32 iap = user_access ? uint32(EInstrAccessPermissionType::PrivilegedNoExecute) : uint32(EInstrAccessPermissionType::Execute);
      rPte.SetPageGenAttribute(EPageGenAttributeType::InstrAccessPermission, iap);
    }
    else
    {
      rPte.SetPageGenAttribute(EPageGenAttributeType::InstrAccessPermission, uint32(EInstrAccessPermissionType::NoExecute));
    }
  }

  EPagingExceptionType WRPteAttributeRISCV::GetExceptionType(const GenPageRequest& rPagingReq) const
  {
    return get_exception_type_from_access_type(rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr), rPagingReq.MemoryAccessType());
  }

  bool WRPteAttributeRISCV::EvaluateArchFaultChoice(const VmAddressSpace& rVmas, PageTableEntry& rPte, bool& rHardFaultChoice) const
  {
    //TODO evaluate if val constraint passing is needed for llptr support
    //if (ll_ptr_fault != 0) mValue = 0;

    uint32 level = rPte.ParentTableLevel();
    uint32 ll_ptr_fault = 0;
    uint32 wr_fault = 0;
    bool wr_only_choice = false;
    bool ll_ptr_only_choice = false;

    if (level == 0)
    {
      std::unique_ptr<ChoiceTree> ll_ptr_choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Last Level Pointer", level));
      ll_ptr_only_choice = ll_ptr_choices_tree->OnlyChoice();
      ll_ptr_fault = ll_ptr_choices_tree->Choose()->Value();
    }

    std::unique_ptr<ChoiceTree> wr_choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Invalid WR", level));
    wr_only_choice = wr_choices_tree->OnlyChoice();
    wr_fault = wr_choices_tree->Choose()->Value();

    if ((wr_fault and wr_only_choice) or (ll_ptr_fault and ll_ptr_only_choice))
    {
      rHardFaultChoice = true;
      return true;
    }

    if (wr_fault or ll_ptr_fault)
    {
      return true;
    }

    return false;
  }

  void WRPteAttributeRISCV::ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, cuint32 pteLevel, ConstraintSet& rTriggerConstr) const
  {
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
        LOG(info) << "{WRPteAttributeRISCV::ExceptionTriggeringConstraint} unknown mem access type, can only generate fault with W=1 R=0." << endl;
        break;
      default:
        LOG(fail) << "{WRPteAttributeRISCV::ExceptionTriggeringConstraint} invalid or unsupported mem access type=" << EMemAccessType_to_string(mem_access) << endl;
        FAIL("wr_pte_invalid_mem_access");
        break;
    }

    // TODO(Noah): Implement a better solution for this issue when one can be devised. The problem
    // is that if the X, W and R bits are all 0, it indicates a non-leaf page table entry. We want
    // to avoid unintentionally generating such entries when are are attempting to set the
    // attributes of a leaf page table entry to trigger a fault. We can allow X, W and R to be 0 at
    // Level 0 because the page table walk will not attempt to progress to a level beyond Level 0.
    bool is_instr = rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr);
    if (is_instr && (pteLevel > 0))
    {
      rTriggerConstr.SubValue(0x0);
    }
  }

  void WRPteAttributeRISCV::ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rPreventConstr) const
  {
    bool is_instr = rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr);
    if (is_instr)
    {
      GetDefaultConstraint(rPreventConstr);
      rPreventConstr.SubValue(0x2);
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
        LOG(info) << "{WRPteAttributeRISCV::SetPteGenAttribute} no data access, leaving default NoAccess value" << endl;
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

  EPagingExceptionType VPteAttributeRISCV::GetExceptionType(const GenPageRequest& rPagingReq) const
  {
    return get_exception_type_from_access_type(rPagingReq.GenBoolAttribute(EPageGenBoolAttrType::InstrAddr), rPagingReq.MemoryAccessType());
  }

  bool VPteAttributeRISCV::EvaluateArchFaultChoice(const VmAddressSpace& rVmas, PageTableEntry& rPte, bool& rHardFaultChoice) const
  {
    uint32 level = rPte.ParentTableLevel();
    uint32 v_fault = 0;
    bool v_only_choice = false;

    std::unique_ptr<ChoiceTree> v_choices_tree(rVmas.GetChoicesAdapter()->GetPagingChoiceTreeWithLevel("Invalid V", level));
    v_only_choice = v_choices_tree->OnlyChoice();
    v_fault = v_choices_tree->Choose()->Value();

    if (v_fault and v_only_choice)
    {
      rHardFaultChoice = true;
      return true;
    }

    if (v_fault)
    {
      return true;
    }

    return false;
  }

  void VPteAttributeRISCV::ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, cuint32 pteLevel, ConstraintSet& rTriggerConstr) const
  {
    rTriggerConstr.AddValue(0x0);
  }

  void VPteAttributeRISCV::ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rPreventConstr) const
  {
    rPreventConstr.AddValue(0x1);
  }

  void VPteAttributeRISCV::SetPteGenAttribute(const GenPageRequest& rPagingReq, PageTableEntry& rPte) const
  {
    if (mValue == 0)
    {
      rPte.SetPageGenAttribute(EPageGenAttributeType::Invalid, 1);
    }
  }

}
