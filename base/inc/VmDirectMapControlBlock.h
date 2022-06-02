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
#ifndef Force_VmDirectMapControlBlock_H
#define Force_VmDirectMapControlBlock_H

#include "Enums.h"
#include "VmControlBlock.h"
#include ARCH_ENUM_HEADER

namespace Force {

  /*!
    \class VmDirectMapControlBlock
    \brief Base class for direct mapping control blocks.
  */
  class VmDirectMapControlBlock : public VmControlBlock {
  public:
    VmDirectMapControlBlock(EPrivilegeLevelType elType, EMemBankType memType); //!< Constructor with EL type and memory bank given.
    ~VmDirectMapControlBlock() override = default; //!< Destructor.
    VmDirectMapControlBlock& operator=(const VmDirectMapControlBlock& rOther) = delete; //!< Copy assignment operator not currently needed.

    Object* Clone() const override { return new VmDirectMapControlBlock(*this); } //!< Clone VmDirectMapControlBlock object.
    const char* Type() const override { return "VmDirectMapControlBlock"; }

    bool Validate(std::string& rErrMsg) const override; //!< Return true only if all initialized context are valid.
  protected:
    VmDirectMapControlBlock(const VmDirectMapControlBlock& rOther) = default; //!< Copy constructor.

    const std::string AdditionalAttributesString() const override; //!< Return addtional attributes description in a string.
  };

}

#endif
