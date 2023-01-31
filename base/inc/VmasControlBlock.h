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
#ifndef Force_VmasControlBlock_H
#define Force_VmasControlBlock_H

#include <vector>

#include "VmControlBlock.h"

namespace Force {

  class RootPageTable;
  class PageTable;
  class Page;
  class TablePte;
  class VmAddressSpace;
  class GenPageRequest;
  class ConstraintSet;
  class PagingChoicesAdapter;
  class TranslationRange;
  class PhysicalRegion;
  class VmVaRange;
  class MemoryAttributes;
  class PageInfoRec;
  class PageInformation;

  /*!
    \class VmasControlBlock
    \brief Base class for control block of a VmasControlBlock object.
  */
  class VmasControlBlock : public VmControlBlock {
  public:
    Object *          Clone()    const override { return new VmasControlBlock(*this); } //!< Clone VmasControlBlock object.
    const char*       Type()     const override { return "VmasControlBlock"; }

    ASSIGNMENT_OPERATOR_ABSENT(VmasControlBlock);
    VmasControlBlock(EPrivilegeLevelType privType, EMemBankType memType); //!< Constructor with privilege type and memory bank given.
    ~VmasControlBlock() override; //!< Destructor.

    void Setup(Generator* pGen) override; //!< Setup the VmasControlBlock object.
    void Initialize() override;           //!< Initialize VmasControlBlock object.
    bool Validate(std::string& rErrMsg) const override; //!< Return true only if all initialized context are valid.

    virtual bool InitializeRootPageTable(VmAddressSpace* pVmas, RootPageTable* pRootTable); //!< Initialize root page table.

    virtual GenPageRequest* PhysicalRegionPageRequest(const PhysicalRegion* pPhysRegion, bool& rRegionCompatible) const { return nullptr; }         //!< Return page-request object for a given physical region type.
    virtual uint32          PteShift()                                             const { return 0; }               //!< Return PTE shift based on PTE size.
    virtual ConstraintSet*  InitialVirtualConstraint()                             const; //!< Get initial constraint set from control block info

    uint32              Asid()               const  { return mAsid; }               //!< Return ASID.
    bool                WriteExecuteNever()  const  { return mWriteExecuteNever; }  //!< Return if writeable memory can't be executed

    virtual bool              IsUpperVa(uint64 VA) const         { return false; } //!< Return true if VA falls in upper VA range
    virtual RootPageTable*    GetRootPageTable(uint64 VA = 0ull) const { return mpRootPageTable; } //!< Return the root table based on VA
    virtual EMemBankType      NextLevelTableMemoryBank(const PageTable* parentTable, const GenPageRequest& rPageReq) const; //!< Return memory bank of next level table.
    virtual EMemBankType      GetTargetMemoryBank(uint64 VA, GenPageRequest* pPageReq, const Page* pPage, const std::vector<ConstraintSet* >& rVmConstraints); //!< Gets the target memory bank based on default bank and choices
    virtual ConstraintSet*    GetPageTableExcludeRegion(uint64 VA, EMemBankType memBank, const std::vector<ConstraintSet* >& rVmConstraints) { return nullptr; }
    virtual ConstraintSet*    GetPhysicalUsableConstraint() { return nullptr; }; //!< Returns the physical usable constraint as determined by memory bank and max phys address
    virtual EPageGranuleType  PageGranuleType(uint64 VA = 0ull)  const { return mGranuleType; } //!< Return page granule type.
    virtual const std::string& PageGranuleSuffix(uint64 VA = 0ull)  const { return mGranuleSuffix; } //!< Return page granule suffix.
    virtual const std::string PagePteIdentifier(EPteType pteType, uint64 VA = 0ull) const; //!< Return PTE identifier.
    const PagingChoicesAdapter* GetChoicesAdapter() const { return mpChoicesAdapter; } //!< Get paging choices adapter.
    virtual void GetAddressErrorRanges(std::vector<TranslationRange>& rRanges) const; //!< Obtain address errror ranges.
    virtual void CommitPageTable(uint64 VA, const PageTable* pParentTable, const TablePte* pTablePte, std::vector<ConstraintSet* >& rVmConstraints);
    virtual void CommitPage(const Page* pPage, std::vector<ConstraintSet* >& rVmConstraints);
    virtual uint32 HighestVaBitCurrent(uint32 rangeNum = 0ul) const { return 63; } //!< Return current highest VA bit.
    virtual bool GetVmVaRanges(std::vector<VmVaRange* >& rVmVaRanges, uint64* pVA) const; //!< Return VmVaRange info.
    virtual ConstraintSet*    GetPageTableUsableConstraint(EMemBankType memBank) const { return nullptr; }; //!< Returns the page table physical usable constraint as determined by max phys address and variable.
    virtual std::vector<const PhysicalRegion* > FilterEssentialPhysicalRegions(const std::vector<PhysicalRegion* >& rPhysRegions) const; //!< Filter essential physical regions.

    virtual bool PhysicalRegionAttributeCompatibility(const PhysicalRegion* pPhysRegion, const VmAddressSpace* pVmas) const { return true; } //!< Check to see whether physical region attribute is compatible
    virtual void GetGranuleType(std::vector<EPageGranuleType>& granuleTypes) const;  //!< Return page granule type
    virtual bool IsPaValid(uint64 PA, EMemBankType bank, std::string& rMsgStr) const { return false; } //!< Check if the PA+bank is valid for the address space.
    virtual const MemoryAttributes* MemoryAttributeControl() const { return nullptr; }
    virtual uint64 MaxVirtualAddress(bool flatMapped) const; //!< Return max supported virtual address
    virtual void PopulatePageInfo(uint64 VA, const VmAddressSpace* pVmas, const Page* pPage, PageInformation& rPageInfo) const; //!< Populate page information into rPageInfo referene
    virtual void PopulateAttributeInfo(const Page* pPage, PageInfoRec& rPageInfoRec) const { } //!< Populate page information into rPageInfo referene
  protected:
    VmasControlBlock(); //!< Default constructor.
    VmasControlBlock(const VmasControlBlock& rOther); //!< Copy constructor.

    const std::string AdditionalAttributesString() const override; //!< Return addtional attributes description in a string.
    virtual PagingChoicesAdapter* PagingChoicesAdapterInstance() const; //!< Return proper paging choices adapter instance.

    virtual void  InitializeMemoryAttributes() { } //!< Initialize memory attributes.
    virtual uint64           GetMemoryAttributes()   const { return 0; }                   //!< Return memory attributes.
    virtual uint32           GetAsid()               const { return 0; }                   //!< Return ASID.
    virtual EPageGranuleType GetGranuleType()        const { return EPageGranuleType(0); } //!< Return page granule type.
    virtual bool             GetWriteExecuteNever()  const { return false; }               //!< Return Write execute never flag

    virtual uint32 MaxTableLevel()       const { return 3; } //!< Return maximum table level.

    virtual RootPageTable*        RootPageTableInstance() const; //!< Return an instance of the RootPageTable class or its sub-classes.
    virtual ConstraintSet* PageTableCompatibleImplAttributes(uint64 VA = 0ull) const { return mpPtCompatibleImplAttrs; } //!< Return constraints of compatible memory attributes types for page table
    virtual EMemAttributeImplType PageTableImplAttribute(uint64 VA = 0ull) const { return mPageTableImplAttr; } //!< Return page table implementation defined attribute.

    virtual ConstraintSet* GetPageTableCompatibleAttributes(EMemAttributeImplType implType) const { return nullptr; } //!< Return constraints of compatible memory attributes types for page table
    virtual EMemAttributeImplType GetPageTableAttribute() const { return EMemAttributeImplType(0); } //!< Return page table implementation defined attribute.
    virtual void GetDefaultVaRange(ConstraintSet& rConstr) const; //!< Return the default VA range.
  protected:
    EPageGranuleType    mGranuleType;        //!< Page granule type of the address space.
    uint64              mMemoryAttributes;   //!< Memory attributes.
    uint32              mAsid;               //!< Address space ID.
    bool                mWriteExecuteNever;  //!< Write execute never flag
    mutable RootPageTable* mpRootPageTable;  //!< Pointer to root level table address space.
    PagingChoicesAdapter* mpChoicesAdapter;  //!< Pointer to paging choices adapter.
    std::string mGranuleSuffix;              //!< Suffix granule string.
    std::string mPteIdentifierSuffix;        //!< PTE identifier suffix.
    EMemAttributeImplType mPageTableImplAttr;//!< Page table implementation defined attribute specified by system register.
    ConstraintSet* mpPtCompatibleImplAttrs;  //!< Constraints of compatible memory attributes types for page table.
  };

}

#endif
