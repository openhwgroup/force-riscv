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
#include "Record.h"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "Log.h"
#include "UtilityFunctions.h"

using namespace std;

namespace Force {

  const string Record::ToString() const
  {
    char print_buffer[32];
    snprintf(print_buffer, 32, ": %d", mId);
    return string(Type()) + print_buffer;
  }

  MemoryInitRecord::MemoryInitRecord(cuint32 threadId, uint32 size, uint32 elementSize, EMemDataType type)
    : MemoryInitRecord(threadId, size, elementSize, type, EMemAccessType::Unknown)
  {
  }

  MemoryInitRecord::MemoryInitRecord(cuint32 threadId, uint32 size, uint32 elementSize, EMemDataType type, const EMemAccessType memAccessType)
    : Record(), mThreadId(threadId), mSize(size), mElementSize(elementSize), mAddress(0), mMemoryId(0), mType(type), mMemAccessType(memAccessType), mpData(nullptr), mpAttrs(nullptr)
  {
    if (mElementSize > mSize) {
      LOG(fail) << "Data element size: " << dec << mElementSize << " larger than whole data size: " << mSize << endl;
      FAIL("element-larger-than-whole-size");
    }
    if (mSize % mElementSize) {
      LOG(fail) << "Data size: " << mSize << " should be multiples of element size: " << mElementSize << endl;
      FAIL("size-element-mod-check");
    }
    mpData = new uint8[mSize];
    mpAttrs = new uint8[mSize];
  }

  MemoryInitRecord::~MemoryInitRecord()
  {
    delete [] mpData;
    delete [] mpAttrs;
  }

  MemoryInitRecord::MemoryInitRecord(const MemoryInitRecord& rOther)
    : Record(rOther), mThreadId(rOther.mThreadId), mSize(rOther.mSize), mElementSize(rOther.mElementSize), mAddress(0), mMemoryId(0), mType(rOther.mType), mMemAccessType(), mpData(nullptr), mpAttrs(nullptr)
  {
    if (rOther.mpData) {
      mpData = new uint8[mSize];
      std::copy(rOther.mpData, rOther.mpData + mSize, mpData);
    }

    if (rOther.mpAttrs) {
      mpAttrs = new uint8[mSize];
      std::copy(rOther.mpAttrs, rOther.mpAttrs + mSize, mpAttrs);
    }
  }

  const string MemoryInitRecord::ToString() const
  {
    stringstream out_stream;

    out_stream << Type() << ": address=0x" << hex << setfill('0') << setw(16) << mAddress << " mem-ID=" << mMemoryId << " size=" << dec << mSize << " element-size=" << mElementSize << " type="
               << EMemDataType_to_string(mType) << " data=";
    out_stream << DataString();

    return out_stream.str();
  }

  const string MemoryInitRecord::DataString() const
  {
    stringstream out_stream;

    out_stream << hex << setfill('0');

    for (uint32 i = 0; i < mSize; i++) {
      out_stream << setw(2) << (int)(mpData[i]);
    }

    return out_stream.str();
  }

  typedef void (*DataSetter)(uint64 value, uint32 nBytes, uint8* dataArray); // Define type of function pointer to data setter function.

  void MemoryInitRecord::SetData(uint64 pa, uint32 memId, uint64 data, uint64 nBytes, bool bigEndian)
  {
    if (nBytes != mSize) {
      LOG(fail) << "{MemoryInitRecord::SetData} called with mis-matching size : " << dec << nBytes << ", required to be " << mSize << endl;
      FAIL("set-data-mismatch-size");
    }

    mAddress = pa;
    mMemoryId = memId;

    if (nullptr != mpAttrs) {
      memset(mpAttrs, 0x0, mSize);
    }

    // multiple elements but element size is 1, just a byte stream.
    if (mElementSize == 1) {
      // just byte stream.
      value_to_data_array_big_endian(data, nBytes, mpData);
      return;
    }

    DataSetter setter_func = bigEndian ? &element_value_to_data_array_big_endian : &element_value_to_data_array_little_endian;

    if (mSize == mElementSize) {
      // single element.
      (*setter_func)(data, nBytes, mpData);
      return;
    }

    // multiple eleemnts, element size > 1
    uint32 element_num = mSize / mElementSize;
    uint32 element_bits = mElementSize << 3;
    uint32 element_mask = (uint32)((1ull << element_bits) - 1);
    uint8* data_ptr_current = mpData;
    uint32 element_value = 0;
    switch (element_num) {
    case 4:
      element_value = (uint32)(data >> (element_bits * 3)) & element_mask;
      (*setter_func)(element_value, mElementSize, data_ptr_current);
      data_ptr_current += mElementSize;
    case 3:
      element_value = (uint32)(data >> (element_bits * 2)) & element_mask;
      (*setter_func)(element_value, mElementSize, data_ptr_current);
      data_ptr_current += mElementSize;
    case 2:
      element_value = (uint32)(data >> element_bits) & element_mask;
      (*setter_func)(element_value, mElementSize, data_ptr_current);
      data_ptr_current += mElementSize;
      element_value = (uint32)(data & element_mask);
      (*setter_func)(element_value, mElementSize, data_ptr_current);
      break;
    default:
      LOG(fail) << "Unexpected element number with size: " << dec << mSize << " element-size: " << mElementSize << endl;
      FAIL("unexpected-element-number");
    }
  }

  void MemoryInitRecord::SetData(uint64 pa, uint32 memId, uint8* dataVec, uint32 size)
  {
    mAddress = pa;
    mMemoryId = memId;

    if (nullptr != mpData) {
      delete [] mpData;
      mpData = nullptr;
    }

    if (size != mSize) {
      delete dataVec;
      dataVec = nullptr;
      LOG(fail) << "{MemoryInitRecord::SetData} setting data with inconsistent data size." << endl;
      FAIL("inconsistent-data-size");
    }

    mpData = dataVec;

    if (nullptr != mpAttrs) {
      memset(mpAttrs, 0x0, mSize);
    }
  }

  void MemoryInitRecord::SetDataWithAttributes(uint64 pa, uint32 memId, uint8* dataVec, uint8* attrVec, uint32 size)
  {
    SetData(pa, memId, dataVec, size);

    if (nullptr != mpAttrs) {
      delete [] mpAttrs;
      mpAttrs = nullptr;
    }

    mpAttrs = attrVec;
  }

  const string RecordArchive::ToString() const
  {
    return "RecordArchive";
  }

  RecordArchive::~RecordArchive()
  {
    for (auto rec_item : mRecords) {
      delete rec_item;
    }

  }

  MemoryInitRecord* RecordArchive::GetMemoryInitRecord(cuint32 threadId, uint32 size, uint32 elementSize, EMemDataType type) const
  {
    MemoryInitRecord* ret_rec = new MemoryInitRecord(threadId, size, elementSize, type);
    ret_rec->SetId(mCurrentId++);
    mRecords.push_back(ret_rec);
    return ret_rec;
  }

  MemoryInitRecord* RecordArchive::GetMemoryInitRecord(cuint32 threadId, uint32 size, uint32 elementSize, EMemDataType type, const EMemAccessType memAccessType) const
  {
    MemoryInitRecord* ret_rec = new MemoryInitRecord(threadId, size, elementSize, type, memAccessType);
    ret_rec->SetId(mCurrentId++);
    mRecords.push_back(ret_rec);
    return ret_rec;
  }

  void RecordArchive::SwapMemoryInitRecords(std::vector<MemoryInitRecord* >& rSwapVec)
  {
    rSwapVec.swap(mRecords);
  }
}
