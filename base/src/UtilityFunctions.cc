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
#include <UtilityFunctions.h>
#include <StringUtils.h>
#include <Log.h>

#include <algorithm>
#include <fmt.h>
#include <map>
#include <sstream>

using namespace std;

namespace Force {

  void check_enum_size(unsigned enum_size)
  {
    if (enum_size > MAX_ENUM_SIZE) {
      LOG(fail) << "Dealing with a enum class size exceeding limit : " << enum_size << endl;
      FAIL("enum-class-too-large");
    }
  }

  /*!
     The caller should ensure the data_array pointer points to a byte stream array of at least num_bytes size.
     num_bytes is limited to 1-8 bytes.
  */
  void value_to_data_array_big_endian(uint64 value, uint32 num_bytes, uint8* data_array)
  {
    switch (num_bytes) {
    case 8:
      data_array[7] = (uint8)(value & 0xff);
      value >>= 8;
      // fall through
    case 7:
      data_array[6] = (uint8)(value & 0xff);
      value >>= 8;
    case 6:
      // fall through
      data_array[5] = (uint8)(value & 0xff);
      value >>= 8;
    case 5:
      // fall through
      data_array[4] = (uint8)(value & 0xff);
      value >>= 8;
      // fall through
    case 4:
      data_array[3] = (uint8)(value & 0xff);
      value >>= 8;
      // fall through
    case 3:
      data_array[2] = (uint8)(value & 0xff);
      value >>= 8;
      // fall through
    case 2:
      data_array[1] = (uint8)(value & 0xff);
      value >>= 8;
      // fall through
    case 1:
      data_array[0] = (uint8)(value & 0xff);
      break;
    default:
      LOG(fail) << "Unexpected data size: " << dec << num_bytes << endl;
      FAIL("unexpected-data-size");
    }
  }

  /*!
    The caller should ensure the data_array pointer points to a byte stream array of at least num_bytes size.
    0 < num_bytes <= 8.
  */
  uint64 data_array_to_value_big_endian(cuint8* data_array, cuint32 num_bytes)
  {
    uint64 value = 0;
    if ((num_bytes == 0) or (num_bytes > sizeof(value))) {
      LOG(fail) << "Unexpected data size: " << dec << num_bytes << endl;
      FAIL("unexpected-data-size");
    }

    for (uint32 i = 0; i < num_bytes; i++) {
      uint64 shifted_byte = ((uint64)data_array[i]) << ((sizeof(value) - i - 1) * 8);
      value |= shifted_byte;
    }

    return value;
  }

  /*!
    The caller should ensure the data_array pointer points to a byte stream array of at least num_bytes size.
    0 < num_bytes <= 8.
   */
  uint64 data_array_to_value_little_endian(cuint8* data_array, cuint32 num_bytes)
  {
    uint64 value = 0;
    if ((num_bytes == 0) or (num_bytes > sizeof(value))) {
      LOG(fail) << "Unexpected data size: " << dec << num_bytes << endl;
      FAIL("unexpected-data-size");
    }

    for (uint32 i = 0; i < num_bytes; i++) {
      uint64 shifted_byte = ((uint64)data_array[i]) << (i * 8);
      value |= shifted_byte;
    }

    return value;
  }

  /*!
    The caller should ensure the data_array pointer points to a byte stream array of at least num_bytes size.
    num_bytes is limited to 1, 2, 4, or 8 bytes.
  */
  void element_value_to_data_array_big_endian(uint64 value, uint32 num_bytes, uint8* data_array)
  {
    switch (num_bytes) {
    case 8:
      data_array[7] = (uint8)(value & 0xff);
      value >>= 8;
      data_array[6] = (uint8)(value & 0xff);
      value >>= 8;
      data_array[5] = (uint8)(value & 0xff);
      value >>= 8;
      data_array[4] = (uint8)(value & 0xff);
      value >>= 8;
      // fall through
    case 4:
      data_array[3] = (uint8)(value & 0xff);
      value >>= 8;
      data_array[2] = (uint8)(value & 0xff);
      value >>= 8;
      // fall through
    case 2:
      data_array[1] = (uint8)(value & 0xff);
      value >>= 8;
      // fall through
    case 1:
      data_array[0] = (uint8)(value & 0xff);
      break;
    default:
      LOG(fail) << "Unexpected element size: " << dec << num_bytes << endl;
      FAIL("unexpected-element-size");
    }
  }

  /*!
    The caller should ensure the data_array pointer points to a byte stream array of at least num_bytes size.
    num_bytes is limited to 1, 2, 4 or 8 bytes.
   */
  void element_value_to_data_array_little_endian(uint64 value, uint32 num_bytes, uint8* data_array)
  {
    uint8* data_ptr_current = data_array;
    switch (num_bytes) {
    case 8:
      data_ptr_current[0] = (uint8)(value & 0xff);
      value >>= 8;
      data_ptr_current[1] = (uint8)(value & 0xff);
      value >>= 8;
      data_ptr_current[2] = (uint8)(value & 0xff);
      value >>= 8;
      data_ptr_current[3] = (uint8)(value & 0xff);
      value >>= 8;
      data_ptr_current += 4;
      // fall through
    case 4:
      data_ptr_current[0] = (uint8)(value & 0xff);
      value >>= 8;
      data_ptr_current[1] = (uint8)(value & 0xff);
      value >>= 8;
      data_ptr_current += 2;
      // fall through
    case 2:
      (data_ptr_current ++)[0] = (uint8)(value & 0xff);
      value >>= 8;
      // fall through
    case 1:
      data_ptr_current[0] = (uint8)(value & 0xff);
      break;
    default:
      LOG(fail) << "Unexpected element size: " << dec << num_bytes << endl;
      FAIL("unexpected-element-size");
    }
  }

  /*!
    The caller should ensure the data_array pointer points to a byte stream array of at least num_bytes size.
    num_bytes is limited to 1, 2, 4, or 8 bytes.
  */
  uint64 data_array_to_element_value_big_endian(cuint8* data_array, cuint32 num_bytes)
  {
    if (not is_power_of_2(num_bytes)) {
      LOG(fail) << "Unexpected element size: " << dec << num_bytes << endl;
      FAIL("unexpected-element-size");
    }

    uint64 value = data_array_to_value_big_endian(data_array, num_bytes);

    // data_array_to_value_big_endian() assumes a 64-bit "element", so we need to shift the bytes to
    // the position appropriate for the element size.
    value >>= (sizeof(value) - num_bytes) * 8;

    return value;
  }

  /*!
    The caller should ensure the data_array pointer points to a byte stream array of at least num_bytes size.
    num_bytes is limited to 1, 2, 4 or 8 bytes.
   */
  uint64 data_array_to_element_value_little_endian(cuint8* data_array, cuint32 num_bytes)
  {
    if (not is_power_of_2(num_bytes)) {
      LOG(fail) << "Unexpected element size: " << dec << num_bytes << endl;
      FAIL("unexpected-element-size");
    }

    return data_array_to_value_little_endian(data_array, num_bytes);
  }

  void verify_alignment(uint64 align)
  {
    if (not is_power_of_2(align)) {
      LOG(fail) << "{verify_alignment} invalid alignment value: 0x" << hex << align << endl;
      FAIL("invalid-alignment");
    }
  }

  /*!
    First verify this is a valid alignment size, that is, it needs to be power of 2.
    Then check half of remaining bits to find out where the set bit is.
  */
  uint32 get_align_shift(uint64 align)
  {
    verify_alignment(align);

    uint32 align_shift = 0;

    if ((align & 0xffffffff) == 0) {
      align_shift += 32;
      align >>= 32;
    }

    if ((align & 0xffff) == 0) {
      align_shift += 16;
      align >>= 16;
    }

    if ((align & 0xff) == 0) {
      align_shift += 8;
      align >>= 8;
    }

    if ((align & 0xf) == 0) {
      align_shift += 4;
      align >>= 4;
    }

    if ((align & 0x3) == 0) {
      align_shift += 2;
      align >>=2;
    }

    if ((align & 1) == 0) {
      align_shift += 1;
    }

    return align_shift;
  }

  uint64 get_align_mask(cuint64 align)
  {
    verify_alignment(align);

    return (~(align - 1));
  }

  uint64 get_alignment(cuint64 alignMask)
  {
    uint64 align = ~alignMask + 1;

    verify_alignment(align);

    return align;
  }

  uint64 get_aligned_value(cuint64 value, cuint64 align)
  {
    return (value & get_align_mask(align));
  }

  uint32 sign_extend32(uint32 value, uint32 size)
  {
    if (size == 0) {
      LOG(fail) << "{sign_extend32} size is 0" << endl;
      FAIL("size-0-sign-extention");
    }
    if (size == 32)
      return value;

    uint32 sign_bit = 1u << (size - 1);
    if (value & sign_bit) {
      uint32 sign_bits = ~((1u << size) - 1);
      value |= sign_bits;
    }
    // << "{sign_extend32} value=0x" << hex << orig_value << " size=" << dec << size << " sign extended to 0x" << hex << value << endl;
    return value;
  }

  uint64 sign_extend64(uint64 value, uint32 size)
  {
    //uint64 orig_value = value;
    if (size == 0) {
      LOG(fail) << "{sign_extend64} size is 0" << endl;
      FAIL("size-0-sign-extention");
    }
    if (size == 64)
      return value;

    uint64 sign_bit = 1ull << (size - 1);
    if (value & sign_bit) {
      uint64 sign_bits = ~((1ull << size) - 1);
      value |= sign_bits;
    }
    // << "{sign_extend64} value=0x" << hex << orig_value << " size=" << dec << size << " sign extended to 0x" << hex << value << endl;
    return value;
  }

  /*!
    TODO temporary will improve for efficiency later.
  */
  EExtendType get_extend_type_amount(const string& ea_name, uint32& amount)
  {
    EExtendType extend_type = EExtendType(0);
    amount = 0;

    string::size_type underscore_pos = ea_name.find("_amountX");
    if (underscore_pos != string::npos) {
      string ext_name = ea_name.substr(0, underscore_pos);
      extend_type = string_to_EExtendType(ext_name);

      char size_char = ea_name[ea_name.size() - 1];
      amount = uint32(size_char - '0');
    }
    else {
      LOG(fail) << "Failed to process extended amount type: " << ea_name << endl;
      FAIL("unknown-extended-amount-type");
    }

    return extend_type;
  }

  uint32 get_shift_amount(const string& shift_name)
  {
    uint32 amount = 0;

    string::size_type sh_pos = shift_name.find("sh");
    if (sh_pos == 0) {
      string size_text = shift_name.substr(2);

      try {
        amount = stoul(size_text);
      }
      catch (invalid_argument&) {
        // Handled below by checking if amount is 0.
      }
      catch (out_of_range&) {
        // Handled below by checking if amount is 0.
      }
    }

    if (amount == 0) {
      LOG(fail) << "Failed to process shift amount: " << shift_name << endl;
      FAIL("unknown-shift-amount");
    }

    return amount;
  }

  uint64 extend_regval(uint64 value, EExtendType extend_type, uint32 shift)
  {
    if (shift > 4) {
      LOG(fail) << "invalid shift number " << shift << endl;
      FAIL("invalid shift number");
    }
    uint64 ret_val = 0;
    switch (extend_type) {
    case EExtendType::LSL:
    case  EExtendType::UXTW:
    case  EExtendType::UXTX:
      ret_val = value << shift;
      break;
    case EExtendType::SXTW:
      ret_val = sign_extend64(value << shift, 32 + shift);
      break;
    case  EExtendType::SXTX:
      ret_val = sign_extend64(value << shift, 64);
      break;
    default:
      LOG(fail) << "unhandled extend type" << EExtendType_to_string(extend_type) << endl;
      FAIL("unhandled-extend-type");
    }
    return ret_val;
  }

  /*!
    The offset parameter contains a full 1's compliment signed value. This function extracts a smaller not-sign-extended
    offset value from the lower offset_size bits, as well as checking to make sure the original can be properly
    represented as such an offset, i.e. it is not out of the offset range.
  */
  uint64 get_offset_field(uint64 offset, uint32 offset_size, bool *offset_valid)
  {
    uint64 masked_offset = 0;
    if (offset_valid != nullptr) {
      *offset_valid = false;
    }

    // The offset meets the size constraint if and only if the bits in the range [63:(offset_size - 1)] are sign bits,
    // i.e. uniformly 0 or 1. Perform an arithmethic shift to test this.
    uint64 shifted_offset = static_cast<uint64>(static_cast<int64>(offset) >> (offset_size - 1));
    if ((shifted_offset == 0) or (shifted_offset == MAX_UINT64)) {
      uint64 offset_mask = (1ull << offset_size) - 1;
      masked_offset = offset & offset_mask;

      if (offset_valid != nullptr) {
        *offset_valid = true;
      }
    }
    else if (offset_valid == nullptr) {
      LOG(fail) << "{get_offset_field} offset 0x" << hex << offset << " cannot be properly represented by a " << dec << offset_size << " bits offset value." << endl;
      FAIL("offset-out-of-range");
    }

    return masked_offset;
  }

  uint64 get_mask64(uint32 size, uint32 shift)
  {
    if (size == 0) return 0ull;
    if (size > 64) size = 64;
    uint64 mask = ~0x0ull >> (64 - size);
    if (shift > 0) mask <<= shift;
    return mask;
  }

  void unimplemented_method(const std::string& method_name)
  {
    LOG(fail) << "{unimplemented_method} method: " << method_name << " not implemented." << endl;
    FAIL("unimplemented-method");
  }

  const string get_gen_mode_string(EGenModeTypeBaseType gen_mode)
  {
    string ret_str;
    EGenModeTypeBaseType test_bit = 1;
    for (EGenModeTypeBaseType i = 0; i < EGenModeTypeSize; ++ i) {
      if (gen_mode & test_bit) {
        EGenModeType bit_enum = EGenModeType(test_bit);
        if (ret_str.size() > 0) {
          ret_str += "-";
        }
        ret_str += EGenModeType_to_string(bit_enum);
      }
      test_bit <<= 1;
    }
    return ret_str;
  }

  /*!
    The following are the 8 valid generator states.

             IssSim NoIss SimOff NoEscape NoIssNoEscape SimOffNoEscape Linear NoIssLinear
    NoIss      0      1     0       0           1             0          0         1
    SimOff     0      1     1       0           1             1          1         1
    NoEscape   0      0     0       1           1             1          1         1
    NoJump     0      0     0       0           0             0          1         1

    value      0      3     2       4           7             6         14        15

    NOTE: The will result in the EGenModeType bitmap layout cannot be freely changed.

    The additional on/off bits that can be combined with the above states appear in the array below.
  */
  static EGenModeType additional_mode_bits[] = {
    EGenModeType::ReExe,
    EGenModeType::Exception,
    EGenModeType::NoSkip,
    EGenModeType::InLoop,
    EGenModeType::DelayInit,
    EGenModeType::LowPower,
    EGenModeType::Filler,
    EGenModeType::Speculative,
    EGenModeType::AddressShortage,
    EGenModeType::RecordingState,
    EGenModeType::RestoreStateLoop,
    EGenModeType(0)};

  const string get_gen_mode_name(EGenModeTypeBaseType gen_mode)
  {
    string ret_str;
    for (int i = 0; ; ++ i) {
      auto bit_mask = EGenModeTypeBaseType(additional_mode_bits[i]);
      if (0 == bit_mask) break;
      auto bit_val = gen_mode & bit_mask;
      gen_mode -= bit_val; // remove the bit from main mode
      if (bit_mask == bit_val) {
        ret_str += EGenModeType_to_string(EGenModeType(bit_mask));
        ret_str += ',';
      }
    }

    switch (gen_mode) {
    case 0:
      ret_str += "IssSim";
      break;
    case 2:
      ret_str += "SimOff";
      break;
    case 3:
      ret_str += "NoIss";
      break;
    case 4:
      ret_str += "NoEscape";
      break;
    case 6:
      ret_str += "SimOffNoEscape";
      break;
    case 7:
      ret_str += "NoIssNoEscape";
      break;
    case 14:
      ret_str += "Linear";
      break;
    case 15:
      ret_str += "NoIssLinear";
      break;
    default:
      LOG(fail) << "Unsupported GenMode configuration: 0x" << hex << uint32(gen_mode) << " => " << get_gen_mode_string(gen_mode) << endl;
      FAIL("unsupported-gen-mode");
    }

    // << "get_gen_mode_name for 0x" << hex << uint32(gen_mode) << " => " << get_gen_mode_string(gen_mode) << " returning " << ret_str << endl;
    return ret_str;
  }

  /*!
    returns low bit in lowest 16 bit and table levels in highest 16 bit.
  */
  uint32 get_root_level_low_bit(uint32 highBit, uint32 lowBit, uint32 tableStep)
  {
    /*uint32 table_low = 0;
    uint32 check_table_low = lowBit;
    uint32 table_levels = 0;

    uint32 levels_lowbit = 0;

    do
    {
      table_low        = check_table_low;
      check_table_low += tableStep;
      table_levels++;
    }
    while (check_table_low <= highBit);

    levels_lowbit = table_low;
    levels_lowbit |= table_levels << 16;
    return(levels_lowbit);*/
    return 0x10027ul; //TODO update to switch off other high bit values
  }

  uint64 round_up_power2(uint64 x)
  {
    if (is_power_of_2(x)) {
      LOG(fail) << "{round_up_power2} invalid round up value: 0x" << hex << x << endl;
      FAIL("invalid-round-up");
    }
    uint64 v = x;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;
    if (v <= x)  {
      LOG(fail) << "{round_up_power2} round up overflow value: 0x" << hex << x << endl;
      FAIL("round-up-overflow");
    }
    return(v);
  }

  uint64 lowest_bit_set(uint64 x)
  {
    return highest_bit_set(x & -x);
  }

  uint64 highest_bit_set(uint64 x)
  {
    if (x == 0) {
      LOG(fail) << "{highest_bit_set} invalid value:" << hex << x << endl;
      FAIL("invalid-value");
    }

    register unsigned int r; // result of log2(v) will go here
    register unsigned int shift;
    uint64 v = x;
    r = (v > 0xFFFFFFFF) << 5;
    v >>= r;
    shift = (v > 0xFFFF) << 4;
    v >>= shift;
    r |= shift;
    shift = (v > 0xFF) << 3;
    v >>= shift;
    r |= shift;
    shift = (v > 0xF) << 2;
    v >>= shift;
    r |= shift;
    shift = (v > 0x3) << 1;
    v >>= shift;
    r |= shift;

    r |= v >> 1;
    return(r);
  }

  std::string value_to_lsdata_str(uint64 value, uint32 element_size, uint32 size, const std::string& delim)
  {
    if ((size < element_size) or (size > 64)) {
      LOG(fail) << "{value_to_lsdata_str} invalid element size:" << hex << element_size << " size:" << size << endl;
      FAIL("invalid-value-out-of-range");
    }

    stringstream ss;
    uint32 element_num = size / element_size;
    uint64 element_mask = ((uint64)1 << element_size) - 1;
    uint64 element_width = element_size >> 2;

    ss << "0x" << fmtx0(value & element_mask, element_width);
    for (uint32 i = 1; i < element_num; i++) {
      uint64 element_value = (value >> (element_size * i)) & element_mask;
      ss  << delim << "0x" << fmtx0(element_value, element_width);
    }
    return ss.str();
  }

  std::string large_value_to_lsdata_str(const std::vector<uint64>& values, uint32 element_size, uint32 size, const std::string& delim)
  {
    if (values.empty()) {
      LOG(fail) << "{large_value_to_lsdata_str} invalid values: it is empty" << endl;
      FAIL("invalid-value-empty");
    }
    if (size > 64) { size = 64; }

    stringstream ss;
    ss << value_to_lsdata_str(values[0], element_size, size, delim);
    for (uint32 i = 1; i < values.size(); ++i)
    {
      ss << delim << value_to_lsdata_str(values[i], element_size, size, delim);
    }
    return ss.str();
  }

  void change_elementform_to_uint64(cuint32 element_size, cuint32 valid_size, const std::vector<uint64>& orignal_value_list, std::vector<uint64>& uint64_value_list)
  {
    //element_size == 64 and valid_size == 32 or 64
    if (64 == element_size) {
      uint64_value_list = orignal_value_list;
      return;
    }
    uint64 data = 0ull;
    uint32 total_bytes = 0;
    for (std::vector<uint64>::const_iterator it = orignal_value_list.begin(); it != orignal_value_list.end(); ++it) {
      data |=  (*it<< (total_bytes * 8));
      total_bytes +=  (valid_size>> 3);
      if (total_bytes >= 8) {
        uint64_value_list.push_back(data);
        total_bytes = 0;
        data = 0ull;
      }
    }
    if (data){
      uint64_value_list.push_back(data);
    }
  }

  void change_uint64_to_elementform(cuint32 element_size, cuint32 valid_size, const std::vector<uint64>& uint64_value_list, std::vector<uint64>& result_value_list)
  {
    if ((64 == element_size) and (64 == valid_size)) {
      result_value_list = uint64_value_list;
      return;
    }
    if (0 !=  64 % valid_size ){
      LOG(fail) <<"{change_uint64_to_elementform} not support!" <<endl;
      FAIL("not-support-size");
    }
    for (std::vector<uint64>::const_iterator it = uint64_value_list.begin(); it != uint64_value_list.end(); ++it) {
      if ((64 == element_size) and (32 == valid_size)){
        result_value_list.push_back(*it & ((1ull << valid_size) -1));
      } else {
        for (uint32 num = 0; num < 64 / valid_size; ++num) {
          uint64 data = (*it >> (num * valid_size)) & ((1ull << valid_size) - 1);
          result_value_list.push_back(data);
        }
      }
    }
  }

  uint64 change_uint64_to_elementform_at_index(cuint32 element_size, cuint32 valid_size, const std::vector<uint64>& uint64_value_list, cuint32 element_index)
  {
    uint32 uint64_val_index = (element_size * element_index) / sizeof_bits<uint64>();
    uint32 uint64_val_shift = ((element_size * element_index) % sizeof_bits<uint64>()) * element_size;
    uint64 uint64_val_mask = get_mask64(element_size, uint64_val_shift);
    uint64 elem_val = (uint64_value_list[uint64_val_index] & uint64_val_mask) >> uint64_val_shift;

    return elem_val;
  }

  uint64 kmg_number(const std::string& strKMG)
  {
    std::map<char, uint64 > kmg_map = {
      {'K', 0x400},
      {'k', 0x400},
      {'M', 0x100000},
      {'m', 0x100000},
      {'G', 0x40000000},
      {'g', 0x40000000}
    };

    if (strKMG == "")
      return 0;

    auto kmg = strKMG.back();
    auto it = kmg_map.find(kmg);
    if (it == kmg_map.end()) {
      LOG(fail) << "Invalid KMG string:\"" << strKMG << "\"." << endl;
      FAIL("invalid-kmg");
    }
    auto num = strKMG.substr(0, strKMG.length() - 1);
    return (parse_uint64(num) * it->second);
  }
  //!< ValueStr is formated as INTn()
  uint64 ParseTypeValue(const string& valueStr)
  {
    size_t begin_pos = valueStr.find("(");
    size_t end_pos = valueStr.find(")");
    if (begin_pos == string::npos || end_pos == string::npos) {
      LOG(fail) << "invalid Operand Data \"" << valueStr << "\""<< endl;
      FAIL("invalid Operand Data");
    }
    return parse_uint64(valueStr.substr(begin_pos+1 , end_pos - begin_pos - 1));
  }

}
