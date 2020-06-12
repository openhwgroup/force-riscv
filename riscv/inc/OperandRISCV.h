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
#ifndef Force_OperandRISCV_H
#define Force_OperandRISCV_H

#include <Operand.h>

namespace Force {

  class BaseOffsetBranchOperandConstraint;

  /*!
    \class BaseOffsetBranchOperand
    \brief Class representing base offset branch operands
  */
  class BaseOffsetBranchOperand : public BranchOperand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(BaseOffsetBranchOperand); //!< Constructor.
    SUBCLASS_DESTRUCTOR_DEFAULT(BaseOffsetBranchOperand); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(BaseOffsetBranchOperand);

    Object* Clone() const override { return new BaseOffsetBranchOperand(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "BaseOffsetBranchOperand"; } //!< Return a string describing the actual type of the Object.

    void Generate(Generator& gen, Instruction& instr) override; //!< Generate operand details.
    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary pre/post amble requests, if any.
    AddressingMode* GetAddressingMode(uint64 alignment=1) const override; //!< Return an AddressingMode instance.
  protected:
    BaseOffsetBranchOperand(const BaseOffsetBranchOperand& rOther) : BranchOperand(rOther) { } //!< Copy constructor

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object.
    void UpdateNoRestrictionTarget(const Instruction& instr) override; //!< Update target address when no-restriction is specified.
    void GenerateWithPreamble(Generator& gen, Instruction& instr) override; //!< Generate with preamble.
    bool GenerateNoPreamble(Generator& gen, Instruction& instr) override; //!< Generate the AddressingOperand using no-preamble approach.
  private:
    void CalculateBaseValueForPreamble(BaseOffsetBranchOperandConstraint* pBranchOprConstr); //!< Determine the value of the base register and update the operand constraint. This method assumes that the target address and offset operand value have already been set.
  };

  /*!
    \class ConditionalBranchOperandRISCV
    \brief Class representing conditional branch instructions for RISCV
  */
  class ConditionalBranchOperandRISCV : public PcRelativeBranchOperand
  {
  public:
    Object* Clone() const override  //!< Return a cloned ConditionalBranchOperandRISCV object of the same type and same contents of the object.
    {
      return new ConditionalBranchOperandRISCV(*this);
    }

    const char* Type() const override { return "ConditionalBranchOperandRISCV"; } //!< Return the type of the ConditionalBranchOperandRISCV object in C string.

    ConditionalBranchOperandRISCV() : PcRelativeBranchOperand() { } //!< Constructor.
    SUPERCLASS_DESTRUCTOR_DEFAULT(ConditionalBranchOperandRISCV); //!< Destructor
    bool IsBranchTaken(const Instruction& instr) const override; //!< Return whether the branch is taken.
    bool IsConditional() const override { return true; } //!< Return true for conditional branch operand.
    BntNode* GetBntNode(const Instruction& instr) const override; //!< Return branch information in a BntNode object.
    void Commit(Generator& gen, Instruction& instr) override; //!< commit branch information
  protected:
    ConditionalBranchOperandRISCV(const ConditionalBranchOperandRISCV& rOther) : PcRelativeBranchOperand(rOther) { } //!< Copy constructor.

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstaint object for ConditionalBranchOperandRISCV.

  };

  /*!
    \class CompressedConditionalBranchOperandRISCV
    \brief Class representing conditional branch instructions for RISCV
  */
  class CompressedConditionalBranchOperandRISCV : public ConditionalBranchOperandRISCV
  {
  public:
    Object* Clone() const override  //!< Return a cloned ConditionalBranchOperandRISCV object of the same type and same contents of the object.
    {
      return new CompressedConditionalBranchOperandRISCV(*this);
    }

    const char* Type() const override { return "CompressedConditionalBranchOperandRISCV"; } //!< Return the type of the CompressedConditionalBranchOperandRISCV object in C string.

    CompressedConditionalBranchOperandRISCV() : ConditionalBranchOperandRISCV() { } //!< Constructor.
    SUBCLASS_DESTRUCTOR_DEFAULT(CompressedConditionalBranchOperandRISCV); //!< Destructor
  protected:
    CompressedConditionalBranchOperandRISCV(const CompressedConditionalBranchOperandRISCV& rOther) : ConditionalBranchOperandRISCV(rOther) { } //!< Copy constructor.

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstaint object for CompressedConditionalBranchOperandRISCV.

  };

  class VectorDataTraits;

  /*!
    \class VectorDataTypeOperand
    \brief Base class of various vector operand data type operands.
  */
  class VectorDataTypeOperand : public ChoicesOperand {
  public:
    Object* Clone() const override  //!< Return a cloned VectorDataTypeOperand object of the same type and same contents of the object.
    {
      return new VectorDataTypeOperand(*this);
    }

    const char* Type() const override { return "VectorDataTypeOperand"; } //!< Return the type of the VectorDataTypeOperand object in C string.

  VectorDataTypeOperand() : ChoicesOperand() { } //!< Constructor.
    ~VectorDataTypeOperand() { } //!< Destructor

    virtual void SetDataTraits(VectorDataTraits& dataTraits) const;
  protected:
    VectorDataTypeOperand(const VectorDataTypeOperand& rOther) //!< Copy constructor.
      : ChoicesOperand(rOther)
    {
    }

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstaint object.
  };

   /*!
    \class VectorLoadStoreOperand
    \brief Base class of vector load-store operands
  */
  class VectorLoadStoreOperand : public BaseOffsetLoadStoreOperand{
  public:
    Object* Clone() const override  //!< Return a cloned object of the same type and same contents of the object.
    {
      return new VectorLoadStoreOperand(*this);
    }

    const char* Type() const override { return "VectorLoadStoreOperand"; } //!< Return the type of the VectorLoadStoreOperand object in C string.

    VectorLoadStoreOperand() : BaseOffsetLoadStoreOperand() { } //!< Constructor.
    ~VectorLoadStoreOperand() { } //!< Destructor

    void Generate(Generator& gen, Instruction& instr) override; //!< Generate BaseOffsetLoadStoreOperand details.
  protected:
    VectorLoadStoreOperand(const VectorLoadStoreOperand& rOther) : BaseOffsetLoadStoreOperand(rOther) { } //!< Copy constructor.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstaint object for BaseOffsetLoadStoreOperand.

  };

  /*!
    \class ConstDataTypeOperand
    \brief Base class of various constant vector operand data type operands.
  */
  class ConstDataTypeOperand : public VectorDataTypeOperand {
  public:
    Object* Clone() const override  //!< Return a cloned ConstDataTypeOperand object of the same type and same contents of the object.
    {
      return new ConstDataTypeOperand(*this);
    }

    const char* Type() const override { return "ConstDataTypeOperand"; } //!< Return the type of the ConstDataTypeOperand object in C string.

    ConstDataTypeOperand() : VectorDataTypeOperand() { } //!< Constructor.
    ~ConstDataTypeOperand() { } //!< Destructor

    void Setup(Generator& gen, Instruction& instr) override { } //!< Especially overriden to do nothing here.
    void Generate(Generator& gen, Instruction& instr) override { mChoiceText = Name(); } //!< Generate method overriden, simply copy Name() to mChoiceText
  protected:
     ConstDataTypeOperand(const ConstDataTypeOperand& rOther) //!< Copy constructor.
      : VectorDataTypeOperand(rOther)
    {
    }
  };

  /*!
    \class RISCMultiVectorRegisterOperand
    \brief Operand class handling number of registers
  */
  class RISCMultiVectorRegisterOperand : public MultiVectorRegisterOperand {
  public:
    Object* Clone() const override //!< Return a cloned RISCMultiVectorRegisterOperand object of the same type and same contents of the object.
    {
      return new RISCMultiVectorRegisterOperand(*this);
    }

    const char* Type() const override { return "RISCMultiVectorRegisterOperand"; } //!< Return the type as C string.

    RISCMultiVectorRegisterOperand() : MultiVectorRegisterOperand(), mDataType() { } //!< Constructor.
    ~RISCMultiVectorRegisterOperand() { } //!< Destructor
    void GetRegisterIndices(uint32 regIndex, ConstraintSet& rRegIndices) const override; //!< Return register indices in a ConstraintSet.
    uint32 NumberRegisters() const override; //!< Return number of registers in the list.
  protected:
    RISCMultiVectorRegisterOperand(const RISCMultiVectorRegisterOperand& rOther) //!< Copy constructor.
      : MultiVectorRegisterOperand(rOther), mDataType(rOther.mDataType)
    {
    }

    const std::string AssemblyText() const override; //!< Return assembly text output of the register list.
    void SetupDataTraits(Generator& gen, Instruction& instr) override; //!< Setup RISCMultiVectorRegisterOperand data traits.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of the appropriate OperandConstraint object.
    const std::string GetNextRegisterName(uint32& indexVar) const; //!< Return the name of the next register in the list.
    ChoicesFilter* GetChoicesFilter(const ConstraintSet* pConstrSet) const override; //!< Return the choices filter.

    std::string mDataType; //!< Data type of the multi vector list in string.
  };

}

#endif
