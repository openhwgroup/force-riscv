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
#ifndef Force_VirtualMemoryInitializer_H
#define Force_VirtualMemoryInitializer_H

#include "Defines.h"
#include "Enums.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class VmManager;
  class RecordArchive;
  class MemoryManager;
  class TranslationRange;
  struct PhysicalPageSplit;

  /*!
    \class VirtualMemoryInitializer
    \brief Access to memory via virtual addresses.
  */
  class VirtualMemoryInitializer {
  public:
    VirtualMemoryInitializer(const VmManager* pVmManager, const RecordArchive* pRecordArchive, MemoryManager* pMemoryManager); //!< Constructor
    VirtualMemoryInitializer(const VirtualMemoryInitializer& rOther) = default; //!< Copy constructor
    ~VirtualMemoryInitializer() = default; //!< Destructor
    VirtualMemoryInitializer& operator=(const VirtualMemoryInitializer& rOther) = delete; //!< Copy assignment operator not currently needed.

    void InitializeMemory(cuint64 va, cuint32 size, cuint32 elementSize, const EMemDataType memDataType, const EMemAccessType memAccessType, uint8* memData, uint8* memAttrs); //!< Initialize memory. This method will deallocate the memory for memData and memAttrs.
    void ReadMemory(cuint64 va, cuint32 size, uint8* memData) const; //!< Read data from memory. Expects memData to be allocated to the specified size.
    void GetMemoryAttributes(cuint64 va, cuint32 size, uint8* memAttrs) const; //!< Get attributes for the specified memory location and size. Expects memAttrs to be allocated to the specified size.
    bool VaNeedInitialization(cuint64 va, cuint32 size) const; //!< Check if the memory from target VA through target VA + size - 1 is need initialized.
    bool InstructionVaInitialized(cuint64 va) const; //!< Check if the memory using instruction is initialized.
    void SetThreadId(cuint32 threadId) { mThreadId = threadId; } //!< Set thread ID.
  private:
    PhysicalPageSplit GetPhysicalPageSplit(cuint64 va, cuint32 size) const; //!< Get PA ranges corresponding to VA range.
    bool IsRangeMapped(cuint64 va, cuint32 size, bool& addressError) const; //!< Check if pages have been allocated for entire VA range.
    void GetTranslationRange(cuint64 va, TranslationRange& rTransRange) const; //!< Get translation range for the specified VA.
    bool PaRangeInitialized(PhysicalPageSplit& rPageSplit) const; //!< Check if physcial address range is initialized.
  private:
    const VmManager* mpVmManager; //!< Virtual memory manager
    const RecordArchive* mpRecordArchive; //!< Record archive
    MemoryManager* mpMemoryManager; //!< Memory manager
    uint32 mThreadId; //!< Thread ID
  };

}

#endif  // Force_VirtualMemoryInitializer_H
