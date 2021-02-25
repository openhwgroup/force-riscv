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
#ifndef Force_VmasControlBlockRISCV_H
#define Force_VmasControlBlockRISCV_H

#include <VmasControlBlock.h>
#include <Variable.h>

namespace Force {

  class PhysicalRegion;
  class MemoryAttributes;
  class PageInfoRec;

  /*!
    \class VmasControlBlockRISCV
    \brief Base class for address space control blocks.
  */
  class VmasControlBlockRISCV : public VmasControlBlock
  {
  public:
    VmasControlBlockRISCV(EPrivilegeLevelType privType, EMemBankType memType); //!< Constructor with EL type given.
    virtual ~VmasControlBlockRISCV() override { } //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(VmasControlBlockRISCV);

    Object*           Clone()    const override { return new VmasControlBlockRISCV(*this); } //!< Clone VmasControlBlockRISCV object.
    const char*       Type()     const override { return "VmasControlBlockRISCV"; }

    void Setup(Generator* pGen) override; //!< Setup VM Context Parameters
    void GetAddressErrorRanges(std::vector<TranslationRange>& rRanges) const override; //!< Obtain address error ranges.
    bool InitializeRootPageTable(VmAddressSpace* pVmas, RootPageTable* pRootTable) override; //!< Initialize root page table.
    EMemBankType NextLevelTableMemoryBank(const PageTable* parentTable, const GenPageRequest& rPageReq) const override; //!< Return memory bank of next level table.
    EMemBankType GetTargetMemoryBank(uint64 VA, GenPageRequest* pPageReq, const Page* pPage, const std::vector<ConstraintSet* >& rVmConstraints) override; //!< Gets the target memory bank based on default bank and choices
    GenPageRequest* PhysicalRegionPageRequest(const PhysicalRegion* pPhysRegion, bool& rRegionCompatible) const override; //!< Return a page request object based on physical region type specified.
    ConstraintSet*  InitialVirtualConstraint() const override; //!< Get initial constraint set from control block info
    ConstraintSet* GetPageTableExcludeRegion(uint64 VA, EMemBankType memBank, const std::vector<ConstraintSet* >& rVmConstraints) override;
    ConstraintSet* GetPhysicalUsableConstraint() override; //!< Returns the physical usable constraint as determined by memory bank and max phys address
    void CommitPageTable(uint64 VA, const PageTable* pParentTable, const TablePte* pTablePte, std::vector<ConstraintSet* >& rVmConstraints) override;
    void CommitPage(const Page* pPage, std::vector<ConstraintSet* >& rVmConstraints) override;

    bool SV32() const; //!< Return true if riscv paging mode is Sv32
    uint32 PteShift() const override; //!< Return PTE shift based on PTE size.
    uint32 HighestVaBitCurrent(uint32 rangeNum = 0ul) const override; //!< Return current highest VA bit.
    ConstraintSet* GetPageTableUsableConstraint(EMemBankType memBank) const override; //!< Returns the page table physical usable constraint as determined by max phys address and variable.
    bool IsPaValid(uint64 PA, EMemBankType bank, std::string& rMsgStr) const override; //!< Check if the PA+bank is valid for the address space.
  protected:
    VmasControlBlockRISCV() : mRegisterPrefix("m") { } //!< Default constructor.
    VmasControlBlockRISCV(const VmasControlBlockRISCV& rOther) : mRegisterPrefix(rOther.mRegisterPrefix) { } //!< Copy constructor.

    void InitializeMemoryAttributes() override; //!< Initialize memory attributes.
    void SetupRootPageTable(RootPageTable* pRootTable, VmAddressSpace* pVmas, EPageGranuleType granType, const std::string& pteSuffix, const std::string& regName); //!< create/setup root page table object
    void FillRegisterReload(RegisterReload* pRegContext) const override; //!< Fill register reload context.

    uint64 GetMaxPhysicalAddress() const override; //!< Return initial maximum physical address.
    uint64 GetMemoryAttributes() const override; //!< Return initial memory attributes.
    bool   GetBigEndian() const override; //!< Return initial big-endian flag.
    bool   GetWriteExecuteNever() const override; //!< Return initial write execute never flag
    EPageGranuleType GetGranuleType() const override; //!< Return address space initial page granule type.

    const std::string StatusRegisterName() const { return (mRegisterPrefix + std::string("status")); } //!< returns status register name
    const std::string AtpRegisterName() const { return "satp"; } //!< returns address translation/protection register name
  protected:
    std::string mRegisterPrefix; //privilege level prefix for context parameters
  };
}

#endif //Force_VmasControlBlockRISCV_H
