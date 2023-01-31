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
#ifndef Force_FieldEncoding_H
#define Force_FieldEncoding_H

#include <string>
#include <vector>

#include "Defines.h"

namespace Force {

   /*!
    \struct EncodingBits
    \brief Describe a contiguous bits block inside an operand/PTE's encoding bits.
   */
  struct EncodingBits {
    uint32 mLsb; //!< Lest significant bit of the encoding bits block
    uint32 mSize; //!< Size of the encoding bits block

    EncodingBits(uint32 lsb, uint32 size) : mLsb(lsb), mSize(size) { } //!< Constructor.
  };

  /*!
    \class FieldEncoding
    \brief A class wrapper the encoding related functionalities of a encoded value.
  */
  class FieldEncoding {
  public:
    explicit FieldEncoding(std::vector<EncodingBits>* pEncVec) : mpEncodingBits(pEncVec), mSize(0) { } //!< Constructor with encoding bits vector reference given.
    ~FieldEncoding(); //!< Destrcuctor.

    void SetBits(const std::string& bits_str); //!< Parse encoding field bit string into EncodingBits objects.
    uint32 Size() const { return mSize; } //!< Return size of the encoding field.
    uint32 Lsb() const; //!< Return LSB of the field.
  private:
    FieldEncoding() : mpEncodingBits(nullptr), mSize(0) { } //!< Default constructor, not to be used.
    FieldEncoding(const FieldEncoding& rOther) : mpEncodingBits(nullptr), mSize(0) { } //!< Copy Constructor, not to be used.
    ASSIGNMENT_OPERATOR_ABSENT(FieldEncoding);
  private:
    std::vector<EncodingBits> * mpEncodingBits; //!< Pointer to a vector container of EncodingBits objects.
    uint32 mSize; //!< Size of the whole encoding field.
  };

  /*!
    Function for encoding a value of type, for example, uint32 or uint64 from sub-fields.
   */
  template<typename EncT>
    inline EncT get_field_encoding(const std::vector<EncodingBits> &encVec, EncT fieldValue)
    {
      EncT ret_value = 0;
      for (std::vector <EncodingBits>::const_reverse_iterator eb_iter = encVec.rbegin(); eb_iter != encVec.rend(); ++ eb_iter) {
        const EncodingBits& eb_ref = (*eb_iter);
        EncT bit_mask = (1ull << eb_ref.mSize) - 1;
    ret_value |= (fieldValue & bit_mask) << eb_ref.mLsb;
        fieldValue >>= eb_ref.mSize;
      }
      return ret_value;
    }

}

#endif
