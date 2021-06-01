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
#ifndef Force_MemoryTraits_H
#define Force_MemoryTraits_H

#include <Defines.h>

#include <map>

namespace Force {

  class ConstraintSet;

  /*!
    \class MemoryTraits
    \brief Class to track memory characteristics associated with physical addresses.
  */
  class MemoryTraits {
  public:
    MemoryTraits();
    COPY_CONSTRUCTOR_ABSENT(MemoryTraits);
    ~MemoryTraits();
    ASSIGNMENT_OPERATOR_ABSENT(MemoryTraits);

    void AddTrait(cuint32 traitId, cuint64 startAddr, cuint64 endAddr); //!< Associate the specified trait with the specified address range. If the specified trait already applies to an overlapping address range, the two ranges are merged.
    bool HasTrait(cuint32 traitId, cuint64 startAddr, cuint64 endAddr) const; //!< Returns true if the specified trait applies to the entire specified address range.
    bool HasTraitPartial(cuint32 traitId, cuint64 startAddr, cuint64 endAddr) const; //!< Returns true if the specified trait applies to any address within the specified address range.
  private:
    std::map<uint32, ConstraintSet*> mTraitRanges; //!< Map from memory trait IDs to associated address ranges
  };

}

#endif  // Force_MemoryTraits_H
