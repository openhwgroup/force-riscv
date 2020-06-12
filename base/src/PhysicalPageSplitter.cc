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
#include <PhysicalPageSplitter.h>
#include <VmMapper.h>
#include <VmUtils.h>
#include <Log.h>

using namespace std;

/*!
  \file PhysicalPageSplitter.cc
  \brief Code supporting splitting a VA range into one or more PA ranges, according to page boundaries.
*/

namespace Force {

  PhysicalPageSplitter::PhysicalPageSplitter(const VmMapper* pVmMapper)
    : mpVmMapper(pVmMapper)
  {
  }

  PhysicalPageSplit PhysicalPageSplitter::GetPhysicalPageSplit(cuint64 va, cuint32 size) const
  {
    PhysicalPageSplit page_split;
    TranslationRange trans_range;

    GetTranslationRange(va, trans_range);
    page_split.mPa1 = trans_range.TranslateVaToPa(va, page_split.mBank1);

    uint64 space_in_page = trans_range.SpaceInPage(va);
    if (space_in_page >= size) {
      page_split.mSize1 = size;
      page_split.mPa2 = 0;
    }
    else {
      page_split.mSize1 = space_in_page;
      uint64 va2 = va + page_split.mSize1;
      TranslationRange trans_range2;
      GetTranslationRange(va2, trans_range2);
      page_split.mPa2 = trans_range2.TranslateVaToPa(va2, page_split.mBank2);
    }

    page_split.mSize2 = size - page_split.mSize1;

    LOG(info) << "{PhysicalPageSplitter::GetPhysicalPageSplit} page crossing, VA=0x" << hex << va << "=>part 1 PA ["<< page_split.mBank1 << "]0x" << page_split.mPa1 << " size=" << dec << page_split.mSize1 << ", part 2 PA [" << page_split.mBank2 << "]0x" << hex << page_split.mPa2 << " size=" << dec << page_split.mSize2 << endl;

    return page_split;
  }

  void PhysicalPageSplitter::GetTranslationRange(cuint64 va, TranslationRange& rTransRange) const
  {
    if (not mpVmMapper->GetTranslationRange(va, rTransRange)) {
      LOG(fail) << "{PhysicalPageSplitter::GetTranslationRange} expecting translation range for 0x" << hex << va << " to be valid." << endl;
      FAIL("cannot-get-valid-translation-range");
    }
  }

}
