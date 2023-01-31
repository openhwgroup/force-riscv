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

#ifndef Force_Memory_H
#define Force_Memory_H

#include <iosfwd>
#include <list>
#include <map>
#include <vector>

#include "Force_Defines.h"
#include "Force_Enums.h"

namespace Force {

  class MemoryBytes;
  struct MetaAccess;

  /*!
    \class Section
    \brief a continous address range with the same type
  */
  struct Section {
    public:
      Section(uint64 address, uint32 size, EMemDataType type) : mAddress(address), mSize(size), mType(type) { }
      bool Intersects(const Section& rOther) const; //!< Return true if the memory range of the other Section overlaps the memory range of this Section.
      bool Contains(const Section& rOther) const; //!< Return true if the memory range of the other Section is entirely contained within the memory range of this Section.
      bool operator<(const Section& rOther) const; //!< Return true if this Section has a smaller base address than the other Section.
    public:
      uint64 mAddress;  //!< base address on memorybytes.
      uint32 mSize;     //!< range size in bytes.
      EMemDataType mType;
    private:
      uint64 GetEndAddress() const; //!< Return the last address of the memory range.
  };

  /*!
    \class Memory
    \brief A memory model to record memory data and states.
  */
  class Memory {
  public:
    void AutoInitialize(uint64 address, uint32 nBytes); //!< Initialize any uninitialized memory in the specified range. Previously initialized memory is unchanged. This is useful when it doesn't matter whether the memory has been initialized previously and the initial value is unimportant.
    void Initialize(uint64 address, uint64 value, uint32 nBytes, EMemDataType type);  //!< Initialize memory.
    void Initialize(uint64 address, cuint8* data, cuint8* attrs, uint32 nBytes, EMemDataType type);  //!< Initialize memory.
    bool IsInitialized(uint64 address, uint32 nBytes) const; //!< check whether address is initialized or not
    uint64 Read(uint64 address, uint32 nBytes);        //!< read data in big-endian
    void Write(uint64 address, uint64 value, uint32 nBytes); //!< write data in big-endian
    void Write(uint64 address, cuint8* data, uint32 nBytes);  //!< write data in the buffer
    uint64 ReadInitialValue(uint64 address, uint32 nBytes);  //!< read initial value in big-endian, failed if some byte not initialized
    void Reserve(uint64 address, uint32 nBytes); //!< Reserve a memory range. Unreserve any overlapping reserved ranges. Any write to any part of the reserved range causes the range to become unreserved.
    void Unreserve(uint64 address, uint32 nBytes); //!< Unreserve all reserved memory ranges that overlap with the specified memory range.
    bool IsReserved(uint64 address, uint32 nBytes); //!< Return true if the specified memory range lies entirely within a reserved memory range.
    void GetMemoryAttributes(cuint64 address, cuint32 nBytes, uint8* memAttrs) const; //!< Get memory attributes for the specified number of bytes starting at the specified address.
    uint8 GetByteMemoryAttributes(cuint64 address) const; //!< Get memory attributes of the byte at the specified address.

    void ReadPartiallyInitialized(cuint64 address, cuint32 nBytes, uint8* data) const; //!< Read data in byte ordering, i.e. data[0] is byte with lowest address, that may not be fully initialized; unitialized bytes will be set to 0.
    void ReadInitialWithPattern(uint64 address, uint32 nBytes, uint8* data) const; //!< read initial value , random value if some byte not initialized
    void Dump(std::ostream& out_str) const;                  //!< dump memory model for debug
    void Dump (std::ostream& out_str, uint64 address, uint64 nBytes) const; //!< dump memory range
    void GetSections(std::vector<Section>& rSections) const;    //!< Get sections the memory object contained, by address ascending order

    Memory(EMemBankType bankType, bool autoInit) : mBankType(bankType), mContent(), mAutoInit(autoInit) { }  //!< Constructor.
    ~Memory(); //!< Destructor.
    EMemBankType MemoryBankType() const { return mBankType; } //!< Return memory bank type.
    bool IsEmpty() const { return mContent.empty(); } //!< Return if the memory module is empty.
//#ifndef UNIT_TEST
//    static void InitializeFillPattern(); //!< initialize fill pattern
//#endif
  private:
    void InitializeMemoryBytes(const MetaAccess& rMetaAccess, EMemDataType type); //!< initialize the memory bytes on meta access
    bool IsInitializedMemoryBytes(const MetaAccess& rMetaAccess) const; //!< check memory bytes are initialized or not
    void EnsureInitialization(uint64 address, uint32 nBytes); //!< Verify the specified memory range is initialized. If mAutoInit is true, uninitialized memory is initialized; otherwise, uninitialize memory triggers a failure.
    void ReadMemoryBytes(MetaAccess& rMetaAccess) const;                //!< read memory bytes on meta access
    void WriteMemoryBytes(const MetaAccess& rMetaAccess);         //!< write memory bytes on meta access
    void ReadInitialValue(MetaAccess& rMetaAccess) const;         //!< read initial value on meta access
    void DumpTitle(std::ostream& out_str) const; //!< dump title
  private:
    EMemBankType mBankType;
    std::map<uint64, MemoryBytes *> mContent;  //!< map containing all memory content, increasing order by the key
    std::list<Section> mReservedRanges; //!< List of reserved memory ranges sorted by start address
    bool mAutoInit; //!< Flag indicating memory should be automatically initialized on access if not already initialized
    static bool msRandomPattern; //!< memory fill rondom pattern
    static uint64 msValuePattern;  //!< memory fill value pattern in big endian
 };
}

#endif
