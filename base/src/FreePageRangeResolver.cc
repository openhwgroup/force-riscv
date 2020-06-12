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
#include <FreePageRangeResolver.h>
#include <Constraint.h>
#include <Choices.h>
#include <UtilityFunctions.h>
#include <RandomUtils.h>
#include <GenException.h>
#include <Log.h>

#include <memory>

using namespace std;

/*!
  \file FreePageRangeResolver.cc
  \brief Code to resolve free page ranges
*/

namespace Force {
  GranulePageSizeTree::GranulePageSizeTree(EPageGranuleType granuleType, ChoiceTree* pPageSizeTree) : mGranuleType(granuleType), mpPageSizeTree(pPageSizeTree), mpPageSizeConstraint(nullptr), mpMinimalPageSizeChoice(nullptr)
  {
    mpPageSizeConstraint = new ConstraintSet();

    uint64 min_ps = -1ull;
    vector<const Choice*> ps_choices;
    mpPageSizeTree->GetAvailableChoices(ps_choices);
    for (auto& choice : ps_choices) {
      auto ps_name = choice->Name();
      auto page_size = kmg_number(ps_name);
      mpPageSizeConstraint->AddValue(page_size);
      if (page_size < min_ps) {
        min_ps = page_size;
        mpMinimalPageSizeChoice = choice;
      }
    }

  }

  GranulePageSizeTree::~GranulePageSizeTree()
  {
    delete mpPageSizeTree;
    delete mpPageSizeConstraint;
    mpMinimalPageSizeChoice = nullptr;
  }

  bool GranulePageSizeTree::Contains(const ConstraintSet& rPageSizes) const
  {
    return mpPageSizeConstraint->ContainsConstraintSet(rPageSizes);
  }

  ChoiceTree* GranulePageSizeTree::CloneTree()
  {
    auto cloned_tree = dynamic_cast<ChoiceTree* >(mpPageSizeTree->Clone());
    return cloned_tree;
  }

  PageSizeChoice::PageSizeChoice(const std::string& name, uint32 value) : mName(name), mSize(0), mValue(value)
  {
    mSize = kmg_number(mName);
  }

  FreePageRangeResolver::FreePageRangeResolver(ConstraintSet* virtualUsable, GranulePageSizeTree& granulePageTree) : mpVirtualUsable(virtualUsable), mRandomPageSizes(),mRandomPageSizeIndices(), mrGranulePageTree(granulePageTree)
  {
  }

  FreePageRangeResolver::~FreePageRangeResolver()
  {
    delete mpVirtualUsable;

    for (auto& page_size : mRandomPageSizes)
      delete page_size.second;

  }

  bool FreePageRangeResolver::ResolveFreePageRanges(const ConstraintSet& requestRanges, const vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, vector<uint64>& resolvedPageSizes)
  {
    if (not requestRanges.IsEmpty())
      mpVirtualUsable->ApplyConstraintSet(requestRanges);

    if (mpVirtualUsable->IsEmpty())
      return false;

    if (not CheckPageSizeRequest(requestPageSizes))
      return false;

    PreHandlePageRange(requestPageSizes);

    return ResolvePageRangeRandomly(requestPageSizes, startAddr, resolvedRanges, resolvedPageSizes);
  }

  bool FreePageRangeResolver::ResolvePageRangeRandomly(const std::vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, std::vector<uint64>& resolvedPageSizes)
  {
    if (mRandomPageSizeIndices.size() == 0)
      return ResolvePageRange(requestPageSizes, startAddr, resolvedRanges, resolvedPageSizes);

    auto ps_tree = mrGranulePageTree.CloneTree();
    std::unique_ptr<ChoiceTree> ps_storage(ps_tree);
    while (ps_tree->HasChoice()) {
      FillRandomPageSize(ps_tree);
      if (ResolvePageRange(requestPageSizes, startAddr, resolvedRanges, resolvedPageSizes))
        return true;
      else {
        DisableLargePageSize(ps_tree);
      }
    }
    return false;
  }
  bool FreePageRangeResolver::CheckPageSizeRequest(const std::vector<uint64>& requestPageSizes)
 {
   ConstraintSet ps_constr;
   for (auto& page_size : requestPageSizes)
     if (page_size)
       ps_constr.AddValue(page_size);

   return mrGranulePageTree.Contains(ps_constr);
 }
 void FreePageRangeResolver::PreHandlePageRange(const std::vector<uint64>& requestPageSizes)
  {
    for (auto i = 0u; i < requestPageSizes.size(); i ++)
      if (requestPageSizes[i] == 0)
        mRandomPageSizeIndices.push_back(i);

  }

  bool FreePageRangeResolver::ResolvePageRange( const std::vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, std::vector<uint64>& resolvedPageSizes)
  {
    uint64 align, size;
    GetAlignmentSize(requestPageSizes, align, size);

    auto align_mask =  ~(align - 1);
    auto align_shift = get_align_shift(align);
    auto virt_usable = mpVirtualUsable->Clone();
    std::unique_ptr<ConstraintSet> storage_virt_usable(virt_usable);

    virt_usable->AlignWithSize(align_mask, size);
    virt_usable->ShiftRight(align_shift);

    try {
      startAddr =virt_usable->ChooseValue() << align_shift;
    }
    catch (const ConstraintError& constraint_error) {
      LOG(debug) << "{FreePageRangeResolver::ResolvePageRange} Failed to resolve, alignment: 0x" << hex << align
                 << ", size: 0x" << size << ", usable va constraint:" << mpVirtualUsable->ToSimpleString() << endl;
      return false;
    }

    uint64 end_addr = startAddr + size - 1;
    if (end_addr < startAddr) { // overflow
      resolvedRanges.AddRange(startAddr, -1ull);
      resolvedRanges.AddRange(0, end_addr);
    }
    else
      resolvedRanges.AddRange(startAddr, end_addr);

    resolvedPageSizes = requestPageSizes;
    for (auto& page_size : mRandomPageSizes)
      resolvedPageSizes[page_size.first] = page_size.second->mSize;

   return true;
  }

  void FreePageRangeResolver::GetAlignmentSize(const std::vector<uint64>& requestPageSizes, uint64& align, uint64& size)
  {
    align = requestPageSizes[0];
    if (align == 0u)
      align = mRandomPageSizes[0]->mSize;

    size = accumulate(requestPageSizes.cbegin(), requestPageSizes.cend(), uint64(0),
      [](cuint64 partialSum, cuint64 requestPageSize) { return (partialSum + requestPageSize); });

    size = accumulate(mRandomPageSizes.cbegin(), mRandomPageSizes.cend(), size,
      [](cuint64 partialSum, const pair<uint32, PageSizeChoice*>& rRandomPageSize) { return (partialSum + rRandomPageSize.second->mSize); });
  }

  void FreePageRangeResolver::FillRandomPageSize(const ChoiceTree* pPageSizeTree)
  {
    for (auto indice : mRandomPageSizeIndices) {
      auto choice = pPageSizeTree->Choose();
      auto ps_choice = new PageSizeChoice(choice->Name(), choice->Value());
      auto itr = mRandomPageSizes.find(indice);
      if (itr != mRandomPageSizes.end())
        delete itr->second;

      mRandomPageSizes[indice] = ps_choice;

    }
  }

  void FreePageRangeResolver::DisableLargePageSize(ChoiceTree* pPageSizeTree)
  {
    auto size = 0u;
    auto value = 0u;
    for (auto& page_size : mRandomPageSizes)
      if (page_size.second->mSize > size) {
        size = page_size.second->mSize;
        value = page_size.second->mValue;
      }

    if (size != 0u) {
      auto choice_ptr = pPageSizeTree->FindChoiceByValue(value);
      choice_ptr->SetWeight(0);
    }

  }

  FreePageCrossRangeResolver::FreePageCrossRangeResolver(ConstraintSet* virtualUsable, GranulePageSizeTree& granulePageTree, GranulePageSizeTree& extraGranulePageTree) : FreePageRangeResolver(virtualUsable, granulePageTree), mrExtraGranulePageTree(extraGranulePageTree), mPageIndice(0)
  {

  }

  bool FreePageCrossRangeResolver::ResolvePageRange(const std::vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, std::vector<uint64>& resolvedPageSizes)
  {
    uint32 high_segment_size = 0, low_segment_size = 0;
    ConstraintSet high_sizes_constr, low_sizes_constr;

    GetPageHighRange(requestPageSizes, high_segment_size, high_sizes_constr);
    if (high_segment_size == 0 or not mrExtraGranulePageTree.Contains(high_sizes_constr))
      return false;
    GetPageLowRange(requestPageSizes, low_segment_size, low_sizes_constr);
    if (low_segment_size == 0 or not mrGranulePageTree.Contains(low_sizes_constr))
       return false;

    startAddr = -1ull - high_segment_size + 1;
    resolvedRanges.AddRange(startAddr, -1ull);
    resolvedRanges.AddRange(0, low_segment_size - 1);
    // << "{FreePageCrossRangeResolver::GetPageCrossRange} To resolve the range:" << resolvedRanges.ToSimpleString() << endl;

    if (not mpVirtualUsable->ContainsConstraintSet(resolvedRanges)) {
      resolvedRanges.Clear();
      return false;
    }
    resolvedPageSizes = requestPageSizes;
    for (auto& page_size : mRandomPageSizes)
      resolvedPageSizes[page_size.first] = page_size.second->mSize;

    return true;
  }

  void FreePageCrossRangeResolver::GetPageHighRange(const std::vector<uint64>& requestPageSizes, uint32& rangeSize, ConstraintSet& rangeConstraint)
  {
     for (auto indice = 0u; indice <= mPageIndice; indice ++) {
      auto page_size =  requestPageSizes[indice];
      if (page_size) {
        rangeSize += page_size;
        rangeConstraint.AddValue(page_size);
      }
      else {
        if (mRandomPageSizes.find(indice) == mRandomPageSizes.end()) {
          LOG(fail) << "{FreePageCrossRangeResolver::GetPageHighRange} Not randomize the page slice:%d" << indice << endl;
          FAIL("Not-randomize-page-slice");
        }
        rangeSize += mRandomPageSizes[indice]->mSize;
        rangeConstraint.AddValue(mRandomPageSizes[indice]->mSize);
      }
    }
     //<< "{FreePageCrossRangeResolver::GetPageHighRange} high range page size constraint:"
     //<< rangeConstraint.ToSimpleString() << endl;
  }

  void FreePageCrossRangeResolver::GetPageLowRange(const std::vector<uint64>& requestPageSizes, uint32& rangeSize, ConstraintSet& rangeConstraint)
  {
    for (auto indice = mPageIndice + 1; indice < requestPageSizes.size(); indice ++) {
      auto page_size = requestPageSizes[indice];
      if (page_size) {
        rangeSize +=  page_size;
        rangeConstraint.AddValue(page_size);
      }
      else {
        if (mRandomPageSizes.find(indice) == mRandomPageSizes.end()) {
          LOG(fail) << "{FreePageCrossRangeResolver::GetPageCrossRange} Not randomize the page slice:%d" << indice << endl;
          FAIL("Not-randomize-page-slice");
        }
        rangeSize += mRandomPageSizes[indice]->mSize;
        rangeConstraint.AddValue(mRandomPageSizes[indice]->mSize);
      }
    }
    // << "{FreePageCrossRangeResolver::GetPageCrossRange} low range page size constraint:"
    // << RangeConstraint.ToSimpleString() << endl;
  }

  bool  FreePageCrossRangeResolver::ResolvePageRangeRandomly(const vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, vector<uint64>& resolvedPageSizes)
  {
    uint64 item;
    ItemChooser<uint64> item_chooser(requestPageSizes);

    while (item_chooser.HasChoice()) {
      mPageIndice = item_chooser.Choose(item);
      FillMinimalPageSize();
      if (ResolvePageRange(requestPageSizes, startAddr, resolvedRanges, resolvedPageSizes))
        return true;

      item_chooser.Erase(mPageIndice);
    }

    return false;
  }

  void FreePageCrossRangeResolver::FillMinimalPageSize()
  {
    for (auto indice : mRandomPageSizeIndices) {
      const Choice* choice = nullptr;
      if (indice <= mPageIndice) { // fill in by high range page
        choice = mrExtraGranulePageTree.MinimalPageSizeChoice();
        // << "{FreePageCrossRangeResolver::FillMinimalPageSize} minial page size choice name:" << choice->Name() << ", value: " << hex << choice->Value() << endl;
      }
      else { // fill by low range page
        choice = mrGranulePageTree.MinimalPageSizeChoice();
      }
      auto ps_choice = new PageSizeChoice(choice->Name(), choice->Value());
      auto itr = mRandomPageSizes.find(indice);
      if (itr != mRandomPageSizes.end())
        delete itr->second;

      mRandomPageSizes[indice] = ps_choice;
    }
  }

  bool FreePageCrossRangeResolver::CheckPageSizeRequest(const std::vector<uint64>& requestPageSizes)
  {
    ConstraintSet ps_constr;
    for (auto& page_size : requestPageSizes)
      if (page_size)
        ps_constr.AddValue(page_size);
    if (ps_constr.IsEmpty())
      return true;

    auto granule_constr = mrGranulePageTree.PageSizeConstraint();
    auto extra_granule_constr = mrExtraGranulePageTree.PageSizeConstraint();
    ps_constr.SubConstraintSet(*granule_constr);
    ps_constr.SubConstraintSet(*extra_granule_constr);

    return ps_constr.IsEmpty();
  }
}
