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
#ifndef Force_AddressSolvingShared_H
#define Force_AddressSolvingShared_H

#include <map>
#include <vector>

#include "ConditionFlags.h"
#include "Defines.h"
#include "Enums.h"
#include "VmConstraint.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class AddressingRegister;
  class Generator;
  class Instruction;
  class RegisterOperand;
  class AddressingOperand;
  class AddressingOperandConstraint;
  class ConstraintSet;
  class VmMapper;
  class AddressTagging;
  class Register;
  class AddressingMode;

  /*!
    \class AddressSolvingShared
    \brief Class holding necessary shared address solving data structures.
  */
  class AddressSolvingShared
  {
  public:
    AddressSolvingShared(); //!< Default constructor.
    virtual ~AddressSolvingShared(); //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(AddressSolvingShared);
    COPY_CONSTRUCTOR_ABSENT(AddressSolvingShared);

    void Initialize(Generator& gen, Instruction& instr, uint32 size, bool isInstr, const EMemAccessType memAccessType); //!< Initialize shared data structures.
    virtual bool Setup(); //!< Setup shared data structures.
    virtual RegisterOperand* SetAddressingOperand(AddressingOperand* pOpr, uint64 alignment); //!< Set addressing operand.
    inline Generator* GetGenerator() const { return mpGenerator; } //!< Return pointer to generator object.
    inline Instruction* GetInstruction() const { return mpInstruction; } //!< Return pointer to instruction object.
    inline const AddressingOperand* GetAddressingOperand() const { return mpAddressingOperand; } //!< Return const pointer to the AddressingOperand object.
    inline const AddressingOperandConstraint* GetAddressingOperandConstraint() const { return mpAddressingOperandConstraint; } //!< Return const pointer to the AddressingOperandConstraint object.
    inline uint32 Size() const { return mSize; } //!< Return access size.
    inline uint32 VmTimeStamp() const { return mVmTimeStamp; } //!< Return VM constraint update time stamp.
    inline bool IsInstruction() const { return mIsInstruction; } //!< Return whether the access is instruction.
    void ApplyVirtualUsableConstraint(ConstraintSet* constrSet, uint32& rTimeStamp) const; //!< Apply virtual usable constraint to specified constraint.
    inline const ConstraintSet* TargetConstraint() const { return mpTargetConstraint; } //!< Return pointer to target constraint.
    const ConstraintSet* PcConstraint() const { return mpPcConstraint; } //!< Return PC spacing constraint.
    VmMapper* GetVmMapper() const { return mpVmMapper; } //!< Return pointer to VmMapper object.
    const AddressTagging* GetAddressTagging() const { return mpAddressTagging; } //!< Return pointer to AddressTagging object.
    inline uint64 AlignMask() const { return mAlignMask; } //!< Return alignment mask.
    inline uint32 AlignShift() const { return mAlignShift; } //!< Return alignment shift.
    inline bool AlignmentOkay(uint64 value) const { return ((value & mAlignMask) == value); } //!< Check if value alignment is okay.
    bool SolveFree() const; //!< Solve a free target.
    uint64 FreeTarget() const { return mFreeTarget; } //!< Return solved free target.
    virtual uint64 FreeOffset(AddressingMode* pAddrMode) const {return 0;} //!< Return free offset value, if applicable.
    bool OperandConflict(const Register* pReg) const; //!< Check if the register is in conflict with previously generated operands.
    bool MapTargetAddressRange(uint64 targetAddress, uint32& rTimeStamp) const; //!< Maps the target address range to a physical address range. Returns false if the mapped-to physical address range is unusable.
    std::vector<ConstraintSet*> TargetListConstraint() const {return mpTargetListConstraint;} //!< Return vector of Target list constraint.
  protected:
    const ConstraintSet* GetAllocatedConstraint(ERegisterType regType) const; //!< Get allocated constraint of register indices.
    virtual const ConstraintSet* GetPcSpaceConstraint() const; //!< Get proper PC space constraint.
    bool ReVerifyTargetAddressRange(uint64 targetAddress) const; //!< Verify target address range again when necessary.
  protected:
    Generator* mpGenerator; //!< Pointer to the generator object.
    Instruction* mpInstruction; //!< Pointer to the instruction object.
    AddressingOperand* mpAddressingOperand; //!< Pointer to the addressing operand.
    AddressingOperandConstraint* mpAddressingOperandConstraint; //!< Pointer to the addressing operand constraint object.
    VmMapper* mpVmMapper; //!< Pointer to VmMapper object.
    const AddressTagging* mpAddressTagging; //!< Pointer to AddressTagging object.
    const ConstraintSet* mpTargetConstraint; //!< Const pointer to target constraint object.
    const ConstraintSet* mpPcConstraint; //!< Const pointer to PC spacing constraint object.
    uint64 mAlignMask; //!< Alignment mask.
    mutable uint64 mFreeTarget; //!< Free address target.
    uint32 mAlignment; //!< Alignment value.
    uint32 mAlignShift; //!< Alignment shift.
    uint32 mSize; //!< Size of access.
    mutable uint32 mVmTimeStamp; //!< Time stamp to track VM constraint updates.
    bool mIsInstruction; //!< Indicate if the access is for instruction.
    EMemDataType mMemDataType; //!< Memory data type.
    EMemAccessType mMemAccessType; //!< Memory accessing mode.
    mutable bool mFreeTried; //!< Tried free target solving.
    mutable bool mFreeValid; //!< Indicate if mFreeTarget is valid.
    std::vector <VmConstraint* > mHardVmConstraints; //!< Hard VM constraints if any.
    mutable std::map<ERegisterType, ConstraintSet* > mIntraAllocations; //!< Constraint reflecting register operand already generated.
    std::vector<ConstraintSet*> mpTargetListConstraint; //!< Target list constraint.
    friend class AddressSolver;
  };

  /*!
    \class RegisterBranchSolvingShared
    \brief Class holding necessary shared register-branch address solving data structures.
  */
  class RegisterBranchSolvingShared : public AddressSolvingShared {
  public:
    RegisterBranchSolvingShared() //!< Default constructor.
      : AddressSolvingShared()
    {
    }

    ~RegisterBranchSolvingShared() { } //!< Destructor.
  protected:
    const ConstraintSet* GetPcSpaceConstraint() const override; //!< Get proper PC space constraint for register-branch addressing mode.
    friend class AddressSolver;
  };

  class ImmediateOperand;

  /*!
    \class BaseOffsetSolvingShared
    \brief Class holding necessary shared base-offset address solving data structures.
  */
  class BaseOffsetSolvingShared : public AddressSolvingShared {
  public:
    BaseOffsetSolvingShared() //!< Default constructor.
      : AddressSolvingShared(), mOffsetValue(0), mOffsetSolved(false)
    {
    }

    ~BaseOffsetSolvingShared() { } //!< Destructor.
    uint64 FreeOffset(AddressingMode* pAddrMode) const override; //!< Return offset value in base register free case.
  protected:
    mutable uint64 mOffsetValue; //!< Offset value in free register case.
    mutable bool mOffsetSolved; //!< Indicate if offset has been solved.
    friend class AddressSolver;
  };

  /*!
    \class BaseOffsetShiftSolvingShared
    \brief Class holding necessary shared base with scaled offset address solving data structures.
  */
  class BaseOffsetShiftSolvingShared : public BaseOffsetSolvingShared {
  public:
    explicit BaseOffsetShiftSolvingShared(uint32 offsetScale) //!< Constructor.
      : BaseOffsetSolvingShared(), mScaleAlignMask(0), mScaleAlignShift(0), mOffsetScale(offsetScale)
    {
    }

    ~BaseOffsetShiftSolvingShared() { } //!< Destructor.

    RegisterOperand* SetAddressingOperand(AddressingOperand* pOpr, uint64 alignment) override; //!< Set addressing operand.
    inline uint64 ScaleAlignMask() const { return mScaleAlignMask; } //!< Return alignment mask adjusted for offset scale.
    inline uint32 ScaleAlignShift() const { return mScaleAlignShift; } //!< Return alignment shift adjusted for offset scale.
  protected:
    uint64 mScaleAlignMask; //!< Alignment mask adjusted for offset scale.
    uint32 mScaleAlignShift; //!< Alignment shift adjusted for offset scale.
    uint32 mOffsetScale; //!< Offset scale value.
  };

  /*!
    \class BaseIndexSolvingShared
    \brief Class holding necessary shared base-index address solving data structures.
  */
  class BaseIndexSolvingShared : public AddressSolvingShared {
  public:
    BaseIndexSolvingShared() //!< Default constructor.
      : AddressSolvingShared(), mpIndexOperand(nullptr), mIndexChoices(), mExtendType(EExtendType(0)), mExtendAmount(0){ }
    virtual ~BaseIndexSolvingShared(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(BaseIndexSolvingShared);
    COPY_CONSTRUCTOR_ABSENT(BaseIndexSolvingShared);
    bool Setup() override; //!< Setup base-index shared data structures.
    inline const std::vector<AddressingRegister* >& GetIndexChoices() const { return mIndexChoices; } //!< Return a const reference to the index choices.
    inline EExtendType ExtendType() const { return mExtendType; } //!< Return extend type.
    inline uint32 ExtendAmount() const { return mExtendAmount; } //!< Return extend amount.
  protected:
    bool GetAvailableIndexChoices(); //!< Get available index choices.
  protected:
    const RegisterOperand* mpIndexOperand; //!< Const pointer to index operand.
    std::vector<AddressingRegister* > mIndexChoices; //!< Available index register choices.
    EExtendType mExtendType; //!< Extension type.
    uint32 mExtendAmount; //!< Extend amount.
  };

  /*!
    \class VectorStridedSolvingShared
    \brief Class holding necessary shared vector strided address solving data structures.
  */
  class VectorStridedSolvingShared : public AddressSolvingShared {
  public:
    VectorStridedSolvingShared();
    COPY_CONSTRUCTOR_ABSENT(VectorStridedSolvingShared);
    ~VectorStridedSolvingShared() override;
    ASSIGNMENT_OPERATOR_ABSENT(VectorStridedSolvingShared);

    bool Setup() override; //!< Setup shared data structures.
    const std::vector<AddressingRegister*>& GetStrideChoices() const { return mStrideChoices; } //!< Return available stride register choices.
    uint32 GetDataElementCount() const; //!< Return the number of data vector register elements.
  private:
    void SetupStrideChoices(); //!< Record the available stride choices.
  private:
    const RegisterOperand* mpStrideOpr; //!< Stride operand
    std::vector<AddressingRegister*> mStrideChoices; //!< Available stride register choices
  };

  class AddressingMultiRegister;

  /*!
    \class VectorIndexedSolvingShared
    \brief Class holding necessary shared vector indexed address solving data structures.
  */
  class VectorIndexedSolvingShared : public AddressSolvingShared {
  public:
    VectorIndexedSolvingShared();
    COPY_CONSTRUCTOR_ABSENT(VectorIndexedSolvingShared);
    ~VectorIndexedSolvingShared() override;
    ASSIGNMENT_OPERATOR_ABSENT(VectorIndexedSolvingShared);

    bool Setup() override; //!< Setup shared data structures.
    const std::vector<AddressingMultiRegister*>& GetIndexChoices() const { return mIndexChoices; } //!< Return available index operand choices.
    uint32 GetIndexElementSize() const; //!< Return the size in bits of each index vector register element.
  private:
    void SetupIndexChoices(); //!< Record the available index choices.
  private:
    const RegisterOperand* mpIndexOpr; //!< Index operand
    std::vector<AddressingMultiRegister*> mIndexChoices; //!< Available index operand choices
  };

  /*!
    \class BaseIndexAmountBitSolvingShared
    \brief Class holding necessary shared base-index address solving data structures.
  */
  class BaseIndexAmountBitSolvingShared : public BaseIndexSolvingShared {
  public:
    BaseIndexAmountBitSolvingShared() //!< Default constructor.
      : BaseIndexSolvingShared(), mExtendAmount0Valid(false), mExtendAmount1Valid(false){}

    ASSIGNMENT_OPERATOR_ABSENT(BaseIndexAmountBitSolvingShared);
    COPY_CONSTRUCTOR_ABSENT(BaseIndexAmountBitSolvingShared);
    bool Setup() override; //!< Setup base-index shared data structures.
    inline bool IsExtendAmount0Valid() const { return mExtendAmount0Valid; } //!< Return whether extend amount 0 is valid.
    inline bool IsExtendAmount1Valid() const { return mExtendAmount1Valid; } //!< Return whether extend amount 1 is valid.
  protected:
    bool mExtendAmount0Valid; //!< Indicate if extend amount 0 is valid.
    bool mExtendAmount1Valid; //!< Indicate if extend amount 1 is valid.
  };


  /*!
    \class DataProcessingSolvingShared
    \brief Class holding necessary shared data processing address solving data structures.
  */
  class DataProcessingSolvingShared : public AddressSolvingShared {
  public:
    DataProcessingSolvingShared() //!< Default constructor.
      : AddressSolvingShared(), mpTargetAddressConstraint(nullptr), mCondFlags()
    {
    }

    ~DataProcessingSolvingShared(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(DataProcessingSolvingShared);
    COPY_CONSTRUCTOR_ABSENT(DataProcessingSolvingShared);
    bool Setup() override; //!< Setup data processing shared data structures.
    const ConstraintSet* GetTargetAddressConstraint() const { return mpTargetAddressConstraint; }
    ConditionFlags GetConditionFlags() const { return mCondFlags; }
  protected:
    ConstraintSet* mpTargetAddressConstraint; //!< Target address constraint.
    ConditionFlags mCondFlags; //!< Condition flags.
  };
 /*!
    \class BaseOffsetMulSolvingShared
    \brief Class holding necessary shared base with multiply offset address solving data structures.
  */
  class BaseOffsetMulSolvingShared : public BaseOffsetSolvingShared {
  public:
    explicit BaseOffsetMulSolvingShared(uint32 offsetMulAata) //!< Constructor.
      : BaseOffsetSolvingShared(), mOffsetMulData(offsetMulAata){}
    ~BaseOffsetMulSolvingShared() { } //!< Destructor.
    inline uint32 OffsetMulData() const {return mOffsetMulData;} //Return multiplier of offset
  protected:
    uint32 mOffsetMulData; //!< Offset Multiple value.
  };


}

#endif
