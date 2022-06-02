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
#include "FieldEncoding.h"

#include <algorithm>

#include "Log.h"
#include "StringUtils.h"
/*!
  \file FieldEncoding.cc
  \brief Code for encoding related functionalities.
*/

using namespace std;

namespace Force {

  FieldEncoding::~FieldEncoding()
  {
    mpEncodingBits = nullptr;
  }

  void FieldEncoding::SetBits(const string& bits_str)
  {
    mSize = 0;
    StringSplitter ss(bits_str, ',');
    while (!ss.EndOfString()) {
      string sub_str = ss.NextSubString();
      unsigned range_low = 0, range_high = 0;
      parse_range32(sub_str, range_low, range_high);
      uint32 range_size = (range_high - range_low) + 1;
      mpEncodingBits->push_back(EncodingBits(range_low, range_size));
      mSize += range_size;
    }
  }

  uint32 FieldEncoding::Lsb() const
  {
    auto itr = min_element(mpEncodingBits->begin(), mpEncodingBits->end(),
      [](const EncodingBits& encBitsA, const EncodingBits& encBitsB) { return (encBitsA.mLsb < encBitsB.mLsb); });

    uint32 ret_lsb = 63;
    if (itr != mpEncodingBits->end()) {
      ret_lsb = itr->mLsb;
    }

    return ret_lsb;
  }

}
