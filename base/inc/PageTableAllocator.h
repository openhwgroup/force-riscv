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
#ifndef Force_PageTableAllocator_H
#define Force_PageTableAllocator_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

namespace Force {

  class ConstraintSet;

  /*!
    \class PageTableAllocator
    \brief This class manages page table allocated.
  */

  class PageTableAllocator {
  public:
    explicit PageTableAllocator(EMemBankType bankType); //!< Constructor with memory bank type given.
    ~PageTableAllocator(); //!< Destructor.

    bool AllocatePageTableBlock(uint64 align, uint64 size, const ConstraintSet* usable, const ConstraintSet* range, uint64& start); //!< Allocate page table with align and size specified.
    inline const ConstraintSet* Allocated() const { return mpAllocated; } //!< Return const pointer to allocate ConstraintSet.

    ASSIGNMENT_OPERATOR_ABSENT(PageTableAllocator);
    COPY_CONSTRUCTOR_ABSENT(PageTableAllocator);
  protected:
    PageTableAllocator() : mMemoryBank(EMemBankType(0)), mpAllocated(nullptr) { } //!< Default constructor.
  protected:
    EMemBankType mMemoryBank; //!< Memory bank for the page tables.
    ConstraintSet* mpAllocated; //!< Allocated page table regions.
  };

}

#endif
