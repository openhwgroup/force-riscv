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
#ifndef Force_InstructionRISCV_H
#define Force_InstructionRISCV_H

#include <Instruction.h>

namespace Force {

  /*!
    \class VectorInstruction
    \brief Base class for instructions.
  */
  class VectorInstruction : public virtual Instruction {
  public:
    Object* Clone() const override { return new VectorInstruction(*this); } //!< Return a cloned VectorInstruction object of the same type and same contents of the object.
    const char* Type() const override { return "VectorInstruction"; } //!< Return the type of the VectorInstruction object in C string.

    VectorInstruction() : Instruction() { } //!< Constructor
    ~VectorInstruction() { } //!< Destructor.

    void Setup(const GenInstructionRequest& instrReq, Generator& gen) override; //!< Setup conditions, constrainting mechanisms before generation instruction.
  protected:
    VectorInstruction(const VectorInstruction& rOther); //!< Copy constructor.
    InstructionConstraint* InstantiateInstructionConstraint() const override; //!< Return an instance of appropriate InstructionConstaint object.
    void LocateDataTypeOperand(); //!< Locate VectorInstruction's data-type operand.
  };

  /*!
    \class VectorLoadStoreInstruction
    \brief derived class for vector load store instructions.
  */
  class VectorLoadStoreInstruction : public VectorInstruction, public LoadStoreInstruction {
  public:
    Object* Clone() const override { return new VectorLoadStoreInstruction(*this); } //!< Return a cloned object of the same type and same contents of the object.
    const char* Type() const override { return "VectorLoadStoreInstruction"; } //!< Return the type of the VectorInstruction object in C string.

  VectorLoadStoreInstruction() : Instruction(), VectorInstruction(), LoadStoreInstruction() { } //!< Constructor
    ~VectorLoadStoreInstruction() { } //!< Destructor.

  protected:
    VectorLoadStoreInstruction(const VectorLoadStoreInstruction& rOther) : VectorInstruction(rOther), LoadStoreInstruction(rOther) { } //!< Copy constructor.
    InstructionConstraint* InstantiateInstructionConstraint() const override; //!< Return an instance of appropriate InstructionConstaint object.
  };

  /*!
    \class RetInstruction
    \brief class for RET instructions.
  */
  class RetInstruction : public Instruction {
  public:
    Object* Clone() const override { return new RetInstruction(*this); } //!< Return a cloned RetInstruction object of the same type and same contents of the object.
    const char* Type() const override { return "RetInstruction"; } //!< Return the type of the RetInstruction object in C string.

    RetInstruction() : Instruction() { } //!< Constructor
    ~RetInstruction() { } //!< Destructor.

    void Setup(const GenInstructionRequest& instrReq, Generator& gen) override; //!< Setup conditions, constrainting mechanisms before generation instruction.
    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return preamble requests if there is any.
  protected:
    RetInstruction(const RetInstruction& rOther) : Instruction(rOther) { } //!< Copy constructor.
    InstructionConstraint* InstantiateInstructionConstraint() const override; //!< Return an instance of appropriate InstructionConstaint object.
  };

}

#endif //Force_InstructionRISCV_H
