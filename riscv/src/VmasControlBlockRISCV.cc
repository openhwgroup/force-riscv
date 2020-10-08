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
#include <VmasControlBlockRISCV.h>
#include <Generator.h>
#include <Config.h>
#include <Choices.h>
#include <Register.h>
#include <MemoryManager.h>
#include <PteAttribute.h>
#include <Page.h>
#include <PageTable.h>
#include <UtilityFunctions.h>
#include <GenRequest.h>
#include <PagingChoicesAdapter.h>
#include <Log.h>
#include <RegisterReload.h>
#include <Random.h>
#include <VmAddressSpace.h>
#include <PhysicalPageManager.h>
#include <Constraint.h>
#include <VmUtils.h>
#include <PageInfoRecord.h>
#include <SetupRootPageTableRISCV.h>
#include <UtilityFunctionsRISCV.h>

#include <memory>
#include <sstream>
/*!
  \file VmasControlBlockRISCV.cc
  \brief Code to configure control block of address spaces.
*/

using namespace std;

namespace Force {

  VmasControlBlockRISCV::VmasControlBlockRISCV(EPrivilegeLevelType privType, EMemBankType memType)
    : VmasControlBlock(privType, memType), mRegisterPrefix("m")
  {
    privilege_prefix(mRegisterPrefix, privType, false);
  }

  void VmasControlBlockRISCV::Setup(Generator* pGen)
  {
    //call base class setup (setup paging choices adapter/generator ptr)
    VmasControlBlock::Setup(pGen);

    VmContextParameter*      context_param = nullptr;
    EVmContextParamType      param_type;
    std::vector<std::string> status_param_names; //mstatus, sstatus, ustatus
    std::vector<std::string> atp_param_names;    //satp, hgatp, vsatp

    //TODO refine context parameters

    switch (mPrivilegeLevel)
    {
      case EPrivilegeLevelType::M:
        status_param_names.push_back("MPRV"); //Modify Privilege - when enabled uses atp for privilege level in *PP register
        status_param_names.push_back("MXR"); //Make Executable Readable - loads to X=1 or R=1 allowed instead of just R=1
        status_param_names.push_back("SUM"); //Supervisor User Memory - permits S access to pages marked as U=1
        //status_param_names.push_back("MPP"); //Machine Previous Privilege - stores previous priv level while in M mode
        //status_param_names.push_back("MBE"); //Machine Big Endian - affects whether memory accesses are big endian, instr fetch always little
        status_param_names.push_back("TVM"); //Trap Virtual Memory - when 1 attempts to access satp CSR or execute SFENCE.VMA raise illegal instr exception
        break;
      case EPrivilegeLevelType::H:
      case EPrivilegeLevelType::S:
        status_param_names.push_back("MXR"); //Make Executable Readable - loads to X=1 or R=1 allowed instead of just R=1
        status_param_names.push_back("SUM"); //Supervisor User Memory - permits S access to pages marked as U=1
        //status_param_names.push_back("SPP"); //Supervisor Previous Privilege - stores previous priv level while in S mode
        //status_param_names.push_back("SBE"); //Supervisor Big Endian - affects whether memory accesses are big endian, instr fetch always little
        break;
      case EPrivilegeLevelType::U:
        //status_param_names.push_back("UBE");
        break;
    }

    atp_param_names.push_back("MODE");

    for (auto& name : status_param_names)
    {
      param_type    = string_to_EVmContextParamType(name);
      context_param = new VmContextParameter(param_type, StatusRegisterName(), name);
      AddParameter(context_param);
    }

    for (auto& name : atp_param_names)
    {
      param_type    = string_to_EVmContextParamType(name);
      context_param = new VmContextParameter(param_type, AtpRegisterName(), name);
      AddParameter(context_param);
    }
  }

  void VmasControlBlockRISCV::FillRegisterReload(RegisterReload* pRegContext) const
  {
    VmasControlBlock::FillRegisterReload(pRegContext);

    auto reg_file = mpGenerator->GetRegisterFile();

    //Add register field updates for any context parameters not managed by mContextParams
    Register* satp_ptr = reg_file->RegisterLookup(AtpRegisterName());
    pRegContext->AddRegisterFieldUpdate(satp_ptr, "PPN", (mpRootPageTable->TableBase() >> 12));
  }

  void VmasControlBlockRISCV::InitializeMemoryAttributes()
  {

  }

  uint64 VmasControlBlockRISCV::GetMemoryAttributes() const
  {
    return 0;
  }

  bool VmasControlBlockRISCV::GetBigEndian() const
  {
    //TODO define behavior when spike updated to support M/S/UBE - endianness bit only applies to data accesses
    /*switch (mPrivilegeLevel)
    {
      case EPrivilegeLevelType::M:
        return (ContextParamValue(EVmContextParamType::MBE) == 1);
        break;
      case EPrivilegeLevelType::S:
        return (ContextParamValue(EVmContextParamType::SBE) == 1);
        break;
      case EPrivilegeLevelType::U:
        return (ContextParamValue(EVmContextParamType::UBE) == 1);
        break;
    }*/

    return false;
  }

  EPageGranuleType VmasControlBlockRISCV::GetGranuleType() const
  {
    return EPageGranuleType::G4K;
  }

  bool VmasControlBlockRISCV::IsPaValid(uint64 PA, EMemBankType bank, std::string& rMsgStr) const
  {
    if (PA > mMaxPhysicalAddress) {
      rMsgStr += string_snprintf(128, "PA 0x%llx is larger than max physical address 0x%llx", PA, mMaxPhysicalAddress);
      return false;
    }

    return true;
  }

  //!< if Force configured for 32-bit ISA, then paging mode must be Sv32...
  
  bool VmasControlBlockRISCV::SV32() const {
    return Config::Instance()->GetGlobalStateValue(EGlobalStateType::RV32) != 0; // force-risc configured to 32-bits 
  }
 
  //!< Return PTE shift based on paging mode...
  //
  uint32 VmasControlBlockRISCV::PteShift() const { 
    return SV32() ? 2 : 3;
  }

  uint64 VmasControlBlockRISCV::GetMaxPhysicalAddress() const
  {
    //TODO switch off mode - max PA is technically 56 for sv39/sv48 and 34 for sv32

    /*Uses all bits of PTE to store PPN[x] values - this causes some odd PA sizes after xlate
      since PPN fields in PTE start at 10, but the page offset is 12 we have extra bits in PTE
      allowing for first PPN to have more than the tablestep worth of bits.*/


    //For now we are assuming Sv48 and are forcing a 48bit max PA for consistency.
    return Config::Instance()->LimitValue(ELimitType::PhysicalAddressLimit);
  }

  bool VmasControlBlockRISCV::GetWriteExecuteNever() const
  {
    return false;
  }

  uint32 VmasControlBlockRISCV::HighestVaBitCurrent(uint32 rangeNum) const
  {
    //TODO switch off mode bit - Sv32/39/48 have 31/38/47 for max va bits
    return SV32() ? 31 : 47;
  }

  void VmasControlBlockRISCV::GetAddressErrorRanges(vector<TranslationRange>& rRanges) const
  {
    TranslationRange addr_err_range;

    uint64 va_bits = mpRootPageTable->HighestLookUpBit();
    
    if (SV32()) va_bits += 1; // addressses are not sign-extended in Sv32, thus no Address Error exception
    
    uint64 error_start = 0x1ull << va_bits;
    uint64 error_end = sign_extend64((0x1ull << va_bits), va_bits+1) - 1;

    addr_err_range.SetBoundary(error_start, error_end);
    addr_err_range.SetMemoryBank(DefaultMemoryBank());

    rRanges.push_back(addr_err_range);
  }


  bool VmasControlBlockRISCV::InitializeRootPageTable(VmAddressSpace* pVmas, RootPageTable* pRootTable)
  {

    mpRootPageTable = RootPageTableInstance();
    if (nullptr == pRootTable)
    {
      //TODO make setup root table not fail if new alloc fails, since we have option to alias. return alloc success
      //TODO also need setup to be able to force non-alias case and make address space usable
      SetupRootPageTable(mpRootPageTable, pVmas, mGranuleType, mPteIdentifierSuffix, AtpRegisterName());
    }
    else
    {
      mpRootPageTable  = pRootTable;
      uint64 root_addr = mpRootPageTable->TableBase();
      uint32 tb_size   = mpRootPageTable->RootTableSize();

      pVmas->AddPhysicalRegion(new PhysicalRegion(root_addr, root_addr + (tb_size - 1), EPhysicalRegionType::PageTable, pRootTable->MemoryBank(), EMemDataType::Data));
      pVmas->UpdatePageTableConstraint(root_addr, root_addr + (tb_size - 1));
      mpRootPageTable->SignUp(pVmas);
    }

    return true;
  }

  void VmasControlBlockRISCV::SetupRootPageTable(RootPageTable* pRootTable, VmAddressSpace* pVmas, EPageGranuleType granType, const std::string& pteSuffix, const std::string& regName)
  {
    SetupRootPageTableRISCV SetupPagingObj;

    if (pRootTable == nullptr)
    {
      LOG(fail) << "{VmasControlBlockRISCV::InitializeRootPageTable} pRootTable is nullptr after initialize" << endl;
      FAIL("root_page_table_nullptr");
    }

    uint32 pteSize       = SV32() ? 2  : 3;
    uint32 tableStep     = SV32() ? 10 : 9;
    uint32 maxTableLevel = SV32() ? 1  : 3;
    uint32 tableLowBit   = SV32() ? 22 : 39;

    pRootTable->Setup(tableStep, HighestVaBitCurrent(), tableLowBit, pteSuffix, pteSize, maxTableLevel);
    
    pRootTable->SetMemoryBank(DefaultMemoryBank());
    auto reg_file = mpGenerator->GetRegisterFile();
    auto mem_mgr  = mpGenerator->GetMemoryManager();

    uint32 tb_size = pRootTable->RootTableSize();
    std::unique_ptr<ConstraintSet> usable_constr_ptr(GetPageTableUsableConstraint(mDefaultMemoryBank));
    uint64 root_addr = SetupPagingObj.SetupRootPageTable(tb_size, mem_mgr, mDefaultMemoryBank, reg_file, regName, usable_constr_ptr.get());

    pRootTable->SetTableBase(root_addr);
    pVmas->AddPhysicalRegion(new PhysicalRegion(root_addr, root_addr + (tb_size - 1), EPhysicalRegionType::PageTable, pRootTable->MemoryBank(), EMemDataType::Data));
    pVmas->UpdatePageTableConstraint(root_addr, root_addr + (tb_size - 1));
    pRootTable->SignUp(pVmas);
  }

  GenPageRequest* VmasControlBlockRISCV::PhysicalRegionPageRequest(const PhysicalRegion* pPhysRegion, bool& rRegionCompatible) const
  {
    EPhysicalRegionType phys_region_type = pPhysRegion->RegionType();
    rRegionCompatible = true;

    auto set_system_pagereq_lambda = [=] (GenPageRequest* pPageReq, const PhysicalRegion* pPhysRegion) {
      pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::FlatMap, true); // use flat-map.
      pPageReq->SetPteAttribute(EPteAttributeType::SystemPage, 1); // Set the sw-configurable system page indicator to a 1 for system pages
      pPageReq->SetGenAttributeValue(EPageGenAttributeType::Invalid, 0);
      pPageReq->SetGenAttributeValue(EPageGenAttributeType::AddrSizeFault, 0);
      pPageReq->SetPrivilegeLevel(mPrivilegeLevel);
      pPageReq->SetBankType(pPhysRegion->MemoryBank());
    };

    bool is_instr = false;
    bool can_alias = false;
    EMemAccessType mem_access = EMemAccessType::Read;
    switch (phys_region_type)
    {
      case EPhysicalRegionType::HandlerMemory:
      case EPhysicalRegionType::ResetRegion:
        is_instr = true;
        break;
      case EPhysicalRegionType::AddressTable:
      case EPhysicalRegionType::BootRegion:
        is_instr = true;
        can_alias = true;
        break;
      case EPhysicalRegionType::PageTable:
      case EPhysicalRegionType::ExceptionStack:
        mem_access = EMemAccessType::ReadWrite;
        break;
      default:
        LOG(fail) << "{VmasControlBlockRISCV::PhysicalRegionPageRequest} not handled physical region type: " << EPhysicalRegionType_to_string(phys_region_type) << endl;
        FAIL("unhandled-physical-region-type");
    }

    GenPageRequest* page_req = mpGenerator->GenPageRequestInstance(is_instr, mem_access);
    set_system_pagereq_lambda(page_req, pPhysRegion);
    page_req->SetGenBoolAttribute(EPageGenBoolAttrType::CanAlias, can_alias); // Set a flag to allow aliasing to system page

    //TODO fix fault regulation logic here once exception pte classes in place
    page_req->SetPteAttribute(EPteAttributeType::V, 1);
    if (is_instr)
    {
      page_req->SetGenBoolAttribute(EPageGenBoolAttrType::NoInstrPageFault, true);
      page_req->SetPteAttribute(EPteAttributeType::X, 1); // make sure instr page is executable
      page_req->SetPteAttribute(EPteAttributeType::WR, 1); //ensure readable page
    }
    else
    {
      page_req->SetGenBoolAttribute(EPageGenBoolAttrType::NoDataPageFault, true);
      page_req->SetPteAttribute(EPteAttributeType::WR, 3); // make sure data page is writable
    }

    return page_req;
  }

  EMemBankType VmasControlBlockRISCV::NextLevelTableMemoryBank(const PageTable* parentTable, const GenPageRequest& rPageReq) const
  {
    return EMemBankType::Default;
  }

  EMemBankType VmasControlBlockRISCV::GetTargetMemoryBank(uint64 VA, GenPageRequest* pPageReq, const Page* pPage, const std::vector<ConstraintSet* >& rVmConstraints)
  {
    return EMemBankType::Default;
  }

  ConstraintSet* VmasControlBlockRISCV::GetPhysicalUsableConstraint()
  {
    ConstraintSet phys_limit_constr(0, MaxPhysicalAddress());

    auto mem_manager   = mpGenerator->GetMemoryManager();
    auto mem_bank      = mem_manager->GetMemoryBank(uint32(EMemBankType::Default));
    auto usable_constr = mem_bank->Usable();

    ConstraintSet* phys_usable = new ConstraintSet(*usable_constr);
    phys_usable->ApplyConstraintSet(phys_limit_constr);

    return phys_usable;
  }

  ConstraintSet* VmasControlBlockRISCV::InitialVirtualConstraint() const
  {
    //RISCV requires bits VA[63:va_max+1] to equal VA[va_max], logic below generates constraint accordingly.
    //Example: SV48 initial virtual is 0x0-0xFFF FFFF FFFF, 0xFFFF 8000 0000 0000-0xFFFF FFFF FFFF FFFF
    auto v_constr = new ConstraintSet();

    uint64 va_bits = mpRootPageTable->HighestLookUpBit();

    if (SV32()) {
      uint64 va_end  = (0x1ull << (va_bits + 1)) - 0x1ull;
      v_constr->AddRange(0, va_end);
      LOG(debug) << "{VmasControlBlockRISCV::InitialVirtualConstraint} For Sv32, va range: 0x0/0x" << std::hex << va_end << std::dec << std::endl;
    } else {
      uint64 va_end  = (0x1ull << va_bits) - 0x1ull;
      v_constr->AddRange(0, va_end);

      LOG(debug) << "{VmasControlBlockRISCV::InitialVirtualConstraint} va_end: 0x0/0x" << std::hex << va_end << std::dec << std::endl;
      uint64 va_start = sign_extend64((0x1ull << va_bits), va_bits+1);
      v_constr->AddRange(va_start, ~0x0ull);

      LOG(debug) << "{VmasControlBlockRISCV::InitialVirtualConstraint} va_start: 0x0/0x" << std::hex << va_start << std::dec << std::endl;
    }

    if (v_constr->IsEmpty())
    {
      LOG(fail) << "{VmasControlBlockRISCV::InitialVirtualConstraint} empty initial virtual constraint" << endl;
      FAIL("empty_initial_virtual_constraint");
    }

    return v_constr;
  }

  ConstraintSet* VmasControlBlockRISCV::GetPageTableUsableConstraint(EMemBankType memBank) const
  {
    // Get the usable constraint specified in the variable.xml
    string var_name = EMemBankType_to_string(memBank) + " page table physical memory range";
    auto page_table_var = mpGenerator->GetVariable(var_name, EVariableType::String);
    ConstraintSet* usable_constr = new ConstraintSet(page_table_var);

    // Apply the physical address limit size constraint.
    ConstraintSet phys_limit_constr(0, MaxPhysicalAddress());
    usable_constr->ApplyConstraintSet(phys_limit_constr);
    return usable_constr;
  }

  ConstraintSet* VmasControlBlockRISCV::GetPageTableExcludeRegion(uint64 VA, EMemBankType memBank, const std::vector<ConstraintSet* >& rVmConstraints)
  {
    ConstraintSet* exclude_region = rVmConstraints[uint32(EVmConstraintType::Existing)]->Clone();

    exclude_region->SubConstraintSet(*rVmConstraints[uint32(EVmConstraintType::FlatMap)]);
    exclude_region->MergeConstraintSet(*rVmConstraints[uint32(EVmConstraintType::PageTable)]);
    exclude_region->MergeConstraintSet(*rVmConstraints[uint32(EVmConstraintType::PageFault)]);
    exclude_region->AddValue(VA);

    return exclude_region;
  }

  void VmasControlBlockRISCV::CommitPageTable(uint64 VA, const PageTable* pParentTable, const TablePte* pTablePte, std::vector<ConstraintSet* >& rVmConstraints)
  {
    uint32 addr_size_fault = pTablePte->PageGenAttributeDefaultZero(EPageGenAttributeType::AddrSizeFault);
    uint32 invalid_desc = pTablePte->PageGenAttributeDefaultZero(EPageGenAttributeType::Invalid);

    uint64 mask = get_mask64(pParentTable->LowestLookUpBit());
    uint64 va_range_low = VA & ~mask;
    uint64 va_range_hi  = VA | mask;

    if (addr_size_fault || invalid_desc)
    {
      rVmConstraints[uint32(EVmConstraintType::PageFault)]->AddRange(va_range_low, va_range_hi);
    }

    rVmConstraints[uint32(EVmConstraintType::PageTable)]->AddRange(pTablePte->PhysicalLower(), pTablePte->PhysicalUpper());
  }

  void VmasControlBlockRISCV::CommitPage(const Page* pPage, std::vector<ConstraintSet* >& rVmConstraints)
  {
    VmasControlBlock::CommitPage(pPage, rVmConstraints);

    uint32 addr_size_fault = pPage->PageGenAttributeDefaultZero(EPageGenAttributeType::AddrSizeFault);
    uint32 invalid_desc = pPage->PageGenAttributeDefaultZero(EPageGenAttributeType::Invalid);

    if (addr_size_fault || invalid_desc)
    {
      rVmConstraints[uint32(EVmConstraintType::PageFault)]->AddRange(pPage->Lower(), pPage->Upper());
    }
  }
}
