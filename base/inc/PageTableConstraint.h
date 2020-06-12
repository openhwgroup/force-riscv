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
#ifndef Force_PageTableConstraint_H
#define Force_PageTableConstraint_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

namespace Force {

  class ConstraintSet;
  class Generator;
  class VmasControlBlock;
  class Variable;

  /*!
    \class PageTableConstraint
    \brief This class manages page table allocated related constraints.
  */

  class PageTableConstraint {
  public:
    explicit PageTableConstraint(EMemBankType bankType); //!< Constructor with memory bank type given.
    ~PageTableConstraint(); //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(PageTableConstraint); //!< Absent operator "=".
    COPY_CONSTRUCTOR_ABSENT(PageTableConstraint); //!< Absent copy constructor.

    void Setup(Generator* pGen, VmasControlBlock* pControlBlock); //!< Setup function.
    uint64 AllocatePageTable(uint64 pageTableAlign, uint64 pageTableSize, const ConstraintSet* excludeRegion=nullptr); //!< Allocate page table with align and size specified.
    inline const ConstraintSet* Allocated() const { return mpAllocated; } //!< Return all allocated page tables.
    const ConstraintSet* BlockAllocated() const { return mpBlockAllocated; } //!< Return all page table block addresses whether or not they have been allocated for a specific page table.
  protected:
    void AllocateUsableBlock(const ConstraintSet* pExcludeRegion=nullptr);   //!< Allocate current page table usable region.
  protected:
    EMemBankType   mMemoryBank;   //!< Memory bank for the page tables.
    ConstraintSet* mpBaseUsable;  //!< Base usable memory constraint for PageTable.
    ConstraintSet* mpUsable;      //!< Current usable memory constraint for PageTable.
    ConstraintSet* mpAllocated;   //!< Allocated page table regions.
    ConstraintSet* mpBlockAllocated; //!< Page table block addresses that may or may not have been allocated for a specific page table.
    uint64 mPageTableSize;        //!< Page table granule size.
    const Variable* mpAllocatingNumber; //!< Number of allocating page tables.
    bool mAllocateOrder;          //!< Indicate if we allocate page table from the top of the range.
  };

}

#endif
