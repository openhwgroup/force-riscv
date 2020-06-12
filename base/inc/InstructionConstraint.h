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
#ifndef Force_InstructionConstraint_H
#define Force_InstructionConstraint_H

#include <Defines.h>

namespace Force {

  class Generator;
  class Instruction;
  class InstructionStructure;
  class GenInstructionRequest;
  class ResourceAccessStage;

  /*!
    \class InstructionConstraint
    \brief Base class for various InstructionConstraint sub classes.
  */
  class InstructionConstraint {
  public:
    InstructionConstraint() : mpInstructionRequest(nullptr),  mpHotResource(nullptr){ } //!< Constructor.
    virtual ~InstructionConstraint(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(InstructionConstraint);

    virtual void Setup(const Generator& gen, const Instruction& instr, const InstructionStructure& instructionStruct); //!< Setup dynamic instruction constraints.

    /*!
      Templated function so that a derived class can conveniently cast base class to the desired derived class type.
      For example, LoadStoreInstructionConstraint* lsi_constraint = instr_constraint->CastInstance<LoadStoreInstructionConstraint>();
     */
    template<typename T>
      T* CastInstance()
      {
        T* cast_instance = dynamic_cast<T* >(this);
        return cast_instance;
      }

    void SetInstructionRequest(const GenInstructionRequest* instrReq) { mpInstructionRequest = instrReq; } //!< Set pointer to associated GenInstructionRequest object.
    const GenInstructionRequest* InstructionRequest() const { return mpInstructionRequest; } //!< Return const pointer to the GenInstructionRequest object associtated with the instruction.
    ResourceAccessStage* GetHotResource() const {return mpHotResource;} //!< get hot resource
    ResourceAccessStage* GiveHotResource() { auto pResource = mpHotResource; mpHotResource = nullptr; return pResource;}
  protected:
    InstructionConstraint(const InstructionConstraint& rOther); //!< Copy constructor, not meant to be used.
  protected:
    const GenInstructionRequest* mpInstructionRequest; //!< Const pointer to the GenInstructionRequest object that originated the instruction.
    ResourceAccessStage *mpHotResource; //!< pointer to hot resource
  };

  class BranchOperand;

  /*!
    \class BranchInstructionConstraint
    \brief Class holding constraints needed by branch instruction generation.
  */
  class BranchInstructionConstraint : public InstructionConstraint {
  public:
    BranchInstructionConstraint() : InstructionConstraint(), mpBranchOperand(nullptr) { } //!< Constructor.
    ~BranchInstructionConstraint(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(BranchInstructionConstraint);

    void Setup(const Generator& gen, const Instruction& instr, const InstructionStructure& instructionStruct) override; //!< Setup dynamic branch instruction constraints.
    const BranchOperand* GetBranchOperand() const { return mpBranchOperand; } //!< Return a constant pointer to the BranchOperand that belongs to the Instruction object.
  protected:
    BranchInstructionConstraint(const BranchInstructionConstraint& rOther); //!< Copy constructor, not meant to be used.
  protected:
    const BranchOperand* mpBranchOperand; //!< Pointer to branch operand.
  };

  class LoadStoreOperand;

  /*!
    \class LoadStoreInstructionConstraint
    \brief Class holding constraints needed by load-store instruction generation.
  */
  class LoadStoreInstructionConstraint : public virtual InstructionConstraint {
  public:
    LoadStoreInstructionConstraint() : InstructionConstraint(), mpLoadStoreOperand(nullptr) { } //!< Constructor.
    ~LoadStoreInstructionConstraint(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(LoadStoreInstructionConstraint);

    void Setup(const Generator& gen, const Instruction& instr, const InstructionStructure& instructionStruct) override; //!< Setup dynamic load-store instruction constraints.
    const LoadStoreOperand* GetLoadStoreOperand() const { return mpLoadStoreOperand; } //!< Return a constant pointer to the LoadStoreOperand that belongs to the Instruction object.
  protected:
    LoadStoreInstructionConstraint(const LoadStoreInstructionConstraint& rOther); //!< Copy constructor, not meant to be used.
  protected:
    const LoadStoreOperand* mpLoadStoreOperand; //!< Pointer to load-store operand.
  };

}

#endif
