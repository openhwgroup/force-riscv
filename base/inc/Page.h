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
#ifndef Force_Page_H
#define Force_Page_H

#include <Defines.h>
#include <Object.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <PageTable.h>
#include <vector>

namespace Force {

  class GenPageRequest;
  class PteStructure;
  class PteAttributeStructure;
  class PteAttribute;
  class TranslationRange;
  class VmFactory;

  /*!
    \class PageTableEntry
    \brief Class to support page table entry objects.
  */
  class PageTableEntry : public Object {
  public:
    Object* Clone() const override;  //!< Return a cloned PageTableEntry object of the same type and same contents of the object.
    const std::string ToString() const override; //!< Return a string describing the current state of the PageTableEntry object.
    const char* Type() const override { return "PageTableEntry"; } //!< Return the type of the PageTableEntry object in C string.

    PageTableEntry(); //!< Constructor
    ~PageTableEntry(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(PageTableEntry);
    const std::string FullId() const; //!< Return PTE full ID.
    EPteType PteType() const; //!< Return PTE type.
    uint32 DescriptorSize() const; //!< Return PageTableEntry size in number of bits.
    uint64 Descriptor() const; //!< Return PageTableEntry descriptor.
    virtual std::string DescriptorDetails() const; //!< Return descriptor details in a string format.
    virtual void DescriptorDetails(std::map<std::string, std::string>& details) const; //!< add descritor details to the given map
    const std::string PageGenAttributeToString(EPageGenAttributeType attrType, uint32 attrValue) const; //!< Return string representation of a PageGenAttributeType value.
    void SetPhysicalBoundary(uint64 lower, uint64 upper); //!< Set physical lower and upper boundary of the address range covered by the PTE.
    inline uint64 PhysicalLower() const { return mPhysicalLower; } //!< Return physical lower bound.
    inline uint64 PhysicalUpper() const { return mPhysicalUpper; } //!< Return physical upper bound.
    const PteAttribute* GetPteAttribute(EPteAttributeType attrType) const; //!< Return a PteAttribute object identified by the type.
    uint32 PageGenAttribute(EPageGenAttributeType attrType) const; //!< Return a EPageGenAttributeType attribute value.
    uint32 PageGenAttributeDefaultZero(EPageGenAttributeType attrType) const; //!< Return a EPageGenAttributeType attribute value, if attribue not set, return 0 as default value.
    void SetPageGenAttribute(EPageGenAttributeType attrType, uint32 value); //!< Set a EPageGenAttributeType attribute value.
    void Initialize(const PteStructure* pPteStruct, const VmFactory* pFactory); //!< Initialize PTE object.
    void Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas); //!< Generate page table entry details.
    virtual uint32 Level() const; //!< Return the table level of the page table entry.
    virtual uint32 ParentTableLevel() const; //!< Return the parent table level of the page table entry
    inline bool ContainsPa(uint64 PA) const { return ((PA >= mPhysicalLower) && (PA <= mPhysicalUpper)); } //!< Return whether the physical address is contained by the PageTableEntry.
  protected:
    PageTableEntry(const PageTableEntry& rOther); //!< Copy constructor.
  protected:
    const PteStructure* mpStructure; //!< Pointer to PteStructure object that describes the structure of the PageTableEntry object.
    std::vector<PteAttribute* > mAttributes; //!< Container holding pointers to all PTE attributes.
    std::map<EPageGenAttributeType, uint32> mGenAttributes; //!< Attributes that generator need to keep about the PTE about generating it.
    uint64 mPhysicalLower; //!< Lower bound of the physical address range that the PTE covers.
    uint64 mPhysicalUpper; //!< Upper bound of the physical address range that the PTE covers.
  };

  /*!
    \class Page
    \brief Class to support page objects.
  */
  class Page : public PageTableEntry {
  public:
    Object* Clone() const override;  //!< Return a cloned Page object of the same type and same contents of the object.
    const std::string ToString() const override; //!< Return a string describing the current state of the Page object.
    const char* Type() const override { return "Page"; } //!< Return the type of the Page object in C string.

    Page(); //!< Default constructor
    ~Page(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(Page);
    uint64 PageSize() const; //!< Return Page size in number of bytes.
    void SetBoundary(uint64 lower, uint64 upper); //!< Set lower and upper boundary of the address range covered by the Page.
    void SetPhysPageId(uint64 physId) { mPhysPageId = physId; } //!< Set physical page ID
    inline uint64 Lower() const { return mLower; } //!< Return lower bound.
    inline uint64 Upper() const { return mUpper; } //!< Return upper bound.
    inline uint64 PhysPageId() const { return mPhysPageId; } //!< return physical page ID
    RootPageTable* GetRootPageTable() const { return mpRootTable; } //!< return root page table
    ETranslationResultType TranslateVaToPa(uint64 VA, uint64& PA, uint32& bank) const; //!< Translate VA to PA/Bank.
    ETranslationResultType TranslatePaToVa(uint64 PA, EMemBankType bank, uint64& VA) const; //!< Translate PA/Bank to VA.
    void SetRootPageTable(RootPageTable* rootTable) { mpRootTable = rootTable; } //!< Set root page table.
    bool Overlaps(const Page& rOther) const; //!< Return true if the two pages overlap.
    bool Contains(uint64 VA) const; //!< Return true if the Page contains the virtual address.
    const std::string VaRangeString() const; //!< Return a string showing the Page object's VA range.
    EMemBankType MemoryBank() const { return mMemoryBank; } //!< Return the page's physical memory bank type.
    void SetMemoryBank(EMemBankType memBank) { mMemoryBank = memBank; } //!< Set memory bank of the page.
    std::string DescriptorDetails() const override; //!< Return Page details in a string format.
    void DescriptorDetails(std::map<std::string, std::string>& details) const override; //!< add descritor details to the given map

    void GetTranslationRange(TranslationRange& rTransRange) const; //!< Get translation range of the page.
    virtual ETranslationResultType TranslationResultType() const { return ETranslationResultType::Mapped; }
  protected:
    Page(const Page& rOther); //!< Copy constructor.
  protected:
    RootPageTable* mpRootTable; //!< Pointer to root level page table.
    uint64 mLower; //!< Lower bound of the address range that the Page covers.
    uint64 mUpper; //!< Upper bound of the address range that the Page covers.
    EMemBankType mMemoryBank; //!< The memory bank where the physical page is located.
    uint64 mPhysPageId; //!< The physical page id that is mapped to the virtual page.
  };

  /*!
    \class TablePte
    \brief Class to support page table objects.
  */
  class TablePte : public PageTableEntry, public PageTable {
  public:
    Object* Clone() const override;  //!< Return a cloned Page object of the same type and same contents of the object.
    const std::string ToString() const override; //!< Return a string describing the current state of the TablePte object.
    const char* Type() const override { return "TablePte"; } //!< Return the type of the TablePte object in C string.

    TablePte(); //!< Constructor
    ~TablePte(); //!< Destructor
    uint32 Level() const override; //!< Return TablePte level.
    uint32 ParentTableLevel() const override; //!< Return parent level of TablePte
  protected:
    TablePte(const TablePte& rOther); //!< Copy constructor.
  protected:
  };

  bool compare_pages(const Page* a, const Page* b); //!< Compare function used in sorting and searching operations for Page objects.


  /*!
    \class AddressErrorPage
    \brief A class used to cover address error space in a VmAddressSpace.
  */
  class AddressErrorPage : public Page {
  public:
    Object* Clone() const override { return new AddressErrorPage(*this); } //!< Return a cloned AddressErrorPage object of the same type and same contents of the object.
    const std::string ToString() const override; //!< Return a string describing the current state of the AddressErrorPage object.
    const char* Type() const override { return "AddressErrorPage"; } //!< Return the type of the AddressErrorPage object in C string.

    AddressErrorPage() : Page() { } //!< Default constructor
    ~AddressErrorPage() { } //!< Destructor
    ETranslationResultType TranslateVaToPa(uint64 VA, uint64& PA, uint32& bank) const //!< Translate VA to PA.
    {
      PA = 0;
      bank = 0;
      return ETranslationResultType::AddressError;
    }
    ETranslationResultType TranslationResultType() const override { return ETranslationResultType::AddressError; }
    std::string DescriptorDetails() const override { return "Address Error page"; } //!< Return Page details in a string format.
  protected:
    AddressErrorPage(const AddressErrorPage& rOther) : Page(rOther) { }//!< Copy constructor.
  };

}

#endif
