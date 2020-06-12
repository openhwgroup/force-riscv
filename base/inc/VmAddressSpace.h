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
#ifndef Force_VmAddressSpace_H
#define Force_VmAddressSpace_H

#include <VmMapper.h>
#include <vector>

namespace Force {

  class AddressReuseMode;
  class GenPageRequest;
  class MemoryConstraint;
  class MemoryConstraintUpdate;
  class Page;
  class PageInformation;
  class PageSizeInfo;
  class PageTable;
  class PageTableConstraint;
  class PagingChoicesAdapter;
  class PhysicalRegion;
  class RootPageTable;
  class TablePte;
  class VmasControlBlock;

  /*!
    \class VmAddressSpace
    \brief Virtual memory mapper class
  */

  class VmAddressSpace : public VmMapper, public Object {
  public:
    Object * Clone() const override { return new VmAddressSpace(*this); } //!< Clone VmAddressSpace object.
    const std::string ToString() const override { return Type(); } //!< Return a string describing the VmAddressSpace object.
    const char* Type() const override { return "VmAddressSpace"; }

    explicit VmAddressSpace(const VmFactory* pFactory, VmasControlBlock* pVmasCtlrBlock); //!< Constructor with pointer to control block given.
    ~VmAddressSpace(); //!< Destructor.

    void Setup(Generator* gen) override; //!< Setup the virtual memory address space object.
    const ConstraintSet* VirtualUsableConstraintSet(bool isInstr) const override; //!< Return const pointer to applicable virtual constraint object.
    ConstraintSet* VirtualUsableConstraintSetClone(bool isInstr) override; //!< Return cloned pointer to applicable virtual constraint object.
    const ConstraintSet* VirtualSharedConstraintSet() const override; //!< Return const pointer to virtual shared constraint object.
    void ApplyVirtualUsableConstraint(const EMemDataType memDataType, const EMemAccessType memAccessType, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const override; //!< Apply virtual usable constraint to specified constraint.
    void Activate()   override; //!< Activate the VmAddressSpace object.
    void Initialize() override; //!< Initialize the VmAddressSpace object.
    void Deactivate() override; //!< Deactivate the VmAddressSpace object.
    bool MapAddressRange(uint64 VA, uint64 size, bool isInstr, const GenPageRequest* pPageReq) override; //!< Map virtual address range.
    uint64 MapAddressRangeForPA(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, const GenPageRequest* pPageReq) override; //!< Map virtual address range.
    ETranslationResultType TranslateVaToPa(uint64 VA, uint64& PA, uint32& bank) const override; //!< Translate VA to PA if mapping exists.
    ETranslationResultType TranslatePaToVa(uint64 PA, EMemBankType bank, uint64& VA) const override; //!< Translate PA to VA if mapping exists for this VMAS
    const Page* GetPage(uint64 VA) const override; //!< Return a pointer to the Page that covers the VA if exists.
    const Page* GetPageWithAssert(uint64 VA) const; //!< Return a pointer to the Page object that VA, fail if not exist, be there or be square.
    bool GetTranslationRange(uint64 VA, TranslationRange& rTransRange) const override; //!< Get translation range from Page that covers the VA, if exists.
    uint64 MaxVirtualAddress() const override; //!< Get the max virtual address supported
    uint64 MaxPhysicalAddress() const override; //!< Get the max physical address supported
    bool InitializeRootPageTable(RootPageTable* pRootTable); //!< Initialize the VmAddressSpace object.
    bool CompatiblePageTableContext(const VmAddressSpace* pOtherVmas) const; //!< Return true if vmas passed in has compatible page table context with this vmas
    uint32 Asid() const; //!< Return the ASID of the address space.
    const std::string ControlBlockInfo() const; //!< Return contol block info as a string.
    const VmasControlBlock* GetControlBlock() const { return mpControlBlock; } //!< Return a const pointer to VmasControlBlock object.
    bool UpdateContext(const VmContext* pVmContext); //!< UpdateContextParams in Control Block
    GenPageRequest* DefaultPageRequest(bool isInstr) const; //!< Return a default GenPageRequest object.
    TablePte* CreateNextLevelTable(uint64, const PageTable* parentTable, const GenPageRequest& pPageReq); //!< Create next level page table object.
    void WriteDescriptor(uint64 descrAddr, EMemBankType memBankType, uint64 descrValue, uint32 descrSize); //!< Write descriptor to memory
    const PagingChoicesAdapter* GetChoicesAdapter() const; //!< Get paging choices adapter.
    bool GetPageInfo(uint64 addr, const std::string& type, uint32 bank, PageInformation& rPageInfo) const override; //!< Return the page information record according to the given address/address type
    void HandleMemoryConstraintUpdate(const MemoryConstraintUpdate& rMemConstrUpdate, const Page* pPage); //!< Update virtual constraint to reflect changes in the physical memory constraint. This method should be called for each page containing part of the range specified by the physical addresses in rMemConstrUpdate.
    void AddPhysicalRegion(PhysicalRegion* pRegion, bool map=false) override; //! Add physical region to be mapped by the address space.
    bool VaInAddressErrorRange(const uint64 VA) const override; //!< Return true if VA is in address error constraint set.
    inline const ConstraintSet* GetVmConstraint(EVmConstraintType constrType) const override //!< Return VM constraint of the specified type.
    {
      return mVmConstraints[uint32(constrType)];
    }
    void UpdatePageTableConstraint(uint64 low, uint64 high); //!< add a range of values to the page table constraint set
    EExceptionConstraintType GetExceptionConstraintType(const std::string& rExceptName) const override; //!< Return exception constraint type for the specified paging exception name.
    bool VerifyStreamingPageCrossing(uint64 start, uint64 end) const override; //!< Verify if instruction stream page crossing is okay.
    void ApplyVmConstraints(const GenPageRequest* pPageReq, ConstraintSet& rConstr) const override; //!< Apply VmConstraints on the passed in rConstr.
    EMemBankType DefaultMemoryBank() const override; //!< Return default memory bank.
    bool VirtualMappingAvailable(uint64 start, uint64 end) const; // See if virtual address range intersect with existing pages.
    const Page* CreatePage(uint64 VA, uint64 size, GenPageRequest* pPageReq, PageSizeInfo& rSizeInfo, std::string& rErrMsg); //!< Try to create a page.
    void GetVmContextDelta(std::map<std::string, uint64> & rDeltaMap) const override; //!< Find the delta map between the VmMapper and currect machine state.
    uint32 GenContextId() const override; //!< Return generator Context ID.
    void DumpPage(std::ofstream& os) const override; //!< dump page

    ASSIGNMENT_OPERATOR_ABSENT(VmAddressSpace);

    virtual bool ValidateContext(std::string& rErrMsg) const override { return false; } //!< Validate a VmMapper's context is valid before switching into it.
    virtual RegisterReload* GetRegisterReload() const override { return nullptr; } //!< Get register reload pointer.
    virtual const AddressTagging* GetAddressTagging() const override { return nullptr; } //!< Get address tagging object.
  protected:
    VmAddressSpace(); //!< Default constructor.
    VmAddressSpace(const VmAddressSpace& rOther); //!< Copy constructor.
    void SetupControlBlock(); //!< Setup control block.
    void SetupPageTableConstraints(); //!< Setup Page Table constraints.
    void AddAddressErrorPages(); //!< Add address error pages.
    void PopulateVirtualUsableConstraint(); //!< Will handle the repopulation of virtual usable constraint on vmas becoming active again
    void UpdateVirtualUsableByPage(const Page* pPage); //!< Will perform an update on the virtual constraints in the pages virtual address range
    virtual Page* PageInstance() const; //!< Return an instance of the Page class or its sub-class.
    const Page* MapAddress(uint64 VA, uint64 size, bool isInstr, const GenPageRequest* pPageReq, bool& newAlloc); //!< Map one virtual address.
    const Page* MapAddressForPA(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, const GenPageRequest* pPageReq); //!< Map one virtual address to the given physical address.
    const Page* SetupPageMapping(uint64 VA, uint64 size, bool isSysPage, GenPageRequest* pPageReq); //!< Setup mapping of VA to page
    const Page* SetupPageMappingForPA(uint64 VA, EMemBankType bank, uint64 size, bool isInstr, GenPageRequest* pPageReq); //!< Setup mapping of VA to page
    bool AllocatePhysicalPage(uint64 VA, uint64 size, GenPageRequest* pPageReq, PageSizeInfo& rSizeInfo, EMemBankType memBank, const PagingChoicesAdapter* pChoicesAdapter); //!< Allocate physical page.
    void ConstructPageTableWalk(Page* pageObj, const GenPageRequest& pPageReq); //!< Construct page table walk.
    void CommitPage(const Page* pageObj, uint64 size); //!< Commit page.
    void MapEssentialPhysicalRegions(); //!< Map essential physical regions.
    void MapPhysicalRegions(); //!< Map local physical regions, such as page tables.
    bool MapPhysicalRegion(const PhysicalRegion* pPhysRegion); //!< Map the specified physical region.
    void DumpPageSummary(std::ofstream& os, const Page* pPage) const; //!< dump page summary
  protected:
    VmasControlBlock*       mpControlBlock; //!< VMAS control block.
    Page*                   mpLookUpPage; //!< Page object used for looking up the actual mapping page.
    mutable GenPageRequest*         mpDefaultPageRequest; //!< Pointer to default page request object.
    MemoryConstraint*       mpVirtualUsable; //!< Virtual usable memory constraint.

    std::vector<const Page* >     mPages; //!< Sorted vector holding pointers to all Page objects.
    std::vector<PhysicalRegion* > mPhysicalRegions; //!< Physical regions to be mapped.
    std::vector<Page* > mNoTablePages; //!< The pages that is not part of the page table hierarch.
    std::vector<ConstraintSet* > mVmConstraints; //!< Container of all the applicable VM constraints.
    std::vector<PageTableConstraint* > mPageTableConstraints; //!< Pointer to page table related constraints.
    bool mFlatMapped;

    friend class VmPagingMapper;
  private:
    void UpdateVirtualSharedByPage(const Page* pPage); //!< Update shared memory in the virtual constraint corresponding to the specified page's virtual address range.
  };

}

#endif
