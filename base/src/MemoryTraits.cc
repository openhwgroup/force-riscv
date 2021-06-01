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

}
