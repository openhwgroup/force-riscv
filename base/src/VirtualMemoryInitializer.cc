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
#include <VirtualMemoryInitializer.h>
#include <PhysicalPageSplitter.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <VmUtils.h>
#include <Record.h>
#include <MemoryManager.h>
#include <cstring>

using namespace std;

/*!
  \file VirtualMemoryInitializer.cc
  \brief Code supporting access to memory via virtual addresses.
*/

namespace Force {

  VirtualMemoryInitializer::VirtualMemoryInitializer(const VmManager* pVmManager, const RecordArchive* pRecordArchive, MemoryManager* pMemoryManager)
    : mpVmManager(pVmManager), mpRecordArchive(pRecordArchive), mpMemoryManager(pMemoryManager), mThreadId(0)
  {
  }

  void VirtualMemoryInitializer::InitializeMemory(cuint64 va, cuint32 size, cuint32 elementSize, const EMemDataType memDataType, const EMemAccessType memAccessType, uint8* memData, uint8* memAttrs)
  {
    PhysicalPageSplit page_split = GetPhysicalPageSplit(va, size);

    if (page_split.mSize2 > 0) {
      MemoryInitRecord* mem_init_data1 = mpRecordArchive->GetMemoryInitRecord(mThreadId, page_split.mSize1, 1, memDataType, memAccessType);
      uint8* mem_data1 = new uint8[page_split.mSize1];
      memcpy(mem_data1, memData, page_split.mSize1);
      uint8* mem_attrs1 = new uint8[page_split.mSize1];
      memcpy(mem_attrs1, memAttrs, page_split.mSize1);

      mem_init_data1->SetDataWithAttributes(page_split.mPa1, page_split.mBank1, mem_data1, mem_attrs1, page_split.mSize1);
      mpMemoryManager->InitializeMemory(mem_init_data1);

      MemoryInitRecord* mem_init_data2 = mpRecordArchive->GetMemoryInitRecord(mThreadId, page_split.mSize2, 1, memDataType, memAccessType);
      uint8* mem_data2 = new uint8[page_split.mSize2];
      memcpy(mem_data2, memData + page_split.mSize1, page_split.mSize2);
      uint8* mem_attrs2 = new uint8[page_split.mSize2];
      memcpy(mem_attrs2, memAttrs + page_split.mSize1, page_split.mSize2);

      delete memData;
      memData = nullptr;
      delete memAttrs;
      memAttrs = nullptr;

      mem_init_data2->SetDataWithAttributes(page_split.mPa2, page_split.mBank2, mem_data2, mem_attrs2, page_split.mSize2);
      mpMemoryManager->InitializeMemory(mem_init_data2);
    }
    else {
      MemoryInitRecord* mem_init_data = mpRecordArchive->GetMemoryInitRecord(mThreadId, page_split.mSize1, elementSize, memDataType, memAccessType);
      mem_init_data->SetDataWithAttributes(page_split.mPa1, page_split.mBank1, memData, memAttrs, page_split.mSize1);
      mpMemoryManager->InitializeMemory(mem_init_data);
    }
  }

  void VirtualMemoryInitializer::ReadMemory(cuint64 va, cuint32 size, uint8* memData) const
  {
    PhysicalPageSplit page_split = GetPhysicalPageSplit(va, size);

    mpMemoryManager->ReadMemoryPartiallyInitialized(PaTuple(page_split.mPa1, page_split.mBank1), page_split.mSize1, memData);
    if (page_split.mSize2 > 0) {
      mpMemoryManager->ReadMemoryPartiallyInitialized(PaTuple(page_split.mPa2, page_split.mBank2), page_split.mSize2, memData + page_split.mSize1);
    }
  }

  void VirtualMemoryInitializer::GetMemoryAttributes(cuint64 va, cuint32 size, uint8* memAttrs) const
  {
    PhysicalPageSplit page_split = GetPhysicalPageSplit(va, size);

    mpMemoryManager->GetMemoryAttributes(PaTuple(page_split.mPa1, page_split.mBank1), page_split.mSize1, memAttrs);
    if (page_split.mSize2 > 0) {
      mpMemoryManager->GetMemoryAttributes(PaTuple(page_split.mPa2, page_split.mBank2), page_split.mSize2, memAttrs + page_split.mSize1);
    }
  }

  bool VirtualMemoryInitializer::PaRangeInitialized(PhysicalPageSplit& rPageSplit) const
  {
    bool initialized1 = mpMemoryManager->PaInitialized(PaTuple(rPageSplit.mPa1, rPageSplit.mBank1), rPageSplit.mSize1);
    bool initialized2 = true;
    if (rPageSplit.mSize2 > 0) {
      initialized2 = mpMemoryManager->PaInitialized(PaTuple(rPageSplit.mPa2, rPageSplit.mBank2), rPageSplit.mSize2);
    }

    return (initialized1 and initialized2);
  }

  bool VirtualMemoryInitializer::VaNeedInitialization(cuint64 va, cuint32 size) const
  {
    bool address_error = false;
    if (not IsRangeMapped(va, size, address_error)) {
      return true;
    }
    if (address_error) {
      return false;
    }
    PhysicalPageSplit page_split = GetPhysicalPageSplit(va, size);
    return not PaRangeInitialized(page_split);
  }

  bool VirtualMemoryInitializer::InstructionVaInitialized(cuint64 va) const
  {
    bool address_error = false;
    if (not IsRangeMapped(va, 4, address_error)) {
      return false;
    }
    if (address_error) {
      return false;
    }

    PhysicalPageSplit page_split = GetPhysicalPageSplit(va, 4);
    return PaRangeInitialized(page_split);
  }

  PhysicalPageSplit VirtualMemoryInitializer::GetPhysicalPageSplit(cuint64 va, cuint32 size) const
  {
    PhysicalPageSplitter page_splitter(mpVmManager->CurrentVmMapper());
    return page_splitter.GetPhysicalPageSplit(va, size);
  }

  bool VirtualMemoryInitializer::IsRangeMapped(cuint64 va, cuint32 size, bool& address_error) const
  {
    VmMapper* vm_mapper = mpVmManager->CurrentVmMapper();
    TranslationRange trans_range1;
    bool mapped1 = vm_mapper->GetTranslationRange(va, trans_range1);
    bool mapped2 = true;
    address_error = (ETranslationResultType::AddressError == trans_range1.TranslationResultType());

    uint64 space_in_page = trans_range1.SpaceInPage(va);
    if (space_in_page < size) {
      uint64 va2 = va + space_in_page;
      TranslationRange trans_range2;
      mapped2 = vm_mapper->GetTranslationRange(va2, trans_range2);
    }

    return (mapped1 and mapped2);
  }

}
