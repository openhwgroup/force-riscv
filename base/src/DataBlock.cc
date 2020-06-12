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
#include <Generator.h>
#include <DataBlock.h>
#include <VaGenerator.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <GenRequest.h>
#include <Log.h>

#include <sstream>
#include <memory>

//Debug
#include <Constraint.h>
#include <Page.h>

using namespace std;

/*!
  \file DataBlock.cc
  \brief Code for updaing PE state
*/

namespace Force {

  DataBlock::DataBlock(const DataBlock& rOther)
    : Object(rOther), mAlign(rOther.mAlign), mSize(rOther.mSize), mData()
  {
    transform(rOther.mData.cbegin(), rOther.mData.cend(), back_inserter(mData),
      [](const DataUnit* pDataUnit) { return new DataUnit(*pDataUnit); });
  }

  DataBlock::~DataBlock()
  {
    Clear();
  }

  Object* DataBlock::Clone() const
  {
    return new DataBlock(*this);
  }

  const std::string DataBlock::ToString() const
  {
    return "DataBlock";
  }

  void DataBlock::Clear()
  {
    for (auto item : mData)
    {
      delete item;
    }

    mData.clear();
    LOG(notice) << "{DataBlock::Clear} data units are cleared!" << endl;
  }

  void DataBlock::AddUnit(uint64 value, uint64 size, bool bigEndian)
  {
    auto unit = new DataUnit(value, size, bigEndian);
    mData.push_back(unit);
    mSize += unit->Size();
  }

  uint64 DataBlock::Allocate(Generator* pGen, VmMapper* pTargetMapper, const GenPageRequest* pPageReq)
  {
    std::unique_ptr<GenPageRequest> local_page_req_storage;
    if (nullptr == pPageReq) {
      GenPageRequest* page_req = pTargetMapper->GenPageRequestRegulated(false, EMemAccessType::Read, true); // request no fault
      local_page_req_storage.reset(page_req);
      pPageReq = page_req;
    }

    VaGenerator va_gen(pTargetMapper, pPageReq);

    uint64 start = va_gen.GenerateAddress(mAlign, mSize, false, EMemAccessType::Read);
    LOG(notice) << "{DataBlock::Setup} generate data block start address...0x" << hex << start << " with size:" << dec << mSize << " and align:" << mAlign << endl;
    uint64 current = start;

    for (auto item : mData)
    {
      uint64 pa = 0;
      uint32 bank = 0;
      pTargetMapper->TranslateVaToPa(current, pa, bank);
      LOG(notice) << "{DataBlock::Setup} allocate memory for value:0x" << hex << item->Value() << " size:0x" << item->Size() << " Big endian:0x" << item->BigEndian() << " to memory:0x" << pa << " bank:" << bank << endl;
      pGen->InitializeMemoryWithEndian(pa, bank, item->Size(), item->Value(), false, item->BigEndian());
      current += item->Size();
    }

    return start;
  }

}
