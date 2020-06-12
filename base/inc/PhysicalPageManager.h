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
#ifndef Force_PhysicalPageManager_H
#define Force_PhysicalPageManager_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <map>
#include <vector>

namespace Force
{

  class  PhysicalPage;
  class  Page;
  class  GenPageRequest;
  class  ConstraintSet;
  class  VmAddressSpace;
  class  PagingChoicesAdapter;
  class  MemoryConstraintUpdate;
  struct PageSizeInfo;

  /*!
    \class PhysicalPageManager
    \brief Top level module for managing relationship between allocated physical and virtual pages.
  */

  class PhysicalPageManager
  {
  public:
    explicit PhysicalPageManager(EMemBankType bankType);         //!< Constructor, sets mem bank type defaults all others
    COPY_CONSTRUCTOR_ABSENT(PhysicalPageManager);
    virtual ~PhysicalPageManager();                        //!< Destructor, cleans up cloned objects and managed structures
    ASSIGNMENT_OPERATOR_ABSENT(PhysicalPageManager);

    void Initialize(const ConstraintSet* pUsableMem, const ConstraintSet* pBoundary); //!< call to setup initial constraint set objects based on the usable physical memory
    bool AllocatePage(uint64 VA, uint64 size, GenPageRequest* pPageReq, PageSizeInfo& rSizeInfo, const PagingChoicesAdapter* pChoicesAdapter); //!< Allocate page if VA lies in free range, return true if allocated
    void CommitPage(const Page* pPage, uint64 size); //!< Commit page to appropriate physical page object
    void SubFromBoundary(const ConstraintSet& rConstr); //!< Subtract constraint from memory boundary.
    void AddToBoundary(const ConstraintSet& rConstr); //!< Add constraint to memory boundary.
    const ConstraintSet* GetUsable() const { return mpFreeRanges; } //!< Return the free ranges.
    void HandleMemoryConstraintUpdate(const MemoryConstraintUpdate& rMemConstrUpdate) const; //!< Update objects dependent on the physical memory constraint.
    const Page* GetVirtualPage(uint64 PA, const VmAddressSpace* pVmas) const;
    inline const std::vector<ConstraintSet* >& GetAttributeConstraints() const { return mAttributeConstraints; } //!< Return vector of constraint sets representing attributes for memory
  protected:
    virtual const std::vector<EPteType>& GetPteTypes() const = 0; //! Return vector of EPteTypes
    virtual void ConvertMemoryAttributes(const ConstraintSet* pMemAttrs, std::vector<EMemoryAttributeType>& memConstraintTypes) = 0; //!< Propagate the memory attributes into the memory constraint types for use in the aliasing logic
    virtual void GetIncompatibleAttributes(const ConstraintSet* pMemAttrs, std::vector<EMemoryAttributeType>& memConstraintTypes) = 0; //!< Propagate a list of incompatible memory attributes types for use in the aliasing logic
  private:
    bool NewAllocation(uint64 VA, PageSizeInfo& rSizeInfo, GenPageRequest* pPageReq); //!< Function to attempt a new page allocation, returns true on successful allocation
    bool AliasAllocation(uint64 VA, PageSizeInfo& rSizeInfo, GenPageRequest* pPageReq); //!< Function to attempt an aliased allocation, returns true on successful allocation
    bool SolveAliasConstraints(const PageSizeInfo& rSizeInfo, GenPageRequest* pPageReq, uint64& physTarget); //!< Function to attempt to solve for a valid random physical target for aliasing

    //Note: Initialize must be called before GetUsablePageAligned and UpdateUsablePageAligned are to be called
    ConstraintSet* GetUsablePageAligned(EPteType pteType);            //!< return the free ranges aligned based on page size
    void UpdateUsablePageAligned(uint64 start_addr, uint64 end_addr); //!< update mUsablePageAligned to remove pages based on given address
    PhysicalPage* FindPhysicalPage(uint64 lower, uint64 upper) const; //!< return phys page pointer for page object covering lower to upper
    PhysicalPage* FindPhysicalPage(uint64 physId) const;              //!< return phys page pointer for page object
    void AddPhysicalPage(PhysicalPage* physPage);                     //!< add physical page in sorted order to mPhysicalPages

    //utility functions for page allocation
    void UpdateMemoryAttributes(GenPageRequest* pPageReq, PhysicalPage* pPhysPage); //!< update the memory attribute constraint sets based on newly allocated page
    bool MemAttrCompatibility(const ConstraintSet* pAllocAttrs, const ConstraintSet* pAliasAttrs); //!< check to see if the two attribute constraint sets are compatible for aliasing
    //Note: GetPageAttrConstraints returns nullptr if page attributes are not set in the page request
    const ConstraintSet* GetPageAttrConstraints(GenPageRequest* pPageReq) const; //!< get the pointer to either arch/impl constraints from the page request

    static uint64 msPageId;                                           //!< Used for setting up unique physical page IDs
    EMemBankType mMemoryBankType;                                     //!< Bank type of the physical memory manager
    ConstraintSet* mpBoundary;                                        //!< Boundary of the allowed physical memory ranges.
    ConstraintSet* mpFreeRanges;                                      //!< Managed set of free ranges in memory
    ConstraintSet* mpAllocatedRanges;                                 //!< Managed set of allocated ranges in memory
    ConstraintSet* mpAliasExcludeRanges;                              //!< Managed set of ranges to avoid aliasing in.
    mutable std::map<EPteType, ConstraintSet* > mUsablePageAligned;   //!< Map of page sizes to page aligned free ranges
    std::vector<PhysicalPage*> mPhysicalPages;                        //!< Vector of PhysicalPages allocated by this PPM
    std::vector<ConstraintSet* > mAttributeConstraints;               //!< Vector of constraint sets representing attributes for memory
  };

  bool phys_page_less_than(const PhysicalPage* lhs, const PhysicalPage* rhs); //!< comp function for physical pages sort/search algos

}

#endif  // Force_PhysicalPageManager_H
