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
#ifndef Force_InstructionStructure_H
#define Force_InstructionStructure_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <FieldEncoding.h>
#include <UopInterface.h>

#include <vector>
#include <map>
#include <string>

namespace Force {

  class OperandStructure;
  class AsmText;

  /*!
    \class InstructionStructure
    \brief Record static structural information about an instruction.
  */
  class InstructionStructure {
  public:
    explicit InstructionStructure(const std::string& iclass) : mOperandStructures(), mShortOperandStructures(), mConstantValue(0), mSize(0), mElementSize(0), mGroup{EInstructionGroupType(0)}, mName(), mForm(), mIsa(), mClass(iclass), mAliasing(), mExtension{EInstructionExtensionType(0)}, mpAsmText(nullptr) { } //!< Constructor with instruction class given.
    ~InstructionStructure();   //!< Destructor, must release children OperandStructure objects.

    const std::string& Name() const { return mName; } //!< Return instruction name.
    const std::string FullName() const; //!< Return instruction full name.
    void AddOperand(OperandStructure* opr_struct); //!< Add child OperandStructure.
    void SetConstantValue(uint32 const_val, uint32 size) { mConstantValue = const_val; mSize += size; } //!< Set encoding constant value.
    uint32 ConstantValue() const { return mConstantValue; } //!< Return encoding constant value.
    const std::vector<OperandStructure* >& OperandStructures() const { return mOperandStructures; } //!< Return constant reference of vector of children operands
    const std::map<const std::string, const OperandStructure* >& GetShortOperandStructures() const { return mShortOperandStructures; } //!< return constant reference of map of children operands.
    uint32 Size() const { return mSize; } //!< Return instruction size.
    uint32 ElementSize() const { return mElementSize; } //!< Return instrution element size.
    const std::string ToString() const;
  private:
    DEFAULT_CONSTRUCTOR_ABSENT(InstructionStructure); //!< No use of default constructor.
    COPY_CONSTRUCTOR_ABSENT(InstructionStructure); //!< No use of copy constructor.
    ASSIGNMENT_OPERATOR_ABSENT(InstructionStructure);
  private:
    std::vector<OperandStructure* > mOperandStructures; //!< Vector of children operands
    std::map<const std::string, const OperandStructure* > mShortOperandStructures; // mapper of children operands, the key is operand short name
    uint32 mConstantValue;     //!< The constant part of the instruction encoding.
    uint32 mSize; //!< Size of the instruction in number of bits.
    uint32 mElementSize; //!< Size of the instruction element in bytes.
  public:
    EInstructionGroupType mGroup; //!< Instruction group
    std::string mName;  //!< Instruction name
    std::string mForm;  //!< Instruction form
    std::string mIsa;   //!< Instruction ISA
    std::string mClass; //!< Associated instruction class
    std::string mAliasing; //!< Note that the instruction is an aliasing of instruction mentioned in this attribute.
    EInstructionExtensionType mExtension; //!< Instruction architectural extension
    AsmText* mpAsmText; //!< Pointer to AsmText object.

    friend class InstructionParser;
  };

  /*!
    \class OperandStructure
    \brief Record static structural information nabout an operand.

    An OperandStructure object directly correspond to an \<O\> element in Force instruction XML files.
    The static information contained in an OperandStructure instance can be shared among all Operand objects that was instantiated from the same OperandStructure object.
    Each OperandStructure object is meant to be unique and should not be copied around.
    This data structure is meant to be accessed by Operand class and instruction file parsing code, therefore some of the member attributes are made public.

    Copy constructor and assignment operators are intentionally made absent.
  */
  class OperandStructure {
  public:
  OperandStructure() : mName(), mShortName(), mClass(), mType(EOperandType(0)), mAccess(ERegAttrType::Read), mSize(0), mMask(0), mEncodingBits(), mSlave(false), mUopParamType(UopParamBool), mDiffers() { } //!< Constructor, empty
    virtual ~OperandStructure() { } //!< Destructor, virtual
    const std::string& Name() const { return mName; }
    const std::string& ShortName() const { return mShortName; }
    uint32 Size() const { return mSize; } //!< Return operand size.
    const EOperandType& Type() const { return mType; } //!< return operand type

    void SetBits(const std::string& bits_str); //!< Assign the opcode bits occupied by the operand.
    uint32 Encoding(uint32 opr_value) const;   //!< Encode the operand value into its opcode bits.
    void AddDiffer(const std::string& differ) { mDiffers.push_back(differ); } //<! Add name of operand that must have a different value
    const std::vector<std::string>& GetDiffers() const { return mDiffers; } //<! Get names of operands that must have a different value
    virtual const std::string ToString() const; //!< Return string representing the Operand structure details.

    /*!
      Templated function to cast sub class of OperandStructure
      For example, const ChoicesOperandStructure* co_struct = op_struct.CastOperandStructure<ChoicesOperandStructure>();
     */
    template<typename T>
      const T* CastOperandStructure() const
      {
        auto cast_struct = dynamic_cast<const T*>(this);
        if (nullptr == cast_struct) {
          FailedTocast();
        } else {
          return cast_struct;
        }
        return nullptr;
      }

    inline bool HasWriteAccess() const //!< Return whether the operand has write access.
    {
      return (ERegAttrTypeBaseType(mAccess) & ERegAttrTypeBaseType(ERegAttrType::Write)) == ERegAttrTypeBaseType(ERegAttrType::Write);
    }
    inline bool HasReadAccess() const //!< Return whether the operand has read access.
    {
      return (ERegAttrTypeBaseType(mAccess) & ERegAttrTypeBaseType(ERegAttrType::Read)) == ERegAttrTypeBaseType(ERegAttrType::Read);
    }
    virtual void AddShortOperand(std::map<const std::string, const OperandStructure* >& rShortStructures) const; //!< add short operand to its structures
    inline bool IsSlave() const { return mSlave; } //!< Return whether the operand is slave or not
    inline EUopParameterType UopParameterType() const { return mUopParamType; } //!< Return type to be used in the Uop interface
  protected:
    COPY_CONSTRUCTOR_ABSENT(OperandStructure); //!< Copy constructor absent.
    void FailedTocast() const; //!< Return failure to cast operand.
  public:
    std::string mName; //!< Operand name
    std::string mShortName; //!< operand short name
    std::string mClass; //!< Operand class name
    EOperandType mType; //!< Operand type
    ERegAttrType mAccess; //!< Register access type if applicable.
    uint32 mSize; //!< Size of operand field.
    uint32 mMask; //!< Operand size-mask used in randomization.
    std::vector<EncodingBits> mEncodingBits; //!< Opcode bits of the operand.
    bool mSlave; //!< slave operand which is sub-ordered by its master
    EUopParameterType mUopParamType; //!< type to be used in the Uop interface if applicable
  private:
    std::vector<std::string> mDiffers; //!< Names of operands that must have a different value
  };

  /*!
    \class AuthOperandStructure
    \brief Record static structural information about an auth operand.
  */
  class AuthOperandStructure : public OperandStructure {
  public:
  AuthOperandStructure() : OperandStructure(), mPointer(), mModifier(), mKey()  { } //!< Constructor, empty
    ~AuthOperandStructure() { } //!< Destructor, virtual
    void SetPointer(const std::string& pointer) { mPointer = pointer; } //!< set pointer
    const std::string& Pointer() const { return mPointer; }  //!< return pointer
    void SetModifier(const std::string& modifier) { mModifier = modifier; } //!< set modifier
    const std::string& Modifier() const {return mModifier; }  //!< return modifier
    void SetKey(const std::string& key) { mKey = key; }  //!< set key
    const std::string& Key() const {return mKey; }  //!< return key
  protected:
    COPY_CONSTRUCTOR_ABSENT(AuthOperandStructure); //!< Copy constructor absent.
  protected:
    std::string mPointer;  //!< the pointer to do auth
    std::string mModifier;//!< the modifier to do auth
    std::string mKey;      //!< the key for auth
  };

  /*!
    \class ChoicesOperandStructure
    \brief Record static structural information about an operand with choices attributes.
  */
  class ChoicesOperandStructure : public OperandStructure {
  public:
    ChoicesOperandStructure() : OperandStructure(), mChoices() { } //!< Constructor, empty
    ~ChoicesOperandStructure() { } //!< Destructor, virtual
  protected:
    COPY_CONSTRUCTOR_ABSENT(ChoicesOperandStructure); //!< Copy constructor absent.
  public:
    std::vector<std::string> mChoices; //!< Names of the choices settting.
  };

  /*!
    \class VectorRegisterOperandStructure
    \brief Record static structural information about a vector register operand.
  */
  class VectorRegisterOperandStructure : public ChoicesOperandStructure {
  public:
    VectorRegisterOperandStructure();
    COPY_CONSTRUCTOR_ABSENT(VectorRegisterOperandStructure);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorRegisterOperandStructure);
    ASSIGNMENT_OPERATOR_ABSENT(VectorRegisterOperandStructure);

    void SetLayoutMultiple(const float layoutMultiple) { mLayoutMultiple = layoutMultiple; } //!< Set the multiple used to adjust the register operand layout for widening and narrowing instructions.
    float GetLayoutMultiple() const { return mLayoutMultiple; } //!< Return the multiple used to adjust the register operand layout for widening and narrowing instructions.
  private:
    float mLayoutMultiple; //!< Multiple used to adjust the register operand layout for widening and narrowing instructions
  };

  class ConstraintSet;

  /*!
    \class ExcludeOperandStructure
    \brief Record static structural information about an operand with exclude attribute.
  */
  class ExcludeOperandStructure : public OperandStructure {
  public:
    ExcludeOperandStructure() : OperandStructure(), mExclude(), mpExcludeConstraint(nullptr) { } //!< Constructor, empty
    ~ExcludeOperandStructure(); //!< Destructor, virtual
    const ConstraintSet* ExcludeConstraint() const; //!< Return the exclude constraint.
  private:
    COPY_CONSTRUCTOR_ABSENT(ExcludeOperandStructure); //!< Copy constructor absent.
    ASSIGNMENT_OPERATOR_ABSENT(ExcludeOperandStructure);
  public:
    std::string mExclude; //!< Value exclusion string.
    mutable ConstraintSet* mpExcludeConstraint; //!< ConstraintSet with some value excluded.
  };

  /*!
    \class GroupOperandStructure
    \brief Record static structural information about an operand with sub operands.
  */
  class GroupOperandStructure : public OperandStructure {
  public:
    GroupOperandStructure() : OperandStructure(), mOperandStructures() { } //!< Constructor, empty
    ~GroupOperandStructure(); //!< Destructor, virtual

    void AddOperand(OperandStructure* opr_struct); //!< Add sub operand.
    void AddShortOperand(std::map<const std::string, const OperandStructure* >& rShortStructures) const override; //!< add short operand to its structures
  protected:
    COPY_CONSTRUCTOR_ABSENT(GroupOperandStructure); //!< Copy constructor absent.
  public:
    std::vector<OperandStructure* > mOperandStructures; //!< Vector of children operands
  };

  /*!
    \class VectorLayoutOperandStructure
    \brief Record static structural information about a vector layout operand.
  */
  class VectorLayoutOperandStructure : public OperandStructure {
  public:
    VectorLayoutOperandStructure();
    COPY_CONSTRUCTOR_ABSENT(VectorLayoutOperandStructure);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorLayoutOperandStructure);
    ASSIGNMENT_OPERATOR_ABSENT(VectorLayoutOperandStructure);

    uint32 GetRegisterCount() const { return mRegCount; } //!< Get the number of registers per vector register group.
    uint32 GetElementWidth() const { return mElemWidth; } //!< Get the width of each element in the vector register group.
    uint32 GetRegisterIndexAlignment() const { return mRegIndexAlignment; } //!< Get a power of 2 to which vector register indices must be aligned.
    void SetRegisterCount(cuint32 regCount) { mRegCount = regCount; } //!< Set the number of registers per vector register group.
    void SetElementWidth(cuint32 elemWidth) { mElemWidth = elemWidth; } //!< Set the width of each element in the vector register group
    void SetRegisterIndexAlignment(cuint32 regIndexAlignment) { mRegIndexAlignment = regIndexAlignment; } //!< Set a power of 2 to which vector register indices must be aligned.
  private:
    uint32 mRegCount; //!< The number of registers per vector register group.
    uint32 mElemWidth; //!< The width of each element in the vector register group.
    uint32 mRegIndexAlignment; //!< A power of 2 to which vector register indices must be aligned
  };

  /*!
    \class AddressingOperandStructure
    \brief Record static structural information about an addressing operand.
  */
  class AddressingOperandStructure : public GroupOperandStructure {
  public:
    AddressingOperandStructure() : GroupOperandStructure(), mBase(), mOffset(), mOffsetScale(0), mMemAccessType(EMemAccessType::Read) { } //!< Constructor, empty
    ~AddressingOperandStructure() { } //!< Destructor, virtual
  protected:
    COPY_CONSTRUCTOR_ABSENT(AddressingOperandStructure); //!< Copy constructor absent.
  public:
    void SetBase(const std::string& base) { mBase = base; } //!< Implement set base method to set base operand name.
    const std::string& Base() const { return mBase; } //!< Return base operand name.
    void SetOffset(const std::string& offset) { mOffset = offset; } //!< Set offset operand name
    const std::string& Offset() const { return mOffset; } //!< Return offset operand name, if applicable.
    void SetOffsetScale(uint32 offsetScale) const { mOffsetScale = offsetScale; } //!< Set offset scale value.
    uint32 OffsetScale() const { return mOffsetScale; } //!< Return offset scale value.
    void SetMemAccessType(EMemAccessType memAccessType) { mMemAccessType = memAccessType; } //!< Set memory access type like read/write
    EMemAccessType MemAccessType() const { return mMemAccessType; }
  protected:
    std::string mBase; //!< Base operand name.
    std::string mOffset; //!< Offset operand name, if applicable.
    mutable uint32 mOffsetScale; //!< Offset scale value.
    EMemAccessType mMemAccessType; //!< indicate memory access type, if appliable
  };

  /*!
    \class BranchOperandStructure
    \brief Record static structural information about an Branch type operand.
  */
  class BranchOperandStructure : public AddressingOperandStructure {
  public:
    BranchOperandStructure() : AddressingOperandStructure(), mCondition(EBranchConditionType(0)) { } //!< Constructor, empty
    ~BranchOperandStructure() { } //!< Destructor, virtual
  protected:
    COPY_CONSTRUCTOR_ABSENT(BranchOperandStructure); //!< Copy constructor absent.
  public:
    EBranchConditionType mCondition; //!< Condition type.
  };

  /*!
    \class LoadStoreOperandStructure
    \brief Record static structural information about an LoadStore type operand.
  */
  class LoadStoreOperandStructure : public AddressingOperandStructure {
  public:
    LoadStoreOperandStructure() : AddressingOperandStructure(), mIndex(), mPreIndex(), mPostIndex(), mExtendAmount(), mElementSize(0), mDataSize(0), mAlignment(0), mSignExtension(0), mUnprivileged(false), mMemLoadType(EMemOrderingType::Init), mMemStoreType(EMemOrderingType::Init), mIsOffsetShift(true), mPredicateReg(), mOffsSize(0) { } //!< Constructor, empty
    ~LoadStoreOperandStructure() { } //!< Destructor, virtual

    void SetIndex(const std::string& index) { mIndex = index; } //!< Set index operand name
    const std::string& Index() const { return mIndex; } //!< Return index operand name, if applicable.
    void SetPreIndex(const std::string& preIndex) { mPreIndex = preIndex; } //!< Set pre-index operand name
    const std::string& PreIndex() const { return mPreIndex; } //!< Return pre-index operand name, if applicable.
    void SetPostIndex(const std::string& postIndex) { mPostIndex = postIndex; } //!< Set post-index operand name
    const std::string& PostIndex() const { return mPostIndex; } //!< Return post-index operand name, if applicable.
    void SetExtendAmount(const std::string& extendAmount) { mExtendAmount = extendAmount; } //!< Set extend operand name
    const std::string& ExtendAmount() const { return mExtendAmount; } //!< return extend operand name
    void SetElementSize(uint32 eSize) const; //!< Set element size.
    uint32 ElementSize() const { return mElementSize; } //!< Return element size.
    void SetDataSize(uint32 dSize) const { mDataSize = dSize; } //!< Set data size.
    uint32 DataSize() const { return mDataSize; } //!< Return data size.
    void SetAlignment(uint32 align) const { mAlignment = align; } //!< Set data alignment.
    void SetSignExtension(uint32 signExtension) const {mSignExtension = signExtension; } //!< set sign extension
    uint32 Alignment() const { return mAlignment; } //!< Return data alignment.
    uint32 SignExtension() const { return mSignExtension; } //!< return sign extension.
    void SetUnprivileged(bool unpriv) { mUnprivileged = unpriv; } //!< Set unprivileged attribute.
    bool Unprivileged() const { return mUnprivileged; } //!< Return unprivileged attribute value, if applicable.
    void SetLoadType(EMemOrderingType ordering) { mMemLoadType = ordering; }
    void SetStoreType(EMemOrderingType ordering) {mMemStoreType = ordering; }
    bool AtomicOrderedAccess() const ; //!< whether the access is atomic or ordered.
    void SetIsOffsetShift(bool isOffsetShift) const{ mIsOffsetShift = isOffsetShift;}
    bool GetIsOffsetShift() const {return mIsOffsetShift;}
    inline uint64 AdjustOffset(uint64 offset) const
    {
      if (mIsOffsetShift) {return offset <<= OffsetScale();}
      else{ return offset *= OffsetScale();}
    }
    bool SpBased() const; //!< whether base register can be stack pointer or not.
    void SetPredicateReg(const std::string& predicateReg){ mPredicateReg = predicateReg;}//!< Set predicate operand name
    const std::string& PredicateReg() const { return mPredicateReg;}//!< Return predicate operand name,if applicable.
    void SetOffsSize(uint32 offsSize) const { mOffsSize = offsSize;}//!< Set offset size name
    uint32 OffsSize() const {return mOffsSize;}//!< Return offset size,if applicable.

  protected:
    COPY_CONSTRUCTOR_ABSENT(LoadStoreOperandStructure); //!< Copy constructor absent.
  public:
    std::string mIndex; //!< Index operand name, if applicable.
    std::string mPreIndex; //!< Pre-index operand name, if applicable.
    std::string mPostIndex; //!< Post-index operand name, if applicable.
    std::string mExtendAmount;  //!< Extend operand name, if applicable.
    mutable uint32 mElementSize; //!< Data element size.
    mutable uint32 mDataSize; //!< Overall data size.
    mutable uint32 mAlignment; //!< Data alignment.
    mutable uint32 mSignExtension; //!< signed extension to Bytes.  no extension if zero
    bool mUnprivileged; //!< Indicate unprivileged access, if applicable.
    EMemOrderingType mMemLoadType; //!< indicate memory ordering type for load
    EMemOrderingType mMemStoreType; //!< indicate memory ordering type for store
    mutable bool mIsOffsetShift;//!< Indicate that OffsetScale is shift or multiply.
    std::string mPredicateReg; //!< Preidcate operand name,if applicable.
    mutable uint32 mOffsSize;//!< offset size
  };

  /*!
    \class SystemOpOperandStructure
    \brief Record static structural information about system op operand.
  */
  class SystemOpOperandStructure : public LoadStoreOperandStructure {
  public:
    SystemOpOperandStructure() : LoadStoreOperandStructure(), mAddressBased(false), mSystemOpType() { } //!< Constructor, empty
    ~SystemOpOperandStructure() { } //!< Destructor, virtual
    void SetAddressBased(bool addr_based) {mAddressBased = addr_based;}
    bool IsAddressBased(void) const {return mAddressBased;}
    void SetSystemOpType(ESystemOpType type) { mSystemOpType = type; } //!< Set the system operation type.
    ESystemOpType SystemOpType() const {return mSystemOpType;} //!< return exception level of address translation.
  protected:
    COPY_CONSTRUCTOR_ABSENT(SystemOpOperandStructure); //!< Copy constructor absent.
  protected:
    bool mAddressBased; //!< address based or not
    ESystemOpType mSystemOpType; //!< System operation type
  };

   /*!
    \class AuthBranchOperandStructure
    \brief Record static structural information about an auth Branch type operand.
  */
  class AuthBranchOperandStructure : public BranchOperandStructure, public AuthOperandStructure {
  public:
    AuthBranchOperandStructure() : BranchOperandStructure(), AuthOperandStructure() { } //!< Constructor, empty
    ~AuthBranchOperandStructure() { } //!< Destructor, virtual
  protected:
    COPY_CONSTRUCTOR_ABSENT(AuthBranchOperandStructure); //!< Copy constructor absent.
  };

  /*!
    \class AuthLoadStoreOperandStructure
    \brief Record static structural information about an auth load/store type operand.
  */
  class AuthLoadStoreOperandStructure : public LoadStoreOperandStructure, public AuthOperandStructure {
  public:
    AuthLoadStoreOperandStructure() : LoadStoreOperandStructure(), AuthOperandStructure() { } //!< Constructor, empty
    ~AuthLoadStoreOperandStructure() { } //!< Destructor, virtual
  protected:
    COPY_CONSTRUCTOR_ABSENT(AuthLoadStoreOperandStructure); //!< Copy constructor absent.
  };

  /*!
    \class AluOperandStructure
    \brief Record static structural information about ALU type operand.
  */
  class AluOperandStructure : public AddressingOperandStructure {
  public:
    AluOperandStructure() : AddressingOperandStructure(), mImmediate(), mOffsetShift(), mOperationType(EAluOperationType::Unknown) { } //!< Default constructor.
    ~AluOperandStructure() { } //!< Destructor, virtual

    void SetOffsetShift(const std::string& offset_shift) { mOffsetShift = offset_shift; } //!< set offset shift
    void SetImmediate(const std::string& imm) {mImmediate = imm; } //!< set immediate
    void SetOperationType(EAluOperationType operationType) { mOperationType = operationType; } //!< set ALU operation type
    const std::string& OffsetShift() const { return mOffsetShift; } //!< get offset shift
    const std::string& Immediate() const { return mImmediate; } //!< get immediate
    EAluOperationType OperationType() const { return mOperationType; } //!< get ALU operation type
  protected:
    COPY_CONSTRUCTOR_ABSENT(AluOperandStructure); //!< Copy constructor absent.
  protected:
    std::string mImmediate; //!< immediate field
    std::string mOffsetShift; //!< offset shift
    EAluOperationType mOperationType; //!< ALU operation type
  };

  /*!
    \class DataProcessingOperandStructure
    \brief Record static structural information about data processing type operand.
  */
  class DataProcessingOperandStructure : public AddressingOperandStructure {
  public:
    DataProcessingOperandStructure() : AddressingOperandStructure(), mOperationType(EDataProcessingOperationType::Unknown), mOperandRoles(), mUop(UopAddWithCarry) { } //!< Default constructor.
    COPY_CONSTRUCTOR_ABSENT(DataProcessingOperandStructure);
    SUBCLASS_DESTRUCTOR_DEFAULT(DataProcessingOperandStructure);
    ASSIGNMENT_OPERATOR_ABSENT(DataProcessingOperandStructure);

    void SetOperationType(EDataProcessingOperationType operationType) { mOperationType = operationType; } //!< set data processing operation type
    EDataProcessingOperationType OperationType() const { return mOperationType; } //!< get data processing operation type
    void AddOperandRole(const std::string& oprName, const std::string& roleName); //!< map operand name to role name
    std::string GetOperandRole(const std::string& oprName) const; //!< get role for operand
    void SetUop(EUop uop) { mUop = uop; } //!< set micro-op type
    EUop Uop() const { return mUop; } //!< get micro-op type
  protected:
    EDataProcessingOperationType mOperationType; //!< data processing operation type
    std::map<std::string, std::string> mOperandRoles; //!< map of operand names to role names
    EUop mUop; //!< Micro-op type.
  };

}

#endif
