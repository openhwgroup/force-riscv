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

  /*!
    \class VsetvlVtypeImmediateOperand
    \brief Operand class for VSETVLI vtype immediate operands.
  */
  class VsetvlVtypeImmediateOperand : public SignedImmediateOperand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VsetvlVtypeImmediateOperand);
    SUBCLASS_DESTRUCTOR_DEFAULT(VsetvlVtypeImmediateOperand);
    ASSIGNMENT_OPERATOR_ABSENT(VsetvlVtypeImmediateOperand);

    Object* Clone() const override { return new VsetvlVtypeImmediateOperand(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VsetvlVtypeImmediateOperand"; } //!< Return a string describing the actual type of the Object.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VsetvlVtypeImmediateOperand);

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object.
  };

  /*!
    \class VectorMaskOperand
    \brief Operand class for vector mask bits.
  */
  class VectorMaskOperand : public ChoicesOperand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VectorMaskOperand);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorMaskOperand);
    ASSIGNMENT_OPERATOR_ABSENT(VectorMaskOperand);

    Object* Clone() const override { return new VectorMaskOperand(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VectorMaskOperand"; } //!< Return a string describing the actual type of the Object.

    void Generate(Generator& gen, Instruction& instr) override; //!< Generate operand details.
    void Commit(Generator& gen, Instruction& instr) override; //!< Commit generated operand.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VectorMaskOperand);

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object.
  };

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

    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary pre/post amble requests, if any.
    AddressingMode* GetAddressingMode(uint64 alignment=1) const override; //!< Return an AddressingMode instance.
  protected:
    BaseOffsetBranchOperand(const BaseOffsetBranchOperand& rOther) : BranchOperand(rOther) { } //!< Copy constructor

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object.
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

  /*!
    \class CompressedRegisterOperand
    \brief Class handling operands with weighted choices.
  */
  class CompressedRegisterOperandRISCV : public RegisterOperand {
  public:
    Object* Clone() const override  //!< Return a cloned CompressedRegisterOperand object of the same type and same contents of the object.
    {
      return new CompressedRegisterOperandRISCV(*this);
    }

    const char* Type() const override { return "CompressedRegisterOperandRISCV"; } //!< Return the type of the CompressedRegisterOperandRISCV object in C string.

    CompressedRegisterOperandRISCV() : RegisterOperand() { } //!< Constructor.
    ~CompressedRegisterOperandRISCV() { } //!< Destructor
  protected:
    explicit CompressedRegisterOperandRISCV(const RegisterOperand& rOther) //!< Copy constructor.
      : RegisterOperand(rOther)
    {
    }

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for CompressedRegisterOperandRISCV.
  };

  /*!
    \class VsetvlAvlRegisterOperand
    \brief Operand class for VSETVL and VSETVLI AVL register operands.
  */
  class VsetvlAvlRegisterOperand : public RegisterOperand {
  public:
    VsetvlAvlRegisterOperand();
    SUBCLASS_DESTRUCTOR_DEFAULT(VsetvlAvlRegisterOperand);
    ASSIGNMENT_OPERATOR_ABSENT(VsetvlAvlRegisterOperand);

    Object* Clone() const override { return new VsetvlAvlRegisterOperand(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VsetvlAvlRegisterOperand"; } //!< Return a string describing the actual type of the Object.
    void Generate(Generator& gen, Instruction& instr) override; //!< Generate operand details.
    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary pre/post amble requests, if any.
  protected:
    VsetvlAvlRegisterOperand(const VsetvlAvlRegisterOperand& rOther);

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object.
  private:
    uint64 mAvlRegVal;
  };

  /*!
    \class VsetvlVtypeRegisterOperand
    \brief Operand class for VSETVL vtype register operands.
  */
  class VsetvlVtypeRegisterOperand : public RegisterOperand {
  public:
    VsetvlVtypeRegisterOperand();
    SUBCLASS_DESTRUCTOR_DEFAULT(VsetvlVtypeRegisterOperand);
    ASSIGNMENT_OPERATOR_ABSENT(VsetvlVtypeRegisterOperand);

    Object* Clone() const override { return new VsetvlVtypeRegisterOperand(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VsetvlVtypeRegisterOperand"; } //!< Return a string describing the actual type of the Object.
    void Generate(Generator& gen, Instruction& instr) override; //!< Generate operand details.
    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary pre/post amble requests, if any.
  protected:
    VsetvlVtypeRegisterOperand(const VsetvlVtypeRegisterOperand& rOther);

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object.
  private:
    uint64 mVtypeRegVal;
  };

  /*!
    \class VtypeLayoutOperand
    \brief Operand class for vector register layouts corresponding to vtype.
  */
  class VtypeLayoutOperand : public VectorLayoutOperand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VtypeLayoutOperand);
    SUBCLASS_DESTRUCTOR_DEFAULT(VtypeLayoutOperand);
    ASSIGNMENT_OPERATOR_ABSENT(VtypeLayoutOperand);

    Object* Clone() const override { return new VtypeLayoutOperand(*this); } //!< Return a cloned VtypeLayoutOperand object of the same type and same contents of the object.
    const char* Type() const override { return "VtypeLayoutOperand"; } //!< Return the type of the VtypeLayoutOperand object in C string.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VtypeLayoutOperand);
  private:
    void SetupVectorLayout(const Generator& rGen, const Instruction& rInstr) override; //!< Determine and set the vector layout attributes.
  };

  /*!
    \class CustomLayoutOperand
    \brief Operand class for vector register layouts corresponding to vtype.
  */
  class CustomLayoutOperand : public VectorLayoutOperand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(CustomLayoutOperand);
    SUBCLASS_DESTRUCTOR_DEFAULT(CustomLayoutOperand);
    ASSIGNMENT_OPERATOR_ABSENT(CustomLayoutOperand);

    Object* Clone() const override { return new CustomLayoutOperand(*this); } //!< Return a cloned VtypeLayoutOperand object of the same type and same contents of the object.
    const char* Type() const override { return "CustomLayoutOperand"; } //!< Return the type of the VtypeLayoutOperand object in C string.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(CustomLayoutOperand);
  private:
    void SetupVectorLayout(const Generator& rGen, const Instruction& rInstr) override; //!< Determine and set the vector layout attributes.
  };

  /*!
    \class WholeRegisterLayoutOperand
    \brief Operand class for fixed vector register layouts that read or write the whole vector register.
  */
  class WholeRegisterLayoutOperand : public VectorLayoutOperand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(WholeRegisterLayoutOperand);
    SUBCLASS_DESTRUCTOR_DEFAULT(WholeRegisterLayoutOperand);
    ASSIGNMENT_OPERATOR_ABSENT(WholeRegisterLayoutOperand);

    Object* Clone() const override { return new WholeRegisterLayoutOperand(*this); } //!< Return a cloned WholeRegisterLayoutOperand object of the same type and same contents of the object.
    const char* Type() const override { return "WholeRegisterLayoutOperand"; } //!< Return the type of the WholeRegisterLayoutOperand object in C string.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(WholeRegisterLayoutOperand);
  private:
    void SetupVectorLayout(const Generator& rGen, const Instruction& rInstr) override; //!< Determine and set the vector layout attributes.
  };

  /*!
    \class VectorIndexedLoadStoreOperandRISCV
    \brief Operand for RISCV vector indexed load/store operations.
  */
  class VectorIndexedLoadStoreOperandRISCV : public VectorIndexedLoadStoreOperand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VectorIndexedLoadStoreOperandRISCV);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorIndexedLoadStoreOperandRISCV);
    ASSIGNMENT_OPERATOR_ABSENT(VectorIndexedLoadStoreOperandRISCV);

    Object* Clone() const override { return new VectorIndexedLoadStoreOperandRISCV(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VectorIndexedLoadStoreOperandRISCV"; } //!< Return a string describing the actual type of the Object.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VectorIndexedLoadStoreOperandRISCV);
  private:
    void GetIndexRegisterNames(std::vector<std::string>& rIndexRegNames) const override; //!< Get the names of the index registers.
    void AdjustMemoryElementLayout(const Generator& rGen, const Instruction& rInstr) override; //!< Finalize memory access dimensions based on runtime state.
    Operand* GetDataOperand(const Instruction& rInstr) const; //!< Return data operand.
  };

  /*!
    \class MultiVectorRegisterOperandRISCV
    \brief Operand class handling number of registers
  */
  class MultiVectorRegisterOperandRISCV : public MultiVectorRegisterOperand {
  public:
    Object* Clone() const override //!< Return a cloned MultiVectorRegisterOperandRISCV object of the same type and same contents of the object.
    {
      return new MultiVectorRegisterOperandRISCV(*this);
    }

    const char* Type() const override { return "MultiVectorRegisterOperandRISCV"; } //!< Return the type as C string.

    MultiVectorRegisterOperandRISCV() : MultiVectorRegisterOperand(), mDataType(), mRegCount(0) { } //!< Constructor.
    ~MultiVectorRegisterOperandRISCV() { } //!< Destructor
    void Setup(Generator& gen, Instruction& instr) override; //!< Setup conditions, constraining mechanisms before generating operand.
    void Generate(Generator& gen, Instruction& instr) override; //!< Generate operand details.
    void GetRegisterIndices(uint32 regIndex, ConstraintSet& rRegIndices) const override; //!< Return the register indices in a ConstraintSet, assuming the specified register is chosen.
    void GetChosenRegisterIndices(const Generator& gen, ConstraintSet& rRegIndices) const override; //!< Return the chosen register indices in a ConstraintSet.
    uint32 NumberRegisters() const override; //!< Return number of registers in the list.
  protected:
    MultiVectorRegisterOperandRISCV(const MultiVectorRegisterOperandRISCV& rOther) //!< Copy constructor.
      : MultiVectorRegisterOperand(rOther), mDataType(rOther.mDataType), mRegCount(rOther.mRegCount)
    {
    }

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of the appropriate OperandConstraint object.
    const std::string GetNextRegisterName(uint32& indexVar) const; //!< Return the name of the next register in the list.
    ChoicesFilter* GetChoicesFilter(const ConstraintSet* pConstrSet) const override; //!< Return the choices filter.
    virtual const uint32 GetMinimumRegisterCount(const Instruction& rInstr) { return 1; } //!< Returns the minimum register count.

    std::string mDataType; //!< Data type of the multi vector list in string.
  private:
    void AdjustRegisterCount(const Instruction& rInstr); //!< Finalize register count based on runtime state.

    uint32 mRegCount; //!< The number of registers per vector register group
  };

  /*!
    \class VectorDataRegisterOperand
    \brief Operand class handling number of registers
  */
  class VectorDataRegisterOperand : public MultiVectorRegisterOperandRISCV {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VectorDataRegisterOperand);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorDataRegisterOperand);
    ASSIGNMENT_OPERATOR_ABSENT(VectorDataRegisterOperand);

    Object* Clone() const override { return new VectorDataRegisterOperand(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VectorDataRegisterOperand"; } //!< Return a string describing the actual type of the Object.

  protected:
    COPY_CONSTRUCTOR_DEFAULT(VectorDataRegisterOperand);

    const uint32 GetMinimumRegisterCount(const Instruction& rInstr) override; //<! Returns the minimum register count.
  };

  /*!
    \class VectorIndexedDataRegisterOperand
    \brief Operand for RISCV vector indexed load/store data register operands.
  */
  class VectorIndexedDataRegisterOperand : public VectorDataRegisterOperand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VectorIndexedDataRegisterOperand);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorIndexedDataRegisterOperand);
    ASSIGNMENT_OPERATOR_ABSENT(VectorIndexedDataRegisterOperand);

    Object* Clone() const override { return new VectorIndexedDataRegisterOperand(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VectorIndexedDataRegisterOperand"; } //!< Return a string describing the actual type of the Object.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VectorIndexedDataRegisterOperand);

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object.
  };

}

#endif
