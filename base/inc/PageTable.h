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
#ifndef Force_PageTable_H
#define Force_PageTable_H

#include <Defines.h>
#include <vector>
#include <map>

namespace Force {

  class PageTableEntry;
  class VmAddressSpace;
  class Page;
  class GenPageRequest;
  class TablePte;
  struct PageTableInfoRec;

  /*!
    \class PageTable
    \brief Page table holdes pointers to all allocated pages or down stream tables.
  */
  class PageTable {
  public:
    PageTable(); //!< Constructor.
    virtual ~PageTable(); //!< Destructor.

    void SetLookUpBitRange(uint32 lowBit, uint32 highBit); //!< Set the lookup bit range.
    void SetMemoryBank(EMemBankType memBank) { mMemoryBank = memBank; } //!< Set memory bank of the page table.
    void SetTableBase(uint64 tableBase)         { mTableBase = tableBase; } //!< Set table base address.
    void SetLevel(uint32 level)                 { mTableLevel = level; } //!< Set table level.

    EMemBankType MemoryBank() const { return mMemoryBank; } //!< Return the page table's memory bank type.
    uint32 LowestLookUpBit()  const { return mLowestLookUpBit; } //!< Return value of the lowest look up bit.
    uint32 TableLevel()       const { return mTableLevel; } //!< Return table level.
    uint64 TableBase()        const { return mTableBase; } //!< Return table base address.
    uint32 PteShift()         const { return 3ul; } //!< Return pte shift

    bool UnconstructedTableLevel(const Page* pageObj, uint64& rLevel); //!< Get level of first unconstructed table in page table walk, returns page level if tablewalk is already present
    void ConstructPageTableWalk(uint64 VA, Page* pageObj, VmAddressSpace* pVmas, const GenPageRequest& pPageReq); //!< Construct page table walk details.
    const TablePte* PageTableWalk(const Page* pageObj, const VmAddressSpace* pVmas, PageTableInfoRec& page_table_rec) const; // page table walk one step at a time without table construction
    const std::string PageTableInfo() const; //!< Return brief page table info in a string format.
  protected:
    ASSIGNMENT_OPERATOR_ABSENT(PageTable);
    //COPY_CONSTRUCTOR_ABSENT(PageTable);
    PageTable(const PageTable& rOther); //!< Copy constructor.
    uint32 GetPteIndex(uint64 address) const; //!< Get index for associated PTE given the address it covers.
    TablePte* GetNextLevelTable(uint64 pageStart) const; //!< Get next level table that covers the address passed in.
    void CommitPageTableEntry(uint64 pageStart, PageTableEntry* pPte, VmAddressSpace* pVmas); //!< Insert PTE object into the table.
  protected:
    EMemBankType mMemoryBank; //!< The memory bank where the page table is located.
    uint32 mMask; //!< Mask to extract index for PTE in the table.
    uint32 mLowestLookUpBit; //!< LSB of the look up bit range.
    uint32 mTableLevel; //!< Level of the table.
    uint64 mTableBase; //!< Table base address.
    std::map<uint32, PageTableEntry* > mEntries; //!< Container of all allocated pages or down stream tables.
  };

  /*!
    \class RootPageTable
    \brief Root page table holds pointers to address-spaces that are using this RootPageTable and its down stream PTEs.
  */
  class RootPageTable : public PageTable {
  public:
    RootPageTable(); //!< Constructor.
    ~RootPageTable(); //!< Destructor.
    void Setup(uint32 tableStep, uint32 highBit, const std::string& rPteSuffix); //!< Setup Page Table parameters
    void SignUp(VmAddressSpace* addressSpace); //!< Sign up with the RootPageTable.
    void SetLookUpBitRangeRoot(uint32 lowBit,uint32 highBit); //!< Set the lookup bit range for Root
    uint32 HighestLookUpBit()     const { return mHighestLookUpBit; } //!< Return value of the lowest look up bit.
    uint32 RootTableSize()        const { return 1ul << (mHighestLookUpBit - mLowestLookUpBit + 1 + PteShift()); } //!< return size of root table
    uint32 TableSize()            const { return 1ul << (mTableStep + PteShift()); } //!< Return size of table
    uint64 TableStep()            const { return mTableStep; } //!< Return table base address.
    uint32 MaxTableLevel()        const { return 3ul; } //!< Return max supported table level
    const VmAddressSpace* GetBaseVmas() const { return mpBaseAddressSpace; } //!< Return pointer to first registered address space, used to determine compatible context for aliasing
    std::string TableIdentifier() const { return mTableIdentifier; } //!< Return table identifier
    std::vector<VmAddressSpace* > & GetAddressSpaces() { return mAddressSpaces; } //!< Return reference to the vector of VmAddressSpace objects.
  protected:
    RootPageTable(const RootPageTable& rOther); //!< Copy constructor.
    ASSIGNMENT_OPERATOR_ABSENT(RootPageTable);
    //COPY_CONSTRUCTOR_ABSENT(RootPageTable);
  protected:
    uint32 mHighestLookUpBit; //!< MSb of the look up bit range.
    uint32 mTableStep; //!< Number of bits resovled by this table.
    VmAddressSpace* mpBaseAddressSpace; //!< Initial address space is used to identify the context compatibility when aliasing root page tables.
    std::string mTableIdentifier; //!< String to identify this table size's choices
    std::vector<VmAddressSpace* > mAddressSpaces; //!< Pointer to all address spaces using this RootPageTable and its down stream PTEs.
  };

}

#endif
