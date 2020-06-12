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
#ifndef Force_AddressTagging_H
#define Force_AddressTagging_H

#include <Defines.h>

namespace Force {

  /*!
    \class AddressTagging
    \brief Base class for address tagging.
  */
  class AddressTagging {
  public:
    AddressTagging(); //!< Constructor
    AddressTagging(const AddressTagging& rOther) = delete; //!< Copy constructor not currently needed.
    virtual ~AddressTagging() = default; //!< Destructor
    AddressTagging& operator=(const AddressTagging& rOther) = delete; //!< Copy assignment operator not currently needed.

    uint64 TagAddress(cuint64 address, cuint64 tagValue, cbool isInstruction) const; //!< Tag address with specified tag value if tagging is enabled for this address.
    uint64 TagAddressRandomly(cuint64 address, cbool isInstruction) const; //!< Tag address with random tag if tagging is enabled for this address.
    uint64 UntagAddress(cuint64 taggedAddress, cbool isInstruction) const; //!< Replace address tag with signed extension or zero extension of most significant non-tag bit if tagging is enabled for this address.
    uint64 GetTagValue(cuint64 taggedAddress) const; //!< Get value of tag from specified address.
    virtual bool CanTagAddress(cuint64 address, cbool isInstruction) const { return false; } //!< Return true if tagging of the specified address is permitted.
    bool AreTaggingCapacitiesEqual(cuint64 addressA, cuint64 addressB, cbool isInstruction) const; //!< Returns true if both addresses can be tagged or both addresses cannot be tagged; returns false otherwise.
  private:
    uint64 ApplyTag(cuint64 address, cuint64 tagValue) const; //!< Insert tag value into address in the appropriate position and return result.
  private:
    cuint8 mTagPosition; //!< Index of least significant tag bit
    cuint64 mMaxTag; //!< Maximum permitted tag value
    uint64 mTagMask; //!< List of bits used to represent the tag
  };

}

#endif  // Force_AddressTagging_H
