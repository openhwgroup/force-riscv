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
#include <MemoryTraits.h>

#include <Constraint.h>
#include <Log.h>

#include <algorithm>

using namespace std;

/*!
  \file MemoryTraits.cc
  \brief Code supporting tracking memory characteristics associated with physical addresses.
*/

namespace Force {

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

  MemoryTraitsRegistry::MemoryTraitsRegistry()
    : mTraitIds(), mExclusiveTraitIds(), mNextTraitId(1)
  {
  }

  uint32 MemoryTraitsRegistry::AddTrait(const EMemoryAttributeType trait)
  {
    return AddTrait(EMemoryAttributeType_to_string(trait));
  }

  uint32 MemoryTraitsRegistry::AddTrait(const string& trait)
  {
    uint32 trait_id = 0;

    auto itr = mTraitIds.find(trait);
    if (itr == mTraitIds.end()) {
      trait_id = mNextTraitId++;
      mTraitIds.emplace(trait, trait_id);
    }
    else {
      LOG(fail) << "{MemoryTraitsRegistry::AddTrait} trait " << trait << " has already been added" << endl;
      FAIL("trait-already-exists");
    }

    return trait_id;
  }

  uint32 MemoryTraitsRegistry::GetTraitId(const EMemoryAttributeType trait) const
  {
    return GetTraitId(EMemoryAttributeType_to_string(trait));
  }

  uint32 MemoryTraitsRegistry::GetTraitId(const string& trait) const
  {
    uint32 trait_id = 0;

    auto itr = mTraitIds.find(trait);
    if (itr != mTraitIds.end()) {
      trait_id = itr->second;
    }

    return trait_id;
  }

  uint32 MemoryTraitsRegistry::RequestTraitId(const EMemoryAttributeType trait)
  {
    return RequestTraitId(EMemoryAttributeType_to_string(trait));
  }

  uint32 MemoryTraitsRegistry::RequestTraitId(const string& trait)
  {
    uint32 trait_id = GetTraitId(trait);
    if (trait_id == 0) {
      trait_id = AddTrait(trait);
    }

    return trait_id;
  }

  void MemoryTraitsRegistry::AddMutuallyExclusiveTraits(const vector<EMemoryAttributeType>& traits)
  {
    set<uint32> exclusive_ids;
    for (EMemoryAttributeType trait : traits) {
      exclusive_ids.insert(AddTrait(trait));
    }

    for (uint32 trait_id : exclusive_ids) {
      mExclusiveTraitIds.emplace(trait_id, exclusive_ids);
    }
  }

  void MemoryTraitsRegistry::AddMutuallyExclusiveTraits(const vector<string>& traits)
  {
    set<uint32> exclusive_ids;
    for (const string& trait : traits) {
      exclusive_ids.insert(AddTrait(trait));
    }

    for (uint32 trait_id : exclusive_ids) {
      mExclusiveTraitIds.emplace(trait_id, exclusive_ids);
    }
  }

  void MemoryTraitsRegistry::GetMutuallyExclusiveTraitIds(cuint32 traitId, vector<uint32>& exclusiveIds) const
  {
    auto itr = mExclusiveTraitIds.find(traitId);
    if (itr != mExclusiveTraitIds.end()) {
      // Don't include the specified trait ID in the output
      copy_if(itr->second.begin(), itr->second.end(), back_inserter(exclusiveIds),
        [traitId](cuint32 exclusiveId) { return (exclusiveId != traitId); });
    }
  }

}
