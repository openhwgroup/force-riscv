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
#include "PhysicalPage.h"

#include <algorithm>

#include "Constraint.h"
#include "Log.h"
#include "MemoryConstraintUpdate.h"
#include "Page.h"
#include "VmAddressSpace.h"
#include "VmasControlBlock.h"

using namespace std;

namespace Force
{
  PhysicalPage::PhysicalPage() : mLower(0), mUpper(0), mCanAlias(true), mPageId(0), mVirtualPages()
  {
  }

  PhysicalPage::PhysicalPage(uint64 lower, uint64 upper) : mLower(lower), mUpper(upper), mCanAlias(true), mPageId(0), mVirtualPages()
  {
  }

  PhysicalPage::PhysicalPage(uint64 lower, uint64 upper, uint64 pageId) : mLower(lower), mUpper(upper), mCanAlias(true), mPageId(pageId), mVirtualPages()
  {
  }

  PhysicalPage::PhysicalPage(uint64 lower, uint64 upper, bool alias, uint64 pageId) : mLower(lower), mUpper(upper), mCanAlias(alias), mPageId(pageId),mVirtualPages()
  {
  }

  PhysicalPage::PhysicalPage(const Page* pPage, uint64 lower, uint64 upper, uint64 pageId) : mLower(lower), mUpper(upper), mCanAlias(true), mPageId(pageId), mVirtualPages()
  {
    mVirtualPages.push_back(pPage);
  }

  PhysicalPage::PhysicalPage(const Page* pPage, uint64 lower, uint64 upper, bool alias, uint64 pageId) : mLower(lower), mUpper(upper), mCanAlias(alias), mPageId(pageId), mVirtualPages()
  {
    mVirtualPages.push_back(pPage);
  }

  PhysicalPage::~PhysicalPage()
  {
    mVirtualPages.clear();
  }

  PhysicalPage::PhysicalPage(const PhysicalPage& rOther) : mLower(rOther.mLower), mUpper(rOther.mUpper), mCanAlias(rOther.mCanAlias), mPageId(rOther.mPageId), mVirtualPages(rOther.mVirtualPages)
  {
  }

  ConstraintSet* PhysicalPage::GetMemoryAttributes() const
  {
    if (mVirtualPages.empty())
    {
      LOG(fail) << "{PhysicalPage::GetMemoryAttribute} no virtual pages assigned to retrieve memory attribute from" << endl;
      FAIL("physical_page_get_mem_attr_no_pages");
    }

    ConstraintSet* mem_attrs = new ConstraintSet();
    for (auto page : mVirtualPages)
    {
      uint32 attr = page->PageGenAttributeDefaultZero(EPageGenAttributeType::MemAttrImpl);
      if (attr == 0) continue;

      if (mem_attrs->ContainsValue(attr))
      {
        LOG(trace) << "{PhysicalPage::GetMemoryAttribute} virtual page shares existing attribute, attr=0x" << hex << attr << endl;
      }
      else
      {
        mem_attrs->AddValue(attr);
      }
    }

    return mem_attrs;
  }

  void PhysicalPage::HandleMemoryConstraintUpdate(const MemoryConstraintUpdate& rMemConstrUpdate) const
  {
    if (!mVirtualPages.empty())
    {
      for (auto page : mVirtualPages)
      {
        LOG(trace) << "{PhysicalPage::HandleMemoryConstraintUpdate} v_page p_lower=0x" << page->PhysicalLower() << " p_upper=0x" << page->PhysicalUpper() << endl;
        if ((rMemConstrUpdate.GetPhysicalStartAddress() <= page->PhysicalUpper()) && (page->PhysicalLower() <= rMemConstrUpdate.GetPhysicalEndAddress())) //range intersects with page
        {
          std::vector<VmAddressSpace*> all_page_vmas;
          if (page->GetRootPageTable() != nullptr)
          {

            LOG(trace) << "{PhysicalPage::HandleMemoryConstraintUpdate} found v_page" << endl;
            all_page_vmas = page->GetRootPageTable()->GetAddressSpaces();
            for (auto vmas : all_page_vmas)
            {
              if (vmas->IsActive() || vmas->IsInitialized())
              {
                LOG(trace) << "{PhysicalPage::HandleMemoryConstraintUpdate} found active vmas " << EMemBankType_to_string(vmas->GetControlBlock()->DefaultMemoryBank()) << endl;
                vmas->HandleMemoryConstraintUpdate(rMemConstrUpdate, page);
              }
              else
              {
                LOG(trace) << "{PhysicalPage::HandleMemoryConstraintUpdate} found inactive vmas " << EMemBankType_to_string(vmas->GetControlBlock()->DefaultMemoryBank()) << endl;
              }
            }
          }
        }
      }
    }
  }

  const Page* PhysicalPage::GetVirtualPage(uint64 PA, const VmAddressSpace* pVmas) const
  {
    if (!mVirtualPages.empty())
    {
      for (auto page : mVirtualPages)
      {
        if (page->ContainsPa(PA))
        {
          if (page->GetRootPageTable() != nullptr)
          {
            RootPageTable* root_page_table = page->GetRootPageTable();
            vector<VmAddressSpace*>& addr_spaces = root_page_table->GetAddressSpaces();

            bool addr_space_matches = any_of(addr_spaces.cbegin(), addr_spaces.cend(),
              [pVmas](const VmAddressSpace* pAddrSpace) { return (pAddrSpace == pVmas); });

            if (addr_space_matches) {
              return page;
            }
          }
        }
      }
    }

    return nullptr;
  }

  void PhysicalPage::AddPage(const Page* pPage)
  {
    auto insert_it = std::lower_bound(mVirtualPages.begin(), mVirtualPages.end(), pPage, &page_less_than);
    mVirtualPages.insert(insert_it, pPage);
  }

  void PhysicalPage::Merge(PhysicalPage* pPhysPage)
  {
    if (!(pPhysPage->Lower() >= Lower() && pPhysPage->Upper() <= Upper()))
    {
      LOG(fail) << "{PhysicalPage::Merge} trying to merge pages without range intersection, larger page range start=0x"
                << hex << Lower() << " end=0x" << Upper() << " smaller page range start=0x" << pPhysPage->Lower() << " end=0x"
                << pPhysPage->Upper() << endl;
      FAIL("physical_page_merge_with_no_intersection");
    }

    LOG(info) << "{PhysicalPage::Merge} trying to merge pages larger page range start=0x" << hex << Lower()
              << " end=0x" << Upper() << " smaller page range start=0x" << pPhysPage->Lower() << " end=0x"
              << pPhysPage->Upper() << endl;

    std::vector<const Page*> merged_pages;

    auto curr_it  = mVirtualPages.begin();
    auto other_it = pPhysPage->mVirtualPages.begin();
    while (!(curr_it == mVirtualPages.end() && other_it == pPhysPage->mVirtualPages.end()))
    {
      if (curr_it == mVirtualPages.end())
      {
        merged_pages.push_back(*other_it);
        ++other_it;
      }
      else if (other_it == pPhysPage->mVirtualPages.end())
      {
        merged_pages.push_back(*curr_it);
        ++curr_it;
      }
      else
      {
        if (page_less_than(*curr_it, *other_it)) //current page compares less
        {
          merged_pages.push_back(*curr_it);
          ++curr_it;
        }
        else if (page_less_than(*other_it, *curr_it)) //other page compares less
        {
          merged_pages.push_back(*other_it);
          ++other_it;
        }
        else //pages equal
        {
          merged_pages.push_back(*curr_it);
          ++curr_it;
          ++other_it;
        }
      }
    }

    mVirtualPages = merged_pages;
    delete pPhysPage;
  }

  bool page_less_than(const Page* lhs, const Page* rhs)
  {
    return (lhs->Upper() < rhs->Lower());
  }
}
