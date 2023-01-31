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
#include "Page.h"

#include <algorithm>
#include <numeric>  // C++UP accumulate defined in numeric
#include <sstream>

#include "GenException.h"
#include "Log.h"
#include "ObjectRegistry.h"
#include "PteAttribute.h"
#include "PteStructure.h"
#include "VmFactory.h"
#include "VmUtils.h"

using namespace std;

namespace Force {

  PageTableEntry::PageTableEntry()
    : Object(), mpStructure(nullptr), mAttributes(), mGenAttributes(), mPhysicalLower(0), mPhysicalUpper(0)
  {

  }

  PageTableEntry::PageTableEntry(const PageTableEntry& rOther)
    : Object(rOther), mpStructure(rOther.mpStructure), mAttributes(), mGenAttributes(), mPhysicalLower(rOther.mPhysicalLower), mPhysicalUpper(rOther.mPhysicalUpper)
  {
    transform(rOther.mAttributes.cbegin(), rOther.mAttributes.cend(), back_inserter(mAttributes),
      [](const PteAttribute* pAttr) { return dynamic_cast<PteAttribute*>(pAttr->Clone()); });
  }

  PageTableEntry::~PageTableEntry()
  {
    for (auto attr_ptr : mAttributes) {
      delete attr_ptr;
    }

    // << "PTE : " << FullId() << " deleted." << endl;
  }

  Object* PageTableEntry::Clone() const
  {
    return new PageTableEntry(*this);
  }

  const string PageTableEntry::ToString() const
  {
    return Type();
  }

  const string PageTableEntry::FullId() const
  {
    return mpStructure->FullId();
  }

  EPteType PageTableEntry::PteType() const
  {
    return mpStructure->mType;
  }

  EPteCategoryType PageTableEntry::PteCategory() const
  {
    return mpStructure->mCategory;
  }

  uint32 PageTableEntry::DescriptorSize() const
  {
    return mpStructure->Size();
  }

  uint32 PageTableEntry::Level() const
  {
    return mpStructure->mLevel;
  }

  uint32 PageTableEntry::ParentTableLevel() const
  {
    return mpStructure->mLevel;
  }

  void PageTableEntry::SetPhysicalBoundary(uint64 lower, uint64 upper)
  {
    mPhysicalLower = lower;
    mPhysicalUpper = upper;
  }

  const PteAttribute* PageTableEntry::GetPteAttribute(EPteAttributeType attrType) const
  {
    PteAttribute* pte_attr = nullptr;

    auto itr = find_if(mAttributes.cbegin(), mAttributes.cend(),
      [attrType](const PteAttribute* pPteAttr) { return (pPteAttr->PteAttributeType() == attrType); });

    if (itr != mAttributes.end()) {
      pte_attr = *itr;
    }
    else {
      LOG(fail) << "{PageTableEntry::GetPteAttribute} attribute not found: " << EPteAttributeType_to_string(attrType) << endl;
      FAIL("pte-attribute-not-found");
    }

    return pte_attr;
  }

  void PageTableEntry::Initialize(const PteStructure* pteStruct, const VmFactory* pFactory)
  {
    mpStructure = pteStruct;

    const vector<PteAttributeStructure* >& attr_vec = mpStructure->AttributeStructures();

    ObjectRegistry* obj_registry = ObjectRegistry::Instance();
    for (auto attr_struct_ptr : attr_vec) {
      PteAttribute* attr = nullptr;
      if (attr_struct_ptr->Factory()) {
        attr = pFactory->PteAttributeInstance(attr_struct_ptr->Type());
      }
      else {
        attr = obj_registry->TypeInstance<PteAttribute>(attr_struct_ptr->Class());
      }
      attr->Initialize(attr_struct_ptr);
      mAttributes.push_back(attr);
    }
  }

  void PageTableEntry::Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas)
  {
    for (auto attr_ptr : mAttributes) {
      try {
        attr_ptr->Generate(rPagingReq, rVmas, *this);
      }
      catch (const ConstraintError& rConstrErr) {
        LOG(fail) << "{PageTableEntry::Generate} Unable to generate page table attribute " << attr_ptr->ToString() << "; " << rConstrErr.what() << endl;
        FAIL("page-gen-attribute-gen-failure");
      }
    }
  }

  uint64 PageTableEntry::Descriptor() const
  {
    uint64 descr_value = accumulate(mAttributes.cbegin(), mAttributes.cend(), uint64(0),
      [](cuint64 partialDescr, const PteAttribute* pAttr) -> uint64 { return (partialDescr | pAttr->Encoding()); });

    return descr_value;
  }

  uint32 PageTableEntry::PageGenAttribute(EPageGenAttributeType attrType) const
  {
    auto attr_finder = mGenAttributes.find(attrType);

    if (attr_finder != mGenAttributes.end()) {
      return attr_finder->second;
    }

    LOG(fail) << "{PageTableEntry::PageGenAttribute} attribute: " << EPageGenAttributeType_to_string(attrType) << " not specified." << endl;
    FAIL("page-gen-attribute-not-specified");

    return 0;
  }

  uint32 PageTableEntry::PageGenAttributeDefaultZero(EPageGenAttributeType attrType) const
  {
    auto attr_finder = mGenAttributes.find(attrType);

    if (attr_finder != mGenAttributes.end()) {
      return attr_finder->second;
    }

    return 0;
  }

  void PageTableEntry::SetPageGenAttribute(EPageGenAttributeType attrType, uint32 value)
  {
    mGenAttributes[attrType] = value;
  }

  string PageTableEntry::DescriptorDetails() const
  {
    stringstream out_str;

    out_str << EPteType_to_string(PteType()) << ": " << hex;

    for (auto attr_ptr : mAttributes) {
      out_str << "-" << EPteAttributeType_to_string(attr_ptr->PteAttributeType()) << "=0x" << attr_ptr->Value() << "(0x" << attr_ptr->Encoding() << ")";
    }

    for (auto gattr_item : mGenAttributes) {
      out_str << "+" << EPageGenAttributeType_to_string(gattr_item.first) << "=" << PageGenAttributeToString(gattr_item.first, gattr_item.second);
    }

    return out_str.str();
  }

  void PageTableEntry::DescriptorDetails(std::map<std::string, std::string>& details) const
  {
    details.clear();
    details["PteType"] = EPteType_to_string(PteType());

    for (auto attr_ptr : mAttributes) {
      stringstream out_str;
      out_str << hex << "0x" << attr_ptr->Value() << "(0x" << attr_ptr->Encoding() << ")";
      details[EPteAttributeType_to_string(attr_ptr->PteAttributeType())] = out_str.str();
    }

    for (auto gattr_item : mGenAttributes) {
      details[EPageGenAttributeType_to_string(gattr_item.first)] = PageGenAttributeToString(gattr_item.first, gattr_item.second);
    }

  }

  const string PageTableEntry::PageGenAttributeToString(EPageGenAttributeType attrType, uint32 attrValue) const
  {
    switch (attrType) {
    case EPageGenAttributeType::MemAttrImpl:
      {
        EMemAttributeImplType mem_type = EMemAttributeImplType(attrValue);
        return EMemAttributeImplType_to_string(mem_type);
      }
    case EPageGenAttributeType::Invalid:
      return std::to_string(attrValue);
    case EPageGenAttributeType::DataAccessPermission:
      return EDataAccessPermissionType_to_string(EDataAccessPermissionType(attrValue));
    case EPageGenAttributeType::InstrAccessPermission:
      return EInstrAccessPermissionType_to_string(EInstrAccessPermissionType(attrValue));
    case EPageGenAttributeType::AddrSizeFault:
      return std::to_string(attrValue);
    case EPageGenAttributeType::Accessed:
      return std::to_string(attrValue);
    case EPageGenAttributeType::Dirty:
      return std::to_string(attrValue);
    default:
      LOG(fail) << "{PageTableEntry::PageGenAttributeToString} attribute not handled: " << EPageGenAttributeType_to_string(attrType) << endl;
      FAIL("unhandled-page-gen-attribute");
    }

    return "";
  }

  Page::Page()
    : PageTableEntry(), mpRootTable(nullptr), mLower(0), mUpper(0), mMemoryBank(EMemBankType(0)), mPhysPageId(0)
  {

  }

  Page::Page(const Page& rOther)
    : PageTableEntry(rOther), mpRootTable(nullptr), mLower(rOther.mLower), mUpper(rOther.mUpper), mMemoryBank(rOther.mMemoryBank), mPhysPageId(rOther.mPhysPageId)
  {

  }

  Page::~Page()
  {

  }

  Object* Page::Clone() const
  {
    return new Page(*this);
  }

  uint64 Page::PageSize() const
  {
    return mUpper - mLower + 1;
  }

  void Page::SetBoundary(uint64 lower, uint64 upper)
  {
    mLower = lower;
    mUpper = upper;
  }

  ETranslationResultType Page::TranslateVaToPa(uint64 VA, uint64& PA, uint32& bank) const
  {
    if ((VA < mLower) || (VA > mUpper)) {
      LOG(fail) << "{Page::TranslateVaToPa} translation out of bound." << endl;
      FAIL("translation-out-of-bound");
    }

    uint64 offset = VA - mLower;
    bank = uint32(mMemoryBank);
    PA = (mPhysicalLower + offset);
    return ETranslationResultType::Mapped;
  }

  ETranslationResultType Page::TranslatePaToVa(uint64 PA, EMemBankType bank, uint64& VA) const
  {
    if ((PA < mPhysicalLower) || (PA > mPhysicalUpper) || bank != mMemoryBank)
    {
      LOG(fail) << "{Page::TranslatePaToVa} translation out of bound." << endl;
      FAIL("translation-out-of-bound");
    }

    uint64 offset = PA - mPhysicalLower;
    VA = (mLower + offset);
    return ETranslationResultType::Mapped;
  }

  bool Page::Overlaps(const Page& rOther) const
  {
    return (mLower <= rOther.mUpper) and (rOther.mLower <= mUpper);
  }

  bool Page::Contains(uint64 VA) const
  {
    return (VA >= mLower) and (VA <= mUpper);
  }

  void Page::GetTranslationRange(TranslationRange& rTransRange) const
  {
    rTransRange.SetPhysicalBoundary(mPhysicalLower, mPhysicalUpper);
    rTransRange.SetMemoryBank(mMemoryBank);
    rTransRange.SetBoundary(mLower, mUpper);
    rTransRange.SetTranslationResult(TranslationResultType());
  }

  const string Page::VaRangeString() const
  {
    stringstream out_str;

    out_str << "VA: 0x" << hex << mLower << "-0x" << mUpper;

    return out_str.str();
  }

  const string Page::ToString() const
  {
    stringstream out_str;

    out_str << Type() << "(" << EPteType_to_string(PteType()) << ") ("
        << "0x" << hex << mLower << "-0x" << mUpper
        << "=>[" << EMemBankType_to_string(mMemoryBank) << "]"
        << "0x" << mPhysicalLower << "-0x" << mPhysicalUpper << ")";

    return out_str.str();
  }

  string Page::DescriptorDetails() const
  {
    return PageTableEntry::DescriptorDetails() + " " + ToString();
  }

  void Page::DescriptorDetails(std::map<std::string, std::string>& details) const
  {
    PageTableEntry::DescriptorDetails(details);
    // Note: the PteType is already added; the address boundary can be retrieved directly from Page object
  }

  TablePte::TablePte()
    : PageTableEntry(), PageTable()
  {

  }

  TablePte::TablePte(const TablePte& rOther)
    : PageTableEntry(rOther), PageTable(rOther)
  {

  }

  TablePte::~TablePte()
  {

  }

  Object* TablePte::Clone() const
  {
    return new TablePte(*this);
  }

  uint32 TablePte::Level() const
  {
    return mTableLevel;
  }

  uint32 TablePte::ParentTableLevel() const
  {
    if (mTableLevel >= 0)
    {
      return mTableLevel + 1;
    }

    LOG(fail) << "{TablePte::ParentTableLevel} invalid table level: " << mTableLevel << endl;
    FAIL("pte-invalid-table-level");

    return 0;
  }

  const string TablePte::ToString() const
  {
    stringstream out_str;

    out_str << Type() << "(" << EPteType_to_string(PteType()) << ") (0x" << hex << mPhysicalLower << "-0x" << mPhysicalUpper << ")";

    return out_str.str();
  }

  bool compare_pages(const Page* a, const Page* b)
  {
    return a->Upper() < b->Lower();
  }

  const std::string AddressErrorPage::ToString() const
  {
    stringstream out_str;
    out_str << "Address Error: " << "(0x" << hex << mLower << "-0x" << mUpper << ")";
    return out_str.str();
  }

}
