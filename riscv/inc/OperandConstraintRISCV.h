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
#ifndef Force_OperandConstraintRISCV_H
#define Force_OperandConstraintRISCV_H

#include <Operand.h>
#include <OperandConstraint.h>

namespace Force {
  class ConstraintSet;

  /*!
    \class BaseOffsetBranchOperandConstraint
    \brief This class carries dynamic constraint properties for BaseOffsetBranchOperand.
  */
  class BaseOffsetBranchOperandConstraint : public BranchOperandConstraint {
  public:
    BaseOffsetBranchOperandConstraint() : BranchOperandConstraint(), mpBase(nullptr), mpOffset(nullptr), mBaseValue(0) { } //!< Constructor.
    COPY_CONSTRUCTOR_ABSENT(BaseOffsetBranchOperandConstraint);
    SUBCLASS_DESTRUCTOR_DEFAULT(BaseOffsetBranchOperandConstraint); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(BaseOffsetBranchOperandConstraint);

    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints.
    RegisterOperand* BaseOperand() const override { return mpBase; } //!< Return pointer to base operand if applicable.
    ImmediateOperand* OffsetOperand() const override { return mpOffset; } //!< Return pointer to immediate offset operand if applicable.
    void GetRegisterOperands(std::vector<const RegisterOperand*>& rRegOps) override { rRegOps.push_back(mpBase); } //!< Return pointer to immediate offset operand if applicable.
    void SetBaseValue(cuint64 baseValue) { mBaseValue = baseValue; } //!< Set base value.
    uint64 BaseValue() const { return mBaseValue; } //!< Return base value.
  private:
    RegisterOperand* mpBase; //!< Base sub operand
    ImmediateOperand* mpOffset; //!< Offset sub operand
    uint64 mBaseValue; //!< Value for base register
  };

  /*!
    \class ConditionalBranchOperandRISCVConstraint
    \brief The class carries dynamic constraint properties for ConditionalBranchOperandRISCV.
  */
  class ConditionalBranchOperandRISCVConstraint : public PcRelativeBranchOperandConstraint {
  public:
  ConditionalBranchOperandRISCVConstraint() : PcRelativeBranchOperandConstraint(), mTaken(false) { } //!< Constructor.
    virtual ~ConditionalBranchOperandRISCVConstraint() { } //!< Destructor.

    void Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct) override; //!< Setup dynamic operand constraints for ConditionalBranchOperandRISCV.
    bool BranchTaken() const { return mTaken; } //!< Return true if branch is taken.
    virtual void SetConditionalBranchTaken(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct) = 0; //!< set condition branch taken or not taken by the current regsiter context
  protected:
    ConditionalBranchOperandRISCVConstraint(const ConditionalBranchOperandRISCVConstraint& rOther) //!< Copy constructor, not meant to be used.
      : PcRelativeBranchOperandConstraint(rOther), mTaken(false) { }
    
  protected:
    bool mTaken; //!< Indicate if branch is taken.
  };

  /*!
    \struct VectorDataTraits
    \brief Data traits of SIMD vector
  */
  struct VectorDataTraits {
    std::string mTypeName; //!< Data type name.
    uint32 mLanes; //!< Number of lanes.
    uint32 mElementSize; //!< Element size in bytes.
    uint32 mVectorSize; //!< Total vector size in bytes.
    bool mIsFP; //!< Is data in floating point format.

    VectorDataTraits() : mTypeName(), mLanes(0), mElementSize(0), mVectorSize(0), mIsFP(false) { } //!< Constructor.
    void SetTraits(const std::string& choiceText); //!< Set vector data traits given the choice name.
  };

  class RISCVectorRegisterOperandConstraint : public VectorRegisterOperandConstraint {
  public:
    RISCVectorRegisterOperandConstraint() : VectorRegisterOperandConstraint(), mpDataTraits(nullptr) { } //!< Constructor.
    ~RISCVectorRegisterOperandConstraint() { } //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(RISCVectorRegisterOperandConstraint);
    void Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct) override; //!< Setup dynamic operand constraints.
    void SetDataTraits(const VectorDataTraits* vecDataTraits) { mpDataTraits = vecDataTraits; } //!< Set pointer to data traits object.
    const VectorDataTraits* DataTraits() const { return mpDataTraits; } //!< Return pointer to data traits object.
  protected:
    RISCVectorRegisterOperandConstraint(const RISCVectorRegisterOperandConstraint& rOther) //!!< Copy constructor, not meant to be used.
      : VectorRegisterOperandConstraint(rOther), mpDataTraits(nullptr)
    {
    }

    const VectorDataTraits* mpDataTraits; //!< Pointer to vector data traits object.
  };

  /*!
    \class VectorDataTypeOperandConstraint
    \brief The class carries dynamic constraint properties for VectorDataTypeOperand.
  */
  class VectorDataTypeOperandConstraint : public ChoicesOperandConstraint {
  public:
    VectorDataTypeOperandConstraint() : ChoicesOperandConstraint() { } //!< Constructor.
    ~VectorDataTypeOperandConstraint() { } //!< Destructor.
  protected:
    VectorDataTypeOperandConstraint(const VectorDataTypeOperandConstraint& rOther) : ChoicesOperandConstraint(rOther) { } //!< Copy constructor, not meant to be used.
  };

  /**!
    \class CompressedConditionalBranchOperandConstraint
    \brief Implements special behavior in SetConditionalBranchTaken(...) and branch type specific methods for the Compressed extension branch instructions.
  */
  class CompressedConditionalBranchOperandConstraint : public ConditionalBranchOperandRISCVConstraint {
  public:
    CompressedConditionalBranchOperandConstraint() : ConditionalBranchOperandRISCVConstraint() {} //!< Constructor
    ~CompressedConditionalBranchOperandConstraint() {} //!< Destructor

    void SetConditionalBranchTaken(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct) override; //!< set condition branch taken or not taken by the current regsiter context

  protected:
    void SetBranchTakenForCBEQZ(uint64 rs1Val); //!< Branch if rs1 is equal zero
    void SetBranchTakenForCBNEZ(uint64 rs1Val); //!< Branch if rs1 is not equal zero
  };

  /**!
    \class FullsizeConditionalBranchOperandConstraint
    \brief Implements special behavior in SetConditionalBranchTaken(...) and branch type speific methods for the RV64 full sized branch instructions. 
  */
  class FullsizeConditionalBranchOperandConstraint : public ConditionalBranchOperandRISCVConstraint {
  public:
    FullsizeConditionalBranchOperandConstraint() : ConditionalBranchOperandRISCVConstraint() {} //!< Constructor
    ~FullsizeConditionalBranchOperandConstraint() {} //!< Destructor

    void SetConditionalBranchTaken(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct) override; //!< set condition branch taken or not taken by the current regsiter context
  
  protected:
    void SetBranchTakenForBEQ(uint64 rs1Val, uint64 rs2Val); //!< Branch if rs1 and rs2 are equal
    void SetBranchTakenForBNE(uint64 rs1Val, uint64 rs2Val); //!< Branch if rs1 is not equal to rs2                                                     
    void SetBranchTakenForBLT(int64 rs1Val, int64 rs2Val); //!< Branch if rs1 is less than rs2, interpreting them as signed values                      
    void SetBranchTakenForBLTU(uint64 rs1Val, uint64 rs2Val); //!< Branch if rs1 is less than rs2, interpreting them as unsigned values                 
    void SetBranchTakenForBGE(int64 rs1Val, int64 rs2Val); //!< Branch if rs1 is greater than or equal to rs2, interpreting them as signed values
    void SetBranchTakenForBGEU(uint64 rs1Val, uint64 rs2Val); //!< Branch if rs1 is greater than or equal to rs2, interpreting them as unsigned values}
  };

   /*!
    \class VectorLoadStoreOperandConstraint
    \brief The class carries dynamic constraint properties for VectorLoadStoreOperand.
  */
  class VectorLoadStoreOperandConstraint : public BaseOffsetLoadStoreOperandConstraint {
  public:
    VectorLoadStoreOperandConstraint() : BaseOffsetLoadStoreOperandConstraint(), mpVectorDataType(nullptr), mpMultiRegisterOperand(nullptr) { } //!< Constructor.
    ~VectorLoadStoreOperandConstraint() { mpVectorDataType = nullptr; mpMultiRegisterOperand = nullptr; } //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(VectorLoadStoreOperandConstraint);
    void Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct) override; //!< Setup dynamic operand constraints for BaseOffsetLoadStoreOperand
    const ChoicesOperand* VectorDataTypeOperand() const { return mpVectorDataType; } //!< Return const pointer to vector data type operand.
    const MultiRegisterOperand* GetMultiRegisterOperand() const {return mpMultiRegisterOperand;}
  protected:
    VectorLoadStoreOperandConstraint(const VectorLoadStoreOperandConstraint& rOther) //!< Copy constructor, not meant to be used.
      : BaseOffsetLoadStoreOperandConstraint(rOther), mpVectorDataType(nullptr), mpMultiRegisterOperand(nullptr) { }
  protected:
    const ChoicesOperand* mpVectorDataType; //!< Pointer to vector data type operand, if applicable.
    const MultiRegisterOperand* mpMultiRegisterOperand;
  };

}

#endif
