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
#ifndef Force_Record_H
#define Force_Record_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <Object.h>
#include <vector>

namespace Force {

  /*!
    \class Record
    \brief Base class of various record object.
  */
  class Record : public Object {
  public:
    Record() : Object(), mId(0) { } //!< Default constructor
    const std::string ToString() const override; //!< Return a string describing the current state of the Record object.
    const char* Type() const override { return "Record"; } //!< Return a string describing the actual type of the Record object
    ~Record() { } //!< Destructor.

    Object* Clone() const override { return new Record(*this); } //!< Return a cloned Record object of the same type.
    uint32 Id() const { return mId; }
  protected:
    Record(const Record& rOther) : Object(rOther), mId(rOther.mId) { } //!< Copy constructor.
    void SetId(uint32 id) { mId = id; } //!< Set Record object ID.
  protected:
    uint32 mId; //!< Sequential ID of the Record.

    friend class RecordArchive;
  };

  /*!
    \class MemoryInitRecord
    \brief Memory initialization record object.
  */
  class MemoryInitRecord : public Record {
  public:
    MemoryInitRecord(cuint32 threadId, uint32 size, uint32 elementSize, EMemDataType type); //!< Constructor with parameters.
    MemoryInitRecord(cuint32 threadId, uint32 size, uint32 elementSize, EMemDataType type, const EMemAccessType memAccessType); //!< Constructor with parameters.
    const char* Type() const override { return "MemoryInitRecord"; } //!< Return a string describing the actual type of the MemoryInitRecord object.
    const std::string ToString() const override; //!< Return a string describing the current state of the MemoryInitRecord object.
    ~MemoryInitRecord(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(MemoryInitRecord);
    Object* Clone() const override { return new MemoryInitRecord(*this); } //!< Return a cloned MemoryInitRecord object of the same type.

    void SetData(uint64 pa, uint32 memId, uint64 data, uint64 nBytes, bool bigEndian); //!< Set data to the MemoryInitRecord object.
    void SetData(uint64 pa, uint32 memId, uint8* dataVec, uint32 size); //!< Set data with a data byte vector.
    void SetDataWithAttributes(uint64 pa, uint32 memId, uint8* dataVec, uint8* attrVec, uint32 size); //!< Set data and corresponding attributes with byte vectors.
    uint32 ThreadId() const { return mThreadId; } //!< Return the ID of the thread requesting the memory initialization.
    uint32 Size() const { return mSize; } //!< Return the size of data for the memory initialization.
    uint64 Address() const { return mAddress; } //!< Return target address of the memory initialization.
    uint32 MemoryId() const { return mMemoryId; } //!< Return target memory ID.
    uint8* InitData() const { return mpData; } //!< Return the pointer to the memory initialization data.
    uint8* InitAttributes() const { return mpAttrs; } //!< Return the pointer to the memory initialization attributes.
    EMemDataType InitType() const { return mType; } //!< Return the initialization data type.
    EMemAccessType AccessType() const { return mMemAccessType; } //!< Return the memory access type.
    const std::string DataString() const; //!< Return the data byte stream in string format for printing.
  protected:
    MemoryInitRecord(const MemoryInitRecord& rOther); //!< Copy constructor.
  protected:
    cuint32 mThreadId; //!< ID of the thread requesting the memory initialization.
    uint32 mSize; //!< Size of the initializing data.
    uint32 mElementSize; //!< Element size of the initializaing data.
    uint64 mAddress; //!< Physical address of the memory initialization target.
    uint32 mMemoryId; //!< Id of the memory type targed by this MemoryInitRecord object.
    EMemDataType mType; //!< Type of the initializing data.
    EMemAccessType mMemAccessType; //!< Memory access type.
    uint8* mpData; //!< The byte stream data.
    uint8* mpAttrs; //!< Attributes corresponding to the data.
  };

  class BntNode;

  /*!
    \class RecordArchive
    \brief Archive of various record objects
  */
  class RecordArchive : public Object {
  public:
    RecordArchive() : Object(), mCurrentId(0), mRecords() { } //!< Constructor
    Object* Clone() const override { return new RecordArchive(*this); } //!< Return a cloned RecordArchive object of the same type
    const std::string ToString() const override; //!< Return a string describing the current state of the RecordArchive object.
    const char* Type() const override { return "RecordArchive"; } //!< Return a string describing the actual type of the RecordArchive Object
    ~RecordArchive(); //!< Destructor.

    MemoryInitRecord* GetMemoryInitRecord(cuint32 threadId, uint32 size, uint32 elementSize, EMemDataType type) const; //!< Return a MemoryInitRecord object.
    MemoryInitRecord* GetMemoryInitRecord(cuint32 threadId, uint32 size, uint32 elementSize, EMemDataType type, const EMemAccessType memAccessType) const; //!< Return a MemoryInitRecord object.
    void SwapMemoryInitRecords(std::vector<MemoryInitRecord* >& rSwapVec); //!< Swap MemoryInitRecords vector
  protected:
    RecordArchive(const RecordArchive& rOther) : Object(rOther), mCurrentId(0), mRecords() { } //!< Copy constructor
  private:
    mutable uint32 mCurrentId; //!< Incrementing IDs to be assigned to each new Record object.
    mutable std::vector<MemoryInitRecord* > mRecords; //!< Container of all Record objects in the current PE.
  };

}

#endif
