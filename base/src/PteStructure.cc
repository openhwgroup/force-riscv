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
#include <PteStructure.h>
#include <Log.h>

using namespace std;

namespace Force {

  /*!
    This copy constructor is not expected to be called.
   */
  PteStructure::PteStructure(const PteStructure& rOther) : mClass(), mType(), mCategory(), mGranule(), mLevel(1), mStage(1), mAttributeStructures(), mSize(1)
  {
    LOG(fail) << "PteStructure type object is not to be copied." << endl;
    FAIL("object-no-copy");
  }

  PteStructure::~PteStructure()
  {
    for (auto attr_struct : mAttributeStructures) {
      delete attr_struct;
    }
  }

  const string PteStructure::FullId() const
  {
    string str_id = EPteType_to_string(mType);
    str_id += "#";
    str_id += EPteCategoryType_to_string(mCategory);
    str_id += "#";
    str_id += EPageGranuleType_to_string(mGranule);
    str_id += "#";
    char print_buffer[32];
    snprintf(print_buffer, 32, "%d", mStage);
    str_id += print_buffer;

    return str_id;
  }

  void PteStructure::AddAttribute(PteAttributeStructure* pAttrStruct)
  {
    mAttributeStructures.push_back(pAttrStruct);
    mSize += pAttrStruct->mSize;
  }

  void PteAttributeStructure::SetBits(const std::string& bitsStr)
  {
    FieldEncoding encoding_obj(&mEncodingBits);
    encoding_obj.SetBits(bitsStr);
    mSize = encoding_obj.Size();
    mMask = (1ull << mSize) - 1;
    // << EPteAttributeType_to_string(mType) << " bitsStr " << bitsStr << " size " << dec << mSize << " mask 0x" << hex << mMask << endl;
    mLsb = encoding_obj.Lsb();
  }

  uint64 PteAttributeStructure::Encoding(uint64 pteValue) const
  {
    return get_field_encoding<uint64>(mEncodingBits, pteValue);
  }

}
