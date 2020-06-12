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
#ifndef Force_UtilityFunctions_H
#define Force_UtilityFunctions_H

#include <Defines.h>
#include <Enums.h>
#include <string>
#include <vector>

namespace Force {

  void check_enum_size(unsigned enum_size); //!< Check if enum class size is too big
  void value_to_data_array_big_endian(uint64 value, uint32 num_bytes, uint8* data_array); //!< Pass on value to byte stream data array in big endian byte order.
  uint64 data_array_to_value_big_endian(cuint8* data_array, cuint32 num_bytes); //!< Convert byte stream data array in big endian byte order to value.
  uint64 data_array_to_value_little_endian(cuint8* data_array, cuint32 num_bytes); //!< Convert byte stream data array in little endian byte order to value.
  void element_value_to_data_array_big_endian(uint64 value, uint32 num_bytes, uint8* data_array); //!< Pass on element value to byte stream data array in big endian byte order.
  void element_value_to_data_array_little_endian(uint64 value, uint32 num_bytes, uint8* data_array); //!< Pass on element value to byte stream data array in little endian byte order.
  uint64 data_array_to_element_value_big_endian(cuint8* data_array, cuint32 num_bytes); //!< Convert byte stream data array in big endian byte order to element value.
  uint64 data_array_to_element_value_little_endian(cuint8* data_array, cuint32 num_bytes); //!< Convert byte stream data array in little endian byte order to element value.
  void verify_alignment(uint64 align); //!< Verify that the align variable contains a valid alignment value.
  uint32 get_align_shift(uint64 align); //!< Return align shift for a given align size.
  uint64 get_aligned_value(cuint64 value, cuint64 align); //!< Return value aligned to the specified alignment.
  uint64 sign_extend64(uint64 value, uint32 size); //!< Sign extend a value with the spedificed size.
  uint32 sign_extend32(uint32 value, uint32 size); //!<  Sign extend a value with the spedificed size.
  EExtendType get_extend_type_amount(const std::string& ea_name, uint32& amount); //!< Decode extend type and amount.
  uint32 get_shift_amount(const std::string& shift_name); //!< Decode shift amount.
  uint64 extend_regval(uint64 value, EExtendType extend_type, uint32 shift); //!< Shift left a value and do extension
  uint64 get_offset_field(uint64 offset, uint32 offset_size, bool* offset_valid=nullptr); //!< Get offset field from a fully sign-extended offset value, given the width of the offset.
  const std::string get_gen_mode_string(EGenModeTypeBaseType gen_mode); //!< Get Generator mode string, by concatenating the name of all the mode bits that are set.
  const std::string get_gen_mode_name(EGenModeTypeBaseType gen_mode); //!< Return the name for a given Generator mode.
  uint64 get_mask64(uint32 size, uint32 shift = 0);  //!< Return a mask of given size, ensure size is smaller or equals to 64.
  uint32 get_root_level_low_bit(uint32 highBit, uint32 lowBit, uint32 tableStep);
  uint64 lowest_bit_set(uint64 x); //!< return lowest bit whose value is 1, the input value should be non-zero.
  uint64 highest_bit_set(uint64 x); //!< return highest bit whose value is 1, the input value should be non-zero.
  uint64 round_up_power2(uint64 x); //!< round value up to power of 2, the value is not aligned to power of 2.

  void unimplemented_method(const std::string& method_name); //!< Report error on unimplemented method.
  #define REPORT_UNIMPLEMENTED_METHOD unimplemented_method(__PRETTY_FUNCTION__) //!< Macro used to report unimplemented method.

  inline bool is_power_of_2(uint64 x) //!< Check if a value is power of 2.
  {
    if (x & (x - 1)) return false;
    return true;
  }

  inline uint32 get_mask64_size(uint64 mask) //!< Return length in bits of a contiguous right aligned mask.
  {
    return highest_bit_set(mask) + 1;
  }

  std::string value_to_lsdata_str(uint64 value, uint32 esize, uint32 size, const std::string& delim=";"); //!< Convert values to the LSData format string.
  std::string large_value_to_lsdata_str(const std::vector<uint64>& values, uint32 esize, uint32 size=64, const std::string& delim=";"); //!< Convert values to the LSData format string.
  void change_elementform_to_uint64(uint32 element_size, uint32 valid_size, std::vector<uint64> orignal_value_list, std::vector<uint64>& uint64_value_list);
  void change_uint64_to_elementform(uint32 element_size, uint32 valid_size, std::vector<uint64> uint64_value_list, std::vector<uint64>& result_value_list);
  uint64 kmg_number(const std::string& strKMG); //!< convert KMG string to number
  uint64 ParseTypeValue(const std::string& valueStr);

}

#endif
