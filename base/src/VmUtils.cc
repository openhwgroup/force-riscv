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
#include "VmUtils.h"

#include <sstream>

#include "Constraint.h"
#include "Log.h"
#include "UtilityFunctions.h"

using namespace std;

/*!
  \file VmUtils.cc
  \brief Code for virtual memory related utility classes/structs/functions.
*/

namespace Force {

  uint32 get_page_shift(EPteType pteType)
  {
    uint32 page_shift = 0;
    switch (pteType) {
    case EPteType::P4K:
      page_shift = 12;
      break;
    case EPteType::P16K:
      page_shift = 14;
      break;
    case EPteType::P64K:
      page_shift = 16;
      break;
    case EPteType::P2M:
      page_shift = 21;
      break;
    case EPteType::P4M:
      page_shift = 22;
      break;
    case EPteType::P32M:
      page_shift = 25;
      break;
    case EPteType::P512M:
      page_shift = 29;
      break;
    case EPteType::P1G:
      page_shift = 30;
      break;
    case EPteType::P512G:
      page_shift = 39;
      break;
    default:
      LOG(fail) << "{VmUtils::get_page_shift} unsupported PTE type enum value : " << dec << (uint64)(pteType) << endl;
      FAIL("unsupported-pte-type");
    }

    return page_shift;
  }

  const char* get_page_size_string(uint64 page_size)
  {
    uint32 page_shift = get_align_shift(page_size);
    switch (page_shift) {
    case 12:
      return "4K";
    case 14:
      return "16K";
    case 16:
      return "64K";
    case 21:
      return "2M";
    case 25:
      return "32M";
    case 29:
      return "512M";
    case 30:
      return "1G";
    case 39:
      return "512G";
    default:
      LOG(fail) << "{VmUtils::get_page_size_string} unsupported page size 0x: " << dec << (uint64)(page_size) << endl;
      FAIL("unsupported-page-size");
    }

    return NULL;
  }

  void PageSizeInfo::StringToPageSizeInfo(const string& pageSizeText, PageSizeInfo& rSizeInfo)
  {
    string psize_str = "P" + pageSizeText;
    EPteType pte_type = string_to_EPteType(psize_str);
    rSizeInfo.mText = pageSizeText;
    rSizeInfo.mType = pte_type;

    rSizeInfo.mPageShift = get_page_shift(pte_type);
    rSizeInfo.mPageMask  = (1ull << rSizeInfo.mPageShift) - 1;
  }

  void PageSizeInfo::ValueToPageSizeInfo(uint64 sizeValue, PageSizeInfo& rSizeInfo)
  {
    uint32 page_shift = get_align_shift(sizeValue);
    EPteType pte_type = EPteType(0);

    switch (page_shift) {
    case 12:
      pte_type = EPteType::P4K;
      break;
    case 14:
      pte_type = EPteType::P16K;
      break;
    case 16:
      pte_type = EPteType::P64K;
      break;
    case 21:
      pte_type = EPteType::P2M;
      break;
    case 22:
      pte_type = EPteType::P4M;
      break;
    case 25:
      pte_type = EPteType::P32M;
      break;
    case 29:
      pte_type = EPteType::P512M;
      break;
    case 30:
      pte_type = EPteType::P1G;
      break;
    case 39:
      pte_type = EPteType::P512G;
      break;
    default:
      LOG(fail) << "{PageSizeInfo::ValueToPageSizeInfo} unsupported PTE page shift value : " << dec << page_shift << endl;
      FAIL("unsupported-pte-page-shift");
    }

    string pte_str       = EPteType_to_string(pte_type);
    rSizeInfo.mText      = pte_str.substr(1); // don't need the leading "P" char.
    rSizeInfo.mType      = pte_type;
    rSizeInfo.mPageShift = page_shift;
    rSizeInfo.mPageMask  = (1ull << page_shift) - 1;
  }

  void PageSizeInfo::UpdateStart(uint64 VA)
  {
    mStart = VA & (~mPageMask);
  }

  void PageSizeInfo::UpdatePhysicalStart(uint64 PA)
  {
    mPhysicalStart = PA & (~mPageMask);
  }

  uint64 PageSizeInfo::GetSizeInPage(uint64 VA, uint64 size) const
  {
    uint64 check_start = VA & (~mPageMask);
    if (check_start != mStart) {
      LOG(fail) << "{PageSizeInfo::GetSizeInPage} specified VA=0x" << hex << VA << " not in page range: 0x" << Start() << "-0x" << End() << endl;
      FAIL("VA-nor-in-range");
    }
    uint64 page_offset = VA & mPageMask;
    uint64 size_in_page = mPageMask - page_offset + 1;
    if (size_in_page < size) return size_in_page;
    return size;
  }

  VmVaRange::VmVaRange(EPageGranuleType granuleType, const string& rGranuleSuffix, ConstraintSet* pConstr)
    : mGranuleType(granuleType), mGranuleSuffix(rGranuleSuffix), mpRangeConstraint(pConstr)
  {

  }

  VmVaRange::~VmVaRange()
  {
    delete mpRangeConstraint;
  }

  uint64 VmVaRange::Size() const
  {
    return mpRangeConstraint->Size();
  }

  const string VmVaRange::ToString() const
  {
    stringstream str_out;

    str_out << "VmVaRange granule: " << EPageGranuleType_to_string(mGranuleType);

    if (nullptr == mpRangeConstraint) {
      str_out << " constraint nullptr.";
    }
    else {
      str_out << " constraint: " << mpRangeConstraint->ToSimpleString();
    }

    return str_out.str();
  }

  TranslationRange::TranslationRange()
    : mLower(0), mUpper(0), mPhysicalLower(0), mPhysicalUpper(0), mMemoryBank(EMemBankType(0)), mTransResultType(ETranslationResultType::NotMapped)
  {

  }

  TranslationRange::~TranslationRange()
  {

  }

  void TranslationRange::SetBoundary(uint64 lower, uint64 upper)
  {
    mLower = lower;
    mUpper = upper;
  }

  void TranslationRange::SetPhysicalBoundary(uint64 lower, uint64 upper)
  {
    mPhysicalLower = lower;
    mPhysicalUpper = upper;
  }

  uint64 TranslationRange::TranslateVaToPa(uint64 VA, uint32& bank) const
  {
    if ((VA < mLower) || (VA > mUpper)) {
      LOG(fail) << "{TranslatonRange::TranslateVaToPa} translation out of bound." << endl;
      FAIL("translation-out-of-bound");
    }

    uint64 offset = VA - mLower;
    bank = uint32(mMemoryBank);
    return (mPhysicalLower + offset);
  }

  uint64 TranslationRange::SpaceInPage(uint64 VA) const
  {
    if (VA > mUpper) return 0;
    if (VA < mLower) return 0;

    return (mUpper - VA + 1);
  }

  const string TranslationRange::ToString() const
  {
    stringstream out_str;

    out_str << "[TranslationRange] 0x" << hex << mLower << "-0x" << mUpper << "=>[" << EMemBankType_to_string(mMemoryBank) << "]0x" << mPhysicalLower << "-0x" << mPhysicalUpper;

    return out_str.str();
  }

  PhysicalRegion::PhysicalRegion(const std::string& name, uint64 lower, uint64 upper, EPhysicalRegionType physRegionType, EMemBankType bank, EMemDataType dType)
    : mName(name), mLower(lower), mUpper(upper), mRegionType(physRegionType), mMemoryBank(bank), mDataType(dType), mFromVA(0)
  {

  }

  PhysicalRegion::PhysicalRegion()
    : mName(), mLower(0), mUpper(0), mRegionType(EPhysicalRegionType(0)), mMemoryBank(EMemBankType(0)), mDataType(EMemDataType(0)), mFromVA(0)
  {
  }

  PhysicalRegion::PhysicalRegion(uint64 lower, uint64 upper, EPhysicalRegionType physRegionType, EMemBankType bank, EMemDataType dType, uint64 VA)
    : mName(EPhysicalRegionType_to_string(physRegionType)), mLower(lower), mUpper(upper), mRegionType(physRegionType), mMemoryBank(bank), mDataType(dType), mFromVA(VA)
  {
    char print_buffer[128];
    snprintf(print_buffer, 128, "[%d]0x%llx-0x%llx", uint32(mMemoryBank), mLower, mUpper);
    mName += print_buffer;
  }

  const string PhysicalRegion::ToString() const
  {
    stringstream out_str;

    out_str << "[PhysicalRegion] " << mName << " for " << EPhysicalRegionType_to_string(mRegionType) << " [" << EMemBankType_to_string(mMemoryBank) << "]0x" << hex << mLower << "-0x" << mUpper
            << " " << EMemDataType_to_string(mDataType);

    return out_str.str();
  }

  const string PaTuple::ToString() const
  {
    stringstream out_str;

    out_str << "[" << dec << mBank << "]0x" << hex << mAddress;

    return out_str.str();
  }

}
