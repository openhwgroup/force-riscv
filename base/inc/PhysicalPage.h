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
#ifndef PHYSICAL_PAGE_H
#define PHYSICAL_PAGE_H

#include <string>
#include <vector>

#include "Defines.h"
#include "Enums.h"
#include "Object.h"
#include ARCH_ENUM_HEADER

namespace Force
{

  class Page;
  class ConstraintSet;
  class VmAddressSpace;
  class MemoryConstraintUpdate;

  class PhysicalPage : public Object //should physicalpage inherit pte?
  {
    public:
    PhysicalPage();
    PhysicalPage(uint64 lower, uint64 upper);  //!< Constructor for generating lookup pages
    PhysicalPage(uint64 lower, uint64 upper, uint64 pageId); //!< Constructor for generating lookup pages w/ ID
    PhysicalPage(const Page* pPage, uint64 lower, uint64 upper, uint64 pageId); //!< Constructor for generating new physical page w/ virtual mapping and ID.
    PhysicalPage(uint64 lower, uint64 upper, bool alias, uint64 pageId); //!< Constructor for generating lookup pages w/ ID and alias attribute
    PhysicalPage(const Page* pPage, uint64 lower, uint64 upper, bool alias, uint64 pageId); //!< Constructor for generating new page w/ virtual mapping, ID, and alias attribute.
    virtual ~PhysicalPage(); //!< Destructor
    PhysicalPage(const PhysicalPage& rOther); //!< Copy constructor

    Object* Clone() const override { return new PhysicalPage(*this); } //!< Object Clone override
    const std::string ToString() const override { return "PhysicalPage"; } //!< Object ToString override
    const char* Type() const override { return "PhysicalPage"; } //!< Object Type override

    void SetCanAlias(bool canAlias) { mCanAlias = canAlias; } //!< setter for can alias flag

    uint64 Lower() const { return mLower; }     //!< accessor for physical lower bound
    uint64 Upper() const { return mUpper; }     //!< accessor for physical upper bound
    bool CanAlias() const { return mCanAlias; } //!< accessor for alias flag
    uint64 PageId() const { return mPageId; }   //!< accessor for page id
    ConstraintSet* GetMemoryAttributes() const; //!< return memory attribute value for underlying virtual pages
    void HandleMemoryConstraintUpdate(const MemoryConstraintUpdate& rMemConstrUpdate) const; //!< Update address spaces dependent on the physical memory constraint.
    const Page* GetVirtualPage(uint64 PA, const VmAddressSpace* pVmas) const; //!< return the virtual page corresponding to the PA if pVmas matches page vmas

    void AddPage(const Page* pPage);     //!< Add a virtual page preserving sorted order
    void Merge(PhysicalPage* pPhysPage); //!< Merge with another physical page, deletes pPhysPage and sets to nullptr

    protected:
    uint64 mLower; //!< Start physical address of this physical page
    uint64 mUpper; //!< End physical address of this physical page
    bool mCanAlias; //!< Flag for whether the page can be aliased
    uint64 mPageId; //!< Unique ID for page, used for aliasing APIs
    std::vector<const Page*> mVirtualPages; //!< Sorted vector of all virtual Page objects that map to this PhysicalPage
  };

  bool page_less_than(const Page* lhs, const Page* rhs); //comparison function for virtual pages

}
#endif //PHYSICAL_PAGE_H
