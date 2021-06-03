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
#include ARCH_ENUM_HEADER

#include <map>
#include <set>
#include <vector>

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

  /*!
    \class MemoryTraitsRegistry
    \brief Class to map between descriptive memory trait identifiers and simple IDs. This simplifies the storage requirements for related classes and allows a uniform representation for architecture-defined and implementation-defined memory traits.
  */
  class MemoryTraitsRegistry {
  public:
    MemoryTraitsRegistry();
    COPY_CONSTRUCTOR_ABSENT(MemoryTraitsRegistry);
    SUPERCLASS_DESTRUCTOR_DEFAULT(MemoryTraitsRegistry);
    ASSIGNMENT_OPERATOR_ABSENT(MemoryTraitsRegistry);

    uint32 AddTrait(const EMemoryAttributeType trait); //!< Generate an ID for the specified trait. Fails if the trait already has an associated ID.
    uint32 AddTrait(const std::string& trait); //!< Generate an ID for the specified trait. Fails if the trait already has an associated ID.
    uint32 GetTraitId(const EMemoryAttributeType trait) const; //!< Get the ID associated with the specified trait. Returns 0 if the specified trait is not found.
    uint32 GetTraitId(const std::string& trait) const; //!< Get the ID associated with the specified trait. Returns 0 if the specified trait is not found.
    uint32 RequestTraitId(const EMemoryAttributeType trait); //!< Get the ID associated with the specified trait. Generate an ID if the specified trait doesn't already have an associated ID.
    uint32 RequestTraitId(const std::string& trait); //!< Get the ID associated with the specified trait. Generate an ID if the specified trait doesn't already have an associated ID.
    void AddMutuallyExclusiveTraits(const std::vector<EMemoryAttributeType>& traits); //!< Generate IDs for the specified traits and mark them as mutually exclusive. Fails if any of the traits already has an associated ID.
    void AddMutuallyExclusiveTraits(const std::vector<std::string>& traits); //!< Generate IDs for the specified traits and mark them as mutually exclusive. Fails if any of the traits already has an associated ID.
    void GetMutuallyExclusiveTraitIds(cuint32 traitId, std::vector<uint32>& exclusiveIds) const; //!< Returns a list of IDs of other memory traits that are mutually exclusive with the specified trait. The ID of the specified trait is not included in the returned list.
  private:
    std::map<std::string, uint32> mTraitIds; //!< Map from descriptive memory trait identifiers to simple IDs
    std::map<uint32, std::set<uint32>> mExclusiveTraitIds; //!< Map from memory trait ID to set of IDs for mutually exclusive memory traits
    uint32 mNextTraitId; //!< Next available ID
  };

  /*!
    \class MemoryTraitsManager
    \brief Class to track global and thread-specific memory characteristics associated with physical addresses. This class assumes that a given trait is either applied globally or on a thread-specific basis, but not both.
  */
  class MemoryTraitsManager {
  public:
    explicit MemoryTraitsManager(MemoryTraitsRegistry* pMemTraitsRegistry);
    COPY_CONSTRUCTOR_ABSENT(MemoryTraitsManager);
    ~MemoryTraitsManager();
    ASSIGNMENT_OPERATOR_ABSENT(MemoryTraitsManager);

    void AddGlobalTrait(const EMemoryAttributeType trait, cuint64 startAddr, cuint64 endAddr); //!< Associate the specified trait globally with the specified address range.
    void AddGlobalTrait(const std::string& trait, cuint64 startAddr, cuint64 endAddr); //!< Associate the specified trait globally with the specified address range.
    void AddThreadTrait(cuint32 threadId, const EMemoryAttributeType trait, cuint64 startAddr, cuint64 endAddr); //!< Associate the specified trait with the specified address range for the specified thread.
    void AddThreadTrait(cuint32 threadId, const std::string& trait, cuint64 startAddr, cuint64 endAddr); //!< Associate the specified trait with the specified address range for the specified thread.
    bool HasTrait(cuint32 threadId, const EMemoryAttributeType trait, cuint64 startAddr, cuint64 endAddr) const; //!< Returns true if the specified trait applies to the entire specified address range for the specified thread.
    bool HasTrait(cuint32 threadId, const std::string& trait, cuint64 startAddr, cuint64 endAddr) const; //!< Returns true if the specified trait applies to the entire specified address range for the specified thread.
  private:
    void AddTrait(const std::string& trait, cuint64 startAddr, cuint64 endAddr, MemoryTraits& memTraits); //!< Associate the specified trait with the specified address range in the given MemoryTraits object.
  private:
    MemoryTraits mGlobalMemTraits; //!< Global memory traits
    std::map<uint32, MemoryTraits*> mThreadMemTraits; //!< Thread-specific memory traits
    MemoryTraitsRegistry* mpMemTraitsRegistry; //!< Mapping between descriptive memory trait identifiers and simple IDs
  };

}

#endif  // Force_MemoryTraits_H
