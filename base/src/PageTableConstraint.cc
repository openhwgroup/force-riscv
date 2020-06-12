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
#include <PageTableConstraint.h>
#include <Constraint.h>
#include <PaGenerator.h>
#include <Random.h>
#include <GenException.h>
#include <Log.h>
#include <VmasControlBlock.h>
#include <MemoryManager.h>
#include <PageTable.h>
#include <Variable.h>
#include <Generator.h>

#include <memory>

using namespace std;

namespace Force {

  PageTableConstraint::PageTableConstraint(EMemBankType bankType)
    : mMemoryBank(bankType), mpBaseUsable(nullptr), mpUsable(nullptr), mpAllocated(nullptr), mpBlockAllocated(nullptr), mPageTableSize(0), mpAllocatingNumber(nullptr), mAllocateOrder(false)
  {
  }

  PageTableConstraint::~PageTableConstraint()
  {
    delete mpBaseUsable;
    delete mpUsable;
    delete mpAllocated;
    delete mpBlockAllocated;
  }

  void PageTableConstraint::Setup(Generator* pGen, VmasControlBlock* pControlBlock)
  {
    mpBaseUsable = pControlBlock->GetPageTableUsableConstraint(mMemoryBank);
    mpUsable     = new ConstraintSet();
    mpAllocated  = new ConstraintSet();
    mpBlockAllocated = new ConstraintSet();
    mAllocateOrder = (Random::Instance()->Random32(0, 1) == 1);
    uint64 bit_55_VA = 0x1ull << 55ull;
    uint64 lo_table_size = pControlBlock->GetRootPageTable()->TableSize();
    uint64 hi_table_size = pControlBlock->GetRootPageTable(bit_55_VA)->TableSize();
    mPageTableSize = std::max(lo_table_size, hi_table_size);
    const VariableModerator* var_mod = pGen->GetVariableModerator(EVariableType::Value);
    mpAllocatingNumber = var_mod->GetVariableSet()->FindVariable("Page tables number per block allocating");
    AllocateUsableBlock();
  }

  uint64 PageTableConstraint::AllocatePageTable(uint64 pageTableAlign, uint64 pageTableSize, const ConstraintSet* excludeRegion)
  {
    PaGenerator pa_gen(mpUsable);
    uint64 pt_start = 0;
    bool allocated = false;

    while (not allocated)
    {
      try
      {
        if (mAllocateOrder)
          pt_start = pa_gen.GetAddressFromBottom(pageTableAlign, pageTableSize, excludeRegion);
        else
          pt_start = pa_gen.GetAddressFromTop(pageTableAlign, pageTableSize, excludeRegion);
        allocated = true;
      }
      catch (const ConstraintError& alloc_err)
      {
        LOG(info) << "{PageTableConstraint::AllocatePageTable} out of page table space allocating with align: 0x" << hex << pageTableAlign << " size: 0x" << pageTableSize << ", Error is " << alloc_err.what() << endl;
        AllocateUsableBlock(excludeRegion);
      }
    }

    uint64 pt_end = pt_start + (pageTableSize - 1);
    LOG(info) << "[PageTableConstraint::AllocatePageTable] " << EMemBankType_to_string(mMemoryBank) << " allocated page table: 0x" << hex << pt_start << "-0x" << pt_end << endl;
    mpUsable->SubRange(pt_start, pt_end);
    mpAllocated->AddRange(pt_start, pt_end);
    return pt_start;
  }

  void PageTableConstraint::AllocateUsableBlock(const ConstraintSet* pExcludeRegion)
  {
    bool allocated = false;
    MemoryManager* mem_manager = MemoryManager::Instance();
    uint64 block_size = mPageTableSize * mpAllocatingNumber->Value();
    uint64 block_start = 0;
    const ConstraintSet* local_constr = mpBaseUsable;
    std::unique_ptr<ConstraintSet> tmp_constr_storage;

    if (pExcludeRegion != nullptr) {
      tmp_constr_storage = std::unique_ptr<ConstraintSet>(mpBaseUsable->Clone());
      tmp_constr_storage.get()->SubConstraintSet(*pExcludeRegion);
      local_constr = tmp_constr_storage.get();
    }

    while (not allocated and (block_size > mPageTableSize))
    {
      allocated = mem_manager->AllocatePageTableBlock(mMemoryBank, mPageTableSize, block_size, local_constr, block_start);

      if (not allocated) {
        block_size >>= 1; // halve.
      }
    }

    if (allocated)
    {
      uint64 block_end = block_start + (block_size - 1);
      mpUsable->AddRange(block_start, block_end);
      mpBlockAllocated->AddRange(block_start, block_end);
      LOG(notice) << "{PageTableConstraint::AllocateUsableBlock} allocating new page table block start:0x" << hex << block_start << " end:0x" << block_end << endl;
    }
    else
    {
      LOG(fail) << "{PageTableConstraint::AllocateUsableBlock} out of page table space allocating" << endl;
      FAIL("out-of-page-table-space");
    }
  }

}

