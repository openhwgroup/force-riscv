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

#include "Force_Memory.h"

#include <fmt.h>

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <ostream>

using namespace std;

/*!
  \file Memory.cc
  \brief Code for big endian memory model
*/

namespace Force {

#define  BASE_INIT_MASK  0x0101010101010101ull
#define  MEM_BYTES       8                         //!< memory bytes, fixed as 8 bytes so far
#define  ADDR_ALIGN(a)   ((a) & ~(MEM_BYTES - 1))
#define  ADDR_OFFSET(a)  ((a) & (MEM_BYTES - 1))

#define MASK(len, pos)   ((len == 64) ? (-1ull << pos) : (((1ull << len) - 1) << pos))

//Replace macros for logging and asserts
#define FAIL(message) assert(false && message)  
#define LOG(level) std::cerr << "Force_Memory: " << #level << " message: "  

  bool Memory::msRandomPattern = true;
  uint64 Memory::msValuePattern = 0;

//utility functions
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
    \class MemoryBytes
    \brief class for a memory chunk.
  */
  class MemoryBytes {
  public:
    explicit MemoryBytes(uint64 addr) : mValue(0ull), mInitialValue(0ull), mAttributes(0ull), mAddress(addr) { }  //!< Constructor.

    ~MemoryBytes() { }      //!< Destructor, empty.

    /*!
      Initialize a memory chunk.
     */
    void Initialize(uint32 offset, uint64 value, uint64 attrs, uint32 nBytes, EMemDataType type)
    {
      if (offset + nBytes > sizeof(mInitialValue)) {
        FAIL("out-of-boundary");
      }
      if (nBytes != sizeof(value) && value >= (1ull << (nBytes << 3))) {
        FAIL("value-out-of-byte-range ");
      }

      if (!DoesInitializationMatch(offset, attrs, nBytes)) {
        LOG(fail) << "Reinitialize memory range (base, offset, size) = (0x" << fmtx0(mAddress, 16)
                                                                            << ", " << offset
                                                                            << ", " << nBytes << ")" << endl ;
        FAIL("reinitilize-memory");
      }

      uint8 attr_byte = (uint8) IsInit;
      attr_byte |= (uint8) type;

      uint64 attribute = 0;
      for (auto i = 0u; i < nBytes; i++) {
        attribute |= (uint64)attr_byte << (i << 3);
      }

      // Only initialize the uninitialized bytes.
      uint64 init_mask = GetInitializedMask(offset, attrs, nBytes);
      MergeMaskedValue(offset, value, ~init_mask, nBytes, mInitialValue);
      MergeMaskedValue(offset, value, ~init_mask, nBytes, mValue);
      MergeMaskedValue(offset, attribute, ~init_mask, nBytes, mAttributes);
    }

    /*!
      Check a memory chunk is initialized or not
    */
    bool IsInitialized(uint32 offset, uint32 nBytes) const
    {
      uint64 mask = BASE_INIT_MASK;
      uint32 mem_bytes = sizeof(mAttributes);
      uint32 pos_in_bits = (mem_bytes - (offset + nBytes)) << 3;

      if (offset + nBytes > mem_bytes) {
        FAIL("out-of-boundary");
      }

      mask <<= pos_in_bits;
      mask <<= (offset << 3);
      mask >>= (offset << 3);

      return (mAttributes & mask) == mask;
    }

    /*!
      Write value to a memory chunk
    */
    void Write(uint32 offset, uint64 value, uint32 nBytes)
    {
      if (offset + nBytes > sizeof(mValue)) {
        FAIL("out-of-boundary");
      }
      if (nBytes != sizeof(value) && value >= (1ull << (nBytes << 3))) {
        FAIL("value-out-of-byte-range ");
      }
      if (!IsInitialized(offset, nBytes)) {
        LOG(fail) << "write un-initialized memory range (base, offset, size) = (0x" << fmtx0(mAddress, 16)
                                                                            << ", " << offset
                                                                            << ", " << nBytes << ")" << endl ;
        FAIL("write-un-initialized-memory");
      }
      MergeValue(offset, value, nBytes, mValue);
    }

    /*!
      Read value from a memory chunk
    */
    uint64 Read(uint32 offset, uint32 nBytes) const
    {
      uint32 mem_bytes = sizeof(mValue);
      uint32 len_in_bits = nBytes << 3;
      uint32 pos_in_bits = (mem_bytes - (offset + nBytes)) << 3;
      uint64 mask = MASK(len_in_bits, pos_in_bits);

      if (offset + nBytes > mem_bytes) {
        FAIL("out-of-boundary");
      }
      if (!IsInitialized(offset, nBytes)) {
        LOG(fail) << "read un-initialized memory range (base, offset, size) = (0x" << fmtx0(mAddress, 16)
                                                                            << ", " << offset
                                                                            << ", " << nBytes << ")" << endl ;
        FAIL("read-un-initialized-memory");
      }

      return ((mValue & mask) >> pos_in_bits);
    }

    /*!
      Read initial value from a memory chunk
    */
    uint64 ReadInitialValue(uint32 offset, uint32 nBytes) const
    {
      uint32 mem_bytes = sizeof(mInitialValue);
      uint32 len_in_bits = nBytes << 3;
      uint32 pos_in_bits = (mem_bytes - (offset + nBytes)) << 3;
      uint64 mask = MASK(len_in_bits, pos_in_bits);

      if (offset + nBytes > mem_bytes) {
        FAIL("out-of-boundary");
      }
      if (!IsInitialized(offset, nBytes)) {
        LOG(fail) << "read un-initialized memory range (base, offset, size) = (0x" << fmtx0(mAddress, 16)
                                                                            << ", " << offset
                                                                            << ", " << nBytes << ")" << endl ;
        FAIL("read-un-initialized-memory");
      }

      return ((mInitialValue & mask) >> pos_in_bits);
    }

    /*!
      Return memory attributes of the specified address offset.
    */
    uint8 GetMemoryAttributes(uint32 offset) const
    {
      uint32 shift = (MEM_BYTES - (offset + 1)) << 3;

      // << "{GetMemoryAttributes} offset=" << dec << offset << " shift: " << shift << endl;

      return static_cast<uint8>((mAttributes >> shift) & 0xff);
    }

    /*!
      Read initial value in big-endian. If a byte is not initialized, randomize it if randomPattern is true; otherwise, set it to the value in valuePattern.
    */
    uint64 ReadInitialWithPattern(cbool randomPattern, cuint64 valuePattern)
    {
      return ApplyPattern(mInitialValue, randomPattern, valuePattern);
    }

    /*!
      Read value in big-endian. If a byte is not initialized, randomize it if randomPattern is true; otherwise, set it to the value in valuePattern.
    */
    uint64 ReadWithPattern(cuint32 offset, cuint32 nBytes, cbool randomPattern, cuint64 valuePattern) const
    {
      if ((offset + nBytes) > MEM_BYTES) {
        FAIL("out-of-boundary");
      }

      uint64 value_with_pattern = ApplyPattern(mValue, randomPattern, valuePattern);

      uint32 len_in_bits = nBytes << 3;
      uint32 pos_in_bits = (MEM_BYTES - (offset + nBytes)) << 3;
      uint64 mask = MASK(len_in_bits, pos_in_bits);

      return ((value_with_pattern & mask) >> pos_in_bits);
    }

    /*!
      Get uniformed data type for memory bytes
      If some bytes are no-init and the others are Instrunction, uniformed attribute is Instruction
      If some bytes are no-init and the others are Data, uniformed attribute is Data
      If some bytes are no-init and the others are Both, uniformed attribute is Both
      If some bytes are Insturction, and some bytes are Data, uniformed attribute is Instruction
    */
    EMemDataType GetUniformedType(void) const
    {
      uint64 init_mask = BASE_INIT_MASK;
      uint64 inst_mask = init_mask << 1;
      uint64 data_mask = init_mask << 2;

      if (!(mAttributes & init_mask)) {
         FAIL("un-initialized-memory-bytes");
      }

      if ((mAttributes & inst_mask) && (mAttributes & data_mask)) {
        return EMemDataType::Both;
      }
      if (mAttributes & inst_mask) {
        return  EMemDataType::Instruction;
      }
      if (mAttributes & data_mask) {
        return  EMemDataType::Data;
      }

       return EMemDataType::Init;
    }

    /*!
      Dump data from memory chunk
    */
    void Dump(ostream& out_str) const
    {
      Dump(out_str, mValue);
      out_str << "  ";
      Dump(out_str, mAttributes);
      out_str << endl;
    }

  private:
    /*!
      Check if specified attributes match current initialization.
    */
    bool DoesInitializationMatch(cuint32 offset, cuint64 attrs, cuint32 nBytes) const
    {
      if (offset + nBytes > MEM_BYTES) {
        FAIL("out-of-boundary");
      }

      uint64 init_attrs = attrs & BASE_INIT_MASK;
      uint64 stored_init_attrs = mAttributes & BASE_INIT_MASK;
      uint32 len_in_bits = nBytes << 3;
      uint32 pos_in_bits = (MEM_BYTES - (offset + nBytes)) << 3;
      uint64 mask = MASK(len_in_bits, pos_in_bits);

      return ((init_attrs << pos_in_bits) == (stored_init_attrs & mask));
    }

    /*!
      Dump data in big endian format
    */
    static void Dump(ostream& out_str, uint64 value)
    {
      uint64 data;
      for (auto i = sizeof(value) - 1; i > 0; i --) {
        data = (value >> (i << 3)) & 0xff;
        out_str << fmtx0(data, 2) << "_";
      }
      data = value & 0xff;  // least significant byte
      out_str << fmtx0(data, 2);
    }

    /*!
      \merge value into big endian data
      \value:  the value to merge
      \data:   the reference for the merged data
      \Cautious: the caller should assure value|nBytes validation.
    */
    static void MergeValue(uint32 offset, uint64 value, uint32 nBytes, uint64& data)
    {
      uint32 mem_bytes = sizeof(data);
      uint32 len_in_bits = nBytes << 3;
      uint32 pos_in_bits = (mem_bytes - (offset + nBytes)) << 3;

      uint64 mask = MASK(len_in_bits, pos_in_bits);
      data &= ~mask;
      data |= value << pos_in_bits;
    }

    /*!
      \merge value into big endian data
      \value: the value to merge
      \mergeMask: a mask indicating which bytes of value should be merged
      \data: the reference for the merged data
      \Cautious: the caller should assure value|nBytes validation.
    */
    static void MergeMaskedValue (uint32 offset, uint64 value, uint64 mergeMask, uint32 nBytes, uint64& data)
    {
      uint32 mem_bytes = sizeof(data);
      uint32 len_in_bits = nBytes << 3;
      uint32 pos_in_bits = (mem_bytes - (offset + nBytes)) << 3;

      uint64 mask = MASK(len_in_bits, pos_in_bits);
      data &= ~(mask & mergeMask);
      data |= (value << pos_in_bits) & mergeMask;
    }

    uint64 ApplyPattern(cuint64 value, cbool randomPattern, cuint64 valuePattern) const
    {
      uint64 init_mask = GetInitializedMask(0, mAttributes, MEM_BYTES);

      // If all bytes are initialized, there's nothing to fill in
      if (init_mask == MAX_UINT64) {
        return value;
      }

      return (valuePattern & ~init_mask) | (value & init_mask);
    }

    static uint64 GetInitializedMask(cuint32 offset, cuint64 attrs, cuint32 nBytes)
    {
      uint32 pos_in_bits = (MEM_BYTES - (offset + nBytes)) << 3;
      uint64 init_mask = BASE_INIT_MASK & (attrs << pos_in_bits); // mask to 0x00000101_01010000 as an example

      init_mask |= init_mask << 1;
      init_mask |= init_mask << 2;
      init_mask |= init_mask << 4;  // mask to 0x0000ffff_ffff0000 as an example

      return init_mask;
    }

    MemoryBytes() : mValue(0), mInitialValue(0), mAttributes(0), mAddress(0) { } //!< Default constructor.
  private:
    enum {
    IsInit  =  (1 << 0),
    IsInstr =  (1 << 1),
    IsData  =  (1 << 2)
    };

    uint64 mValue;         //!< Current memory data value, big endian format.
    uint64 mInitialValue;  //!< Initial memory data value, big endian format.
    uint64 mAttributes;    //!< Memory bytes attributes, big endian format.
    uint64 mAddress;       //!< Memory bytes starting address.
  };

  bool Section::Intersects(const Section& rOther) const
  {
    if ((GetEndAddress() >= rOther.mAddress) and (rOther.GetEndAddress() >= mAddress)) {
      return true;
    }

    return false;
  }

  bool Section::Contains(const Section& rOther) const
  {
    if ((mAddress <= rOther.mAddress) and (GetEndAddress() >= rOther.GetEndAddress())) {
      return true;
    }

    return false;
  }

  bool Section::operator<(const Section& rOther) const
  {
    if (mAddress < rOther.mAddress) {
      return true;
    }

    return false;
  }

  uint64 Section::GetEndAddress() const
  {
    return (mAddress + mSize - 1);
  }

  Memory::~Memory()
  {
    for (auto &map_item : mContent)
      delete map_item.second;
  }

  /*!
    \class MetaAccess
    \brief class for aligned memory access.
  */
  struct MetaAccess {
  public:
    MetaAccess(uint64 addr, uint32 offset, uint32 nbytes, uint64 data)
      : MetaAccess(addr, offset, nbytes, data, 0)
    {
    }

    MetaAccess(uint64 addr, uint32 offset, uint32 nbytes, uint64 data, uint64 attrs)
      : mAddress(addr), mOffset(offset), mSize(nbytes), mData(data), mAttrs(attrs)
    {
    }

  private:
    MetaAccess() : mAddress(0), mOffset(0), mSize(0), mData(0), mAttrs(0) { } //!< Default constructor.
  public:
      uint64 mAddress;  //!< address aligned by dword
      uint32 mOffset;
      uint32 mSize;
      uint64 mData;     //!< data in big endian
      uint64 mAttrs;    //!< attributes corresponding to data
  };

  //!< switch a byte ordering buffer to big endian dword number, buffer[0] respondes to the lowest address
  static uint64 ToBigEndian(cuint8* buffer, uint32 nSize)
  {
    uint64 data = 0;

    if (nSize == 0 || nSize > sizeof(data)) {
      FAIL("unsupported-number-of-bytes");
    }

    for (auto i = 0u; i < nSize; i ++)
      data |= ((uint64) buffer[i]) << ((nSize - 1 - i) * 8);

    return data;
  }

  void Memory::AutoInitialize(uint64 address, uint32 nBytes)
  {
    // Need to pass in the memory attributes to the Initialize() call to indicate which bytes
    // are already initialized
    vector<uint8> attrs(nBytes, 0);
    GetMemoryAttributes(address, nBytes, attrs.data());

    vector<uint8> data(nBytes, 0);
    Initialize(address, data.data(), attrs.data(), nBytes, EMemDataType::Both);
  }

  //!< the parameter value is in big endian
  void Memory::Initialize(uint64 address, uint64 value, uint32 nBytes, EMemDataType type)
  {
    uint32 mem_bytes = sizeof(value);

    if (nBytes == 0 || nBytes > mem_bytes) {
      FAIL("unsupported-number-of-bytes");
    }
    if (nBytes != mem_bytes && value >= (1ull << (nBytes << 3))) {
      FAIL("value-out-of-byte-range ");
    }
    if (IsInitialized(address, nBytes)) {
      LOG(fail) << "reinitialize memory range (address, size) = (0x"
                << fmtx0(address) << ", " << nBytes << ")" << endl;
      FAIL("reinitilize-memory");
    }

    uint32 nbytes = (ADDR_OFFSET(address) + nBytes <= mem_bytes) ?
                    nBytes : mem_bytes - ADDR_OFFSET(address);
    MetaAccess ma_low(ADDR_ALIGN(address), ADDR_OFFSET(address), nbytes,
                                           value >> ((nBytes - nbytes) << 3));
    InitializeMemoryBytes(ma_low, type);
    if (ma_low.mAddress == ADDR_ALIGN(address + nBytes - 1)) {  // the whole line resides in low address access
      return;
    }

    MetaAccess ma_high(ADDR_ALIGN(address + nBytes - 1), 0, nBytes - ma_low.mSize,
                                  value - (ma_low.mData << ((nBytes - ma_low.mSize) << 3)));
    InitializeMemoryBytes(ma_high, type);
  }

  //!< the data stream is byte ordering, data[0] responds to the lowerest address.
  void Memory::Initialize(uint64 address, cuint8* data, cuint8* attrs, uint32 nBytes, EMemDataType type)
  {
    if (nBytes == 0) {
      FAIL("unsupported-number-of-bytes");
    }

    // handle crossing part
    uint32 nSize = ADDR_ALIGN(address + MEM_BYTES - 1) - address;
    if (nSize) {
      uint64 data_value = ToBigEndian(data, (nSize > nBytes) ? nBytes : nSize);
      uint64 attr_value = ToBigEndian(attrs, (nSize > nBytes) ? nBytes : nSize);
      MetaAccess ma_first(ADDR_ALIGN(address), ADDR_OFFSET(address), (nSize > nBytes) ? nBytes : nSize, data_value, attr_value);
      InitializeMemoryBytes(ma_first, type);
    }
    if (nSize >= nBytes) {
      return;
    }

    // handle aligned parts
    uint32 dword;
    address += nSize;
    data += nSize;
    attrs += nSize;
    nBytes -= nSize;
    for (dword = 0; dword < nBytes/8; dword ++) {
      uint64 data_value = ToBigEndian(data + (dword << 3), 8);
      uint64 attr_value = ToBigEndian(attrs + (dword << 3), 8);
      MetaAccess ma_mid(address + (dword << 3), 0, 8, data_value, attr_value);
      InitializeMemoryBytes(ma_mid, type);
    }

    // handle remaining part
    nBytes = nBytes % 8;
    if (nBytes) {
      uint64 data_value = ToBigEndian(data + (dword << 3), nBytes);
      uint64 attr_value = ToBigEndian(attrs + (dword << 3), nBytes);
      MetaAccess ma_last(address + (dword << 3), 0, nBytes, data_value, attr_value);
      InitializeMemoryBytes(ma_last, type);
    }
  }

  bool Memory::IsInitialized(uint64 address, uint32 nBytes) const
  {
    if (nBytes == 0) {
      FAIL("unsupported-number-of-bytes");
    }

    // handle crossing part
    uint32 nSize = ADDR_ALIGN(address + MEM_BYTES - 1) - address;
    if (nSize) {
      MetaAccess ma(ADDR_ALIGN(address), ADDR_OFFSET(address),  (nSize > nBytes) ? nBytes : nSize, 0);
      if (!IsInitializedMemoryBytes(ma)) {
        return false;
      }
    }
    if (nSize >= nBytes) {
      return true;
    }

    //handle aligned parts
    uint32 dword;
    address += nSize;
    nBytes -= nSize;
    for (dword = 0; dword < nBytes/8; dword ++) {
      MetaAccess ma(address + (dword << 3), 0, 8, 0);
        if (!IsInitializedMemoryBytes(ma)) {
          return false;
        }
    }

    // handle remaining part
    nBytes = nBytes % 8;
    if (nBytes) {
       MetaAccess ma(address + (dword << 3), 0, nBytes, 0);
       if (!IsInitializedMemoryBytes(ma)) {
         return false;
       }
    }

    return true;
  }

  uint64 Memory::Read(uint64 address, uint32 nBytes)
  {
    uint32 mem_bytes = 8;

    if (nBytes == 0 || nBytes > mem_bytes) {
      FAIL("unsupported-number-of-bytes");
    }

    EnsureInitialization(address, nBytes);

    uint32 nbytes = (ADDR_OFFSET(address) + nBytes <= mem_bytes) ?
                    nBytes : mem_bytes - ADDR_OFFSET(address);
    MetaAccess ma_low(ADDR_ALIGN(address), ADDR_OFFSET(address), nbytes, 0);
    ReadMemoryBytes(ma_low);
    if (ma_low.mAddress == ADDR_ALIGN(address + nBytes - 1)) {  // the whole line resides in low address access
      return ma_low.mData;
    }

    MetaAccess ma_high(ADDR_ALIGN(address + nBytes - 1), 0, nBytes - ma_low.mSize, 0);
    ReadMemoryBytes(ma_high);
    return (ma_low.mData << (ma_high.mSize << 3)) | ma_high.mData;
  }

  void Memory::Write(uint64 address, uint64 value, uint32 nBytes)
  {
    uint32 mem_bytes = sizeof(value);
    if (nBytes == 0 || nBytes > sizeof(value)) {
      FAIL("unsupported-number-of-bytes");
    }

    if (nBytes != sizeof(value) && value >= (1ull << (nBytes << 3))) {
      FAIL("value-out-of-byte-range ");
    }

    EnsureInitialization(address, nBytes);

    Unreserve(address, nBytes);

    uint32 nbytes = (ADDR_OFFSET(address) + nBytes <= mem_bytes) ?
                    nBytes : mem_bytes - ADDR_OFFSET(address);
    MetaAccess ma_low(ADDR_ALIGN(address), ADDR_OFFSET(address), nbytes,
                      value >> ((nBytes - nbytes) << 3));
    WriteMemoryBytes(ma_low);
    if (ma_low.mAddress == ADDR_ALIGN(address + nBytes - 1)) {  // the whole line resides in low address access
      return;
    }

    MetaAccess ma_high(ADDR_ALIGN(address + nBytes - 1), 0, nBytes - ma_low.mSize,
                                  value - (ma_low.mData << ((nBytes - ma_low.mSize) << 3)));
    WriteMemoryBytes(ma_high);
  }

  void Memory::Write(uint64 address, cuint8* data, uint32 nBytes)
  {
    if (nBytes == 0) {
      FAIL("unsupported-number-of-bytes");
    }

    Unreserve(address, nBytes);

    // handle crossing part
    uint32 nSize = ADDR_ALIGN(address + MEM_BYTES - 1) - address;
    if (nSize) {
        auto value = ToBigEndian(data,  (nSize > nBytes) ? nBytes : nSize);
        MetaAccess ma_first(ADDR_ALIGN(address), ADDR_OFFSET(address), (nSize > nBytes) ? nBytes : nSize, value);
        WriteMemoryBytes(ma_first);
    }
    if (nSize >= nBytes) {
      return;
    }

    // handle aligned parts
    uint32 dword;
    address += nSize;
    data += nSize;
    nBytes -= nSize;
    for (dword = 0; dword < nBytes/8; dword ++) {
      auto value = ToBigEndian(data + (dword << 3), 8);
      MetaAccess ma_mid(address + (dword << 3), 0, 8, value);
      WriteMemoryBytes(ma_mid);
    }

    // handle remaining part
    nBytes = nBytes % 8;
    if (nBytes) {
      auto  value = ToBigEndian(data + (dword << 3), nBytes);
      MetaAccess ma_last(address + (dword << 3), 0, nBytes, value);
      WriteMemoryBytes(ma_last);
    }

  }

  void Memory::Reserve(uint64 address, uint32 nBytes)
  {
    Unreserve(address, nBytes);

    // Insert reserved Section in sorted position
    Section section(address, nBytes, EMemDataType::Both);
    auto itr = upper_bound(mReservedRanges.begin(), mReservedRanges.end(), section);
    mReservedRanges.insert(itr, section);
  }

  void Memory::Unreserve(uint64 address, uint32 nBytes)
  {
    // Remove all reserved Sections that intersect the input Section; the first Section that could
    // intersect is the one immediately preceding the input Seciton's upper bound
    Section section(address, nBytes, EMemDataType::Both);
    auto itr = upper_bound(mReservedRanges.begin(), mReservedRanges.end(), section);
    if (itr != mReservedRanges.begin()) {
      --itr;
    }

    mReservedRanges.erase(remove_if(itr, mReservedRanges.end(),
      [&section](const Section& rSection) { return rSection.Intersects(section); }),
      mReservedRanges.end());
  }

  bool Memory::IsReserved(uint64 address, uint32 nBytes)
  {
    // Return true if a reserved Section contains the input Section; the first Section that could
    // contain the input Section is the one immediately preceding the input Section's upper bound
    Section section(address, nBytes, EMemDataType::Both);
    auto itr = upper_bound(mReservedRanges.begin(), mReservedRanges.end(), section);
    if (itr != mReservedRanges.begin()) {
      --itr;
    }

    bool contained = any_of(itr, mReservedRanges.end(),
      [&section](const Section& rSection) { return rSection.Contains(section); });

    return contained;
  }

  uint64 Memory::ReadInitialValue(uint64 address, uint32 nBytes)
  {
    uint32 mem_bytes = 8;

    if (nBytes == 0 || nBytes > mem_bytes) {
      FAIL("unsupported-number-of-bytes");
    }

    EnsureInitialization(address, nBytes);

    uint32 nbytes = (ADDR_OFFSET(address) + nBytes <= mem_bytes) ?
                    nBytes : mem_bytes - ADDR_OFFSET(address);
    MetaAccess ma_low(ADDR_ALIGN(address), ADDR_OFFSET(address), nbytes, 0);
    ReadInitialValue(ma_low);
    if (ma_low.mAddress == ADDR_ALIGN(address + nBytes - 1)) {  // the whole line resides in low address access
      return ma_low.mData;
    }

    MetaAccess ma_high(ADDR_ALIGN(address + nBytes - 1), 0, nBytes - ma_low.mSize, 0);
    ReadInitialValue(ma_high);
    return (ma_low.mData << (ma_high.mSize << 3)) | ma_high.mData;

  }

  void Memory::DumpTitle(ostream& out_str) const
  {
    out_str << setw(18) << setfill(' ') << "Address" <<" : "
            << setw(23) << setfill(' ') << "Data in Big Endian"  << "  "
            << setw(24) << setfill(' ') << "Attribute(Init: 0x01; Inst: 0x02; Data: 0x04)"<< endl;
  }

  void Memory::Dump(ostream& out_str) const
  {
    DumpTitle(out_str);
    for (auto const& map_item : mContent) {
      out_str << "0x" << fmtx0(map_item.first, 16) << " : ";
      map_item.second->Dump(out_str);
    }
  }

  void Memory::Dump(std::ostream& out_str, uint64 address, uint64 nBytes) const
  {
    if (address != ADDR_ALIGN(address)) {
      LOG(fail) << "{Memory::Dump} Unaligned address: 0x" << hex << address << endl;
      FAIL("fail-dump-unalgined address");
    }
    if (nBytes % MEM_BYTES) {
      LOG(fail) << "{Memory::Dump} Unaligned memory bytes:0x" << hex << nBytes << endl;
      FAIL("fail-dump-unaligned memory bytes");
    }

    DumpTitle(out_str);

    for (uint64 addr = address; addr < address + nBytes; addr += MEM_BYTES) {
      auto it = mContent.find(addr);
      if (it != mContent.end()) {
        out_str << "0x" << fmtx0(it->first, 16) << " : ";
        it->second->Dump(out_str);
      }
    }

  }

  void Memory::InitializeMemoryBytes(const MetaAccess& rMetaAccess, EMemDataType type)
  {
    auto it = mContent.find(rMetaAccess.mAddress);
    if (it == mContent.end()) {
      MemoryBytes *mb = new MemoryBytes(rMetaAccess.mAddress);
      mb->Initialize(rMetaAccess.mOffset, rMetaAccess.mData, rMetaAccess.mAttrs, rMetaAccess.mSize, type);
      mContent[rMetaAccess.mAddress] = mb;
    }
    else {
      it->second->Initialize(rMetaAccess.mOffset, rMetaAccess.mData, rMetaAccess.mAttrs, rMetaAccess.mSize, type);
    }
  }

  bool Memory::IsInitializedMemoryBytes(const MetaAccess& rMetaAccess) const
  {
    auto it = mContent.find(ADDR_ALIGN(rMetaAccess.mAddress));
    if (it == mContent.end() || !it->second->IsInitialized(rMetaAccess.mOffset, rMetaAccess.mSize)) {
      return false;
    }

    return true;
  }

  void Memory::EnsureInitialization(uint64 address, uint32 nBytes)
  {
    if (!IsInitialized(address, nBytes)) {
      if (mAutoInit) {
        AutoInitialize(address, nBytes);
      }
      else {
        LOG(fail) << "read uninitialized memory range (address, size) = (0x"
                  << fmtx0(address) << ", " << nBytes << ")" << endl;
        FAIL("read-un-initialized-memory");
      }
    }
  }

  void Memory::ReadMemoryBytes(MetaAccess& rMetaAccess) const
  {
    auto it = mContent.find(rMetaAccess.mAddress);
    if (it == mContent.end()) {
      LOG(fail) << "Failed to read memory 0x" << hex << rMetaAccess.mAddress << "." << "No matched memory bytes object, please initialize first" << endl;
      FAIL("no-matched-memory-bytes");
    }
    rMetaAccess.mData = it->second->Read(rMetaAccess.mOffset, rMetaAccess.mSize);
  }

  void Memory::WriteMemoryBytes(const MetaAccess& rMetaAccess)
  {
    auto it = mContent.find(rMetaAccess.mAddress);
    if (it == mContent.end()) {
      LOG(fail) << "Failed to write memory 0x" << hex << rMetaAccess.mAddress << ". " << "No matched memory bytes object, please initialize first" << endl;
      FAIL("no-matched-memory-bytes");
    }
    it->second->Write(rMetaAccess.mOffset, rMetaAccess.mData, rMetaAccess.mSize);
  }

  void Memory::ReadInitialValue(MetaAccess& rMetaAccess) const
  {
    auto it = mContent.find(rMetaAccess.mAddress);
    if (it == mContent.end()) {
      LOG(fail) << "Failed to init memory 0x" << hex << rMetaAccess.mAddress << ". " << "No matched memory bytes object, please initialize first" << endl;
      FAIL("no-matched-memory-bytes");
    }
    rMetaAccess.mData = it->second->ReadInitialValue(rMetaAccess.mOffset, rMetaAccess.mSize);

  }

  void Memory::GetMemoryAttributes(cuint64 address, cuint32 nBytes, uint8* memAttrs) const
  {
    uint32 attrs_read = 0;
    uint64 current_base_address = ADDR_ALIGN(address);
    auto it = mContent.find(current_base_address);
    uint64 offset = ADDR_OFFSET(address);
    while (attrs_read < nBytes) {
      uint32 length = MEM_BYTES - offset;

      // If this is the last block to read, reduce the length accordingly
      if (length > (nBytes - attrs_read)) {
        length = nBytes - attrs_read;
      }

      if (it != mContent.end()) {
        for (uint32 i = 0; i < length; i++) {
          memAttrs[attrs_read] = it->second->GetMemoryAttributes(offset + i);
          attrs_read++;
        }
      }
      else {
        for (uint32 i = 0; i < length; i++) {
          memAttrs[attrs_read] = 0x0;
          attrs_read++;
        }
      }

      // The next entry in the map may not be contiguous, so we can't just increment the iterator here
      current_base_address += 8;
      it = mContent.find(current_base_address);

      offset = 0;
    }
  }

  uint8 Memory::GetByteMemoryAttributes(cuint64 address) const
  {
    uint64 aligned_addr = ADDR_ALIGN(address);
    auto it = mContent.find(aligned_addr);
    if (it == mContent.end()) {
      return 0;
    }
    return it->second->GetMemoryAttributes(ADDR_OFFSET(address));
  }

  //!< the data stream is byte ordering, data[0] responds to the lowerest address.
  void Memory::ReadInitialWithPattern(uint64 address, uint32 nBytes, uint8* data) const
  {
    if (address != ADDR_ALIGN(address)) {
      FAIL("un-aligned address");
    }
    if (nBytes % MEM_BYTES != 0) {
      FAIL("unsupported-number-of-bytes");
    }

    auto it = mContent.find(address);
    if (it == mContent.end()) {
      LOG(fail) << "Failed to read memory 0x" << hex << address << "." << "No matched memory bytes object, please initialize first" << endl;
      FAIL("no-matched-memory-bytes");
    }

    for (auto i = 0u; i< nBytes; i += MEM_BYTES) {
      auto initialValue = it->second->ReadInitialWithPattern(Memory::msRandomPattern, Memory::msValuePattern);  // in big-endian format
      for (int j = 0; j < MEM_BYTES; j ++) {
        data[j] = (initialValue >> ((MEM_BYTES - 1 - j) << 3)) & 0xffu;
      }

      data += MEM_BYTES;
      it ++;
    }

  }

  void Memory::ReadPartiallyInitialized(cuint64 address, cuint32 nBytes, uint8* data) const
  {
    uint32 bytes_read = 0;
    uint64 current_base_address = ADDR_ALIGN(address);
    auto it = mContent.find(current_base_address);
    uint64 offset = ADDR_OFFSET(address);
    while (bytes_read < nBytes) {
      uint32 length = MEM_BYTES - offset;

      // If this is the last block to read, reduce the length accordingly
      if (length > (nBytes - bytes_read)) {
        length = nBytes - bytes_read;
      }

      uint64 value = 0x0;
      if (it != mContent.end()) {
        value = it->second->ReadWithPattern(offset, length, false, 0x0);
      }

      value_to_data_array_big_endian(value, length, data + bytes_read);
      bytes_read += length;

      // The next entry in the map may not be contiguous, so we can't just increment the iterator here
      current_base_address += 8;
      it = mContent.find(current_base_address);

      offset = 0;
    }
  }

  void Memory::GetSections(std::vector<Section>& rSections) const
  {
    if (mContent.empty()) {
      LOG(warn) << "{Memory::GetSections} memory contents empty." << endl;
      return;
    }
    map<uint64, MemoryBytes *>::const_iterator mem_iter = mContent.begin();
    uint64 address = mem_iter->first;
    EMemDataType type = mem_iter->second->GetUniformedType();
    uint32 size = MEM_BYTES;
    uint64 next_addr = address + MEM_BYTES;
    ++ mem_iter;

    for (; mem_iter != mContent.end(); ++ mem_iter) {
      // LOG(debug) << "{GetSections} item address: 0x" << hex << mem_iter->first << " section start: 0x" << address << " data type: " << EMemDataType_to_string(type) << endl;
      auto dataType = mem_iter->second->GetUniformedType();
      if ((dataType == type) && (mem_iter->first == next_addr)) {
        size += MEM_BYTES;
        next_addr += MEM_BYTES;
      }
      else  {
        Section section(address, size, type);
        rSections.push_back(section);
        address = mem_iter->first;
        size = MEM_BYTES;
        next_addr = address + MEM_BYTES;
        type = dataType;
      }
    }
    // push the last one
    Section section(address, size, type);
    rSections.push_back(section);
  }

//#ifndef UNIT_TEST
//  void Memory::InitializeFillPattern()
//  {
//    bool exists;
//    auto value_pattern = Config::Instance()->GlobalStateValue(EGlobalStateType::MemoryFillPattern, exists);
//    if (exists) {
//      Memory::msValuePattern = ToBigEndian((uint8*)&value_pattern, sizeof(value_pattern));
//      Memory::msRandomPattern = false;
//      return;
//    }
//
//    auto fill_pattern = Config::Instance()->GlobalStateString(EGlobalStateType::MemoryFillPattern, exists);
//    if (!exists) {
//      return;
//    }
//    if (fill_pattern == "Random" || fill_pattern == "random") {
//      Memory::msRandomPattern = true;
//    }
//    else {
//      LOG(fail) << "Illegal memory fill pattern : \"" << fill_pattern << "\"" << ", valid option is \"Random\" or hexadecimal value" << endl;
//      FAIL("illegal-memory-fill-pattern");
//    }
//
//  }
//#endif

}
