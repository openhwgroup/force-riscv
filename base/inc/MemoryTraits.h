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

#include <fstream>
#include <map>
#include <set>
#include <vector>

namespace Force {

  class ConstraintSet;

  /*!
    \class MemoryTraitsRange
    \brief Class to represent memory characteristics associated with a limited range of physical addresses.
  */
  class MemoryTraitsRange {
  public:
    MemoryTraitsRange(const std::map<uint32, ConstraintSet*>& rTraitRanges, cuint64 startAddr, cuint64 endAddr);
    MemoryTraitsRange(const std::vector<uint32>& rTraitIds, cuint64 startAddr, cuint64 endAddr);
    MemoryTraitsRange(const MemoryTraitsRange& rOther);
    ~MemoryTraitsRange();
    ASSIGNMENT_OPERATOR_ABSENT(MemoryTraitsRange);

    MemoryTraitsRange* CreateMergedMemoryTraitsRange(const MemoryTraitsRange& rOther) const; //!< Returns new MemoryTraitsRange object that combines the memory trait mappings of this and the other MemoryTraitsRange. Both this and the other MemoryTraitsRange must have equal starting and ending addresses.
    bool IsCompatible(const MemoryTraitsRange& rOther) const; //!< Returns true if the following holds true for any given address: either at least one of the MemoryTraitsRanges has no traits specified for that address or both MemoryTraitsRanges have the same traits specified for that address.
    bool IsEmpty() const; //!< Returns true if there are no assigned traits.
  private:
    bool HasCompatibleTraits(const MemoryTraitsRange& rOther) const; //!< Returns true if for every address associated with every trait in this MemoryTraitsRange, the other MemoryTraitsRange has the same trait associated with the address or no trait associated with the address.
  private:
    std::map<uint32, ConstraintSet*> mTraitRanges; //!< Map from memory trait IDs to associated address ranges
    ConstraintSet* mpEmptyRanges; //!< Address ranges with no associated memory traits
    cuint64 mStartAddr; //!< Starting address
    cuint64 mEndAddr; //!< Ending address

    friend class MemoryTraitsJson; // Accesses mTraitRanges in order to dump data without having to otherwise expose this member.
  };

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
    const ConstraintSet* GetTraitAddressRanges(cuint32 traitId) const; //!< Returns the address ranges associated with the specified trait.
    MemoryTraitsRange* CreateMemoryTraitsRange(cuint64 startAddr, cuint64 endAddr) const; //!< Returns new MemoryTraitsRange object representing memory traits associated with addresses in the specified range.
  private:
    std::map<uint32, ConstraintSet*> mTraitRanges; //!< Map from memory trait IDs to associated address ranges

    friend class MemoryTraitsJson; // Accesses mTraitRanges in order to dump data without having to otherwise expose this member.
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
    uint32 AddTrait(const std::string& rTrait); //!< Generate an ID for the specified trait. Fails if the trait already has an associated ID.
    uint32 GetTraitId(const EMemoryAttributeType trait) const; //!< Get the ID associated with the specified trait. Returns 0 if the specified trait is not found.
    uint32 GetTraitId(const std::string& rTrait) const; //!< Get the ID associated with the specified trait. Returns 0 if the specified trait is not found.
    std::string GetTraitName(cuint32 traitId) const; //!< Get the name associated with the specified trait. Returns an empty string if the specified trait is not found.
    uint32 RequestTraitId(const EMemoryAttributeType trait); //!< Get the ID associated with the specified trait. Generate an ID if the specified trait doesn't already have an associated ID.
    uint32 RequestTraitId(const std::string& rTrait); //!< Get the ID associated with the specified trait. Generate an ID if the specified trait doesn't already have an associated ID.
    void GetMutuallyExclusiveTraitIds(cuint32 traitId, std::vector<uint32>& rExclusiveIds) const; //!< Returns a list of IDs of other memory traits that are mutually exclusive with the specified trait. The ID of the specified trait is not included in the returned list.
    bool IsThreadTrait(cuint32 threadId); //!< Returns true if the specified trait is thread-specific.
  protected:
    void AddMutuallyExclusiveTraits(const std::vector<EMemoryAttributeType>& rTraits); //!< Generate IDs for the specified traits, if needed, and mark them as mutually exclusive.
    void AddThreadTraits(const std::vector<EMemoryAttributeType>& rTraits); //!< Generate IDs for the specified traits, if needed, and mark them as thread-specific.
  private:
    void AddMutuallyExclusiveTraitIds(const std::set<uint32>& rTraitIds); //!< Mark the specified traits as mutually exclusive.
  private:
    std::map<std::string, uint32> mTraitIds; //!< Map from descriptive memory trait identifiers to simple IDs
    std::map<uint32, std::set<uint32>> mExclusiveTraitIds; //!< Map from memory trait ID to set of IDs for mutually exclusive memory traits
    std::set<uint32> mThreadTraitIds; //!< Set of thread-specific memory trait IDs
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

    void AddTrait(cuint32 threadId, const EMemoryAttributeType trait, cuint64 startAddr, cuint64 endAddr); //!< Associate the specified trait with the specified address range. If the trait is thread-specific, it is further associated only with the indicated thread; otherwise, the association applies to all threads.
    void AddTrait(cuint32 threadId, const std::string& rTrait, cuint64 startAddr, cuint64 endAddr); //!< Associate the specified trait with the specified address range. If the trait is thread-specific, it is further associated only with the indicated thread; otherwise, the association applies to all threads.
    void AddTrait(cuint32 threadId, cuint32 traitId, cuint64 startAddr, cuint64 endAddr); //!< Associate the specified trait with the specified address range. If the trait is thread-specific, it is further associated only with the indicated thread; otherwise, the association applies to all threads.
    bool HasTrait(cuint32 threadId, const EMemoryAttributeType trait, cuint64 startAddr, cuint64 endAddr) const; //!< Returns true if the specified trait applies to the entire specified address range for the specified thread.
    bool HasTrait(cuint32 threadId, const std::string& rTrait, cuint64 startAddr, cuint64 endAddr) const; //!< Returns true if the specified trait applies to the entire specified address range for the specified thread.
    const ConstraintSet* GetTraitAddressRanges(cuint32 threadId, cuint32 traitId) const; //!< Returns the address ranges associated with the specified trait for the specified thread.
    MemoryTraitsRange* CreateMemoryTraitsRange(cuint32 threadId, cuint64 startAddr, cuint64 endAddr) const; //!< Returns new MemoryTraitsRange object representing memory traits associated with addresses in the specified range for the specified thread.
    MemoryTraitsRegistry* GetMemoryTraitsRegistry() const { return mpMemTraitsRegistry; } //!< Returns the mapping between descriptive memory trait identifiers and simple IDs.
  private:
    void AddGlobalTrait(cuint32 traitId, cuint64 startAddr, cuint64 endAddr); //!< Associate the specified trait with the specified address range globally.
    void AddThreadTrait(cuint32 threadId, cuint32 traitId, cuint64 startAddr, cuint64 endAddr); //!< Associate the specified trait with the specified address range for the specified thread.
    void AddToMemoryTraits(cuint32 traitId, cuint64 startAddr, cuint64 endAddr, MemoryTraits& rMemTraits); //!< Associate the specified trait with the specified address range in the given MemoryTraits object.
  private:
    MemoryTraits mGlobalMemTraits; //!< Global memory traits
    std::map<uint32, MemoryTraits*> mThreadMemTraits; //!< Thread-specific memory traits
    MemoryTraitsRegistry* mpMemTraitsRegistry; //!< Mapping between descriptive memory trait identifiers and simple IDs

    friend class MemoryTraitsJson; // Accesses mGlobalMemTraits and mThreadMemTraits in order to dump data without having to otherwise expose these members.
  };

  /*!
    \class MemoryTraitsJson
    \brief Class to dump memory characteristics in JSON format.
  */
  class MemoryTraitsJson {
  public:
    MemoryTraitsJson(const MemoryTraitsRegistry* pMemTraitsRegistry);
    COPY_CONSTRUCTOR_ABSENT(MemoryTraitsJson);
    DESTRUCTOR_DEFAULT(MemoryTraitsJson);
    ASSIGNMENT_OPERATOR_ABSENT(MemoryTraitsJson);

    void DumpTraits(std::ofstream& rOutFile, cuint32 threadId, const MemoryTraitsManager& rMemTraitsManager) const; //!< Dumps memory trait associations for the specified thread into a partial JSON format. The output is meant to be further enclosed in a JSON object or JSON array in order to represent a fully-formatted JSON file.
    void DumpTraitsRange(std::ofstream& rOutFile, const MemoryTraitsRange& rMemTraitsRange) const; //!< Dumps memory trait associations for a particular address range into a partial JSON format. The output is meant to be further enclosed in a JSON object or JSON array in order to represent a fully-formatted JSON file.
  private:
    void CategorizeTraitRanges(const std::map<uint32, ConstraintSet*>& rTraitRanges, std::map<std::string, const ConstraintSet*>& rArchMemAttributeRanges, std::map<std::string, const ConstraintSet*>& rImplMemAttributeRanges) const; //!< Breaks memory traits up into constituent categories.
    void DumpCategorizedTraitRanges(std::ofstream& rOutFile, const std::map<std::string, const ConstraintSet*>& rArchMemAttributeRanges, const std::map<std::string, const ConstraintSet*>& rImplMemAttributeRanges) const; //!< Dumps memory trait associations in the categorized maps into a partial JSON format.
    void DumpTraitRangeCategory(std::ofstream& rOutFile, const std::map<std::string, const ConstraintSet*>& rTraitRanges) const; //!< Dumps memory trait associations in the specified map into a JSON format. This method is meant to be called with a map containing traits of a given category, e.g. architecture memory attributes.
  private:
    const MemoryTraitsRegistry* mpMemTraitsRegistry; //!< Mapping between descriptive memory trait identifiers and simple IDs
  };

}

#endif  // Force_MemoryTraits_H
