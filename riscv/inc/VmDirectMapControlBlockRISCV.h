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
#ifndef Force_VmDirectMapControlBlockRISCV_H
#define Force_VmDirectMapControlBlockRISCV_H

#include "VmDirectMapControlBlock.h"

namespace Force {

  /*!
    \class VmDirectMapControlBlockRISCV
    \brief Base class for RISC-V direct mapping control blocks.
  */
  class VmDirectMapControlBlockRISCV : public VmDirectMapControlBlock
  {
  public:
    VmDirectMapControlBlockRISCV(EPrivilegeLevelType elType, EMemBankType memType); //!< Constructor with EL type given.
    ~VmDirectMapControlBlockRISCV() override = default; //!< Destructor.
    VmDirectMapControlBlockRISCV& operator=(const VmDirectMapControlBlockRISCV& rOther) = delete; //!< Copy assignment operator not currently needed.

    Object* Clone() const override { return new VmDirectMapControlBlockRISCV(*this); } //!< Clone VmDirectMapControlBlockRISCV object.
    const char* Type() const override { return "VmDirectMapControlBlockRISCV"; }

    void Setup(Generator* pGen) override; //!< Setup VM Context Parameters.
    void Initialize() override; //!< Initialize VmControlBlock object.
  protected:
    VmDirectMapControlBlockRISCV(const VmDirectMapControlBlockRISCV& rOther) = default; //!< Copy constructor.
    uint64 GetMaxPhysicalAddress() const override; //!< Return initial maximum physical address.
  };

}

#endif
