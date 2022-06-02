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
#ifndef Force_DataBlock_H
#define Force_DataBlock_H

#include <vector>

#include "Defines.h"
#include "Object.h"

namespace Force {

  class Generator;
  class VmMapper;
  class GenPageRequest;

  class DataUnit {
  public:
    DataUnit(uint64 value, uint64 size, bool bigEndian) : mValue(value), mSize(size), mBigEndian(bigEndian) {} //!< Most used constructor.
    COPY_CONSTRUCTOR_DEFAULT(DataUnit); //!< Use default copy constructor.
    virtual ~DataUnit() {}
    uint64 Value() { return mValue; }
    uint64 Size() { return mSize; }
    bool BigEndian() { return mBigEndian; }
  private:
    DEFAULT_CONSTRUCTOR_ABSENT(DataUnit); //!< Not used default constructor.
  private:
    uint64 mValue;
    uint64 mSize;
    bool mBigEndian;
  };

  /*!
    \class DataBlock
    \brief manage a list of given data units
   */
  class DataBlock : public Object {
  public:
    Object* Clone() const override;  //!< Return a cloned DataBlock object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the DataBlock object.
    const char* Type() const override { return "DataBlock"; } //!< Return the type of the object

    explicit DataBlock(uint64 align) : Object(), mAlign(align), mSize(0), mData() {} //!< Default constructor.
    ~DataBlock(); //!< Destructor.

    void AddUnit(uint64 value, uint64 size, bool bigEndian);  //!< add data unit with value, size and endian
    uint64 Allocate(Generator* pGen, VmMapper* pTargetMapper, const GenPageRequest* pPageReq = nullptr);  //!< allocate the given data unit list, delete them, and return the start addr of the allocated block
    void Clear();  //!< clear all data units stored to start refresh again

  private:
    DEFAULT_CONSTRUCTOR_ABSENT(DataBlock); //!< Make default constructor absent to prevent it from being called.
    DataBlock(const DataBlock& rOther);  //!< Copy constructor.
  private:
    uint64 mAlign; //!< Alignment.
    uint64 mSize;  //!< Up to date data units size.
    std::vector<DataUnit*> mData;  //!< A list of data units.
  };

}

#endif
