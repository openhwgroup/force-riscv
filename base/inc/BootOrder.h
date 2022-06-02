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
#ifndef Force_BootOrder_H
#define Force_BootOrder_H

#include <list>
#include <string>
#include <vector>

#include "Defines.h"
#include "Enums.h"
#include "Object.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class Register;
  class GenRequest;

  /*!
    \class BootElement
    \brief An element of the boot loading process.
  */
  class BootElement {
  public:
    explicit BootElement(EBootElementActionType actType=EBootElementActionType(0)) : mAction(actType) { }; //!< Constructor with parameter.
    COPY_CONSTRUCTOR_ABSENT(BootElement);
    SUPERCLASS_DESTRUCTOR_DEFAULT(BootElement); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(BootElement);

    virtual const std::string ToString() const { return "BootElement"; } //!< Return a string describing the current state of the object.
    EBootElementActionType ActionType() const { return mAction; } //!< Return Boot element action type.

    virtual Register* GetRegister() const { return nullptr; } //!< Return the pointer of register.
    virtual const std::string RegisterName() const { return ""; } //!< Return the register name.

    template<typename T> T* CastInstance() //<! Templated function so that a derived class can conveniently cast base class to the desired derived class type.
    {
      T* cast_instance = dynamic_cast<T* >(this);
      return cast_instance;
    }
    virtual uint64 Value() const { return 0; } //!< Return the value of boot element.
    virtual void SetValue(uint64 value) { } //!< Set the specific boot value.

  protected:
    EBootElementActionType mAction; //!< Action type.
  };

  /*!
    \class RegisterElement
    \brief A register of the boot loading process.
  */
  class RegisterElement : public BootElement {
  public:
    explicit RegisterElement(Register* pRegister, EBootElementActionType actType=EBootElementActionType::LoadRegister); //!< Constructor with parameters.
    COPY_CONSTRUCTOR_ABSENT(RegisterElement);
    SUBCLASS_DESTRUCTOR_DEFAULT(RegisterElement); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(RegisterElement);

    uint64 Value() const override { return mValue; } //!< Return the value of boot element.
    Register* GetRegister() const override { return mpRegister; } //!< Return the pointer of register.
    const std::string ToString() const override; //!< Return a string describing the current state of the object.
    const std::string RegisterName() const override; //!< Return the register name.
    void SetValue(uint64 value) override { mValue = value; } //!< Set the specific boot value.

  protected:
    Register* mpRegister; //!< Register pointer.
    uint64 mValue; //!< Register boot load value.
  };

  /*!
    \class LargeRegisterElement
    \brief A LargeRegister of the boot loading process.
  */
  class LargeRegisterElement : public RegisterElement {
  public:
    explicit LargeRegisterElement(Register* pRegister); //!< Constructor with parameters.
    ~LargeRegisterElement() { } //!< Destructor.
    const std::string ToString() const override; //!< Return a string describing the current state of the object.
    const std::vector<uint64> Values() const { return mValues; } //!< Return the values of large register.
    void SetIndex(uint32 index) { mIndex = index; mIndexForced = true; } //!< set large register index, it is used for calculate imm offset.
    uint32 ImmOffset() const; //!< Return the imm offset of large register.
  protected:
    std::vector<uint64> mValues; //!< Large register values.
    uint32 mIndex; //!< Large register index values.
    bool mIndexForced; //!< whether index is forced.
  };

  /*!
    \class InstructionBarrierElement
    \brief An element with InstructionBarrier action of the boot loading process.
  */
  class InstructionBarrierElement : public BootElement {
  public:
    InstructionBarrierElement() : BootElement(EBootElementActionType::InstructionBarrier) { } //!< Constructor.
    ~InstructionBarrierElement() { } //!< Destructor.
    const std::string ToString() const override { return "InstructionBarrierElement"; }; //!< Return a string describing the current state of the object.
  };

  class State;

  /*!
    \class BootOrder
    \brief Order BootElements of the boot loading process.
   */
  class BootOrder : public Object {
  public:
    Object * Clone() const override;                         //!<  Clone function for BootOrder object.
    const std::string ToString() const override;   //!< ToString function to quickly review contents of this class
    const char* Type() const override { return "BootOrder"; }    //!< Type function for BootOrder class

    //\section construction_destructor basic functions for a class
    BootOrder () : Object(), mRegisterList(), mpLastGPR(nullptr), mLoadingBaseAddr(0), mLoadingSize(0) { }                         //!< Default constructor.
    ~BootOrder();                         //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(BootOrder);

    virtual void InitiateRegisterList(std::list<Register*>& regList) { }; //!< Initialize for mRegisterList;
    void RegisterLoadingRequests(std::vector<GenRequest*>& rRequests); //!< Action function to load all registers according to the order defined by AdjustOrder().
    void CreateStateElements(State* pState); //!< Create StateElements to load all regsiters according to the order defined by AdjustOrder().
    void JumpToTestStart(std::vector<GenRequest*>& rRequests); //!< Action function to jump to start of the main test.

    const BootElement* LastGPRElement() const { return mpLastGPR; }  //!< access the last GPR boot element information
    virtual void LastRegistersLoadingRequest(std::vector<GenRequest*>& rRequests, bool load_all) { } //!< load the last set of registers
    virtual uint32 GetLastRegistersRequestOffset() { return 0; } //!< return total instr offset of the last set of registers

    virtual void AdjustOrder() { }                              //!< Virtual function that needs to be overriden to provide customized ordering of registers.
    void AssignLastGPR(Register* pReg); //!< Assign last GPR pointer.
    void SetLoadingBaseAddress(uint64 bootLoadingAddr) { mLoadingBaseAddr = bootLoadingAddr; } //!< Set the loading base address.
    virtual uint32 GetLoadingAddressSize() const { return mLoadingSize; } //!< Return the loading memory range size.
  protected:
    BootOrder(const BootOrder& rOther); //!< Copy constructor.
    virtual void AppendInstructionBarrier(std::vector<GenRequest*>& rRequests) { } //!< Append architecturally defined instruction barrier.
  protected:
    std::list<BootElement*> mRegisterList;               //!< vector that holds registers in certain order
    BootElement* mpLastGPR;                              //!< point to last available GPR
    uint64 mLoadingBaseAddr; //!< loading instruction base address.
    uint32 mLoadingSize; //!< total size of loading instruction data.
  };

  bool boot_comparator(const Register* lRegister, const Register* rRegister); //!< Ordering two Register objects by their boot attributes.

}

#endif
