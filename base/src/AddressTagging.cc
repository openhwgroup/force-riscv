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
#include "AddressTagging.h"

#include "Random.h"
#include "UtilityFunctions.h"

/*!
  \file AddressTagging.cc
  \brief Code supporting address tagging.
*/

namespace Force {

  AddressTagging::AddressTagging()
    : mTagPosition(56), mMaxTag(0xff), mTagMask(0)
  {
    mTagMask = get_mask64(mMaxTag, mTagPosition);
  }

  uint64 AddressTagging::TagAddress(cuint64 address, cuint64 tagValue, cbool isInstruction) const
  {
    uint64 tagged_address = address;
    if (CanTagAddress(address, isInstruction)) {
      tagged_address = ApplyTag(address, tagValue);
    }

    return tagged_address;
  }

  uint64 AddressTagging::TagAddressRandomly(cuint64 address, cbool isInstruction) const
  {
    uint64 tagged_address = address;
    if (CanTagAddress(address, isInstruction)) {
      uint64 tagValue = Random::Instance()->Random32(0, mMaxTag);
      tagged_address = ApplyTag(address, tagValue);
    }

    return tagged_address;
  }

  uint64 AddressTagging::UntagAddress(cuint64 taggedAddress, cbool isInstruction) const
  {
    uint64 address = taggedAddress;
    if (CanTagAddress(address, isInstruction)) {
      address = address & ~mTagMask;
      address = sign_extend64(address, mTagPosition);
    }

    return address;
  }

  uint64 AddressTagging::GetTagValue(cuint64 taggedAddress) const
  {
    return taggedAddress >> mTagPosition;
  }

  bool AddressTagging::AreTaggingCapacitiesEqual(cuint64 addressA, cuint64 addressB, cbool isInstruction) const
  {
    return (CanTagAddress(addressA, isInstruction) == CanTagAddress(addressB, isInstruction));
  }

  uint64 AddressTagging::ApplyTag(cuint64 address, cuint64 tagValue) const
  {
    uint64 tagged_address = (address & ~mTagMask) | (tagValue << mTagPosition);
    return tagged_address;
  }

}
