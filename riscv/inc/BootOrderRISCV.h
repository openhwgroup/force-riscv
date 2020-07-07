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
#ifndef Force_BootOrderRISCV_H
#define Force_BootOrderRISCV_H

#include <BootOrder.h>

namespace Force {

  class Register;

  /*!
    \class BootOrderRISCV
    \brief Class processing BootElement ordering in RISCV
  */
  class BootOrderRISCV : public BootOrder {
  public:
    Object * Clone() const override;                         //!<  Clone function for BootOrderRISCV object.
    const char* Type() const override { return "BootOrderRISCV"; }    //!< Type function for BootOrderRISCV class
    //BootOrderRISCV(): BootOrder(), mUseEret(false) {} //!< default constructor
    BootOrderRISCV() : BootOrder() {} //!< default constructor
    ~BootOrderRISCV();                    //!< destructor
    ASSIGNMENT_OPERATOR_ABSENT(BootOrderRISCV);
    void InitiateRegisterList(std::list<Register*>& regList) override; //!< Initialize for mRegisterList;
    void AdjustOrder() override;         //!< override the base function to adjust the order for RISCV architecture.
    void LastRegistersLoadingRequest(std::vector<GenRequest*>& rRequests, bool load_all) override; //!< load the last set of registers
    uint32 GetLastRegistersRequestOffset() override; //!< return total instr offset of the last set of registers
    //bool UseEret() const { return mUseEret; } //!< Return whether to use ERET at the end of boot code.
  protected:
    BootOrderRISCV(const BootOrderRISCV& rOther); //!< Copy constructor.
    //void AppendInstructionBarrier(std::vector<GenRequest*>& rRequests) override; //!< Append instruction barrier.
  private:
    void InsertSystemRegister(const std::list<BootElement*>::iterator& rBootElemItr, size_t& numSysRegs); //!< Move the specified system register boot element into the appropriate place in the boot element list.
  };

}

#endif
