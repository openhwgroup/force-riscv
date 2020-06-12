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

#include <map>
#include <iosfwd>
#include <vector>

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

namespace Force {

  class MemoryBytes;
  struct MetaAccess;

  /*!
    \class Section
    \brief a continous address range with the same type
  */
  struct Section {
    uint64 mAddress;  //!< base address on memorybytes.
    uint32 mSize;     //!< range size in bytes, multiple 8-bytes.
    EMemDataType mType;

    Section(uint64 address, uint32 size, EMemDataType type) : mAddress(address), mSize(size), mType(type) { }
  };

  /*!
    \class Memory
    \brief A memory model to record memory data and states.
  */
  class Memory {
  public:
    void Initialize(uint64 address, uint64 value, uint32 nBytes, EMemDataType type);  //!< Initialize memory.
    void Initialize(uint64 address, cuint8* data, cuint8* attrs, uint32 nBytes, EMemDataType type);  //!< Initialize memory.
    bool IsInitialized(uint64 address, uint32 nBytes) const; //!< check whether address is initialized or not
    uint64 Read(uint64 address, uint32 nBytes) const;        //!< read data in big-endian
    void Write(uint64 address, uint64 value, uint32 nBytes); //!< write data in big-endian
    void Write(uint64 address, cuint8* data, uint32 nBytes);  //!< write data in the buffer
    uint64 ReadInitialValue(uint64 address, uint32 nBytes) const;  //!< read initial value in big-endian, failed if some byte not initialized
    void GetMemoryAttributes(cuint64 address, cuint32 nBytes, uint8* memAttrs) const; //!< Get memory attributes for the specified number of bytes starting at the specified address.
    uint8 GetByteMemoryAttributes(cuint64 address) const; //!< Get memory attributes of the byte at the specified address.

    void ReadPartiallyInitialized(cuint64 address, cuint32 nBytes, uint8* data) const; //!< Read data in byte ordering, i.e. data[0] is byte with lowest address, that may not be fully initialized; unitialized bytes will be set to 0.
    void ReadInitialWithPattern(uint64 address, uint32 nBytes, uint8* data) const; //!< read initial value , random value if some byte not initialized
    void Dump(std::ostream& out_str) const;                  //!< dump memory model for debug
    void Dump (std::ostream& out_str, uint64 address, uint64 nBytes) const; //!< dump memory range
    void GetSections(std::vector<Section>& rSections) const;    //!< Get sections the memory object contained, by address ascending order

    explicit Memory(EMemBankType bankType) : mBankType(bankType), mContent() { }  //!< Constructor.
    ~Memory(); //!< Destructor.
    EMemBankType MemoryBankType() const { return mBankType; } //!< Return memory bank type.
    bool IsEmpty() const { return mContent.empty(); } //!< Return if the memory module is empty.
#ifndef UNIT_TEST
    static void InitializeFillPattern(); //!< initialize fill pattern
#endif
  private:
    EMemBankType mBankType;
    std::map<uint64, MemoryBytes *> mContent;  //!< map containing all memory content, increasing order by the key

    void InitializeMemoryBytes(const MetaAccess& rMetaAccess, EMemDataType type); //!< initialize the memory bytes on meta access
    bool IsInitializedMemoryBytes(const MetaAccess& rMetaAccess) const; //!< check memory bytes are initialized or not
    void ReadMemoryBytes(MetaAccess& rMetaAccess) const;                //!< read memory bytes on meta access
    void WriteMemoryBytes(const MetaAccess& rMetaAccess);         //!< write memory bytes on meta access
    void ReadInitialValue(MetaAccess& rMetaAccess) const;         //!< read initial value on meta access
    void DumpTitle(std::ostream& out_str) const; //!< dump title
    static bool msRandomPattern; //!< memory fill rondom pattern
    static uint64 msValuePattern;  //!< memory fill value pattern in big endian
 };
}

#endif
