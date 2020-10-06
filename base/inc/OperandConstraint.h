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
#ifndef Force_OperandConstraint_H
#define Force_OperandConstraint_H

#include <Defines.h>
#include <vector>
#include <string>
#include <type_traits>
#include <Enums.h>
#include ARCH_ENUM_HEADER

namespace Force {

  class ChoiceTree;
  class Generator;
  class Instruction;
  class OperandStructure;
  class ConstraintSet;
  class OperandRequest;
  class ChoicesOperand;
  class RegisterOperand;
  class OperandDataRequest;
  class GenPageRequest;
  class Operand;

  /*!
    \class OperandConstraint
    \brief Base class for various OperandConstraint sub classes.
  */
  class OperandConstraint {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(OperandConstraint);
    OperandConstraint() : mpConstraintSet(nullptr), mConstraintForced(false) { } //!< Constructor.
    virtual ~OperandConstraint(); //!< Destructor.
    virtual void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct); //!< Setup dynamic operand constraints.
    virtual bool HasConstraint() const { return (nullptr != mpConstraintSet); } //!< Return if there is additional constraint on the operand.
    virtual const ConstraintSet* GetConstraint() const { return mpConstraintSet; } //!< Return pointer to constraint set for the operand.
    bool ConstraintForced() const { return mConstraintForced; } //!< Return whether there is a forced constraint.

    /*!
      Templated function so that a derived class can conveniently cast base class to the desired derived class type.
      For example, ImmediateOperandConstraint* imm_constraint = opr_constraint->CastInstance<ImmediateOperandConstraint>();
     */
    template<typename T>
      T* CastInstance()
      {
        T* cast_instance = dynamic_cast<T*>(this);
        return cast_instance;
      }

    /*!
      This is a const variant of CastInstance().
     */
    template<typename T>
      typename std::enable_if<std::is_const<T>::value, T*>::type CastInstance() const
      {
        T* cast_instance = dynamic_cast<T*>(this);
        return cast_instance;
      }

    void ApplyUserRequest(const OperandRequest& rOprReq); //!< Apply user request details.
    void SubConstraintValue(uint64 value, const OperandStructure& rOperandStruct) const; //!< Subtract value from constraint set.
    void SubDifferOperandValues(const Instruction& rInstr, const OperandStructure& rOperandStruct); //!< Subtract values of operands that must have different values from the constraint set, so this operand is generated with a different value.
  protected:
    OperandConstraint(const OperandConstraint& rOther); //!< Copy constructor, not meant to be used.
    ConstraintSet* DefaultConstraintSet(const OperandStructure& rOperandStruct) const; //!< Return a default ConstraintSet object.
  protected:
    mutable ConstraintSet* mpConstraintSet; //!< Pointer to constraint set for the operand.
    bool mConstraintForced; //!< Return true if constraint is forced from the front end with single value.
  private:
    virtual void GetAdjustedDifferValues(const Instruction& rInstr, const OperandConstraint& rDifferOprConstr, cuint64 differVal, ConstraintSet& rAdjDifferValues) const; //!< Return a list of values to remove from the constraint set to avoid conflicting with the specified differ operand value.
  };

  /*!
    \class ImmediateOperandConstraint
    \brief The class carries dynamic constraint properties for ImmediateOperand.
  */
  class ImmediateOperandConstraint : public OperandConstraint {
  public:
    ImmediateOperandConstraint() : OperandConstraint() { } //!< Constructor.
    ~ImmediateOperandConstraint() { } //!< Destructor.
  protected:
    ImmediateOperandConstraint(const ImmediateOperandConstraint& rOther) : OperandConstraint(rOther) { } //!< Copy constructor, not meant to be used.
  };

  /*!
    \class ImmediatePartialOperandConstraint
    \brief The class carries dynamic constraint properties for ImmediatePartialOperand.
  */
  class ImmediatePartialOperandConstraint : public ImmediateOperandConstraint {
  public:
    ImmediatePartialOperandConstraint() : ImmediateOperandConstraint(), mAllowReserved(false) { } //!< Constructor.
    ~ImmediatePartialOperandConstraint() { } //!< Destructor.
    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints.
    bool HasConstraint() const override; //!< Return whether this ImmediatePartialOperandConstraint has additional constraints.
    const ConstraintSet* GetConstraint() const override; //!< Return pointer to constraint set for the operand.
    virtual bool HasReservedConstraint() const { return false; } //!< Indicate whether there is reserved constraint.
    virtual bool HasExcludeConstraint() const { return false; } //!< Indicate whether there is exclude constraint.
    virtual const ConstraintSet* GetReservedConstraint() const { return nullptr; } //!< Return reserved constraint set.
    virtual const ConstraintSet* GetExcludeConstraint() const { return nullptr; } //!< Return exclude constraint set.
  protected:
    ImmediatePartialOperandConstraint(const ImmediatePartialOperandConstraint& rOther) : ImmediateOperandConstraint(rOther), mAllowReserved(false) { } //!< Copy constructor, not meant to be used.
  protected:
    bool mAllowReserved; //!< Inidate whehter reserved values are allowed.
  };

  class ExcludeOperandStructure;

  /*!
    \class ImmediateExcludeOperandConstraint
    \brief The class carries dynamic constraint properties for ImmediateExcludeOperand.
  */
  class ImmediateExcludeOperandConstraint : public ImmediatePartialOperandConstraint {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(ImmediateExcludeOperandConstraint);
    ImmediateExcludeOperandConstraint() : ImmediatePartialOperandConstraint(), mpOperandStructure(nullptr) { } //!< Constructor.
    ~ImmediateExcludeOperandConstraint() { } //!< Destructor.
    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints.
    bool HasExcludeConstraint() const override { return true; } //!< Indicate whether there is exclude constraint.
    const ConstraintSet* GetExcludeConstraint() const override; //!< Return exclude constraint set.
  protected:
    ImmediateExcludeOperandConstraint(const ImmediateExcludeOperandConstraint& rOther) : ImmediatePartialOperandConstraint(rOther), mpOperandStructure(nullptr) { } //!< Copy constructor, not meant to be used.
  protected:
    const ExcludeOperandStructure* mpOperandStructure; //!< Const pointer to operand structure object.
  };

  /*!
    \class ImmediateGe1OperandConstraint
    \brief The class carries dynamic constraint properties for ImmediateGe1Operand.
  */
  class ImmediateGe1OperandConstraint : public ImmediatePartialOperandConstraint {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(ImmediateGe1OperandConstraint);
    ImmediateGe1OperandConstraint() : ImmediatePartialOperandConstraint(), mpExcludeConstraint(nullptr) { } //!< Constructor.
    ~ImmediateGe1OperandConstraint(); //!< Destructor.
    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints.
    bool HasExcludeConstraint() const override { return true; } //!< Indicate whether there is exclude constraint.
    const ConstraintSet* GetExcludeConstraint() const override; //!< Return exclude constraint set.
  protected:
    ImmediateGe1OperandConstraint(const ImmediateGe1OperandConstraint& rOther) : ImmediatePartialOperandConstraint(rOther), mpExcludeConstraint(nullptr) { } //!< Copy constructor, not meant to be used.
  protected:
    mutable ConstraintSet* mpExcludeConstraint; //!< Pointer to constraint set for the operand.
  };

  /*!
    \class ChoicesOperandConstraint
    \brief The class carries dynamic constraint properties for ChoicesOperand.
  */
  class ChoicesOperandConstraint : public OperandConstraint {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(ChoicesOperandConstraint);
    ChoicesOperandConstraint() : OperandConstraint(), mpChoiceTree(nullptr) { } //!< Constructor.
    ~ChoicesOperandConstraint(); //!< Destructor.
    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints for ChoicesOperand
    virtual const ChoiceTree* GetChoiceTree() const { return mpChoiceTree; } //!< Return const pointer to mpChoiceTree.

  protected:
    ChoiceTree* mpChoiceTree; //!< Pointer to associated choice tree.
  protected:
    ChoicesOperandConstraint(const ChoicesOperandConstraint& rOther) : OperandConstraint(rOther), mpChoiceTree(nullptr) { } //!< Copy constructor, not meant to be used.
    ChoiceTree* SetupExtraChoiceTree(uint32 index, const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct); //!< Setup extra choices tree.
  };

  /*!
    \class RegisterOperandConstraint
    \brief The class carries dynamic constraint properties for RegisterOperand.
  */
  class RegisterOperandConstraint : public ChoicesOperandConstraint {
  public:
    RegisterOperandConstraint() : ChoicesOperandConstraint() { } //!< Constructor.
    ~RegisterOperandConstraint() { } //!< Destructor.
    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints for RegisterOperandConstraint.
    void AddWriteConstraint(const Generator& rGen, const OperandStructure& rOperandStruct); //!< Add write related constraint.
    virtual const OperandDataRequest* GetOperandDataRequest(const Instruction& rInstr, const std::string& rOprName); //!< get operand data request
  protected:
    RegisterOperandConstraint(const RegisterOperandConstraint& rOther) : ChoicesOperandConstraint(rOther) { } //!< Copy constructor, not meant to be used.
  };

  /*!
    \class FpRegisterOperandConstraint
    \brief The class carries dynamic constraint properties for FpRegisterOperand.
  */
  class FpRegisterOperandConstraint : public RegisterOperandConstraint {
  public:
    FpRegisterOperandConstraint() : RegisterOperandConstraint() { } //!< Constructor.
    ~FpRegisterOperandConstraint() { } //!< Destructor.
  protected:
    FpRegisterOperandConstraint(const FpRegisterOperandConstraint& rOther) : RegisterOperandConstraint(rOther) { } //!< Copy constructor, not meant to be used.
  };

  /*!
    \class VectorRegisterOperandConstraint
    \brief The class carries dynamic constraint properties for VectorRegisterOperand.
  */
  class VectorRegisterOperandConstraint : public RegisterOperandConstraint {
  public:
    VectorRegisterOperandConstraint() : RegisterOperandConstraint() { } //!< Constructor.
    ~VectorRegisterOperandConstraint() { } //!< Destructor.
  protected:
    VectorRegisterOperandConstraint(const VectorRegisterOperandConstraint& rOther) : RegisterOperandConstraint(rOther) { } //!< Copy constructor, not meant to be used.
  };

  /*!
    \class ImpliedRegisterOperandConstraint
    \brief The class carries dynamic constraint properties for ImpliedRegisterOperand.
  */
  class ImpliedRegisterOperandConstraint : public RegisterOperandConstraint {
  public:
    ImpliedRegisterOperandConstraint() : RegisterOperandConstraint(), mRegisterIndex(0) { } //!< Constructor.
    ~ImpliedRegisterOperandConstraint() { } //!< Destructor.
    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup method for ImpliedRegisterOperand.
    uint32 RegisterIndex() const { return mRegisterIndex; } //!< Return register index of the implied register.
  protected:
    ImpliedRegisterOperandConstraint(const ImpliedRegisterOperandConstraint& rOther) : RegisterOperandConstraint(rOther), mRegisterIndex(0) { } //!< Copy constructor, not meant to be used.
    uint32 mRegisterIndex; //!< Index of the implied register.
  };

  /*!
    \class GroupOperandConstraint
    \brief The class carries dynamic constraint properties for GroupOperand.
  */
  class GroupOperandConstraint : public virtual OperandConstraint {
  public:
    GroupOperandConstraint() : OperandConstraint() { } //!< Constructor.
    ~GroupOperandConstraint() { } //!< Destructor.
  protected:
    GroupOperandConstraint(const GroupOperandConstraint& rOther) : OperandConstraint(rOther) { } //!< Copy constructor, not meant to be used.
  };

  class ImmediateOperand;
  class SignedImmediateOperand;
  class VmMapper;

  /*!
    \class AddressingOperandConstraint
    \brief The class carries dynamic constraint properties for AddressingOperand.
  */
  class AddressingOperandConstraint : public GroupOperandConstraint {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(AddressingOperandConstraint);
    AddressingOperandConstraint(); //!< Constructor.
    ~AddressingOperandConstraint(); //!< Destructor.
    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Do setup for routines common to base classes.

    uint64 PC() const { return mPC; } //!< Return PC value.
    bool UsePreamble() const { return mUsePreamble; } //!< Return whether preamble sequence is needed.
    bool NoPreamble() const { return mNoPreamble; } //!< Return whether no-preamble is an abosolute requirement.
    void SetUsePreamble(Generator& rGen); //!< Set mUsePreamble attribute to true.
    void GetBaseConstraint(uint64 baseValue, uint64 maxAddress, uint32 accessSize, ConstraintSet& rResultConstr) const; //!< Return a ConstraintSet object via reference from the base and access size parameters.
    virtual RegisterOperand* BaseOperand() const { return nullptr; } //!< Return pointer to base operand if applicable.
    virtual RegisterOperand* IndexOperand() const { return nullptr; } //!< Return pointer to register index operand if applicable.
    virtual ImmediateOperand* OffsetOperand() const { return nullptr; } //!< Return pointer to immediate offset operand if applicable.
    virtual ImmediateOperand* ExtendAmountOperand() const { return nullptr; } //!< Return pointer to immediate extend amount operand.
    virtual void GetRegisterOperands(std::vector<const RegisterOperand* >& rRegOps) { } //!< Get pointers to sub RegisterOperands.
    virtual bool IsInstruction() const { return false; } //!< Is the memory access for instruction.
    const GenPageRequest* GetPageRequest() const { return mpPageRequest; } //!< Return page request object.
    const ConstraintSet* TargetConstraint() const { return mpTargetConstraint; } //!< Return pointer to target constraint if any.
    const std::vector<ConstraintSet* >& DataConstraints() const { return mDataConstraints; } //!< Return data constraints if any.
    bool HasDataConstraints() const { return mDataConstraints.size(); } //!< Return true if has any data constraint.
    bool TargetConstraintForced() const; //!< Return true if target constraint is forced.
    inline VmMapper* GetVmMapper() const { return mpVmMapper; } //!< Return VM mapper object.
  protected:
    AddressingOperandConstraint(const AddressingOperandConstraint& rOther); //!< Copy constructor, not meant to be used.
    SignedImmediateOperand* GetSignedOffsetOperand(const Instruction& rInstr, const OperandStructure& rOperandStruct) const; //!< Return a pointer to a signed-immediate sub operand of the AddressingOperand object.
    virtual void SetupVmMapper(const Generator& rGen); //!< setup the VM mapper object.
  protected:
    uint64 mPC; //!< Value for the current PC.
    bool mUsePreamble; //!< Indicate whether preamble sequence is needed.
    bool mNoPreamble; //!< Indicate forced requirement of no preamble instructions.
    GenPageRequest* mpPageRequest; //!< Pointer to page request object/
    ConstraintSet* mpTargetConstraint; //!< Addressing target constraint.
    VmMapper* mpVmMapper; //!< Pointer to VM mapper object/
    std::vector<ConstraintSet* > mDataConstraints; //!< Data of addressing constraint.
  };

  /*!
    \class BranchOperandConstraint
    \brief The class carries dynamic constraint properties for BranchOperand.
  */
  class BranchOperandConstraint : public AddressingOperandConstraint {
  public:
    BranchOperandConstraint() : AddressingOperandConstraint(), mSimulationEnabled(false) { } //!< Constructor.
    ~BranchOperandConstraint() { } //!< Destructor.
    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints for BranchOperand
    bool IsInstruction() const override { return true; } //!< Is the memory access for instruction.
    inline bool SimulationEnabled() const { return mSimulationEnabled; } //!< Is simulation enabled
  protected:
    BranchOperandConstraint(const BranchOperandConstraint& rOther) : AddressingOperandConstraint(rOther), mSimulationEnabled(false) { } //!< Copy constructor, not meant to be used.
  protected:
    bool mSimulationEnabled; //!< whether simulation is enabled or not
  };

  /*!
    \class RegisterBranchOperandConstraint
    \brief The class carries dynamic constraint properties for RegisterBranchOperand.
  */
  class RegisterBranchOperandConstraint : public BranchOperandConstraint {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(RegisterBranchOperandConstraint);
    RegisterBranchOperandConstraint() : BranchOperandConstraint(), mpRegisterOperand(nullptr) , mUnalignedPC(0){ } //!< Constructor.
    ~RegisterBranchOperandConstraint() { mpRegisterOperand = nullptr; } //!< Destructor.

    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints for RegisterBranchOperand
    const RegisterOperand* GetRegisterOperand() const { return mpRegisterOperand; } //!< Return const pointer to register operand.
    RegisterOperand* BaseOperand() const override { return mpRegisterOperand; } //!< Return pointer to base operand.

    void GetRegisterOperands(std::vector<const RegisterOperand* >& rRegOps) override  //!< Get pointers to sub RegisterOperands of RegisterBranchOperand.
    {
      rRegOps.push_back(mpRegisterOperand);
    }

    bool UnalignedPC() const { return mUnalignedPC; } //!< Return whether unaligned PC is required.
  protected:
    RegisterBranchOperandConstraint(const RegisterBranchOperandConstraint& rOther) : BranchOperandConstraint(rOther), mpRegisterOperand(nullptr), mUnalignedPC(rOther. mUnalignedPC)  { } //!< Copy constructor, not meant to be used.
  protected:
    RegisterOperand* mpRegisterOperand; //!< Pointer to RegisterOperand sub operand.
    bool mUnalignedPC; //!< indicate whether to adjust a PC missalignment or not
  };

  /*!
    \class LoadStoreOperandConstraint
    \brief The class carries dynamic constraint properties for LoadStoreOperand.
  */
  class LoadStoreOperandConstraint : public AddressingOperandConstraint {
  public:
    LoadStoreOperandConstraint() : AddressingOperandConstraint(), mAlignedData(EDataAlignedType::Unaligned), mAlignedSp(ESpAlignedType(0)),mSpAlignment(0) { } //!< Constructor.
    ~LoadStoreOperandConstraint() { } //!< Destructor.
    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints for LoadStoreOperand
    EDataAlignedType AlignedData() const { return mAlignedData; } //!< Return whether data alignment is required.
    uint32 SpAlignment() const { return mSpAlignment; } //!< Return stack pointer alignment.
    bool BaseOperandSpAligned() const; //!< Return whether base operand sp alignment is required.
  protected:
    LoadStoreOperandConstraint(const LoadStoreOperandConstraint& rOther) : AddressingOperandConstraint(rOther), mAlignedData(EDataAlignedType::Unaligned), mAlignedSp(ESpAlignedType(0)),mSpAlignment(0) { } //!< Copy constructor, not meant to be used.
  protected:
    EDataAlignedType mAlignedData; //!< Indicate the type of data alignment.
    ESpAlignedType mAlignedSp; //!< Indicate the type of sp alignment.
    uint32 mSpAlignment; //!< Indicate stack pointer alignment.
  private:
    void SetupTargetConstraint(const Instruction& rInstr, const OperandStructure& rOperandStruct); //!< Configure target constraint based on instruction parameters.
  };

  /*!
    \class AluImmediateOperandConstraint
    \brief This class carries dynamic constraint properties for AluImmediateOperand.
  */
  class AluImmediateOperandConstraint : public AddressingOperandConstraint {
  public:
    AluImmediateOperandConstraint() : AddressingOperandConstraint(), mpBase(nullptr), mpOffset(nullptr), mpOffsetShift(nullptr), mResultType(0) { } //!< Constructor.
    ~AluImmediateOperandConstraint() { mpBase = nullptr; mpOffset = nullptr; mpOffsetShift = nullptr; } //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(AluImmediateOperandConstraint);
    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints for AluImmediateOperand
    RegisterOperand* BaseOperand() const override { return mpBase; } //!< Return pointer to base operand.
    ImmediateOperand* OffsetOperand() const override { return mpOffset; } //!< Return pointer to immediate offset operand.
    ChoicesOperand* OffsetShiftOperand() const { return mpOffsetShift; } //!< Return pointer to offset shift operand.

    void GetRegisterOperands(std::vector<const RegisterOperand* >& rRegOps) override  //!< Get pointers to sub RegisterOperands of AluImmediateOperand
    {
      rRegOps.push_back(mpBase);
    }

    inline uint32 ResultType() const { return mResultType; } //!< Return result type desired.
  protected:
    AluImmediateOperandConstraint(const AluImmediateOperandConstraint& rOther) //!< Copy constructor, not meant to be used.
      : AddressingOperandConstraint(rOther), mpBase(nullptr), mpOffset(nullptr), mpOffsetShift(nullptr), mResultType(0) { }
  protected:
    RegisterOperand* mpBase; //!< Pointer to base sub operand.
    ImmediateOperand* mpOffset; //!< Pointer to offset sub operand.
    ChoicesOperand* mpOffsetShift; //!< Pointer to offset shift sub operand.
    uint32 mResultType; //!< Result type.
  };

  /*!
    \class DataProcessingOperandConstraint
    \brief This class carries dynamic constraint properties for DataProcessingOperand.
  */
  class DataProcessingOperandConstraint : public AddressingOperandConstraint {
  public:
    DataProcessingOperandConstraint() : AddressingOperandConstraint(), mpBase(nullptr), mResultType(0) { } //!< Constructor.
    COPY_CONSTRUCTOR_ABSENT(DataProcessingOperandConstraint);
    ~DataProcessingOperandConstraint() { mpBase = nullptr; } //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(DataProcessingOperandConstraint);
    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints for DataProcessingOperand
    RegisterOperand* BaseOperand() const override { return mpBase; } //!< Return pointer to base operand.
    inline uint32 ResultType() const { return mResultType; } //!< Return result type desired.
  protected:
    RegisterOperand* mpBase; //!< Pointer to base sub operand.
    uint32 mResultType; //!< Result type.
  };

  /*!
    \class BaseOffsetLoadStoreOperandConstraint
    \brief The class carries dynamic constraint properties for BaseOffsetLoadStoreOperand.
  */
  class BaseOffsetLoadStoreOperandConstraint : public LoadStoreOperandConstraint {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(BaseOffsetLoadStoreOperandConstraint);
    BaseOffsetLoadStoreOperandConstraint() : LoadStoreOperandConstraint(), mpBase(nullptr), mpOffset(nullptr), mBaseValue(0) { } //!< Constructor.
    ~BaseOffsetLoadStoreOperandConstraint() { mpBase = nullptr; mpOffset = nullptr; } //!< Destructor.

    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints for BaseOffsetLoadStoreOperand
    RegisterOperand* BaseOperand() const override { return mpBase; } //!< Return pointer to base operand.
    ImmediateOperand* OffsetOperand() const override { return mpOffset; } //!< Return pointer to immediate offset operand.
    void SetBaseValue(uint64 baseValue) { mBaseValue = baseValue; } //!< Set base value.
    uint64 BaseValue() const { return mBaseValue; } //!< Return base value.

    void GetRegisterOperands(std::vector<const RegisterOperand* >& rRegOps) override  //!< Get pointers to sub RegisterOperands of BaseOffsetLoadStoreOperand
    {
      rRegOps.push_back(mpBase);
    }
  protected:
    BaseOffsetLoadStoreOperandConstraint(const BaseOffsetLoadStoreOperandConstraint& rOther) //!< Copy constructor, not meant to be used.
      : LoadStoreOperandConstraint(rOther), mpBase(nullptr), mpOffset(nullptr), mBaseValue(0) { }
  protected:
    RegisterOperand* mpBase; //!< Pointer to base sub operand.
    ImmediateOperand* mpOffset; //!< Pointer to offset sub operand, if applicable.
    uint64 mBaseValue; //!< Value for base register.
  };

   /*!
    \class BaseRegisterLoadStoreOperandConstraint
    \brief The class carries dynamic constraint properties for BaseIndexLoadStoreOperand.
  */
  class BaseIndexLoadStoreOperandConstraint : public LoadStoreOperandConstraint {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(BaseIndexLoadStoreOperandConstraint);
    BaseIndexLoadStoreOperandConstraint() : LoadStoreOperandConstraint(), mpBase(nullptr), mpIndex(nullptr), mpExtendAmount(nullptr), mBaseValue(0ull), mIndexValue(0ull), mExtendAmountName(),mIndexUsePreamble(false), mIndexPreOperands() { } //!< Constructor.
    ~BaseIndexLoadStoreOperandConstraint() { mpBase = nullptr; mpIndex = nullptr; mpExtendAmount = nullptr; } //!< Destructor.

    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints for BaseOffsetLoadStoreOperand
    RegisterOperand* BaseOperand() const override { return mpBase; } //!< Return pointer to base operand.
    RegisterOperand* IndexOperand() const override { return mpIndex; } //!< Return pointer to register index operand.
    ImmediateOperand* ExtendAmountOperand() const  override{ return mpExtendAmount; } //!< Return pointer to immediate extend amount operand.
    void SetBaseValue(uint64 baseValue) { mBaseValue = baseValue; } //!< Set base value.
    uint64 BaseValue() const { return mBaseValue; } //!< Return base value.
    void SetIndexValue(uint64 indexValue) { mIndexValue = indexValue; } //!< Set index value.
    uint64 IndexValue() const { return mIndexValue; } //!< Return index value.
    std::string ExtentAmountName() { return mExtendAmountName; } //!< Return extendamountname value.
    inline void SetIndexUsePreamble() { mIndexUsePreamble = true; } //!< Set mIndexUsePreamble attribute to true.
    inline bool IndexUsePreamble() const { return mIndexUsePreamble; }  //!< Return whether preamble sequence is needed for index regsiter.
    bool IndexChoiceValueDuplicate() const; //!< Return whether has same choice value before index operand.

    void GetRegisterOperands(std::vector<const RegisterOperand* >& rRegOps) override  //!< Get pointers to sub RegisterOperands of BaseIndexLoadStoreOperand
    {
      rRegOps.push_back(mpBase);
      rRegOps.push_back(mpIndex);
    }
  protected:
    BaseIndexLoadStoreOperandConstraint(const BaseIndexLoadStoreOperandConstraint& rOther) //!< Copy constructor, not meant to be used.
      : LoadStoreOperandConstraint(rOther), mpBase(nullptr), mpIndex(nullptr), mpExtendAmount(nullptr), mBaseValue(0ull), mIndexValue(0ull), mExtendAmountName(),mIndexUsePreamble(false), mIndexPreOperands() { }
  protected:
    RegisterOperand* mpBase; //!< Pointer to base sub operand.
    RegisterOperand* mpIndex; //!< Pointer to index register operand.
    ImmediateOperand* mpExtendAmount; //!< Pointer to amount sub operand
    uint64 mBaseValue;        //!< Value for base register.
    uint64 mIndexValue;      //!< Value for index register
    std::string mExtendAmountName; //!< name for ExtendAmount
    bool mIndexUsePreamble; //!< Indicate index register requirement of preamble instructions.
    std::vector<Operand* > mIndexPreOperands; //!< The operands that before index operand.
  };

  /*!
    \class PcRelativeBranchOperandConstraint
    \brief The class carries dynamic constraint properties for PcRelativeBranchOperand.
  */
  class PcRelativeBranchOperandConstraint : public BranchOperandConstraint {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(PcRelativeBranchOperandConstraint);
    PcRelativeBranchOperandConstraint() : BranchOperandConstraint(), mpOffset(nullptr) { } //!< Constructor.
    ~PcRelativeBranchOperandConstraint() { mpOffset = nullptr; } //!< Destructor.

    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints for BaseOffsetBranchOperand
    ImmediateOperand* OffsetOperand() const override { return mpOffset; } //!< Return pointer to immediate offset operand.
  protected:
    PcRelativeBranchOperandConstraint(const PcRelativeBranchOperandConstraint& rOther) //!< Copy constructor, not meant to be used.
      : BranchOperandConstraint(rOther), mpOffset(nullptr) { }
  protected:
    ImmediateOperand* mpOffset; //!< Pointer to offset sub operand, if applicable./////
  };

  /*!
    \class PcOffsetLoadStoreOperandConstraint
    \brief The class carries dynamic constraint properties for PcOffsetLoadStoreOperand.
  */
  class PcOffsetLoadStoreOperandConstraint : public LoadStoreOperandConstraint {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(PcOffsetLoadStoreOperandConstraint);
    PcOffsetLoadStoreOperandConstraint() : LoadStoreOperandConstraint(), mpOffset(nullptr) { } //!< Constructor.
    ~PcOffsetLoadStoreOperandConstraint() { mpOffset = nullptr; } //!< Destructor.

    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints for BaseOffsetLoadStoreOperand
    ImmediateOperand* OffsetOperand() const override { return mpOffset; } //!< Return pointer to immediate offset operand.
  protected:
    PcOffsetLoadStoreOperandConstraint(const PcOffsetLoadStoreOperandConstraint& rOther) //!< Copy constructor, not meant to be used.
      : LoadStoreOperandConstraint(rOther), mpOffset(nullptr) { }
  protected:
    ImmediateOperand* mpOffset; //!< Pointer to offset sub operand, if applicable.
  };

   /*!
    \class VectorStridedLoadStoreOperandConstraint
    \brief The class carries dynamic constraint properties for VectorStridedLoadStoreOperand.
  */
  class VectorStridedLoadStoreOperandConstraint : public LoadStoreOperandConstraint {
  public:
    VectorStridedLoadStoreOperandConstraint();
    COPY_CONSTRUCTOR_ABSENT(VectorStridedLoadStoreOperandConstraint);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorStridedLoadStoreOperandConstraint);
    ASSIGNMENT_OPERATOR_ABSENT(VectorStridedLoadStoreOperandConstraint);

    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints.
    RegisterOperand* BaseOperand() const override { return mpBase; } //!< Return pointer to base operand if applicable.
    void GetRegisterOperands(std::vector<const RegisterOperand* >& rRegOps) override; //!< Get pointers to sub RegisterOperands.
    RegisterOperand* StrideOperand() const { return mpStride; } //!< Return pointer to stride operand.
    uint64 BaseValue() const { return mBaseValue; } //!< Return base value.
    uint64 StrideValue() const { return mStrideValue; } //!< Return stride value.
    void SetBaseValue(cuint64 baseValue) { mBaseValue = baseValue; } //!< Set base value.
    void SetStrideValue(cuint64 strideValue) { mStrideValue = strideValue; } //!< Set stride value.
  private:
    RegisterOperand* mpBase; //!< Base operand
    RegisterOperand* mpStride; //!< Stride operand
    uint64 mBaseValue; //!< Base value
    uint64 mStrideValue; //!< Stride value
  };

   /*!
    \class VectorIndexedLoadStoreOperandConstraint
    \brief The class carries dynamic constraint properties for VectorIndexedLoadStoreOperand.
  */
  class VectorIndexedLoadStoreOperandConstraint : public LoadStoreOperandConstraint {
  public:
    VectorIndexedLoadStoreOperandConstraint();
    COPY_CONSTRUCTOR_ABSENT(VectorIndexedLoadStoreOperandConstraint);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorIndexedLoadStoreOperandConstraint);
    ASSIGNMENT_OPERATOR_ABSENT(VectorIndexedLoadStoreOperandConstraint);

    void Setup(const Generator& rGen, const Instruction& rInstr, const OperandStructure& rOperandStruct) override; //!< Setup dynamic operand constraints.
    RegisterOperand* BaseOperand() const override { return mpBase; } //!< Return pointer to base operand if applicable.
    void GetRegisterOperands(std::vector<const RegisterOperand* >& rRegOps) override; //!< Get pointers to sub RegisterOperands.
    RegisterOperand* IndexOperand() const override { return mpIndex; } //!< Return pointer to index operand.
    uint64 BaseValue() const { return mBaseValue; } //!< Return base value.
    const std::vector<uint64>& IndexElementValues() const { return mIndexElemValues; } //!< Return index element values.
    uint32 IndexElementSize() const { return mIndexElemSize; } //!< Return index element size in bytes.
    void SetBaseValue(cuint64 baseValue) { mBaseValue = baseValue; } //!< Set base value.
    void SetIndexElementValues(const std::vector<uint64>& rIndexElemValues) { mIndexElemValues = rIndexElemValues; } //!< Set index element values.
    void SetIndexElementSize(cuint32 indexElemSize) { mIndexElemSize = indexElemSize; } //!< Set the index element size in bytes.
  private:
    RegisterOperand* mpBase; //!< Base operand
    RegisterOperand* mpIndex; //!< Index operand
    uint64 mBaseValue; //!< Base value
    std::vector<uint64> mIndexElemValues; //!< Index element values
    uint32 mIndexElemSize; //!< Index element size in bytes
  };

}

#endif
