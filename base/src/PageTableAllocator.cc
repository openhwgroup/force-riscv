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
#include <PageTableAllocator.h>
#include <Constraint.h>
#include <PaGenerator.h>
#include <GenException.h>
#include <Log.h>

using namespace std;

namespace Force {

  PageTableAllocator::PageTableAllocator(EMemBankType bankType)
    : mMemoryBank(bankType), mpAllocated(nullptr)
  {
    mpAllocated = new ConstraintSet();
  }

  PageTableAllocator::~PageTableAllocator()
  {
    delete mpAllocated;
  }

  bool PageTableAllocator::AllocatePageTableBlock(uint64 align, uint64 size, const ConstraintSet* usable, const ConstraintSet* range, uint64& start)
  {
    bool allocated = true;
    PaGenerator pa_gen(usable);
    try
    {
      bool is_instr = false;
      start = pa_gen.GenerateAddress(align, size, is_instr, range);
    }
    catch (const ConstraintError& alloc_err)
    {
      LOG(notice) << "{PageTableAllocator::AllocatePageTableBlock} out of page table space allocating with align: 0x" << hex << align << " size: 0x" << size << ", Error is " << alloc_err.what() << endl;
      allocated = false;
    }

    if (allocated)
    {
      uint64 pt_end = start + (size - 1);
      LOG(notice) << "{PageTableAllocator::AllocatePageTableBlock} Allocated page table at 0x" << hex << start << "-0x" << pt_end << " align 0x" << align << " from memory bank " << EMemBankType_to_string(mMemoryBank) << endl;
      mpAllocated->AddRange(start, pt_end);
    }
    return allocated;
  }
}
