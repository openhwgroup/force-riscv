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
#include <GenPC.h>
#include <VmUtils.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <Generator.h>
#include <Constraint.h>
#include <Log.h>

#include <sstream>

/*!
  \file GenPC.cc
  \brief Code managing Generator PC vicinity properties.
*/

using namespace std;

namespace Force {

  GenPC::GenPC()
    : Object(), mpPcTransRange(nullptr), mpCrossOverRange(nullptr), mValue(0), mLastValue(0), mAlignMask(0xfffffffffffffffcull), mPaStart1(0), mPaEnd1(0), mPaStart2(0), mPaEnd2(0), mInstructionSpace(0), mPaBank1(0), mPaBank2(0), mPaValid(false), mPaPart2Valid(false)
  {

  }

  GenPC::GenPC(uint64 alignmentMask)
    : Object(), mpPcTransRange(nullptr), mpCrossOverRange(nullptr), mValue(0), mLastValue(0), mAlignMask(alignmentMask), mPaStart1(0), mPaEnd1(0), mPaStart2(0), mPaEnd2(0), mInstructionSpace(0), mPaBank1(0), mPaBank2(0), mPaValid(false), mPaPart2Valid(false)
  {

  }

  GenPC::GenPC(const GenPC& rOther)
    : Object(rOther), mpPcTransRange(nullptr), mpCrossOverRange(nullptr), mValue(0), mLastValue(0), mAlignMask(0xfffffffffffffffcull), mPaStart1(0), mPaEnd1(0), mPaStart2(0), mPaEnd2(0), mInstructionSpace(0), mPaBank1(0), mPaBank2(0), mPaValid(false), mPaPart2Valid(false)
  {

  }

  GenPC::~GenPC()
  {
    delete mpPcTransRange;
    delete mpCrossOverRange;
  }

  Object* GenPC::Clone() const
  {
    return new GenPC(*this);
  }

  const string GenPC::ToString() const
  {
    stringstream out_str;

    out_str << "[GenPC] 0x" << hex << mValue << " space: " << dec << mInstructionSpace << hex;

    if (mPaValid) {
      out_str << " PA: [" << EMemBankType_to_string(EMemBankType(mPaBank1)) << "](0x" << mPaStart1 << "-0x" << mPaEnd1 << ")";
    }
    if (mPaPart2Valid) {
      out_str << ",[" << EMemBankType_to_string(EMemBankType(mPaBank2)) << "](0x" << mPaStart2 << "-0x" << mPaEnd2 << ")";
    }
    if (nullptr != mpPcTransRange) {
      out_str << " " << mpPcTransRange->ToString();
    }
    if (nullptr != mpCrossOverRange) {
      out_str << " " << mpCrossOverRange->ToString();
    }

    return out_str.str();
  }

  void GenPC::SetInstructionSpace(uint32 bytes)
  {
    if (bytes != mInstructionSpace) {
      mInstructionSpace = bytes;
    }
  }

  void GenPC::Update(uint32 iSpace)
  {
    delete mpPcTransRange;
    mpPcTransRange = nullptr;
    delete mpCrossOverRange;
    mpCrossOverRange = nullptr;
    mPaValid = false;
    mPaPart2Valid = false;
    mInstructionSpace = iSpace;
  }

  static TranslationRange* map_pc_part(Generator* pGen, uint64 addr, uint64 size)
  {
    auto vm_mapper = pGen->GetVmManager()->CurrentVmMapper();
    vm_mapper->MapAddressRange(addr, size, true);
    auto new_trans_range = new TranslationRange();
    if (not vm_mapper->GetTranslationRange(addr, *new_trans_range)) {
      LOG(fail) << "{map_pc_part} expecting translation range for 0x" << hex << addr << " to be valid." << endl;
      FAIL("cannot-get-valid-translation-range");
    }
    // << "{map_pc_part new TranslationRange: " << new_trans_range->ToString() << " for address 0x" << hex << addr << " size " << dec << size << endl;
    return new_trans_range;
  }

  void GenPC::MapPC(Generator *pGen)
  {
    // << "mapping address : 0x" << hex << mValue << " instruction space " << dec << mInstructionSpace << endl;
    if (nullptr == mpPcTransRange) {
      mpPcTransRange = map_pc_part(pGen, mValue, mInstructionSpace);
      if (nullptr != mpCrossOverRange) {
        LOG(fail) << "{GenPC::MapPC} dangling cross over translation range object." << endl;
        FAIL("dangling-translation-range-object");
      }
    }
    else if (not mpPcTransRange->Contains(mValue)) {
      delete mpPcTransRange;
      mpPcTransRange = nullptr;
      delete mpCrossOverRange;
      mpCrossOverRange = nullptr;
      mPaValid = false;
      mPaPart2Valid = false;
      mpPcTransRange = map_pc_part(pGen, mValue, mInstructionSpace);
    }

    mPaStart1 = mpPcTransRange->TranslateVaToPa(mValue, mPaBank1);
    uint64 range_space = mpPcTransRange->Upper() - mValue + 1;
    if (range_space >= mInstructionSpace) {
      // has plenty room
      mPaEnd1 = mPaStart1 + (mInstructionSpace - 1);
      mPaPart2Valid = false;
      mPaValid = true;
      return;
    }

    // cross over to the next page
    mPaEnd1 = mpPcTransRange->PhysicalUpper();
    uint64 part2_va = mpPcTransRange->Upper() + 1;
    uint32 part2_size = mInstructionSpace - range_space;
    if (nullptr == mpCrossOverRange) {
      mpCrossOverRange = map_pc_part(pGen, part2_va, part2_size);
    }
    else if (not mpCrossOverRange->Contains(part2_va)) {
      delete mpCrossOverRange;
      mpCrossOverRange = nullptr;
      mPaPart2Valid = false;
      mpCrossOverRange = map_pc_part(pGen, part2_va, part2_size);
    }

    mPaStart2 = mpCrossOverRange->TranslateVaToPa(part2_va, mPaBank2);
    mPaEnd2 = mPaStart2 + (part2_size - 1);
    mPaPart2Valid = true;
    mPaValid = true;
  }

  uint64 GenPC::GetPA(Generator* pGen, uint32& bank, bool& fault)
  {
    if (not mPaValid) {
      MapPC(pGen);
    }
    if (mpPcTransRange->TranslationResultType() == ETranslationResultType::AddressError)
      fault = true;
    else
      fault = false;

    bank = mPaBank1;
    return mPaStart1;
  }

  void GenPC::GetPA(Generator* pGen, PaTuple& rPaTuple)
  {
    if (not mPaValid) {
      MapPC(pGen);
    }

    rPaTuple.Assign(mPaStart1, mPaBank1);
  }

}
