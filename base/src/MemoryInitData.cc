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
#include <MemoryInitData.h>
#include <Random.h>
#include <Generator.h>
#include <VirtualMemoryInitializer.h>
#include <UtilityFunctions.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <Constraint.h>
#include <ChoicesModerator.h>
#include <Choices.h>
#include <GenException.h>
#include <Operand.h>
#include <OperandConstraint.h>
#include <VaGenerator.h>
#include <Log.h>

using namespace std;

namespace Force {
  MemoryInitData::MemoryInitData(uint64 va, uint32 dataSize, uint32 elementSize, EMemAccessType memAccessType)
    : mVa(va), mDataSize(dataSize), mElementSize(elementSize), mMemAccessType(memAccessType), mpDataChoices(nullptr), mpDataTargetConstr(nullptr), mCurrentDataIndex(0), mDataConstraints()
  {
  }

  MemoryInitData::~MemoryInitData()
  {
    delete mpDataChoices;
    delete mpDataTargetConstr;
  }

  void MemoryInitData::Setup(const Generator& gen, const LoadStoreOperand* opr)
  {
    SetupChoices(gen);

    auto op_constr = opr->GetOperandConstraint()->CastInstance<AddressingOperandConstraint>();

    if (op_constr->HasDataConstraints())
    {
      auto data_constr = op_constr->DataConstraints();
      mDataConstraints.assign(data_constr.begin(), data_constr.end());
      if (gen.IsDataBigEndian() and (mElementSize > sizeof(uint64)))
      {
        uint32 size = mDataConstraints.size();
        for (uint32 i = 0; i < size-1; i+=2) {
          swap(mDataConstraints[i], mDataConstraints[i+1]);
        }
      }
    }

    mpDataTargetConstr = new ConstraintSet();
    opr->GetDataTargetConstraint(*mpDataTargetConstr);
  }

  void MemoryInitData::Setup(const Generator& gen, ConstraintSet* dataTargetConstr)
  {
    SetupChoices(gen);

    mpDataTargetConstr = dataTargetConstr;
  }

  typedef void (*DataSetter)(uint64 value, uint32 nBytes, uint8* dataArray); // Define type of function pointer to data setter function.
  typedef uint64 (*DataGetter)(cuint8* dataArray, cuint32 nBytes); // Define type of function pointer to data getter function.

  void MemoryInitData::Commit(const Generator& gen)
  {
    if (mDataSize % mElementSize) {
      LOG(fail) << "Data size should be multiples of element size." << endl;
      FAIL("data-size-not-multiples-of-element-size");
    }

    InitializeMemory(gen);
  }

  void MemoryInitData::SetupChoices(const Generator& gen)
  {
    const ChoicesModerator* choices_mod = gen.GetChoicesModerator(EChoicesType::OperandChoices);
    try {
      mpDataChoices = choices_mod->CloneChoiceTree("Load data");
    }
    catch (const ChoicesError& choices_err) {
      LOG(fail) << "{MemoryInitData::Setup} " << choices_err.what() << endl;
      FAIL("init-data-setup-error");
    }
  }

  void MemoryInitData::InitializeMemory(const Generator& gen)
  {
    VirtualMemoryInitializer* virt_mem_initializer = gen.GetVirtualMemoryInitializer();
    uint8* mem_data = new uint8[mDataSize];
    virt_mem_initializer->ReadMemory(mVa, mDataSize, mem_data);

    uint8* mem_attrs = new uint8[mDataSize];
    virt_mem_initializer->GetMemoryAttributes(mVa, mDataSize, mem_attrs);

    // Generate missing memory values
    GenerateInitData(gen, mem_data, mem_attrs);

    virt_mem_initializer->InitializeMemory(mVa, mDataSize, mElementSize, EMemDataType::Data, mMemAccessType, mem_data, mem_attrs);
  }

  void MemoryInitData::GenerateInitData(const Generator& gen, uint8* memData, uint8* memAttrs) const
  {
    DataSetter setter_func = gen.IsDataBigEndian() ?  &element_value_to_data_array_big_endian :  &element_value_to_data_array_little_endian;

    uint8* element_mem_data = memData;
    uint8* element_mem_attrs = memAttrs;
    for (uint32 total_size = 0; total_size < mDataSize; total_size += mElementSize) {
      uint32 micro_increment = sizeof(uint64);

      if (mElementSize > micro_increment) {
        for (uint32 microSize = 0; microSize < mElementSize; microSize += micro_increment) {
          uint64 current_data = GetMemoryData(gen, element_mem_data, micro_increment);
          uint64 current_attrs = GetMemoryAttributes(gen, element_mem_attrs, micro_increment);
          uint64 micro_value = GenerateInitDataBlock(gen, micro_increment, GetInitializedMask(current_attrs), current_data);

          (*setter_func)(micro_value, micro_increment, element_mem_data);
          element_mem_data += micro_increment;
          (*setter_func)(current_attrs, micro_increment, element_mem_attrs);
          element_mem_attrs += micro_increment;
        }
      } else {
        uint64 current_data = GetMemoryData(gen, element_mem_data, mElementSize);
        uint64 current_attrs = GetMemoryAttributes(gen, element_mem_attrs, mElementSize);
        uint64 element_value = GenerateInitDataBlock(gen, mElementSize, GetInitializedMask(current_attrs), current_data);

        (*setter_func)(element_value, mElementSize, element_mem_data);
        element_mem_data += mElementSize;
        (*setter_func)(current_attrs, mElementSize, element_mem_attrs);
        element_mem_attrs += mElementSize;
      }
    }
  }

  uint64 MemoryInitData::GetMemoryData(const Generator& gen, cuint8* mem_data, cuint32 dataSize) const
  {
    uint32 max_data_size = sizeof(uint64);
    if (dataSize > max_data_size) {
      LOG(fail) << "Specified data size of " << dataSize << " exceeds max data size of " << max_data_size << endl;
      FAIL("unsupported-data-size");
    }

    DataGetter getter_func = gen.IsDataBigEndian() ? &data_array_to_element_value_big_endian : &data_array_to_element_value_little_endian;
    return (*getter_func)(mem_data, dataSize);
  }

  uint64 MemoryInitData::GetMemoryAttributes(const Generator& gen, cuint8* mem_attrs, cuint32 dataSize) const
  {
    uint32 max_data_size = sizeof(uint64);
    if (dataSize > max_data_size) {
      LOG(fail) << "Specified data size of " << dataSize << " exceeds max data size of " << max_data_size << endl;
      FAIL("unsupported-data-size");
    }

    DataGetter getter_func = gen.IsDataBigEndian() ? &data_array_to_element_value_big_endian : &data_array_to_element_value_little_endian;
    return (*getter_func)(mem_attrs, dataSize);
  }

  uint64 MemoryInitData::GetInitializedMask(cuint64 memAttrs) const
  {
    uint64 init_mask = 0x0;
    uint64 byte_mem_attrs_mask = 0xFF;
    for (uint32 i = 0; i < sizeof(memAttrs); i++) {
      uint64 byte_mem_attrs = (memAttrs & byte_mem_attrs_mask) >> (8 * i);
      if ((byte_mem_attrs & EMemDataTypeBaseType(EMemDataType::Init)) == EMemDataTypeBaseType(EMemDataType::Init)) {
        init_mask |= byte_mem_attrs_mask;
      }

      byte_mem_attrs_mask <<= 8;
    }

    return init_mask;
  }

  uint64 MemoryInitData::GenerateInitDataBlock(const Generator& gen, cuint64 size, cuint64 initMask, cuint64 currentData) const
  {
    if (mCurrentDataIndex < mDataConstraints.size()) {
      if (mDataConstraints[mCurrentDataIndex] != nullptr) {
        return mDataConstraints[mCurrentDataIndex++]->ChooseValue();
      }
      ++mCurrentDataIndex;
    }

    auto data_type = 2; // always random data for store target data

    if (mMemAccessType != EMemAccessType::Write) {
      data_type = mpDataChoices->Choose()->Value();
    }

    uint64 data = 0;
    switch (data_type) {
    case 0: // address type data
      data = GenerateTargetConstrainedData(gen, size, initMask, currentData, false, EMemAccessType::ReadWrite);
      break;
    case 1: // instruction type data
      data = GenerateTargetConstrainedData(gen, size, initMask, currentData, true, EMemAccessType::Unknown);
      break;
    case 2: // random data
      {
        data = GenerateRandomDataWithSize(size, initMask, currentData);
        // << "{InitData::GenerateInitDataBlock} random data 0x" << hex << data << endl;
        break;
      }
    default:
      LOG(fail) << "Unsupported init data type:" << data_type << endl;
      FAIL("unsupported-data-type");
    }
    return data;
  }

  uint64 MemoryInitData::GenerateTargetConstrainedData(const Generator& gen, cuint64 size, cuint64 initMask, cuint64 currentData, cbool isInstr, EMemAccessType memAccessType) const
  {
    uint64 data = 0;
    auto vm_mapper = gen.GetVmManager()->CurrentVmMapper();

    try {
      if (initMask != 0x0) {
        return GenerateRandomDataWithSize(size, initMask, currentData);
      }

      // << "Data target constraint:" << mpDataTargetConstr.ToSimpleString() << endl;
      VaGenerator va_gen(vm_mapper, nullptr, nullptr, false);
      uint64 align = size;
      data = va_gen.GenerateAddress(align, size, isInstr, memAccessType, mpDataTargetConstr);
      LOG(info) << "{InitData::GenerateTargetConstrainedData} Generated VA 0x" << hex << data << endl;
    }
    catch (const ConstraintError& err) {
      LOG(info) << "{InitData::GenerateTargetConstrainedData} Constraint Error: " << err.what() << ". Switch to random data." <<  endl;
      data = GenerateRandomDataWithSize(size, initMask, currentData);
    }
    return data;
  }

  uint64 MemoryInitData::GenerateRandomDataWithSize(cuint64 size, cuint64 initMask, cuint64 currentData) const
  {
    uint64 mask = 0xffffffffffffffffull;
    if (size < 8u) {
      mask = (1ull << (size * 8)) - 1;
    }
    auto rnd_handle = Random::Instance();
    uint64 random_data = rnd_handle->Random64(0, mask);
    random_data = (random_data & ~initMask) | currentData;
    return random_data;
  }

}
