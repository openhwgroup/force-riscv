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
#include "MemoryTraits.h"

#include <algorithm>
#include <memory>

#include "Constraint.h"
#include "Log.h"

using namespace std;

/*!
  \file MemoryTraits.cc
  \brief Code supporting tracking memory characteristics associated with physical addresses.
*/

namespace Force {

  MemoryTraitsRange::MemoryTraitsRange(const map<uint32, ConstraintSet*>& rTraitRanges, cuint64 startAddr, cuint64 endAddr)
    : mTraitRanges(), mpEmptyRanges(new ConstraintSet(startAddr, endAddr)), mStartAddr(startAddr), mEndAddr(endAddr)
  {
    for (const auto& trait_range : rTraitRanges) {
      auto trait_addresses = new ConstraintSet();
      trait_range.second->CopyInRange(mStartAddr, mEndAddr, *trait_addresses);

      if (not trait_addresses->IsEmpty()) {
        mTraitRanges.emplace(trait_range.first, trait_addresses);
        mpEmptyRanges->SubConstraintSet(*trait_addresses);
      }
      else {
        delete trait_addresses;
      }
    }
  }

  MemoryTraitsRange::MemoryTraitsRange(const vector<uint32>& rTraitIds, cuint64 startAddr, cuint64 endAddr)
    : mTraitRanges(), mpEmptyRanges(new ConstraintSet()), mStartAddr(startAddr), mEndAddr(endAddr)
  {
    if (not rTraitIds.empty()) {
      for (uint32 trait_id : rTraitIds) {
        mTraitRanges.emplace(trait_id, new ConstraintSet(startAddr, endAddr));
      }
    }
    else {
      mpEmptyRanges->AddRange(startAddr, endAddr);
    }
  }

  MemoryTraitsRange::MemoryTraitsRange(const MemoryTraitsRange& rOther)
    : mTraitRanges(), mpEmptyRanges(rOther.mpEmptyRanges->Clone()), mStartAddr(rOther.mStartAddr), mEndAddr(rOther.mEndAddr)
  {
    for (const auto& trait_range : rOther.mTraitRanges) {
      mTraitRanges.emplace(trait_range.first, trait_range.second->Clone());
    }
  }

  MemoryTraitsRange::~MemoryTraitsRange()
  {
    for (auto trait_range : mTraitRanges) {
      delete trait_range.second;
    }

    delete mpEmptyRanges;
  }

  MemoryTraitsRange* MemoryTraitsRange::CreateMergedMemoryTraitsRange(const MemoryTraitsRange& rOther) const
  {
    if ((mStartAddr != rOther.mStartAddr) or (mEndAddr != rOther.mEndAddr)) {
      LOG(fail) << "{MemoryTraitsRange::CreateMergedMemoryTraitsRange} the starting and ending addresses for both MemoryTraitsRanges must be equal" << endl;
      FAIL("address-ranges-not-equal");
    }

    MemoryTraitsRange* merged_mem_traits_range = new MemoryTraitsRange(rOther);

    for (const auto& trait_range : mTraitRanges) {
      auto itr = merged_mem_traits_range->mTraitRanges.find(trait_range.first);
      if (itr != merged_mem_traits_range->mTraitRanges.end()) {
        itr->second->MergeConstraintSet(*(trait_range.second));
      }
      else {
        merged_mem_traits_range->mTraitRanges.emplace(trait_range.first, trait_range.second->Clone());
      }

      merged_mem_traits_range->mpEmptyRanges->SubConstraintSet(*(trait_range.second));
    }

    return merged_mem_traits_range;
  }

  bool MemoryTraitsRange::IsCompatible(const MemoryTraitsRange& rOther) const
  {
    bool compatible = HasCompatibleTraits(rOther);
    if (compatible) {
      compatible = rOther.HasCompatibleTraits(*this);
    }

    return compatible;
  }

  bool MemoryTraitsRange::IsEmpty() const
  {
    return mTraitRanges.empty();
  }

  bool MemoryTraitsRange::HasCompatibleTraits(const MemoryTraitsRange& rOther) const
  {
    bool compatible = true;

    for (const auto& trait_range : mTraitRanges) {
      // Get the addresses associated with the trait in this MemoryTraitsRange, but remove addresses
      // that are beyond the boundaries of the other MemoryTraitsRange
      ConstraintSet trait_addresses;
      trait_range.second->CopyInRange(rOther.mStartAddr, rOther.mEndAddr, trait_addresses);

      // Get the addresses associated with the trait in the other MemoryTraitsRange, and combine
      // them with the addresses with no associated traits
      unique_ptr<ConstraintSet> other_compatible_addresses(rOther.mpEmptyRanges->Clone());
      auto itr = rOther.mTraitRanges.find(trait_range.first);
      if (itr != rOther.mTraitRanges.end()) {
        other_compatible_addresses->MergeConstraintSet(*(itr->second));
      }

      // If the first set of addresses are not entirely contained in the second set of addresses,
      // then there is a mismatch, and the traits are not compatible
      if (not other_compatible_addresses->ContainsConstraintSet(trait_addresses)) {
        compatible = false;
        break;
      }
    }

    return compatible;
  }

  MemoryTraits::MemoryTraits()
    : mTraitRanges()
  {
  }

  MemoryTraits::~MemoryTraits()
  {
    for (auto trait_range : mTraitRanges) {
      delete trait_range.second;
    }
  }

  void MemoryTraits::AddTrait(cuint32 traitId, cuint64 startAddr, cuint64 endAddr)
  {
    auto itr = mTraitRanges.find(traitId);
    if (itr != mTraitRanges.end()) {
      itr->second->AddRange(startAddr, endAddr);
    }
    else {
      auto constr = new ConstraintSet(startAddr, endAddr);
      mTraitRanges.emplace(traitId, constr);
    }
  }

  bool MemoryTraits::HasTrait(cuint32 traitId, cuint64 startAddr, cuint64 endAddr) const
  {
    bool has_trait = false;

    auto itr = mTraitRanges.find(traitId);
    if (itr != mTraitRanges.end()) {
      has_trait = itr->second->ContainsRange(startAddr, endAddr);
    }

    return has_trait;
  }

  bool MemoryTraits::HasTraitPartial(cuint32 traitId, cuint64 startAddr, cuint64 endAddr) const
  {
    bool has_trait_partial = false;

    auto itr = mTraitRanges.find(traitId);
    if (itr != mTraitRanges.end()) {
      has_trait_partial = itr->second->Intersects(ConstraintSet(startAddr, endAddr));
    }

    return has_trait_partial;
  }

  const ConstraintSet* MemoryTraits::GetTraitAddressRanges(cuint32 traitId) const
  {
    const ConstraintSet* trait_addresses = nullptr;

    auto itr = mTraitRanges.find(traitId);
    if (itr != mTraitRanges.end()) {
      trait_addresses = itr->second;
    }

    return trait_addresses;
  }

  MemoryTraitsRange* MemoryTraits::CreateMemoryTraitsRange(cuint64 startAddr, cuint64 endAddr) const
  {
    return new MemoryTraitsRange(mTraitRanges, startAddr, endAddr);
  }

  MemoryTraitsRegistry::MemoryTraitsRegistry()
    : mTraitIds(), mExclusiveTraitIds(), mThreadTraitIds(), mNextTraitId(1)
  {
  }

  uint32 MemoryTraitsRegistry::AddTrait(const EMemoryAttributeType trait)
  {
    return AddTrait(EMemoryAttributeType_to_string(trait));
  }

  uint32 MemoryTraitsRegistry::AddTrait(const string& rTrait)
  {
    uint32 trait_id = 0;

    auto itr = mTraitIds.find(rTrait);
    if (itr == mTraitIds.end()) {
      trait_id = mNextTraitId++;
      mTraitIds.emplace(rTrait, trait_id);
    }
    else {
      LOG(fail) << "{MemoryTraitsRegistry::AddTrait} trait " << rTrait << " has already been added" << endl;
      FAIL("trait-already-exists");
    }

    return trait_id;
  }

  uint32 MemoryTraitsRegistry::GetTraitId(const EMemoryAttributeType trait) const
  {
    return GetTraitId(EMemoryAttributeType_to_string(trait));
  }

  uint32 MemoryTraitsRegistry::GetTraitId(const string& rTrait) const
  {
    uint32 trait_id = 0;

    auto itr = mTraitIds.find(rTrait);
    if (itr != mTraitIds.end()) {
      trait_id = itr->second;
    }

    return trait_id;
  }

  std::string MemoryTraitsRegistry::GetTraitName(cuint32 traitId) const
  {
    string trait_name;

    auto itr = find_if(mTraitIds.cbegin(), mTraitIds.cend(),
      [traitId](const pair<string, uint32>& traitIdEntry) { return (traitIdEntry.second == traitId); });

    if (itr != mTraitIds.end()) {
      trait_name = itr->first;
    }

    return trait_name;
  }

  uint32 MemoryTraitsRegistry::RequestTraitId(const EMemoryAttributeType trait)
  {
    return RequestTraitId(EMemoryAttributeType_to_string(trait));
  }

  uint32 MemoryTraitsRegistry::RequestTraitId(const string& rTrait)
  {
    uint32 trait_id = GetTraitId(rTrait);
    if (trait_id == 0) {
      trait_id = AddTrait(rTrait);
    }

    return trait_id;
  }

  void MemoryTraitsRegistry::AddMutuallyExclusiveTraits(const vector<EMemoryAttributeType>& rTraits)
  {
    set<uint32> exclusive_ids;
    for (EMemoryAttributeType trait : rTraits) {
      exclusive_ids.insert(RequestTraitId(trait));
    }

    AddMutuallyExclusiveTraitIds(exclusive_ids);
  }

  void MemoryTraitsRegistry::AddThreadTraits(const vector<EMemoryAttributeType>& rTraits)
  {
    for (EMemoryAttributeType trait : rTraits) {
      mThreadTraitIds.insert(RequestTraitId(trait));
    }
  }

  void MemoryTraitsRegistry::GetMutuallyExclusiveTraitIds(cuint32 traitId, vector<uint32>& rExclusiveIds) const
  {
    auto itr = mExclusiveTraitIds.find(traitId);
    if (itr != mExclusiveTraitIds.end()) {
      // Don't include the specified trait ID in the output
      copy_if(itr->second.cbegin(), itr->second.cend(), back_inserter(rExclusiveIds),
        [traitId](cuint32 exclusiveId) { return (exclusiveId != traitId); });
    }
  }

  bool MemoryTraitsRegistry::IsThreadTrait(cuint32 threadId)
  {
    bool is_thread_trait = false;

    auto itr = mThreadTraitIds.find(threadId);
    if (itr != mThreadTraitIds.end()) {
      is_thread_trait = true;
    }

    return is_thread_trait;
  }

  void MemoryTraitsRegistry::AddMutuallyExclusiveTraitIds(const set<uint32>& rExclusiveIds)
  {
    for (uint32 trait_id : rExclusiveIds) {
      auto itr = mExclusiveTraitIds.find(trait_id);

      if (itr != mExclusiveTraitIds.end()) {
        copy(rExclusiveIds.cbegin(), rExclusiveIds.cend(), inserter(itr->second, itr->second.begin()));
      }
      else {
        mExclusiveTraitIds.emplace(trait_id, rExclusiveIds);
      }
    }
  }

  MemoryTraitsManager::MemoryTraitsManager(MemoryTraitsRegistry* pMemTraitsRegistry)
    : mGlobalMemTraits(), mThreadMemTraits(), mpMemTraitsRegistry(pMemTraitsRegistry)
  {
  }

  MemoryTraitsManager::~MemoryTraitsManager()
  {
    for (auto mem_traits : mThreadMemTraits) {
      delete mem_traits.second;
    }

    delete mpMemTraitsRegistry;
  }

  void MemoryTraitsManager::AddTrait(cuint32 threadId, const EMemoryAttributeType trait, cuint64 startAddr, cuint64 endAddr)
  {
    AddTrait(threadId, mpMemTraitsRegistry->RequestTraitId(trait), startAddr, endAddr);
  }

  void MemoryTraitsManager::AddTrait(cuint32 threadId, const string& rTrait, cuint64 startAddr, cuint64 endAddr)
  {
    AddTrait(threadId, mpMemTraitsRegistry->RequestTraitId(rTrait), startAddr, endAddr);
  }

  void MemoryTraitsManager::AddTrait(cuint32 threadId, cuint32 traitId, cuint64 startAddr, cuint64 endAddr)
  {
    if (mpMemTraitsRegistry->IsThreadTrait(traitId)) {
      AddThreadTrait(threadId, traitId, startAddr, endAddr);
    }
    else {
      AddGlobalTrait(traitId, startAddr, endAddr);
    }
  }

  bool MemoryTraitsManager::HasTrait(cuint32 threadId, const EMemoryAttributeType trait, cuint64 startAddr, cuint64 endAddr) const
  {
    return HasTrait(threadId, EMemoryAttributeType_to_string(trait), startAddr, endAddr);
  }

  bool MemoryTraitsManager::HasTrait(cuint32 threadId, const string& rTrait, cuint64 startAddr, cuint64 endAddr) const
  {
    uint32 trait_id = mpMemTraitsRegistry->GetTraitId(rTrait);
    if (trait_id == 0) {
      return false;
    }

    bool has_trait = mGlobalMemTraits.HasTrait(trait_id, startAddr, endAddr);
    if (not has_trait) {
      auto itr = mThreadMemTraits.find(threadId);

      if (itr != mThreadMemTraits.end()) {
        has_trait = itr->second->HasTrait(trait_id, startAddr, endAddr);
      }
    }

    return has_trait;
  }

  const ConstraintSet* MemoryTraitsManager::GetTraitAddressRanges(cuint32 threadId, cuint32 traitId) const
  {
    const ConstraintSet* trait_addresses = mGlobalMemTraits.GetTraitAddressRanges(traitId);
    if (trait_addresses == nullptr) {
      auto itr = mThreadMemTraits.find(threadId);

      if (itr != mThreadMemTraits.end()) {
        trait_addresses = itr->second->GetTraitAddressRanges(traitId);
      }
    }

    return trait_addresses;
  }

  MemoryTraitsRange* MemoryTraitsManager::CreateMemoryTraitsRange(cuint32 threadId, cuint64 startAddr, cuint64 endAddr) const
  {
    MemoryTraitsRange* mem_traits_range = nullptr;

    unique_ptr<MemoryTraitsRange> global_mem_traits_range(mGlobalMemTraits.CreateMemoryTraitsRange(startAddr, endAddr));
    auto itr = mThreadMemTraits.find(threadId);
    if (itr != mThreadMemTraits.end()) {
      unique_ptr<MemoryTraitsRange> thread_mem_traits_range(itr->second->CreateMemoryTraitsRange(startAddr, endAddr));
      mem_traits_range = global_mem_traits_range->CreateMergedMemoryTraitsRange(*thread_mem_traits_range);
    }
    else {
      mem_traits_range = global_mem_traits_range.release();
    }

    return mem_traits_range;
  }

  void MemoryTraitsManager::AddGlobalTrait(cuint32 traitId, cuint64 startAddr, cuint64 endAddr)
  {
    AddToMemoryTraits(traitId, startAddr, endAddr, mGlobalMemTraits);
  }

  void MemoryTraitsManager::AddThreadTrait(cuint32 threadId, cuint32 traitId, cuint64 startAddr, cuint64 endAddr)
  {
    MemoryTraits* thread_mem_traits = nullptr;

    auto itr = mThreadMemTraits.find(threadId);
    if (itr != mThreadMemTraits.end()) {
      thread_mem_traits = itr->second;
    }
    else {
      thread_mem_traits = new MemoryTraits();
      mThreadMemTraits.emplace(threadId, thread_mem_traits);
    }

    AddToMemoryTraits(traitId, startAddr, endAddr, *thread_mem_traits);
  }

  void MemoryTraitsManager::AddToMemoryTraits(cuint32 traitId, cuint64 startAddr, cuint64 endAddr, MemoryTraits& rMemTraits)
  {
    vector<uint32> exclusive_ids;
    mpMemTraitsRegistry->GetMutuallyExclusiveTraitIds(traitId, exclusive_ids);
    for (uint32 exclusive_id : exclusive_ids) {
      if (rMemTraits.HasTraitPartial(exclusive_id, startAddr, endAddr)) {
        LOG(fail) << "{MemoryTraitsManager::AddToMemoryTraits} a trait mutually exclusive with the specified trait is already associated with an address in the range 0x" << hex << startAddr << "-0x" << endAddr << endl;
        FAIL("trait-conflict");
      }
    }

    rMemTraits.AddTrait(traitId, startAddr, endAddr);
  }

  MemoryTraitsJson::MemoryTraitsJson(const MemoryTraitsRegistry* pMemTraitsRegistry)
    : mpMemTraitsRegistry(pMemTraitsRegistry)
  {
  }

  void MemoryTraitsJson::DumpTraits(ofstream& rOutFile, cuint32 threadId, const MemoryTraitsManager& rMemTraitsManager) const
  {
    map<string, const ConstraintSet*> arch_mem_attr_ranges;
    map<string, const ConstraintSet*> impl_mem_attr_ranges;

    CategorizeTraitRanges(rMemTraitsManager.mGlobalMemTraits.mTraitRanges, arch_mem_attr_ranges, impl_mem_attr_ranges);
    auto itr = rMemTraitsManager.mThreadMemTraits.find(threadId);
    if (itr != rMemTraitsManager.mThreadMemTraits.end()) {
      CategorizeTraitRanges(itr->second->mTraitRanges, arch_mem_attr_ranges, impl_mem_attr_ranges);
    }

    DumpCategorizedTraitRanges(rOutFile, arch_mem_attr_ranges, impl_mem_attr_ranges);
  }

  void MemoryTraitsJson::DumpTraitsRange(ofstream& rOutFile, const MemoryTraitsRange& rMemTraitsRange) const
  {
    map<string, const ConstraintSet*> arch_mem_attr_ranges;
    map<string, const ConstraintSet*> impl_mem_attr_ranges;

    CategorizeTraitRanges(rMemTraitsRange.mTraitRanges, arch_mem_attr_ranges, impl_mem_attr_ranges);
    DumpCategorizedTraitRanges(rOutFile, arch_mem_attr_ranges, impl_mem_attr_ranges);
  }

  void MemoryTraitsJson::CategorizeTraitRanges(const map<uint32, ConstraintSet*>& rTraitRanges, map<string, const ConstraintSet*>& rArchMemAttributeRanges, map<string, const ConstraintSet*>& rImplMemAttributeRanges) const
  {
    for (const auto& trait_range : rTraitRanges) {
      string trait_name = mpMemTraitsRegistry->GetTraitName(trait_range.first);

      bool is_arch_mem_attr = false;
      try_string_to_EMemoryAttributeType(trait_name, is_arch_mem_attr);
      if (is_arch_mem_attr) {
        rArchMemAttributeRanges.emplace(trait_name, trait_range.second);
      }
      else {
        rImplMemAttributeRanges.emplace(trait_name, trait_range.second);
      }
    }
  }

  void MemoryTraitsJson::DumpCategorizedTraitRanges(ofstream& rOutFile, const map<string, const ConstraintSet*>& rArchMemAttributeRanges, const map<string, const ConstraintSet*>& rImplMemAttributeRanges) const
  {
    rOutFile << "\"ArchMemAttributes\": ";
    DumpTraitRangeCategory(rOutFile, rArchMemAttributeRanges);
    rOutFile << "," << endl;

    rOutFile << "\"ImplMemAttributes\": ";
    DumpTraitRangeCategory(rOutFile, rImplMemAttributeRanges);
  }

  void MemoryTraitsJson::DumpTraitRangeCategory(ofstream& rOutFile, const map<string, const ConstraintSet*>& rTraitRanges) const
  {
    bool first_trait = true;
    rOutFile << dec;
    rOutFile << "[";

    for (const auto& trait_range : rTraitRanges) {
      if (not first_trait) {
        rOutFile << ", ";
      }

      rOutFile << "{";

      rOutFile << "\"Name\": \"" << trait_range.first << "\", ";

      bool first_range = true;
      rOutFile << "\"Ranges\": ";
      rOutFile << "[";

      for (Constraint* constr : trait_range.second->GetConstraints()) {
        if (not first_range) {
          rOutFile << ", ";
        }

        rOutFile << "{";

        rOutFile << "\"StartPhysAddr\": " << constr->LowerBound() << ", ";
        rOutFile << "\"EndPhysAddr\": " << constr->UpperBound();

        rOutFile << "}";

        first_range = false;
      }

      rOutFile << "]";

      rOutFile << "}";

      first_trait = false;
    }

    rOutFile << "]";
  }

}
