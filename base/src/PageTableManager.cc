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
#include <Defines.h>
PICKY_IGNORED
#include <PageTableManager.h>
#include <Constraint.h>
#include <Page.h>
#include <PageTable.h>
#include <PageTableAllocator.h>
#include <VmConstraint.h>
#include <VmAddressSpace.h>
#include <VmUtils.h>
#include <PagingChoicesAdapter.h>
#include <UtilityFunctions.h>
#include <Log.h>

#include <algorithm>
#include <memory>

using namespace std;

namespace Force
{

  PageTableManager::PageTableManager(EMemBankType bankType) : mBankType(bankType), mpPageTableAllocator(nullptr)
  {
    mpPageTableAllocator = new PageTableAllocator(bankType);
  }

  PageTableManager::~PageTableManager()
  {
    for (auto rpt : mRootPageTables)
    {
      delete rpt;
    }

    for (auto vmc : mVmConstraints)
    {
      delete vmc;
    }

    delete mpPageTableAllocator;
  }

  bool PageTableManager::AllocateRootPageTable(VmAddressSpace* pVmas)
  {
    //attempt to call into the setup to allocate a new root page table, or alias an existing root table based on context match
    const PagingChoicesAdapter* pChoicesAdapter = pVmas->GetChoicesAdapter();

    bool alias_first = (pChoicesAdapter->GetPlainPagingChoice("Root Page Table Aliasing") == 1);

    if (alias_first)
    {
      bool ret_val = AliasRootPageTable(pVmas);
      if (false == ret_val)
      {
        ret_val = NewRootPageTable(pVmas);
      }
      return ret_val;
    }
    else
    {
      bool ret_val = NewRootPageTable(pVmas);
      if (false == ret_val)
      {
        ret_val = AliasRootPageTable(pVmas);
      }
      return ret_val;
    }
  }

  bool PageTableManager::NewRootPageTable(VmAddressSpace* pVmas)
  {
    return pVmas->InitializeRootPageTable(nullptr);
  }

  bool PageTableManager::AliasRootPageTable(VmAddressSpace* pVmas)
  {
    for (auto root_table : mRootPageTables)
    {
      auto base_vmas = root_table->GetBaseVmas();
      if (nullptr == base_vmas)
      {
        LOG(fail) << "{PageTableManager::AliasRootPageTable} base vmas of root table is not initialized. root_table=" << root_table << endl;
      }
      bool compatible_context = base_vmas->CompatiblePageTableContext(pVmas);
      if (compatible_context)
      {
        //might not be a case where this would fail, check conditions when implementing alias in CB
        return pVmas->InitializeRootPageTable(root_table);
      }
    }

    return false;
  }

  void PageTableManager::CommitRootPageTable(RootPageTable* pRootTable)
  {
    //checking for overlap done when trying to allocate new root page table
    auto insert_it = std::lower_bound(mRootPageTables.begin(), mRootPageTables.end(), pRootTable, &root_page_table_less_than);
    mRootPageTables.insert(insert_it, pRootTable);
    UpdateVmConstraints(pRootTable);
  }

  bool PageTableManager::AllocatePageTable(VmAddressSpace* pVmas)
  {
    //need to collect the current context/page table representation of the current context
    //need to query solving module with necessary context for valid allocation/alias
    //temp: will currently just call into existing solving, without doing additional context verification/aliasing solving.
    return false;
  }

  void PageTableManager::CommitPageTable(PageTable* pTable, const VmAddressSpace* pVmas)
  {
    //add page table object ownership to PTM class, store
    //deduce parameter ranges potentially affected by new tablewalk mapping
    //update the mConstraints vector of cset's accordingly.
  }

  void PageTableManager::UpdateVmConstraints(const PageTable* pTable)
  {
  }

  bool root_page_table_less_than(const RootPageTable* pLhsRpt, const RootPageTable* pRhsRpt)
  {
    return (pLhsRpt->TableBase() + pLhsRpt->TableSize() - 1) < pRhsRpt->TableBase();
  }

}
