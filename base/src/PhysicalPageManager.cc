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
#include <PhysicalPageManager.h>
#include <PhysicalPage.h>
#include <GenRequest.h>
#include <Constraint.h>
#include <Page.h>
#include <VmUtils.h>
#include <VmAddressSpace.h>
#include <PagingChoicesAdapter.h>
#include <MemoryConstraintUpdate.h>
#include <UtilityFunctions.h>
#include <VmMappingStrategy.h>
#include <Log.h>

#include <algorithm>
#include <memory>

using namespace std;

namespace Force
{

  //msPageId starts at 1 to allow for 0 to serve as an invalid ID
  uint64 PhysicalPageManager::msPageId = 1;

  PhysicalPageManager::PhysicalPageManager(EMemBankType bankType)
    : mMemoryBankType(bankType), mpBoundary(nullptr), mpFreeRanges(nullptr), mpAllocatedRanges(nullptr), mpAliasExcludeRanges(nullptr), mUsablePageAligned(), mPhysicalPages(), mAttributeConstraints(EMemoryAttributeTypeSize, nullptr)
  {
  }

  PhysicalPageManager::~PhysicalPageManager()
  {
    //mUsablePageAligned's constraint sets are allocated in initialize, ensure full map is deleted
    for (auto& item : mUsablePageAligned)
    {
      delete item.second;
    }

    for (auto& page : mPhysicalPages)
    {
      delete page;
      page = nullptr;
    }

    for (auto& cset : mAttributeConstraints)
    {
      delete cset;
      cset = nullptr;
    }

    mUsablePageAligned.clear();
    mPhysicalPages.clear();
    mAttributeConstraints.clear();

    delete mpBoundary;
    delete mpFreeRanges;
    delete mpAllocatedRanges;
    delete mpAliasExcludeRanges;
  }

  void PhysicalPageManager::Initialize(const ConstraintSet* pUsableMem, const ConstraintSet* pBoundary)
  {
    if (pUsableMem == nullptr)
    {
      LOG(fail) << "{PhysicalPageManager::Initialize} nullptr passed as usable physical memory" << endl;
      FAIL("nullptr_usable_memory");
    }

    mpBoundary           = pBoundary->Clone();  //boundary will be used in checking if mapped physical address is okay.
    mpFreeRanges         = pUsableMem->Clone(); //free ranges will be updated as memory is mapped by vmas
    mpAllocatedRanges    = new ConstraintSet(); //allocated ranges starts empty, will reflect the allocated physical page ranges
    mpAliasExcludeRanges = new ConstraintSet(); //alias exlcudes start empty, but contain non aliasable locations

    generate(mAttributeConstraints.begin(), mAttributeConstraints.end(),
      []() { return new ConstraintSet(); });

    if (mpFreeRanges->IsEmpty())
    {
      LOG(fail) << "{PhysicalPageManager::Initialize} attempting to initialize with empty usable memory" << endl;
      FAIL("empty_usable_memory");
    }

    for (auto& type : GetPteTypes())
    {
      mUsablePageAligned.insert(std::make_pair(type, GetUsablePageAligned(type)));
    }

    LOG(info) << "{PhysicalPageManager::Initialize} init complete, boundary= " << mpBoundary->ToSimpleString() << ", usable=" << mpFreeRanges->ToSimpleString() << endl;
  }

  void PhysicalPageManager::SubFromBoundary(const ConstraintSet& rConstr)
  {
    mpBoundary->SubConstraintSet(rConstr);
  }

  void PhysicalPageManager::AddToBoundary(const ConstraintSet& rConstr)
  {
    mpBoundary->MergeConstraintSet(rConstr);
  }

  bool PhysicalPageManager::NewAllocation(uint64 VA, PageSizeInfo& rSizeInfo, GenPageRequest* pPageReq)
  {
    bool   ret_val       = false;

    // Select mapping strategy
    VmMappingStrategy* mapping_strategy = nullptr;
    if (pPageReq->GenBoolAttributeDefaultFalse(EPageGenBoolAttrType::FlatMap)) mapping_strategy = new VmFlatMappingStrategy();
    else mapping_strategy = new VmRandomMappingStrategy();
    std::unique_ptr<VmMappingStrategy> mapping_strategy_storage(mapping_strategy);

    if (mapping_strategy->AllocatePhysicalPage(VA, mUsablePageAligned[rSizeInfo.mType], mpBoundary, pPageReq, rSizeInfo)) {
      bool can_alias = pPageReq->GenBoolAttributeDefaultTrue(EPageGenBoolAttrType::CanAlias);
      PhysicalPage* phys_page = new PhysicalPage(rSizeInfo.PhysicalStart(), rSizeInfo.PhysicalEnd(), can_alias, msPageId++);
      rSizeInfo.UpdatePhysPageId(phys_page->PageId());
      UpdateMemoryAttributes(pPageReq, phys_page);
      AddPhysicalPage(phys_page);
      ret_val = true;
    }

    return ret_val;
  }

  bool PhysicalPageManager::AliasAllocation(uint64 VA, PageSizeInfo& rSizeInfo, GenPageRequest* pPageReq)
  {
    //step 1 - determine physical address target of aliasing
    //         flatMap case: VA,
    //         random alloc - no opts: do constraint solve, ignore attribute constraints if ForceMemAttrs is set
    //         random alloc - paTarget/PhysPageId: target specific addr
    //         random alloc - targetaliasattrs - constraint solving w/ specific attrs from request
    //step 2 - determine page overlapping for aliasing
    //       - num_overlap = 0 - error case, phys page targeting should at least intersect w/ 1 page
    //       - num_overlap = 1 - simple case, alloc page only hits alias page and can do a simple merge
    //       - num_overlap > 1 - hard case, alloc page hits more than 1 alias page. merge all pages into alloc page
    //step 3 - mem attribute compatibility checks - mem attrs currently checked in random alloc constraint solving
    //       - need to be checked in all other cases, will likely do redundant checking for now.. can maybe optimize
    //       - if ForceMemAttrs is specified, ignore all mem attr compatibility checks
    //step 4 - page merging - if target is found and overlapping pages have compatible attributes
    //       - merge into existing - if new alloc page is smaller or = alias page. Update canAlias flag if needed.
    //       - merge existing into new alloc page - if new alloc page is larger or overlaps multiple pages merge into new page and remove old pages

    bool is_flat_map = false;
    pPageReq->GetGenBoolAttribute(EPageGenBoolAttrType::FlatMap, is_flat_map);

    bool has_can_alias = false;
    bool can_alias = false;
    has_can_alias = pPageReq->GetGenBoolAttribute(EPageGenBoolAttrType::CanAlias, can_alias);
    if (!has_can_alias) can_alias = true;

    bool force_mem_attrs = false;
    pPageReq->GetGenBoolAttribute(EPageGenBoolAttrType::ForceMemAttrs, force_mem_attrs);

    bool has_phys_id = false;
    uint64 phys_id = 0x0ull;
    has_phys_id = pPageReq->GetAttributeValue(EPageRequestAttributeType::AliasPageId, phys_id);

    bool has_pa_target = false;
    uint64 pa_target = 0x0ull;
    has_pa_target = pPageReq->GetAttributeValue(EPageRequestAttributeType::PA, pa_target);

    bool ret_val = false;

    //--------DETERMINE PHYSICAL TARGET--------
    uint64 phys_target = 0x0ull;

    if (is_flat_map)
    {
      phys_target = VA;
    }
    else //random alloc
    {
      if (has_pa_target)
      {
        phys_target = pa_target;
      }
      else if (has_phys_id)
      {
        PhysicalPage* found_page = FindPhysicalPage(phys_id);
        if (found_page == nullptr) return ret_val;
        phys_target = found_page->Lower();
      }
      else
      {
        bool alias_found = SolveAliasConstraints(rSizeInfo, pPageReq, phys_target);
        if (!alias_found) return ret_val;
      }
    }

    rSizeInfo.UpdatePhysicalStart(phys_target);

    //--------DETERMINE PAGE OVERLAP--------
    PhysicalPage* alloc_page = new PhysicalPage(rSizeInfo.PhysicalStart(), rSizeInfo.PhysicalEnd(), can_alias, msPageId++);
    auto          range_pair = std::equal_range(mPhysicalPages.begin(), mPhysicalPages.end(), alloc_page, &phys_page_less_than);
    uint32       num_overlap = std::distance(range_pair.first, range_pair.second);

    const ConstraintSet* alloc_mem_attrs = pPageReq->MemAttrImplConstraint();

    if (num_overlap == 0) //picked a target w/ no overlap, error case
    {
      LOG(warn) << "{PhysicalPageManager::AliasAllocation} aliased allocation not possible to phys page target, no overlapping pages. start=0x"
                << hex << rSizeInfo.PhysicalStart() << " end=0x" << rSizeInfo.PhysicalEnd() << endl;
      return ret_val;
    }
    else if (num_overlap == 1)
    {
      //--------MEMORY ATTRIBUTE CHECKS--------
      if (!force_mem_attrs)
      {
        ConstraintSet* page_mem_attrs = (*range_pair.first)->GetMemoryAttributes();
        std::unique_ptr<ConstraintSet> page_mem_attrs_storage(page_mem_attrs);
        if (!page_mem_attrs_storage.get()->IsEmpty())
        {
          bool mem_attr_compatibility = MemAttrCompatibility(alloc_mem_attrs, page_mem_attrs);
          if (!mem_attr_compatibility) return ret_val;
        }
      }
      //--------PAGE MERGING--------
      if (alloc_page->Lower() < (*range_pair.first)->Lower() || alloc_page->Upper() > (*range_pair.first)->Upper())
      {
        if (!is_flat_map)
        {
          if (!(*range_pair.first)->CanAlias())
          {
            LOG(trace) << "{PhysicalPageManager::AliasAllocation} targeted alias page is marked as not aliasable." << endl;
            return ret_val;
          }
        }

        alloc_page->Merge(*(range_pair.first));
        mPhysicalPages.erase(range_pair.first);
        rSizeInfo.UpdatePhysPageId(alloc_page->PageId());
        UpdateMemoryAttributes(pPageReq, alloc_page);
        AddPhysicalPage(alloc_page);

        LOG(trace) << "{PhysicalPageManager::AliasAllocation} single overlap new page merged new_page_lower=0x" << hex
                   << alloc_page->Lower() << " new_page_upper=0x" << alloc_page->Upper() << " old_page_lower=0x"
                   << (*range_pair.first)->Lower() << " old_page_upper=0x" << (*range_pair.first)->Upper() << endl;
      }
      else
      {
        if (!is_flat_map)
        {
          if (!(*range_pair.first)->CanAlias())
          {
            LOG(trace) << "{PhysicalPageManager::AliasAllocation} targeted alias page is marked as not aliasable." << endl;
            return ret_val;
          }
          else if ((*range_pair.first)->CanAlias() && !can_alias)
          {
            (*range_pair.first)->SetCanAlias(can_alias);
            mpAliasExcludeRanges->AddRange((*range_pair.first)->Lower(), (*range_pair.first)->Upper());
            LOG(trace) << "{PhysicalPageManager::AliasAllocation} updating existing page can alias flag to false mpAliasExcludeRanges="
              << hex << mpAliasExcludeRanges->ToSimpleString() << endl;
          }
        }
        rSizeInfo.UpdatePhysPageId((*range_pair.first)->PageId());
        delete alloc_page;
        LOG(trace) << "{PhysicalPageManager::AliasAllocation} single overlap new page not merged page_lower=0x" << hex
                   << alloc_page->Lower() << " page_upper=0x" << alloc_page->Upper() << " old_page_lower=0x"
                   << (*range_pair.first)->Lower() << " old_page_upper=0x" << (*range_pair.first)->Upper() << endl;
      }
      ret_val = true;
    }
    else
    {
      //--------MEMORY ATTRIBUTE CHECKS--------
      if (!force_mem_attrs)
      {
        for (auto it = range_pair.first; it != range_pair.second; ++it)
        {
          ConstraintSet* page_mem_attrs = (*range_pair.first)->GetMemoryAttributes();
          std::unique_ptr<ConstraintSet> page_mem_attrs_storage(page_mem_attrs);
          if (!page_mem_attrs_storage.get()->IsEmpty())
          {
            bool mem_attr_compatibility = MemAttrCompatibility(alloc_mem_attrs, page_mem_attrs);
            if (!mem_attr_compatibility) return ret_val;
          }

          if (!is_flat_map)
          {
            if (!(*it)->CanAlias())
            {
              LOG(trace) << "{PhysicalPageManager::AliasAllocation} targeted alias page is marked as not aliasable." << endl;
              return ret_val;
            }
          }
        }
      }
      //--------PAGE MERGING--------

      for (auto it = range_pair.first; it != range_pair.second; ++it)
      {
        alloc_page->Merge(*it);
      }
      mPhysicalPages.erase(range_pair.first, range_pair.second);
      rSizeInfo.UpdatePhysPageId(alloc_page->PageId());
      UpdateMemoryAttributes(pPageReq, alloc_page);
      AddPhysicalPage(alloc_page);
      ret_val = true;
    }

    return ret_val;
  }

  //NOTE: handling aliasing with regimes/threads with MMU=off
  bool PhysicalPageManager::SolveAliasConstraints(const PageSizeInfo& rSizeInfo, GenPageRequest* pPageReq, uint64& physTarget)
  {
    uint64 max_physical = rSizeInfo.MaxPhysical() + 1ull;

    bool force_mem_attrs = false;
    pPageReq->GetGenBoolAttribute(EPageGenBoolAttrType::ForceMemAttrs, force_mem_attrs);

    // ---- ACQUIRE ATTRIBUTE TYPES ----
    bool use_target_alias_attrs = false;
    const ConstraintSet* target_alias_attrs = pPageReq->TargetAliasAttrsConstraint();
    const ConstraintSet* page_attrs = GetPageAttrConstraints(pPageReq);

    if (target_alias_attrs != nullptr)
    {
      if (!target_alias_attrs->IsEmpty())
      {
        use_target_alias_attrs = true;
      }
    }

    std::vector<EMemoryAttributeType> mem_constr_types;
    std::vector<EMemoryAttributeType> incompatible_mem_constr_types;

    if (use_target_alias_attrs)
    {
      ConvertMemoryAttributes(target_alias_attrs, mem_constr_types);
      GetIncompatibleAttributes(target_alias_attrs, incompatible_mem_constr_types);
    }
    else if (page_attrs != nullptr)
    {
      ConvertMemoryAttributes(page_attrs, mem_constr_types);
      GetIncompatibleAttributes(page_attrs, incompatible_mem_constr_types);
    }

    // ---- INITIAL CONSTRAINT SETUP ----
    ConstraintSet* page_alias_constr = mpAllocatedRanges->Clone();
    std::unique_ptr<ConstraintSet> page_alias_constr_storage(page_alias_constr);
    page_alias_constr->SubConstraintSet(*mpAliasExcludeRanges);
    if (!page_alias_constr->IsEmpty()) // call UpperBound with care, ensure the set is not empty.
    {
      page_alias_constr->SubRange(max_physical, page_alias_constr->UpperBound());
    }

    for (auto attr_type : mem_constr_types) //form constraint set of combined set of attributes;
    {
      ConstraintSet* attr_constr = mAttributeConstraints.at(uint32(attr_type));
      page_alias_constr->ApplyConstraintSet(*attr_constr);
    }

    // ---- NORMALIZE ATTRIBUTE CONSTRAINT WITH PAGE SIZE ----
    uint64 page_size = get_page_shift(rSizeInfo.mType);
    uint64 page_mask = get_mask64(page_size);
    page_alias_constr->AlignWithPage(~page_mask);

    // ---- EXPANDED RANGE CONFLICT CHECKING ----
    ConstraintSet* conflict_constr = mpAliasExcludeRanges->Clone();
    std::unique_ptr<ConstraintSet> conflict_constr_storage(conflict_constr);

    for (auto attr_type : incompatible_mem_constr_types)
    {
      ConstraintSet* attr_constr = mAttributeConstraints.at(uint32(attr_type));
      conflict_constr->MergeConstraintSet(*attr_constr);
    }
    conflict_constr->AlignWithPage(~page_mask);
    page_alias_constr->SubConstraintSet(*conflict_constr);

    // ---- PHYSICAL TARGET SELECTION ----
    if (page_alias_constr->IsEmpty())
    {
      return false;
    }

    uint64 page_num = page_alias_constr->ChooseValue();
    physTarget = (page_num << rSizeInfo.mPageShift);

    return true;
  }

  bool PhysicalPageManager::MemAttrCompatibility(const ConstraintSet* pAllocAttrs, const ConstraintSet* pAliasAttrs)
  {
    if (pAllocAttrs == nullptr)
    {
      LOG(trace) << "{PhysicalPageManager::MemAttrCompatibility} alloc page has no attributes, should match any page. can alias" << endl;
      return true;
    }

    if (pAliasAttrs == nullptr)
    {
      LOG(trace) << "{PhysicalPageManager::MemAttrCompatibility} alias page has no attributes, can alias." << endl;
      return true;
    }

    std::vector<EMemoryAttributeType> alloc_constr_types;
    std::vector<EMemoryAttributeType> alias_constr_types;
    ConvertMemoryAttributes(pAllocAttrs, alloc_constr_types);
    ConvertMemoryAttributes(pAliasAttrs, alias_constr_types);

    //two types of comparisons:
    //  1) compare attributes in each set against each other, make sure they are valid. I.E. size of each vector should be 1 w/ current implementation
    //    if alloc_constr_types are invalid, should fail as this would be invalid use case of the attrs in the page req.
    //    if alias_constr_types are invalid, could have been forced earlier in the case so just return false indicating we can't alias to that location.
    //
    //  2) compare attributes between the two vectors to make sure they match. currently can do a 1 to 1 check if failing on size > 1

    if ((alloc_constr_types.size() == 0) || (alias_constr_types.size() == 0))
    {
      LOG(trace) << "{PhysicalPageManager::MemAttrCompatibility} attr constraints either empty, or no valid constraints contained. should allow aliasing" << endl;
      return true;
    }

    if (alloc_constr_types.size() > 1)
    {
      LOG(fail) << "{PhysicalPageManager::MemAttrCompatibility} specified invalid attributes in page request w/o forcing mem attrs." << endl;
      FAIL("phys_page_req_has_invalid_mem_attrs");
    }

    if (alias_constr_types.size() > 1)
    {
      LOG(trace) << "{PhysicalPageManager::MemAttrCompatibility} page being aliased to was forced to invalid state, not aliasing without forcing mem attrs" << endl;
      return false;
    }

    if (alloc_constr_types[0] == alias_constr_types[0])
    {
      LOG(trace) << "{PhysicalPageManager::MemAttrCompatibility} pages memory attributes match. allow aliasing" << endl;
      return true;
    }

    LOG(trace) << "{PhysicalPageManager::MemAttrCompatibility} fallthrough case hit, defaulting to not allow aliasing" << endl;
    return false;
  }

  bool PhysicalPageManager::AllocatePage(uint64 VA, uint64 size, GenPageRequest* pPageReq, PageSizeInfo& rSizeInfo, const PagingChoicesAdapter* pChoicesAdapter)
  {
    //control flow
    // -check for instr/data choices to check for alias priority.
    // -check the force_alias flag to determine if we retry normal allocation if alias fails.
    // -if alias choices conditions met
    //   -call AliasAllocation, if true/return true, if false and force_alias/return false, if false and !force_alias try NormalAllocation
    // -if !force_alias and !alias choice conditons
    //   -call NormalAllocation, if returns false attempt AliasAllocation and return its return value.
    // -if pa_target specified, just try AliasAllocation and return its return value.

    // -if force_alias true
    //   -call AliasAllocation and return its return value.
    if (pPageReq->GenBoolAttributeDefaultFalse(EPageGenBoolAttrType::ForceAlias))
      return AliasAllocation(VA, rSizeInfo, pPageReq);

    bool alloc_result = false;

    bool is_instr = pPageReq->GenBoolAttributeDefaultFalse(EPageGenBoolAttrType::InstrAddr);
    bool alias_first = false;
    if (is_instr)
      alias_first = (pChoicesAdapter->GetPlainPagingChoice("Instruction Page Aliasing") == 1);
    else
      alias_first = (pChoicesAdapter->GetPlainPagingChoice("Data Page Aliasing") == 1);

    if (alias_first)
    {
      alloc_result = AliasAllocation(VA, rSizeInfo, pPageReq);
      if (not alloc_result) alloc_result = NewAllocation(VA, rSizeInfo, pPageReq);
      return alloc_result;
    }

    alloc_result = NewAllocation(VA, rSizeInfo, pPageReq);
    if (not alloc_result) alloc_result = AliasAllocation(VA, rSizeInfo, pPageReq);
    return alloc_result;
  }

  void PhysicalPageManager::CommitPage(const Page* pPage, uint64 size)
  {
    PhysicalPage* phys_page = FindPhysicalPage(pPage->PhysicalLower(), pPage->PhysicalUpper());
    if (phys_page == nullptr)
    {
      LOG(fail) << "{PhysicalPageManager::CommitPage} unable to find physical page to propogate virtual page link to." << endl;
      FAIL("unable_to_find_phys_page_for_commit");
    }
    phys_page->AddPage(pPage);

    std::unique_ptr<ConstraintSet> page_mem_attrs_storage(phys_page->GetMemoryAttributes());

    if (!page_mem_attrs_storage.get()->IsEmpty())
    {
      std::vector<EMemoryAttributeType> mem_constr_types;
      ConvertMemoryAttributes(page_mem_attrs_storage.get(), mem_constr_types);
      for (const auto attr_type : mem_constr_types)
      {
        mAttributeConstraints.at(uint32(attr_type))->AddRange(phys_page->Lower(), phys_page->Upper());
      }
    }
  }

  void PhysicalPageManager::HandleMemoryConstraintUpdate(const MemoryConstraintUpdate& rMemConstrUpdate) const
  {
    PhysicalPage lookup_page(rMemConstrUpdate.GetPhysicalStartAddress(), rMemConstrUpdate.GetPhysicalEndAddress());
    auto find_iter = std::equal_range(mPhysicalPages.begin(), mPhysicalPages.end(), &lookup_page, &phys_page_less_than);

    if (find_iter.first != mPhysicalPages.end())
    {
      for (auto it = find_iter.first; it != find_iter.second; ++it)
      {
        (*find_iter.first)->HandleMemoryConstraintUpdate(rMemConstrUpdate);
      }
    }
  }

  const Page* PhysicalPageManager::GetVirtualPage(uint64 PA, const VmAddressSpace* pVmas) const
  {
    PhysicalPage* phys_page = FindPhysicalPage(PA, PA);
    if (phys_page == nullptr)
    {
      LOG(warn) << "{PhysicalPageManager::GetVirtualPage} unable to find physical page, can't return virtual page" << endl;
      return nullptr;
    }
    return phys_page->GetVirtualPage(PA, pVmas);
  }

  ConstraintSet* PhysicalPageManager::GetUsablePageAligned(EPteType pteType)
  {
    ConstraintSet* aligned_set = mpFreeRanges->Clone();
    uint64 page_size           = get_page_shift(pteType);
    uint64 page_mask           = get_mask64(page_size);
    aligned_set->AlignWithPage(~page_mask);
    return aligned_set;
  }

  void PhysicalPageManager::UpdateUsablePageAligned(uint64 start_addr, uint64 end_addr)
  {
    for (auto& type : GetPteTypes())
    {
      uint64 page_size     = get_page_shift(type);
      uint64 page_mask     = get_mask64(page_size);
      uint64 aligned_start = (start_addr & ~page_mask) >> page_size;
      uint64 aligned_end   = ((end_addr & ~page_mask) + page_mask) >> page_size;

      mUsablePageAligned[type]->SubRange(aligned_start, aligned_end);
    }
  }

  PhysicalPage* PhysicalPageManager::FindPhysicalPage(uint64 lower, uint64 upper) const
  {
    PhysicalPage lookup_page(lower, upper);
    auto find_iter = std::equal_range(mPhysicalPages.begin(), mPhysicalPages.end(), &lookup_page, &phys_page_less_than);

    if (std::distance(find_iter.first, find_iter.second) > 1)
    {
      LOG(fail) << "{PhysicalPageManager::FindPhysicalPage} found multiple allocated physical pages for range lower=0x"
                << hex << lower << " to upper=0x" << upper << endl;
      FAIL("find_physical_page_returned_multiple_pages");
      return nullptr;
    }
    else if (std::distance(find_iter.first, find_iter.second) == 0 || find_iter.first == mPhysicalPages.end())
    {
      LOG(warn) << "{PhysicalPageManager::FindPhysicalPage} unable to find allocated physical page for range lower=0x"
                << hex << lower << " to upper=0x" << upper << endl;
      return nullptr;
    }

    return (*find_iter.first);
  }

  PhysicalPage* PhysicalPageManager::FindPhysicalPage(uint64 physId) const
  {
    PhysicalPage* phys_page = nullptr;

    auto itr = find_if(mPhysicalPages.cbegin(), mPhysicalPages.cend(),
      [physId](const PhysicalPage* pPhysPage) { return (pPhysPage->PageId() == physId); });

    if (itr != mPhysicalPages.end()) {
      phys_page = *itr;
     }

    return phys_page;
  }

  void PhysicalPageManager::AddPhysicalPage(PhysicalPage* physPage)
  {
    auto insert_it = std::lower_bound(mPhysicalPages.begin(), mPhysicalPages.end(), physPage, &phys_page_less_than);
    mPhysicalPages.insert(insert_it, physPage);

    mpFreeRanges->SubRange(physPage->Lower(), physPage->Upper());
    mpAllocatedRanges->AddRange(physPage->Lower(), physPage->Upper());
    if (!physPage->CanAlias()) mpAliasExcludeRanges->AddRange(physPage->Lower(), physPage->Upper());
    UpdateUsablePageAligned(physPage->Lower(), physPage->Upper());
  }

  void PhysicalPageManager::UpdateMemoryAttributes(GenPageRequest* pPageReq, PhysicalPage* pPhysPage)
  {
    const ConstraintSet* mem_attrs = GetPageAttrConstraints(pPageReq);
    std::vector<EMemoryAttributeType> mem_constr_types;
    if (mem_attrs != nullptr)
    {
      ConvertMemoryAttributes(mem_attrs, mem_constr_types); 
      for (auto attr_type : mem_constr_types) //update each applicable constraint set with the page range of the applicable page
      {
        mAttributeConstraints.at(uint32(attr_type))->AddRange(pPhysPage->Lower(), pPhysPage->Upper());
      }
    }
  }

  const ConstraintSet* PhysicalPageManager::GetPageAttrConstraints(GenPageRequest* pPageReq) const
  {
    const ConstraintSet* mem_attr_arch = pPageReq->MemAttrArchConstraint();
    const ConstraintSet* mem_attr_impl = pPageReq->MemAttrImplConstraint();
    const ConstraintSet* mem_attrs = nullptr;

    if (mem_attr_impl != nullptr)
    {
      mem_attrs = mem_attr_impl;
    }
    else if (mem_attr_arch != nullptr)
    {
      mem_attrs = mem_attr_arch;
    }

    return mem_attrs;
  }

  bool phys_page_less_than(const PhysicalPage* lhs, const PhysicalPage* rhs)
  {
    return (lhs->Upper() < rhs->Lower());
  }
}
