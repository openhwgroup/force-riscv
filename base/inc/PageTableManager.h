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
#ifndef PAGE_TABLE_MANAGER_H
#define PAGE_TABLE_MANAGER_H

#include <Object.h>
#include <Defines.h>
#include <Enums.h>
#include <PageTableAllocator.h>
#include ARCH_ENUM_HEADER
#include <map>
#include <vector>

namespace Force
{
  class  TablePte;
  class  PageTable;
  class  RootPageTable;
  class  ConstraintSet;
  class  VmConstraint;
  class  VmAddressSpace;
  class  PagingChoicesAdapter;
  class  PageTableAllocator;

  /*!
    \class PageTableManager
    \brief Top level module for managing allocated page tables for sharing and allocation solving
  */

  class PageTableManager : public Object
  {
  public:
    explicit PageTableManager(EMemBankType bankType);         //!< Constructor, sets mem bank type defaults all others
    virtual ~PageTableManager();                     //!< Destructor, cleans up cloned objects and managed structures

    Object* Clone() const override              { return nullptr; }            //!< override for Object Clone / returns nullptr, object not meant to be copied
    const std::string ToString() const override { return "PageTableManager"; } //!< override for Object ToString
    const char* Type() const override           { return "PageTableManager"; } //!< override for Object Type

    bool AllocatePageTable(VmAddressSpace* pVmas); //!< allocate new or use existing page table
    void CommitPageTable(PageTable* pTable, const VmAddressSpace* pVmas); //!< add page table and link to vmas

    bool AllocateRootPageTable(VmAddressSpace* pVmas); //!< allocate a new or alias an existing root page table
    void CommitRootPageTable(RootPageTable* pRootTable); //!< add a newly created root page table to the sorted vector of root tables

    //Interfaces to PageTableAllocator functions
    const ConstraintSet* Allocated() const { return mpPageTableAllocator->Allocated(); } //!< Interface to get the allocated constraint set from PTA
    bool AllocatePageTableBlock(uint64 align, uint64 size, const ConstraintSet* usable, const ConstraintSet* range, uint64& start) { return mpPageTableAllocator->AllocatePageTableBlock(align, size, usable, range, start); }
  private:
    bool NewRootPageTable(VmAddressSpace* pVmas); //!< function to attempt to create a new root page table for a given context.
    bool AliasRootPageTable(VmAddressSpace* pVmas); //!< function to attempt to alias an existing root table based on context matching the address spaces

    void UpdateVmConstraints(const PageTable* pTable); //!< utility function to extract the constraints from the table entry and apply them to the managed set

    //std::vector<PageTable*> mPageTables; //!< sorted vector of page tables mapped in this memory bank
    std::vector<RootPageTable*> mRootPageTables; //!< sorted vector of root page tables mapped in this bank
    std::vector<VmConstraint*> mVmConstraints; //!< vector of VM Constraints describing current context of existing mappings

    //std::vector<std::map<ContextGroup, PageTable*> > sort page tables in asc mem order by context group? check which vmas correspond to which groups?
    //need to think about storage format in order to support both fullshare/partial share scenarios

    EMemBankType mBankType;

    PageTableAllocator* mpPageTableAllocator;

    ASSIGNMENT_OPERATOR_ABSENT(PageTableManager);
    COPY_CONSTRUCTOR_ABSENT(PageTableManager);
  };

  bool root_page_table_less_than(const RootPageTable* pLhsRpt, const RootPageTable* pRhsRpt); //!< comp function for page table sort/search algos
}

#endif //PAGE_TABLE_MANAGER_H
