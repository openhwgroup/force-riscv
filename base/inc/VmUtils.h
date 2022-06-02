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
#ifndef Force_VmUtils_H
#define Force_VmUtils_H

#include <string>

#include "Defines.h"
#include "Enums.h"
#include "Object.h"
#include ARCH_ENUM_HEADER

namespace Force {

  uint32 get_page_shift(EPteType pteType); //!< Return equivalent number of bit shifts the page size cover.
  const char* get_page_size_string(uint64 page_size); //!< Return page size string based on the size given.

  /*!
    \class PageSizeInfo
    \brief A info struct describing page size infomation.
  */
  struct PageSizeInfo {
  public:
    std::string mText; //!< Page size in string format.
    EPteType mType;    //!< Page size in enum format.
    uint64 mPageMask;  //!< Page mask for the page size.
    uint32 mPageShift; //!< Page shift value in bits to mask boundary.
  protected:
    uint64 mStart; //!< Page starting virtual address.
    uint64 mPhysicalStart; //!< Page starting physical address.
    mutable uint64 mMaxPhysical; // VMAS Max Physical address allowed.
    mutable uint64 mPhysicalPageId; // Physical Page Id number, used for aliasing target.
  public:
    PageSizeInfo() : mText(), mType(EPteType(0)), mPageMask(0), mPageShift(0), mStart(0), mPhysicalStart(0), mMaxPhysical(0), mPhysicalPageId(0) { } //!< Constructor.
    uint64 Start() const { return mStart; } //!< Return the starting virtual address.
    uint64 End() const { return (mStart | mPageMask); } //!< Return the ending virtual address.
    uint64 Size() const { return (mPageMask + 1); } //!< Return page size.
    uint64 PhysicalStart() const { return mPhysicalStart; } //!< Return the starting physical address.
    uint64 PhysicalEnd() const { return (mPhysicalStart | mPageMask); } //!< Return the ending physical address.
    uint64 MaxPhysical() const { return mMaxPhysical; } //!< Return the max physical address
    uint64 PhysPageId()  const { return mPhysicalPageId; } //!< Return the physical page id.
    void UpdateStart(uint64 VA); //!< Update starting virtual address so that the page range covers VA.
    void UpdatePhysicalStart(uint64 PA); //!< Update starting physical address so that the page range covers PA.
    void UpdateMaxPhysical(uint64 MaxPA) { mMaxPhysical = MaxPA; } //!< Update max physical address for the given page range
    void UpdatePhysPageId(uint64 physPageId) { mPhysicalPageId = physPageId; } //!< Update physical page id for the given page
    uint64 GetSizeInPage(uint64 VA, uint64 size) const; //!< Return the size located in the page range starting from VA.
    static void StringToPageSizeInfo(const std::string& pageSizeText, PageSizeInfo& rSizeInfo); //!< Convert a page size text string to PageSizeInfo object.
    static void ValueToPageSizeInfo(uint64 sizeValue, PageSizeInfo& rSizeInfo); //!< Convert a page size value to PageSizeInfo object.
  };

  class ConstraintSet;

  /*!
    \class VmVaRange
    \brief A class containing VM VA range information.
  */
  class VmVaRange {
  public:
    VmVaRange(EPageGranuleType granuleType, const std::string& rGranuleSuffix, ConstraintSet* pConstr); //!< Constructor.
    ~VmVaRange(); //!< Destructor.
    const std::string ToString() const; //!< Return string presentation of the object.
    const std::string& GranuleSuffix() const { return mGranuleSuffix; } //!< Return granule suffix for the VmVaRange.
    uint64 Size() const; //!< Return range size.
    ConstraintSet* GetConstraint() { return mpRangeConstraint; } //!< Return pointer to the ConstraintSet object.

    ASSIGNMENT_OPERATOR_ABSENT(VmVaRange);
    COPY_CONSTRUCTOR_ABSENT(VmVaRange);
    DEFAULT_CONSTRUCTOR_ABSENT(VmVaRange);
  private:
    EPageGranuleType mGranuleType; //!< VA region granule type.
    std::string mGranuleSuffix; //!< VA region Granule suffix string.
    ConstraintSet* mpRangeConstraint; //!< Address range constraint.
  };

  /*!
    \class TranslationRange
    \brief A translation range used for convenience when translation on a continuous block is very likely, such as PC advancement.
  */
  class TranslationRange {
  public:
    TranslationRange(); //!< Default constructor
    ~TranslationRange(); //!< Destructor
    void SetBoundary(uint64 lower, uint64 upper); //!< Set lower and upper boundary of the virtual address range covered by the TranslationRange.
    inline uint64 Lower() const { return mLower; } //!< Return lower bound.
    inline uint64 Upper() const { return mUpper; } //!< Return upper bound.
    void SetPhysicalBoundary(uint64 lower, uint64 upper); //!< Set physical lower and upper boundary of the address range covered by the TranslationRange.
    inline uint64 PhysicalLower() const { return mPhysicalLower; } //!< Return physical lower bound.
    inline uint64 PhysicalUpper() const { return mPhysicalUpper; } //!< Return physical upper bound.
    uint64 TranslateVaToPa(uint64 VA, uint32& bank) const; //!< Translate VA to PA.

    inline bool Contains(uint64 VA) const //!< Return true if the Page contains the virtual address.
    {
      return (VA >= mLower) and (VA <= mUpper);
    }

    uint64 SpaceInPage(uint64 VA) const; //!< If the Page contains the VA, return space available from the VA to the end of the page, otherwise return 0.
    inline EMemBankType MemoryBank() const { return mMemoryBank; } //!< Return the page's physical memory bank type.
    inline void SetMemoryBank(EMemBankType memBank) { mMemoryBank = memBank; } //!< Set memory bank of the page.
    inline void SetTranslationResult(ETranslationResultType resultType) { mTransResultType = resultType; }
    inline ETranslationResultType TranslationResultType() const { return mTransResultType; }
    const std::string ToString() const; //!< Return TranslationRange details in a string format.
  protected:
    uint64 mLower; //!< Lower bound of the address range that the TranslationRange covers.
    uint64 mUpper; //!< Upper bound of the address range that the TranslationRange covers.
    uint64 mPhysicalLower; //!< Lower bound of the physical address range that the TranslationRange covers.
    uint64 mPhysicalUpper; //!< Upper bound of the physical address range that the TranslationRange covers.
    EMemBankType mMemoryBank; //!< The memory bank where the physical addresses are located.
    ETranslationResultType mTransResultType; //!< Translation result type
  };

  /*!
    \class PhysicalRegion
    \brief Contains information regarding a physical memory region, mainly for paging system to map accordingly.
  */
  class PhysicalRegion : public Object {
  public:
    PhysicalRegion(const std::string& name, uint64 lower, uint64 upper, EPhysicalRegionType physRegionType, EMemBankType bank, EMemDataType dType); //!< Constructor with parameters
    PhysicalRegion(uint64 lower, uint64 upper, EPhysicalRegionType physRegionType, EMemBankType bank, EMemDataType dType, uint64 VA=0ull); //!< Constructor with parameters but no name given.  Name will formed automatically.
    PhysicalRegion(); //!< Default constructor.
    ~PhysicalRegion() { } //!< Destructor.

    Object* Clone() const override { return new PhysicalRegion(*this); } //!< Clone PhysicalRegion object.
    const char* Type() const override { return "PhysicalRegion"; } //!< Return a string describing the actual type of the PhysicalRegion.

    uint64 Lower() const { return mLower; } //!< Return lower boundary.
    uint64 Upper() const { return mUpper; } //!< Return upper boundary.
    uint64 FromVA() const { return mFromVA; } //!< Return from VA, using for PageTable.
    EPhysicalRegionType RegionType() const { return mRegionType; } //!< Return physical region type.
    EMemBankType MemoryBank() const { return mMemoryBank; } //!< Return memory bank of the region.
    EMemDataType DataType() const { return mDataType; } //!< Return physical region data type.
    const std::string ToString() const override; //!< Return PhysicalReggion details in a string format.
    uint64 Size() const { return (mUpper - mLower) + 1; } //!< Return region size.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(PhysicalRegion);
  private:
    std::string mName; //!< Name of the region.
    uint64 mLower; //!< Lower bound of the physical memory region.
    uint64 mUpper; //!< Upper bound of the physical memory region.
    EPhysicalRegionType mRegionType; //!< The type of physical region.
    EMemBankType mMemoryBank; //!< The memory bank where the physical addresses are located.
    EMemDataType mDataType; //!< Desired memory data type.
    uint64 mFromVA; //!< For page table region, it indicate which VA is using this region.
  };

  /*!
    \struct PaTuple
    \brief A data structure holding physical address and the memory bank it belongs to.
  */
  struct PaTuple {
    uint64 mAddress; //!< Physical address
    uint32 mBank; //!< Memory bank.
    PaTuple () : mAddress(0), mBank(0) { } //!< Constructor.
    PaTuple(cuint64 addr, cuint32 bank) : mAddress(addr), mBank(bank) { } //!< Constructor with parameters.

    inline void Assign(uint64 addr, uint32 bank) //!< Assign PA tuple fields.
    {
      mAddress = addr;
      mBank = bank;
    }

    const std::string ToString() const; //!< Return a string describing the details of the object.
  };

}

#endif
