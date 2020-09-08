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
#ifndef Force_AddressSolver_H
#define Force_AddressSolver_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <Object.h>
#include <OperandSolution.h>
#include <OperandSolutionMap.h>
#include <UopInterface.h>
#include <vector>

namespace Force {

  class Register;
  class AddressSolvingShared;
  class ConstraintSet;
  class BaseIndexSolvingShared;
  class BaseIndexAmountBitSolvingShared;

  /*!
    \class AddressingRegister
    \brief Class for representing addressing register.
  */
  class AddressingRegister : public Object {
  public:
    const std::string ToString() const override; //!< Return a string describing the current state of the Operand object.
    const char* Type() const override { return "AddressingRegister"; } //!< Return object type.
    Object* Clone() const override; //!< Clone AddressingRegister object.

    AddressingRegister(); //!< Default constructor.
    ASSIGNMENT_OPERATOR_ABSENT(AddressingRegister);
    inline void SetRegister(const Register* pReg) { mpRegister = pReg; } //!< Set addressing register.
    inline const Register* GetRegister() const { return mpRegister; } //!< Return pointer to register register.
    void SetWeight(uint32 weight) { mWeight = weight; } //!< Set solution weight.
    uint32 Weight() const { return mWeight; } //!< Return solution weight.
    inline void SetRegisterValue(uint64 value) { mRegisterValue = value; } //!< Set register value.
    inline uint64 RegisterValue() const { return mRegisterValue; } //!< Return register value.
    inline void SetRegisterValue(const std::vector<uint64>& values) { mRegisterValues = values; } //!< Set large register values.
    inline std::vector<uint64> RegisterValues() const { return mRegisterValues; } //!< Return large register values.
    inline uint32& VmTimeStampReference() const { return mVmTimeStamp; } //!< Return reference to the time stamp variable.
    void SetFree(bool isFree) { mFree = isFree; } //!< Set indication if the register is a free resource.
    bool IsFree() const { return mFree; } //!< Return if the register is a free resource.
    virtual ~AddressingRegister() { } //!< Destructor.
  protected:
    AddressingRegister(const AddressingRegister& rOther); //!< Copy constructor.
    bool IsNonSystemRegisterOperand(const Operand& rOpr) const; //!< Return true if operand is a non-system register operand, e.g. a GPR operand.
  protected:
    const Register* mpRegister; //!< Pointer to addressing register.
    uint64 mRegisterValue; //!< Register value.
    std::vector<uint64> mRegisterValues; //!< Large Register value.
    uint32 mWeight; //!< Solution weight.
    mutable uint32 mVmTimeStamp; //!< Time stamp to track VM constraint updates.
    bool mFree; //!< Indicate if the address solution is using free base register.
  };

  class BaseOffsetConstraint;
  class RegisterOperand;

  /*!
    \class AddressingMode
    \brief Base class for various addressing mode for address solving.
  */
  class AddressingMode : public AddressingRegister {
  public:
    const std::string ToString() const override; //!< Return a string describing the current state of the Operand object.
    const char* Type() const override { return "AddressingMode"; } //!< Return object type.
    Object* Clone() const override = 0; //!< Clone AddressingMode object.

    AddressingMode(); //!< Default constructor.
    void SetBase(const Register* pReg) { SetRegister(pReg); } //!< Set base register.
    const Register* Base() const { return GetRegister(); } //!< Return pointer to base register.
    void SetBaseValue(uint64 value) { SetRegisterValue(value); } //!< Set base value.
    inline uint64 BaseValue() const { return mRegisterValue; } //!< Return base value.
    void SetBaseValue(const std::vector<uint64>& values) { SetRegisterValue(values); } //!< Set base values for large register.
    inline std::vector<uint64> BaseValues() const { return mRegisterValues; } //!< Return base values for large register.
    uint64 TargetAddress() const { return mTargetAddress; } //!< Return target address.
    virtual ~AddressingMode() { } //!< Destructor.
    virtual bool BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const; //!< Check if base value is usable.
    virtual bool Solve(const AddressSolvingShared& rShared) { return false; } //!< Solve for address.
    virtual bool SolveFree(const AddressSolvingShared& rShared) { return false; } //!< Solve for address.
    virtual bool HasOffset() const { return false; } //!< Return false for has-offset query by default.
    virtual uint32 OffsetScale() const { return 0; } //!< Return offset scale if applicable.
    virtual bool ShouldApplyIndexFilters() const { return false; } //!< Return whether filters applying to index registers should be applied.
    virtual AddressSolvingShared* AddressSolvingSharedInstance() const; //!< Return correct AddressSolvingShared object for the addressing mode.
    virtual bool ChooseSolution(const AddressSolvingShared& rAddrSolShared) { return true; } //!< Choose sub solution choice if necessary.
    virtual void NonSystemRegisterOperands(std::vector<const RegisterOperand*>& rRegOperands) const { } //!< Return list of non-system register operands.
    virtual void SetRegisterOperandChoice(const std::string& oprName, Register* regPtr) { } //!< Specify a register to use for the indicated operand.
    virtual void SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const; //!< Update operands to reflect chosen solution.
    virtual uint64 AdjustOffset(uint64 mOffset) const{ return mOffset;}
    virtual uint64 IndexValue() const{ return 0;} //!< Return index value.
    virtual uint32 AmountBit() const{ return 1;} //!< Return amount bit value.
  protected:
    AddressingMode(const AddressingMode& rOther); //!< Copy constructor.
    bool SolveWithValue(uint64 value, const AddressSolvingShared& rShared, const ConstraintSet* pTargetConstr, uint64& rTargetAddr) const; //!< Solve address with value given.
    bool SolveWithBase(cuint64 baseValue, const AddressSolvingShared& rShared, const BaseOffsetConstraint& rBaseOffsetConstr, const ConstraintSet* pTargetConstr, uint64& rTargetAddr) const; //!< Solve address with base value given.
  protected:
    mutable uint64 mTargetAddress; //!< Target address.
  private:
    virtual bool ChooseTargetAddress(const AddressSolvingShared& rShared, ConstraintSet& rConstrSet, uint64& rTargetAddr) const; //!< Select target address from constrained set of possibilities.
  };

  /*!
    \class BaseOnlyMode
    \brief Address solving class for addressing mode: base only.
  */
  class BaseOnlyMode : public AddressingMode {
  public:
    const char* Type() const override { return "BaseOnlyMode"; } //!< Return BaseOnlyMode object type.
    Object* Clone() const override; //!< Clone BaseOnlyMode object.

    BaseOnlyMode() : AddressingMode() { } //!< Default constructor.
    ~BaseOnlyMode() { } //!< Destructor.

    bool Solve(const AddressSolvingShared& rShared) override; //!< Solve for base only mode address.
    bool SolveFree(const AddressSolvingShared& rShared) override; //!< Solve free for base only mode address.
  protected:
    BaseOnlyMode(const BaseOnlyMode& rOther) : AddressingMode(rOther) { } //!< Copy constructor.
  };

  /*!
    \class BaseOnlyAlignedMode
    \brief Address solving class for addressing mode: base only aligned mode.
  */
  class BaseOnlyAlignedMode : public BaseOnlyMode {
  public:
    const char* Type() const override { return "BaseOnlyAlignedMode"; } //!< Return BaseOnlyAlignedMode object type.
    Object* Clone() const override; //!< Clone BaseOnlyAlignedMode object.

    explicit BaseOnlyAlignedMode(uint64 baseAlign); //!< Constructor with base alignment parameter.
    ~BaseOnlyAlignedMode() { } //!< Destructor.

    bool Solve(const AddressSolvingShared& rShared) override; //!< Solve for base only aligned mode address.
    bool SolveFree(const AddressSolvingShared& rShared) override; //!< Solve base only aligned mode with free base register.
    bool BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const override { return ((baseValue & mBaseMask) == baseValue); } //!< Check if base value is usable.
  protected:
    BaseOnlyAlignedMode() : BaseOnlyMode(), mBaseMask(0) { } //!< Default constructor.
    BaseOnlyAlignedMode(const BaseOnlyAlignedMode& rOther) : BaseOnlyMode(rOther), mBaseMask(rOther.mBaseMask) { } //!< Copy constructor.
    virtual uint32 GetRandomLowerBits(const AddressSolvingShared& rShared) const; //!< Return randomized lower bits.
  protected:
    uint64 mBaseMask; //!< Mask for checking if base is useable.
  };

  /*!
    \class RegisterBranchMode
    \brief Address solving class for addressing mode: register branch.
  */
  class RegisterBranchMode : public BaseOnlyAlignedMode {
  public:
    const char* Type() const override { return "RegisterBranchMode"; } //!< Return RegisterBranchMode object type.
    Object* Clone() const override { return new RegisterBranchMode(*this); } //!< Clone RegisterBranchMode object.

    explicit RegisterBranchMode(uint64 baseAlign) : BaseOnlyAlignedMode(baseAlign) { } //!< Constructor with base alignment parameter.
    ~RegisterBranchMode() { } //!< Destructor.
    AddressSolvingShared* AddressSolvingSharedInstance() const override; //!< Return correct AddressSolvingShared object for the register-branch addressing mode.
  protected:
    RegisterBranchMode() : BaseOnlyAlignedMode() { } //!< Default constructor.
    RegisterBranchMode(const RegisterBranchMode& rOther) : BaseOnlyAlignedMode(rOther) { } //!< Copy constructor.
  };

  /*!
    \class UnalignedRegisterBranchMode
    \brief Address solving class for addressing mode: unaligned register branch.
  */
  class UnalignedRegisterBranchMode : public RegisterBranchMode {
  public:
    const char* Type() const override { return "UnalignedRegisterBranchMode"; } //!< Return UnalignedRegisterBranchMode object type.
    Object* Clone() const override { return new UnalignedRegisterBranchMode(*this); } //!< Clone UnalignedRegisterBranchMode object.

    explicit UnalignedRegisterBranchMode(uint64 baseAlign) : RegisterBranchMode(baseAlign) { } //!< Constructor with base alignment parameter.
    ~UnalignedRegisterBranchMode() { } //!< Destructor.
    bool BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const override; //!< Check if base value is usable for unaligned branch.
  protected:
    UnalignedRegisterBranchMode() : RegisterBranchMode() { } //!< Default constructor.
    UnalignedRegisterBranchMode(const UnalignedRegisterBranchMode& rOther) : RegisterBranchMode(rOther) { } //!< Copy constructor.
    uint32 GetRandomLowerBits(const AddressSolvingShared& rShared) const override; //!< Return randomized lower bits for unaligned branch.
  };

  /*!
    \class BaseOffsetMode
    \brief Address solving class for addressing mode: base with immediate offset.
  */
  class BaseOffsetMode : public AddressingMode {
  public:
    const char* Type() const override { return "BaseOffsetMode"; } //!< Return BaseOffsetMode object type.
    Object* Clone() const override; //!< Clone BaseOffsetMode object.

    BaseOffsetMode() : AddressingMode() { } //!< Default constructor.
    ~BaseOffsetMode() { } //!< Destructor.

    bool BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const override { return true; } //!< Check if base value is usable.
    bool Solve(const AddressSolvingShared& rShared) override; //!< Solve for address.
    bool SolveFree(const AddressSolvingShared& rShared) override; //!< Solve for address with free base register.
    bool HasOffset() const override { return true; } //!< Return true for has-offset query.
    AddressSolvingShared* AddressSolvingSharedInstance() const override; //!< Return correct AddressSolvingShared object for the base-offset addressing mode.
    void SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const override; //!< Update operands to reflect chosen solution.
    virtual uint32 calculate_offset_value(uint64 offset, uint32 scale, uint32 mask) const {return (offset & mask);};
    bool SolveOffsetHasConstraint(const AddressSolvingShared* rShared);//! Solve address when offset and base are not free.
  protected:
    BaseOffsetMode(const BaseOffsetMode& rOther) : AddressingMode(rOther) { } //!< Copy constructor.
  };

  /*!
    \class BaseOffsetShiftMode
    \brief Address solving class for addressing mode: base with scaled immediate offset.
  */
  class BaseOffsetShiftMode : public BaseOffsetMode {
  public:
    const char* Type() const override { return "BaseOffsetShiftMode"; } //!< Return BaseOffsetShiftMode object type.
    Object* Clone() const override; //!< Clone BaseOffsetShiftMode object.

    explicit BaseOffsetShiftMode(uint32 offsetScale); //!< Constructor with offset scale parameter given.
    ~BaseOffsetShiftMode() { } //!< Destructor.

    bool BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const override; //!< Check if base value is usable.
    uint32 OffsetScale() const override { return mOffsetScale; } //!< Return offset scale if any.
    AddressSolvingShared* AddressSolvingSharedInstance() const override; //!< Return correct AddressSolvingShared object for the base with scaled offset addressing mode.
    uint64 AdjustOffset(uint64 mOffset) const override { return mOffset <<= mOffsetScale;}
    uint32 calculate_offset_value(uint64 offset, uint32 scale, uint32 mask) const override;
  protected:
    BaseOffsetShiftMode() : BaseOffsetMode(), mOffsetScale(0) { } //!< Default constructor.
    BaseOffsetShiftMode(const BaseOffsetShiftMode& rOther) : BaseOffsetMode(rOther), mOffsetScale(rOther.mOffsetScale) { } //!< Copy constructor.
  private:
    bool ChooseTargetAddress(const AddressSolvingShared& rShared, ConstraintSet& rConstrSet, uint64& rTargetAddr) const override; //!< Select target address from constrained set of possibilities.
  private:
    uint32 mOffsetScale; //!< Offset scale value.
  };

  /*!
    \class IndexSolution
    \brief Contain index-extend information for a solution.
  */
  class IndexSolution : public AddressingRegister {
  public:
    const char* Type() const override { return "IndexSolution"; } //!< Return IndexSolution object type.
    Object* Clone() const override //!< Clone IndexSolution object.
    {
      return new IndexSolution(*this);
    }

    IndexSolution(const AddressingRegister& rOther, uint64 rBaseRegisterValue, uint64 target)
      : IndexSolution(rOther, 0, rBaseRegisterValue, target, 0)
    {
    }

    IndexSolution(const AddressingRegister& rOther, uint64 rIndexRegisterValue, uint64 rBaseRegisterValue, uint64 target, uint32 amount)
      : AddressingRegister(rOther), mBaseRegisterValue(rBaseRegisterValue), mTargetAddress(target), mAmountBit(amount)
    {
      mRegisterValue = rIndexRegisterValue;
    }

    ~IndexSolution() { } //!< Destructor.
    uint64 BaseValue() const { return mBaseRegisterValue; } //!< Return base register value.
    uint64 TargetAddress() const { return mTargetAddress; } //!< Return target address.
    uint32 AmountBit() const { return mAmountBit; } //!< Return amount bit value.
  protected:
    IndexSolution() : AddressingRegister(), mBaseRegisterValue(0), mTargetAddress(0), mAmountBit(0) { } //!< Default constructor.
  protected:
    uint64 mBaseRegisterValue; //!< Base register value.
    uint64 mTargetAddress; //!< Target address for the solution with the index-extend setup.
    uint32 mAmountBit; //!< Amount bit value for this index solution.
  };

  /*!
    \class BaseIndexMode
    \brief Address solving class for addressing mode: base with index.
  */
  class BaseIndexMode : public AddressingMode {
  public:
    BaseIndexMode();
    ~BaseIndexMode() override;
    ASSIGNMENT_OPERATOR_ABSENT(BaseIndexMode);

    const std::string ToString() const override; //!< Return a string describing the current state of the Object.
    bool ShouldApplyIndexFilters() const override { return true; } //!< Return whether filters applying to index registers should be applied.
    bool ChooseSolution(const AddressSolvingShared& rAddrSolShared) override; //!< Choose sub solution choice if necessary.
    void SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const override; //!< Update operands to reflect chosen solution.
    uint64 IndexValue() const override {return mpChosenIndexSolution->RegisterValue(); } //!< Return index value.
    const Register* Index() const { return mpChosenIndexSolution->GetRegister(); } //!< Return pointer to index register.
    std::vector<IndexSolution*>& GetSolutionChoicesForFiltering() { return mIndexSolutionChoices; } //!< Return reference to solution choices, only meant to be used by solution filtering.
    std::vector<IndexSolution*>& GetFilteredChoicesForFiltering() { return mFilteredChoices; } //!< Return reference to filtered choices, only meant to be used by solution filtering.
  protected:
    BaseIndexMode(const BaseIndexMode& rOther);

    bool HasIndexSolutions() const { return (not mIndexSolutionChoices.empty()); } //!< Return true if there is at least one index solution available.
    void AddIndexSolution(IndexSolution* pIndexSolution) { mIndexSolutionChoices.push_back(pIndexSolution); } //!< Add an index solution.
    const IndexSolution* GetChosenIndexSolution() const { return mpChosenIndexSolution; } //!< Return the chosen index solution.
  private:
    void RemoveUnusableSolutionChoices(); //!< Remove unusable solution choices.
    virtual RegisterOperand* GetIndexOperand(const AddressSolvingShared& rShared) const = 0; //!< Return index operand.
  private:
    IndexSolution* mpChosenIndexSolution; //!< Pointer to the chosen index solution
    std::vector<IndexSolution*> mIndexSolutionChoices; //!< Available index register solution choices
    std::vector<IndexSolution*> mFilteredChoices; //!< Filtered index register choices
  };

  /*!
    \class BaseIndexExtendMode
    \brief Address solving class for addressing mode: base with extended index.
  */
  class BaseIndexExtendMode : public BaseIndexMode {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(BaseIndexExtendMode);
    SUBCLASS_DESTRUCTOR_DEFAULT(BaseIndexExtendMode);
    ASSIGNMENT_OPERATOR_ABSENT(BaseIndexExtendMode);

    Object* Clone() const override { return new BaseIndexExtendMode(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "BaseIndexExtendMode"; } //!< Return a string describing the actual type of the Object.
    bool Solve(const AddressSolvingShared& rShared) override; //!< Solve for address.
    bool SolveFree(const AddressSolvingShared& rShared) override; //!< Solve for address.
    AddressSolvingShared* AddressSolvingSharedInstance() const override; //!< Return correct AddressSolvingShared object for the base-index addressing mode.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(BaseIndexExtendMode);

    void SolveWithAmountBit(const BaseIndexSolvingShared& rSharedBI, uint32 amountBit); //!< Solve with amount bit specified.
    void CreateFreeBaseIndexSolutionWithAmount(const BaseIndexSolvingShared& rBaseIndexShared, const AddressingRegister& rIndexChoice, const uint64 index_value, uint32 amountBit) ; //!< Create solution for a specified index choice when base register is free.
    void SolveTargetHasConstraint(const BaseIndexSolvingShared& rSharedBI, uint32 amountBit);
  private:
    RegisterOperand* GetIndexOperand(const AddressSolvingShared& rShared) const override; //!< Return index operand.
    virtual void CreateFreeBaseIndexSolution(const BaseIndexSolvingShared& rBaseIndexShared, const AddressingRegister& rIndexChoice, const uint64 index_value); //!< Create solution for a specified index choice when base register is free.
  };

  class VectorStridedSolvingShared;

  /*!
    \class VectorStridedMode
    \brief Address solving class for vector strided addressing mode.
  */
  class VectorStridedMode : public BaseIndexMode {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VectorStridedMode);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorStridedMode);
    ASSIGNMENT_OPERATOR_ABSENT(VectorStridedMode);

    Object* Clone() const override { return new VectorStridedMode(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VectorStridedMode"; } //!< Return a string describing the actual type of the Object.
    bool Solve(const AddressSolvingShared& rShared) override; //!< Solve for address.
    bool SolveFree(const AddressSolvingShared& rShared) override; //!< Solve for address.
    AddressSolvingShared* AddressSolvingSharedInstance() const override; //!< Return correct AddressSolvingShared object for the addressing mode.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VectorStridedMode);
  private:
    RegisterOperand* GetIndexOperand(const AddressSolvingShared& rShared) const override; //!< Return index operand.
    void SolveFixedTargetConstraintForced(const VectorStridedSolvingShared& rStridedShared); //!< Create solutions when base register is fixed and target address is forced.
    void SolveFixed(const VectorStridedSolvingShared& rStridedShared); //!< Create solutions when base register is fixed.
    void SolveFreeStrideFree(const VectorStridedSolvingShared& rStridedShared, const AddressingRegister& rStrideChoice); //!< Create solution for a specified stride choice when base and stride registers are free.
    void SolveFreeStrideFixed(const VectorStridedSolvingShared& rStridedShared, const AddressingRegister& rStrideChoice); //!< Create solution for a specified stride choice when base register is free and stride register is fixed.
    bool AreTargetAddressesUsable(const VectorStridedSolvingShared& rStridedShared, cuint64 baseVal, cuint64 strideVal); //!< Return true if the target address generated by the base and stride values satisfy all constraints.
    bool IsTargetAddressUsable(const VectorStridedSolvingShared& rStridedShared, cuint64 targetAddr, const ConstraintSet* pTargetConstr); //!< Return true if the specified target address satisfies all constraints.
  };

  class VectorIndexedSolvingShared;

  /*!
    \class VectorIndexedMode
    \brief Address solving class for vector indexed addressing mode.
  */
  class VectorIndexedMode : public BaseIndexMode {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VectorIndexedMode);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorIndexedMode);
    ASSIGNMENT_OPERATOR_ABSENT(VectorIndexedMode);

    Object* Clone() const override { return new VectorIndexedMode(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VectorIndexedMode"; } //!< Return a string describing the actual type of the Object.
    bool Solve(const AddressSolvingShared& rShared) override; //!< Solve for address.
    bool SolveFree(const AddressSolvingShared& rShared) override; //!< Solve for address.
    AddressSolvingShared* AddressSolvingSharedInstance() const override; //!< Return correct AddressSolvingShared object for the addressing mode.
    void SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const override; //!< Update operands to reflect chosen solution.
    std::vector<uint64> IndexValues() const; //!< Return index values.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VectorIndexedMode);
  private:
    RegisterOperand* GetIndexOperand(const AddressSolvingShared& rShared) const override; //!< Return index operand.
    void SolveFixedTargetConstraintForced(const VectorIndexedSolvingShared& rIndexedShared); //!< Create solutions when base register is fixed and target address is forced.
    void SolveFixed(const VectorIndexedSolvingShared& rIndexedShared); //!< Create solutions when base register is fixed.
    void SolveFreeIndexFree(const VectorIndexedSolvingShared& rIndexedShared, const AddressingRegister& rIndexChoice); //!< Create solution for a specified index choice when base and index registers are free.
    void SolveFreeIndexFixed(const VectorIndexedSolvingShared& rIndexedShared, const AddressingRegister& rIndexChoice); //!< Create solution for a specified index choice when base register is free and index register is fixed.
    bool AreTargetAddressesUsable(const VectorIndexedSolvingShared& rIndexedShared, cuint64 baseVal, const std::vector<uint64>& rIndexRegValues); //!< Return true if the target address generated by the base and index values satisfy all constraints.
    bool IsTargetAddressUsable(const VectorIndexedSolvingShared& rIndexedShared, cuint64 targetAddr, const ConstraintSet* pTargetConstr); //!< Return true if the specified target address satisfies all constraints.
  };

  /*!
    \class BaseIndexAmountBitExtendMode
    \brief Address solving class for addressing mode: base with extendedd index.
  */
  class BaseIndexAmountBitExtendMode : public BaseIndexExtendMode {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(BaseIndexAmountBitExtendMode);
    SUBCLASS_DESTRUCTOR_DEFAULT(BaseIndexAmountBitExtendMode);
    ASSIGNMENT_OPERATOR_ABSENT(BaseIndexAmountBitExtendMode);

    Object* Clone() const override { return new BaseIndexAmountBitExtendMode(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "BaseIndexAmountBitExtendMode"; } //!< Return a string describing the actual type of the Object.
    bool Solve(const AddressSolvingShared& rShared) override; //!< Solve for address.
    AddressSolvingShared* AddressSolvingSharedInstance() const override; //!< Return correct AddressSolvingShared object for the addressing mode.
    void SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const override; //!< Update operands to reflect chosen solution.
    uint32 AmountBit() const override; //!< Return amount bit value.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(BaseIndexAmountBitExtendMode);
  private:
    void CreateFreeBaseIndexSolution(const BaseIndexSolvingShared& rBaseIndexShared, const AddressingRegister& rIndexChoice, const uint64 index_value) override; //!< Create solution for a specified index choice when base register is free.
  };

  class ImmediateOperand;

  /*!
    \class AluImmediateMode
    \brief Address solving class for addressing mode: ALU with immediate operand.
  */
  class AluImmediateMode : public AddressingMode {
  public:
    explicit AluImmediateMode(EAluOperationType operationType) : AddressingMode(), mOperationType(operationType), mOffsetValue(0) { } //!< Default constructor.
    ~AluImmediateMode() { } //!< Destructor.

    const char* Type() const override { return "AluImmediateMode"; } //!< Return AluImmediateMode object type.
    Object* Clone() const override; //!< Clone AluImmediateMode object.

    bool BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const override { return true; } //!< Check if base value is usable.
    bool Solve(const AddressSolvingShared& rShared) override; //!< Solve for address.
    bool SolveFree(const AddressSolvingShared& rShared) override; //!< Solve for address with free base register.
    bool HasOffset() const override { return true; } //!< Return true for has-offset query.
    void SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const override; //!< Update operands to reflect chosen solution.
  protected:
    AluImmediateMode(const AluImmediateMode& rOther) : AddressingMode(rOther), mOperationType(rOther.mOperationType), mOffsetValue(rOther.mOffsetValue) { } //!< Copy constructor.
    uint64 AlignMask(const AddressSolvingShared& rShared, cuint32 offset_shift_amount) const; //!< Return alignment mask.
    uint32 AlignShift(const AddressSolvingShared& rShared, cuint32 offset_shift_amount) const; //!< Return alignment shift.
    void SolveBaseValue(cuint32 offset_shift_amount); //!< Solve for base register value with solved target address and offset value.
    void SolveOffsetValue(cuint32 offset_shift_amount); //!< Solve for offset value with solved target address and base register value.
  protected:
    const EAluOperationType mOperationType; //!< ALU operation type.
    uint32 mOffsetValue; //!< Solved offset value.
  };

  class Operand;
  class DataProcessingOperandStructure;

  /*!
    \class DataProcessingMode
    \brief Address solving class for addressing mode: data processing.
  */
  class DataProcessingMode : public AddressingMode {
  public:
    DataProcessingMode(const DataProcessingOperandStructure& rDataProcOprStruct, const std::vector<Operand*>& rSubOperands); //!< Default constructor.
    SUBCLASS_DESTRUCTOR_DEFAULT(DataProcessingMode); //!< Destructor.

    const std::string ToString() const override; //!< Return a string describing the current state of the DataProcessingMode object.
    const char* Type() const override { return "DataProcessingMode"; } //!< Return DataProcessingMode object type.
    Object* Clone() const override; //!< Clone DataProcessingMode object.

    bool BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const override { return true; } //!< Check if base value is usable.
    bool Solve(const AddressSolvingShared& rShared) override; //!< Solve for address.
    bool SolveFree(const AddressSolvingShared& rShared) override; //!< Solve for address with free base register.
    AddressSolvingShared* AddressSolvingSharedInstance() const override; //!< Return correct AddressSolvingShared object for the data processing addressing mode.
    void NonSystemRegisterOperands(std::vector<const RegisterOperand*>& rRegOperands) const override; //!< Return list of non-system register operands.
    void SetRegisterOperandChoice(const std::string& oprName, Register* regPtr) override; //!< Specify a register to use for the indicated operand.
    void SetOperandResults(const AddressSolvingShared& rShared, RegisterOperand& rBaseOperand) const override; //!< Update operands to reflect chosen solution.
  protected:
    DataProcessingMode(const DataProcessingMode& rOther) : AddressingMode(rOther), mOperationType(rOther.mOperationType), mUop(rOther.mUop), mSubOperands(rOther.mSubOperands), mOperandSolutions(rOther.mOperandSolutions) { } //!< Copy constructor.
    void MapOperandSolutions(const DataProcessingOperandStructure& rDataProcOprStruct); //!< Map operand roles to operand solution objects.
  protected:
    const EDataProcessingOperationType mOperationType; //!< Data processing operation type.
    const EUop mUop; //!< Micro-op type.
    std::vector<Operand*> mSubOperands; //!< Sub-operands.
    OperandSolutionMap mOperandSolutions; //!< Map of operand solutions.
  };

  class Generator;
  class Instruction;
  class AddressingOperand;

  /*!
    \class AddressSolver
    \brief Class for address solving.
  */
  class AddressSolver {
  public:
    AddressSolver(AddressingOperand* pOpr, AddressingMode* pModeTemp, uint64 alignment); //!< Constructor with template addressing mode provided.
    virtual ~AddressSolver(); //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(AddressSolver);
    COPY_CONSTRUCTOR_ABSENT(AddressSolver);

    const AddressingMode* Solve(Generator& gen, Instruction& instr, uint32 size, bool isInstr, const EMemAccessType memAccessType=EMemAccessType::Unknown); //!< Solve address.
    const AddressingMode* SolveMultiRegister(Generator& gen, Instruction& instr, uint32 size, bool isInstr, const EMemAccessType memAccessType=EMemAccessType::Unknown); //!< Solve for address with potentially multiple source register operands.
    std::vector<AddressingMode* >& GetSolutionChoicesForFiltering() { return mSolutionChoices; } //!< Return reference to solution choices, only meant to be used by solution filtering.
    std::vector<AddressingMode* >& GetFilteredChoicesForFiltering() { return mFilteredChoices; } //!< Return reference to filtered choices, only meant to be used by solution filtering.
    const AddressingOperand* GetAddressingOperand() const; //!< Return pointer to addressing operand.
    const RegisterOperand* GetBaseOperand() const { return mpBaseOperand; } //!< Return pointer to base operand.
    const AddressSolvingShared* GetAddressSolvingShared() const { return mpAddressSolvingShared; } //!< Return address solving shared.
    void SetOperandResults(); //!< Update operands to reflect chosen solution.
  protected:
    AddressSolver(); //!< Default constructor.
    virtual bool GetAvailableBaseChoices(Generator& gen, Instruction& instr, std::vector<AddressingMode* >& rBaseChoices) const; //!< Get available choices.
    bool GetUsableRegisters(const Generator& gen, const RegisterOperand* pRegOperand, vector<Register*>& rUsableRegisters) const; //!< Get a vector of usable registers from the choices for a given Register Operand
    bool GetRegisterChoiceCombinations(const Generator& gen, Instruction& instr, std::vector<AddressingMode* >& rRegChoiceCombos) const; //!< Get an AddressingMode instance for each register choice combination.
    const AddressingMode* SolveWithModes(const Generator& gen, const Instruction& instr, const std::vector<AddressingMode* >& rModes); //!< Solve for each of the specified modes and then choose one of the solutions.
    void ChooseSolution(const Generator& rGen, const Instruction& rInstr); //!< Choose from viable solutions.
    bool UpdateSolution(); //!< Update current solution.
    void RemoveUnusableSolutionChoices(); //!< Removes all solution choices with zero weight.
    bool IsRegisterUsable(const Register* regPtr, cbool hasIss) const; //!< Return true if register can be used.
  protected:
    RegisterOperand* mpBaseOperand; //!< Pointer to the base operand.
    AddressingMode* mpModeTemplate; //!< Addressing mode object template.
    AddressSolvingShared* mpAddressSolvingShared; //!< Shared object holding necessary address solving data structures.
    const AddressingMode* mpChosenSolution; //!< Chosen addressing solution.
    std::vector<AddressingMode* > mSolutionChoices; //!< Solution choices.
    std::vector<AddressingMode* > mFilteredChoices; //!< Filtered choices.
  private:
    bool ShouldEnableAddressShortage(const Instruction& instr) const; //!< Indicate whether there is a potential shortage of base register choices with valid addresses.
  };

  /*!
    \class AddressSolverWithOnlyChoice
    \brief Class for address solving for addressing operand with only one Choice
  */
  class AddressSolverWithOnlyChoice : public AddressSolver {
  public:
    AddressSolverWithOnlyChoice(AddressingOperand* pOpr, AddressingMode* pModeTemp, uint64 alignment) : AddressSolver(pOpr, pModeTemp, alignment) { } //!< Constructor with template addressing mode provided.
  private:
    AddressSolverWithOnlyChoice() : AddressSolver() { } //!< Default constructor.

    ASSIGNMENT_OPERATOR_ABSENT(AddressSolverWithOnlyChoice);
    COPY_CONSTRUCTOR_ABSENT(AddressSolverWithOnlyChoice);

    bool GetAvailableBaseChoices(Generator& gen, Instruction& instr, std::vector<AddressingMode* >& rBaseChoices) const override; //!< Get available choices.
  };
  /*!
    \class BaseOffsetMulMode
    \brief Address solving class for addressing mode: base with Multiply immediate offset.
  */
  class BaseOffsetMulMode : public BaseOffsetMode {
  public:
    const char* Type() const override { return "BaseOffsetMulMode"; } //!< Return BaseOffsetMulMode object type.
    Object* Clone() const override; //!< Clone BaseOffsetMulMode object.

    explicit BaseOffsetMulMode(uint32 offsetMulData): BaseOffsetMode(), mOffsetMulData(offsetMulData) {}; //!< Constructor with offset multiplier parameter given.
    ~BaseOffsetMulMode() { } //!< Destructor.

    bool BaseValueUsable(uint64 baseValue, const AddressSolvingShared* pAddrSolShared) const override; //!< Check if base value is usable.
    uint32 OffsetScale() const override { return mOffsetMulData; } //!< Return offset scale if any.
    AddressSolvingShared* AddressSolvingSharedInstance() const override; //!< Return correct AddressSolvingShared object for the base with scaled offset addressing mode.
    uint64 AdjustOffset(uint64 mOffset) const override { return mOffset *= mOffsetMulData;}
    uint32 calculate_offset_value(uint64 offset, uint32 scale, uint32 mask) const override;
  protected:
    BaseOffsetMulMode() : BaseOffsetMode(), mOffsetMulData(1) { } //!< Default constructor.
    BaseOffsetMulMode(const BaseOffsetMulMode& rOther) : BaseOffsetMode(rOther), mOffsetMulData(rOther.mOffsetMulData) { } //!< Copy constructor.
  private:
    bool ChooseTargetAddress(const AddressSolvingShared& rShared, ConstraintSet& rConstrSet, uint64& rTargetAddr) const override; //! Select target address from constrained set of possibilities.
  private:
    uint32 mOffsetMulData; //!< Offset multiply value.
  };

}

#endif
