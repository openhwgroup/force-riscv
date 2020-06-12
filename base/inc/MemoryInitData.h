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
#ifndef Force_MemoryInitData_H
#define Force_MemoryInitData_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>

namespace Force {

  class Generator;
  class ChoiceTree;
  class LoadStoreOperand;
  class ConstraintSet;

  /*!
    \class MemoryInitData
    \brief A class to help managing ording load-store target data
  */

  class MemoryInitData {
  public:
    MemoryInitData(uint64 va, uint32 dataSize, uint32 elementSize, EMemAccessType memAccessType);  //!< Constructor with data size parameters.
    ~MemoryInitData();

    ASSIGNMENT_OPERATOR_ABSENT(MemoryInitData);
    COPY_CONSTRUCTOR_DEFAULT(MemoryInitData);
    void Setup(const Generator& gen, const LoadStoreOperand* opr); //!< set up constraint.
    void Setup(const Generator& gen, ConstraintSet* dataTargetConstr); //!< set up with directly-specified constraint.
    void Commit(const Generator& gen); //!< Commit data to generator.
  private:
    void SetupChoices(const Generator& gen); //!< Configure relevant choices.
    void InitializeMemory(const Generator& gen); //!< Initialize memory data.
    void GenerateInitData(const Generator& gen, uint8* memData, uint8* memAttrs) const; //!< Generate initialization data.
    uint64 GetMemoryData(const Generator& gen, cuint8* memData, cuint32 dataSize) const; //!< Get existing memory data for no more than 8 bytes, starting at the specified address.
    uint64 GetMemoryAttributes(const Generator& gen, cuint8* memAttrs, cuint32 dataSize) const; //!< Get memory attributes for no more than 8 bytes, starting at the specified address.
    uint64 GetInitializedMask(cuint64 memAttrs) const; //!< Return a mask where each byte that is initialized is 0xFF and all other bytes are 0x00.
    uint64 GenerateInitDataBlock(const Generator& gen, cuint64 size, cuint64 initMask, cuint64 currentData) const; //!< Generate a block of initialization data no larger than 8 bytes.
    uint64 GenerateTargetConstrainedData(const Generator& gen, cuint64 size, cuint64 initMask, cuint64 currentData, cbool isInstr, EMemAccessType memAccessType) const; //!< generate target data.
    uint64 GenerateRandomDataWithSize(cuint64 size, cuint64 initMask, cuint64 currentData) const; //!< generate random data with size in bytes.
  private:
    uint64 mVa; //!< Initialization target virtual address.
    uint32 mDataSize; //!< Data size.
    uint32 mElementSize; //!< Element size.
    EMemAccessType mMemAccessType; //!< memory access type
    const ChoiceTree *mpDataChoices; //!< data choices tree
    ConstraintSet* mpDataTargetConstr; //!< The pointer to data target constraint
    mutable uint32 mCurrentDataIndex; //!< current data index.
    std::vector<ConstraintSet* > mDataConstraints; //!< Data of addressing target constraint.
  };

}

#endif
