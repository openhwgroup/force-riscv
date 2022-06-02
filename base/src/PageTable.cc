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
#include "PageTable.h"

#include <sstream>

#include "Log.h"
#include "Page.h"
#include "PageInfoRecord.h"
#include "PteAttribute.h"
#include "UtilityAlgorithms.h"
#include "UtilityFunctions.h"
#include "VmAddressSpace.h"
#include "VmUtils.h"
#include "VmasControlBlock.h"

/*!
  \file PageTable.cc
  \brief Code supporting page table management.
*/

using namespace std;

namespace Force {

  PageTable::PageTable()
    : mMemoryBank(EMemBankType(0)), mMask(0), mLowestLookUpBit(0), mTableLevel(0), mTableBase(0), mEntries()
  {

  }

  PageTable::PageTable(const PageTable& rOther)
    : mMemoryBank(EMemBankType(0)), mMask(0), mLowestLookUpBit(0), mTableLevel(0), mTableBase(0), mEntries()
  {

  }

  PageTable::~PageTable()
  {
    for (auto entry_iter : mEntries) {
      delete entry_iter.second;
    }
    // << "page table deleted." << endl;
  }

  void PageTable::SetLookUpBitRange(uint32 lowBit, uint32 highBit)
  {
    mLowestLookUpBit = lowBit;
    if (highBit < lowBit) {
      LOG(fail) << "{PageTable::SetLookUpBitRange} high bit " << dec << highBit << " is lower than low bit " << lowBit << endl;
      FAIL("incorrect-high-low-bit");
    }

    mMask = (1u << (highBit - lowBit + 1)) - 1;
  }

  uint32 PageTable::GetPteIndex(uint64 address) const
  {
    // << "GetPteIndex : address 0x" << hex << address << " shift value " << dec << mLowestLookUpBit << " shifted address 0x" << hex << (address >> mLowestLookUpBit) << " mask 0x" << mMask << endl;
    return (address >> mLowestLookUpBit) & mMask;
  }

  bool PageTable::UnconstructedTableLevel(const Page* pageObj, uint64& rLevel)
  {
    uint32 level_gap = (TableLevel() - pageObj->Level());
    if (level_gap > MAX_PAGE_TABLE_LEVEL)
    {
      LOG(fail) << "{VmAddressSpace::PageTableWalk} level gap too large: " << dec << level_gap << " page level " << pageObj->Level() << " table level " << TableLevel() << endl;
      FAIL("table-level-gap-too-large");
    }

    uint64 page_start = pageObj->Lower();
    if (level_gap == 0)
    {
      rLevel = pageObj->Level();
      return false;
    }

    auto pPte = GetNextLevelTable(page_start);
    if (nullptr == pPte)
    {
      rLevel = TableLevel();
      return true;
    }

    return pPte->UnconstructedTableLevel(pageObj, rLevel);
  }

  const TablePte* PageTable::PageTableWalk(const Page* pageObj, const VmAddressSpace* pVmas, PageTableInfoRec& page_table_rec) const
  {
    uint32 level_gap = (TableLevel() - pageObj->Level());
    if (level_gap > MAX_PAGE_TABLE_LEVEL) {
      LOG(fail) << "{VmAddressSpace::PageTableWalk} level gap too large: " << dec << level_gap << " page level " << pageObj->Level() << " table level " << TableLevel() << endl;
      FAIL("table-level-gap-too-large");
    }

    uint64 page_start = pageObj->Lower();
    uint32 pte_index = GetPteIndex(page_start);
    page_table_rec.descr_addr = TableBase() + (pte_index << pVmas->GetControlBlock()->PteShift());
    page_table_rec.level = TableLevel();
    if (level_gap == 0) {
      page_table_rec.descr_value = pageObj->Descriptor();
      pageObj->DescriptorDetails(page_table_rec.descr_details);
      return nullptr;
    }

    auto pPte = GetNextLevelTable(page_start);
    if (nullptr == pPte) {
      LOG(fail) << "{VmAddressSpace::PageTableWalk} next table doesn't exist: page level: " << dec << pageObj->Level() << " table level " << TableLevel() << endl;
      FAIL("next-level-table-not-found");
    }

    page_table_rec.descr_value = pPte->Descriptor();
    pPte->DescriptorDetails(page_table_rec.descr_details);

    return pPte;
  }

  void PageTable::ConstructPageTableWalk(uint64 VA, Page* pageObj, VmAddressSpace* pVmas, const GenPageRequest& pPageReq)
  {
    uint32 level_gap = (TableLevel() - pageObj->Level());
    if (level_gap > MAX_PAGE_TABLE_LEVEL) {
      LOG(fail) << "{VmAddressSpace::ConstructPageTableWalk} level gap too large: " << dec << level_gap << " page level " << pageObj->Level() << " table level " << TableLevel() << endl;
      FAIL("table-level-gap-too-large");
    }

    uint64 page_start = pageObj->Lower();
    if (level_gap == 0) {
      // insert Page object to this table.
      CommitPageTableEntry(page_start, pageObj, pVmas);
      return;
    }

    auto next_level_table = GetNextLevelTable(page_start);
    if (nullptr == next_level_table) {
      // need to create more level(s) of tables.
      next_level_table = pVmas->CreateNextLevelTable(VA, this, pPageReq);
      CommitPageTableEntry(page_start, next_level_table, pVmas);
    } else {
      // << "found existing page table : " << next_level_table->ToString() << endl;
    }

    next_level_table->ConstructPageTableWalk(VA, pageObj, pVmas, pPageReq);
  }

  TablePte* PageTable::GetNextLevelTable(uint64 pageStart) const
  {
    uint32 pte_index = GetPteIndex(pageStart);
    auto pte_finder = mEntries.find(pte_index);
    if (pte_finder != mEntries.end()) {
      TablePte* cast_pte = dynamic_cast<TablePte* > (pte_finder->second);
      if (nullptr == cast_pte) {
        LOG(fail) << "{PageTable::GetNextLevelTable} expecting a TablePte type object but getting " << pte_finder->second->Type() << endl;
        FAIL("incorrect-table-entry-type");
      }
      return cast_pte;
    }

    return nullptr;
  }

  void PageTable::CommitPageTableEntry(uint64 pageStart, PageTableEntry* pPte, VmAddressSpace* pVmas)
  {
    uint32 pte_index = GetPteIndex(pageStart);
    auto pte_finder = mEntries.find(pte_index);
    if (pte_finder != mEntries.end()) {
      LOG(fail) << "{PageTable::CommitPageTableEntry} inserting PTE where there is an existing PTE at index 0x" << hex << pte_index << " in " << this->PageTableInfo() << " descriptor: " << pPte->DescriptorDetails() << " existing descriptor: " << pte_finder->second->DescriptorDetails() << endl;
      FAIL("duplicated-pte-in-table");
    }
    mEntries[pte_index] = pPte;

    // write descriptor to memory.
    uint64 descr_addr = TableBase() + (pte_index << pVmas->GetControlBlock()->PteShift());
    uint64 descr_value = pPte->Descriptor();
    LOG(notice) << "Writing descriptor 0x" << hex << descr_value << " to address [" << uint32(mMemoryBank) << "]0x" << descr_addr << " size " << dec << pPte->DescriptorSize() << " " << pPte->DescriptorDetails() << endl;
    pVmas->WriteDescriptor(descr_addr, mMemoryBank, descr_value, pPte->DescriptorSize() / 8);
  }

  const string PageTable::PageTableInfo() const
  {
    stringstream out_str;

    out_str << "PageTable: " << EMemBankType_to_string(mMemoryBank) << hex << " Level=0x" << mTableLevel << " Base=0x" << mTableBase << " Mask=0x" << mMask << " Lowest-lookup-bit=" << dec << mLowestLookUpBit;

    return out_str.str();
  }

  RootPageTable::RootPageTable()
    : PageTable(), mHighestLookUpBit(0), mTableStep(0), mPteShift(0), mMaxTableLevel(0), mpBaseAddressSpace(nullptr), mTableIdentifier(""), mAddressSpaces()
  {
  }

  //move to unprotected if needed and check impl
  RootPageTable::RootPageTable(const RootPageTable& rOther)
    : PageTable(rOther), mHighestLookUpBit(rOther.mHighestLookUpBit), mTableStep(rOther.mTableStep),
      mPteShift(rOther.mPteShift), mMaxTableLevel(rOther.mMaxTableLevel), 
      mpBaseAddressSpace(nullptr), mTableIdentifier(rOther.mTableIdentifier), mAddressSpaces()
  {
  }

  RootPageTable::~RootPageTable()
  {
  }

  void RootPageTable::Setup(uint32 tableStep, uint32 highBit, uint32 tableLowBit, const string& rPteSuffix, uint32 rPteShift, uint32 rMaxTableLevel)
  {
    mTableStep        = tableStep;
    mHighestLookUpBit = highBit;
    mPteShift         = rPteShift;
    mMaxTableLevel    = rMaxTableLevel;

    uint32 init_table_level = mMaxTableLevel + 1;

    mTableLevel = init_table_level - 1;
    SetLookUpBitRange(tableLowBit, highBit);

    uint32 table_size = TableSize();
    string psize_string = string(get_page_size_string(table_size));
    mTableIdentifier  = string("P") + psize_string + string("#Table") + rPteSuffix;
  }

  void RootPageTable::SignUp(VmAddressSpace* addressSpace)
  {
    if (nullptr == mpBaseAddressSpace)
    {
      mpBaseAddressSpace = addressSpace;
    }

    insert_sorted<VmAddressSpace *>(mAddressSpaces, addressSpace);
  }

  void RootPageTable::SetLookUpBitRangeRoot(uint32 lowBit,uint32 highBit)
  {
    mHighestLookUpBit = highBit;
    SetLookUpBitRange(lowBit,highBit);
  }
}
