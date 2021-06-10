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
#ifndef Force_GenRequest_H
#define Force_GenRequest_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

#include <map>
#include <set>
#include <vector>

/*!
  Declare pybind11::object type without actual definitions.
  To avoid spreading pybind11 classes to other backend modules unnecessarily.
*/
namespace pybind11 {
  class object; //!< Base pybind11 Python object class.
}
namespace py = pybind11;

namespace Force {

  class ConstraintSet;
  class Register;
  class BntNode;

  /*!
    \class GenRequest
    \brief Base class for all requests object to test generator thread.
   */
  class GenRequest {
  public:
    GenRequest() { } //!< Default constructor.
    virtual ~GenRequest() { } //!< Virtual destructor.

    virtual EGenAgentType GenAgentType() const = 0; //!< Return type of GenAgent to process this type of GenRequest.
    virtual void SetPrimaryValue(uint64 value) {} //!< Set primary value, with integer value parameter.
    virtual void SetPrimaryString(const std::string& valueStr) {} //!< Set primary string.
    virtual void AddDetail(const std::string& attrName, uint64 value) {} //!< Add request detail, with integer value parameter.
    virtual void AddDetail(const std::string& attrName, const std::string& valueStr); //!< Add request detail, with value string parameter.
    virtual const std::string ToString() const; //!< Return a string describing the current state of the GenRequest object.
    virtual bool AddingInstruction() const { return false; } //!< Indicates whether the GenRequest will result in adding instruction to the instruction stream, return false by default.
    virtual bool DelayHandle() const { return true; } //!< Indicates whether the GenRequest can be insert by other request, return true by default.
    virtual const char* RequestType() const { return "GenRequest"; } //!< Return GenRequest type.
    virtual void CleanUp() { } //!< do some clean up

    /*!
      Templated function so that a derived class can conveniently cast base class to the desired derived class type.
      For example, GenLoadRegister* load_reg_req = gen_req->CastInstance<GenLoadRegister>();
    */
    template<typename T>
      T* CastInstance()
      {
        T* cast_instance = dynamic_cast<T* >(this);
        return cast_instance;
      }

  protected:
    void UnsupportedRequestDetailAttribute(const std::string& attrName) const; //!< Called to report error on unsupported request detail attribute.
  };

  class OperandRequest;
  class OperandDataRequest;

  /*!
    \class GenInstructionRequest
    \brief A request to the test generator thread to generate an instruction.
  */
  class GenInstructionRequest : public GenRequest {
  public:
    explicit GenInstructionRequest(const std::string& id); //!< Constructor with id given.
    ~GenInstructionRequest(); //!< Destructor.
    const std::string ToString() const override; //!< Return a string describing the current state of the GenInstructionRequest object.
    EGenAgentType GenAgentType() const override { return EGenAgentType::GenInstructionAgent; } //!< Return type of the GenAgent to process GenInstructionRequest.
    const char* RequestType() const override { return "GenInstructionRequest"; } //!< Return GenInstructionRequest type.
    virtual GenInstructionRequest* Clone() const { return new GenInstructionRequest(*this); } //!< Return a clone of the GenInstructionRequest object.

    const std::string& InstructionId() const { return mInstructionId; } //!< Return instruction ID.
    void AddOperandRequest(const std::string& oprName, uint64 value); //!< Add individual operand request, with integer value parameter.
    void AddOperandRequest(const std::string& oprName, const std::string& valueStr); //!< Add individual operand request, with value string parameter.
    void AddOperandDataRequest(const std::string& oprName, const std::string& valueStr) const; //!< Add individual operand data request, with value string parameter.
    void AddLSDataRequest(const std::string& valueStr); //!< Add LSData request with value string parameter.
    void AddLSTargetListRequest(const std::string& valueStr); //!< Add LSTargets request with value string parameter.
    void SetOperandDataRequest(const std::string& oprName, uint64 value, uint32 size) const; //!< Add individual operand data request, with value parameter.
    void SetOperandDataRequest(const std::string& oprName, const std::string& valueStr, uint32 size) const; //!< Add individual operand data request, with value parameter.
    void SetOperandDataRequest(const std::string& oprName, std::vector<uint64> values, uint32 size) const; //!< For LargeRegister(Z),add individual operand data request, with values parameter.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add request detail, with value string parameter.
    bool AddingInstruction() const override { return true; } //!< Indicates GenInstructionRequest will add instruction to the instruction stream.

    const OperandRequest* FindOperandRequest(const std::string& opName) const; //!< Find OperandRequest for the operand with the given name, if any.
    const OperandDataRequest* FindOperandDataRequest(const std::string& opName) const;  //!< Find OperandDataRequest for the operand with the given name, if any.
    void NotAppliedOperandRequests() const; //!< ignore all operand requests that is not applied.
    void SetBoolAttribute(EInstrBoolAttrType attrType, bool isSet); //!< Set a boolean atribute to true or false.
    bool BoolAttribute(EInstrBoolAttrType attrType) const; //!< Return whether a boolean attribute is set.
    void SetConstraintAttribute(EInstrConstraintAttrType attrType, ConstraintSet* pConstrSet); //!< Set a constraint attribute with constraint.
    const ConstraintSet* ConstraintAttribute(EInstrConstraintAttrType attrType) const //!< Return whether a constraint attribute, if available.
    {
      return mInstructionConstraints[int(attrType)];
    }
    const std::vector<ConstraintSet* >& LSDataConstraints() const { return mLSDataConstraints; } //!< Return LSData constraints.
    const std::vector<ConstraintSet* >& LSTargetListConstraints() const { return mLSTargetListConstraints; } //!< Return LSTarget constraints.

  protected:
    GenInstructionRequest(const GenInstructionRequest& rOther); //!< Copy constructor.
  protected:
    std::string mInstructionId; //!< Instruction ID of this GenInstructionRequest object.
    uint64 mBoolAttributes; //!< Boolean type instruction attributes.
    std::map<std::string, OperandRequest* > mOperandRequests; //!< Container of all OperandRequest objects.
    mutable std::map<std::string, OperandDataRequest* > mOperandDataRequests; //!< Container of all OperandDataReqest objects.
    std::vector<ConstraintSet* > mInstructionConstraints; //!< Container of all instruction constraints.
    std::vector<ConstraintSet* > mLSDataConstraints; //!< Container of all LSData constraints.
    std::vector<ConstraintSet* > mLSTargetListConstraints; //!< Container of all LSTarget constraints for vector gather/scatter instructions.
  };

  /*!
    \class GenCallBackRequest
    \brief A request to call back functions on the front-end.
  */
  class GenCallBackRequest : public GenRequest {
  public:
    explicit GenCallBackRequest(ECallBackType callbackType) : GenRequest(), mCallBackType(callbackType) {} //!< Constructor.
    ~GenCallBackRequest(); // Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenCallBackRequest);

    const std::string ToString() const override; //!< Return a string describing the current state of the GenCallBackRequest object.
    EGenAgentType GenAgentType() const override { return EGenAgentType::GenCallBackAgent; } //!< Return type of the GenAgent to process GenCallBackRequest.
    const char* RequestType() const override { return "GenCallBackRequest"; } //!< Return GenCallBackRequest type.
    ECallBackType CallBackType() const { return mCallBackType; } //!< Return callback type.
  protected:
    ECallBackType mCallBackType; //!< callback type of the GenCallBackRequest object.
  };

  /*!
    \class GenCallBackBntRequest
    \brief A request to call back bnt on the front-end
  */
  class GenCallBackBntRequest : public GenCallBackRequest {
  public:
    explicit GenCallBackBntRequest(BntNode* pBntNode) : GenCallBackRequest(ECallBackType::Bnt), mpBntNode(pBntNode) {} //!< Constructor.
    ~GenCallBackBntRequest(); // Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenCallBackBntRequest);
    COPY_CONSTRUCTOR_ABSENT(GenCallBackBntRequest);
    const BntNode* GetBntNode() const { return mpBntNode; } //!< return the pointer to the bnt node
  protected:
    BntNode* mpBntNode; //!< pointer to BntNode object
  };

  /*!
    \class GenCallBackEretRequest
    \brief A request to call back eret preamble sequence on the front-end
  */
  class GenCallBackEretRequest : public GenCallBackRequest {
  public:
    explicit GenCallBackEretRequest(const std::string& seq_name) : GenCallBackRequest(ECallBackType::Eret), mPreambleSequence(seq_name) { } //!< constructor
    ~GenCallBackEretRequest() { } // Destructor

    const std::string& GetPreambleSequence() const { return mPreambleSequence; } //!< return preamble sequence
  protected:
    std::string mPreambleSequence; //!< preable sequence name
  };

  /*!
    \class GenSequenceRequest
    \brief A request to the test generator thread to generate a sequence, in most cases are a sequence of instructions.
  */
  class GenSequenceRequest : public GenRequest {
  public:
    explicit GenSequenceRequest(ESequenceType seqType); //!< Constructor with id given.
    explicit GenSequenceRequest(const std::string& seqTypeStr); //!< Constructor with id in string form given.
    const std::string ToString() const override; //!< Return a string describing the current state of the GenSequenceRequest object.
    const uint64 ValueVariable (uint32 argNum) const { return (mIntArgs[argNum-1]); }
    EGenAgentType GenAgentType() const override { return EGenAgentType::GenSequenceAgent; } //!< Return type of the GenAgent to process GenSequenceRequest.
    const char* RequestType() const override { return "GenSequenceRequest"; } //!< Return GenSequenceRequest type.
    ESequenceType SequenceType() const { return mSequenceType; } //!< Return sequence type.
    static GenSequenceRequest* GenSequenceRequestInstance(const std::string& rSequenceType); //!< Return a GenSequenceRequest type based on the string parameter.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
  protected:
    GenSequenceRequest(const GenSequenceRequest& rOther); //!< Copy constructor.
  protected:
    ESequenceType mSequenceType; //!< Sequence type of the GenSequenceRequest object.
    std::vector<uint64>mIntArgs;
  };

  class State;

  /*!
    \class GenStateTransitionRequest
    \brief A request to initiate a StateTransition
  */
  class GenStateTransitionRequest : public GenRequest {
  public:
    GenStateTransitionRequest(const State* pTargetState, const EStateTransitionType stateTransType, const EStateTransitionOrderMode orderMode, const std::vector<EStateElementType>& rStateElemTypeOrder);
    SUBCLASS_DESTRUCTOR_DEFAULT(GenStateTransitionRequest);
    ASSIGNMENT_OPERATOR_ABSENT(GenStateTransitionRequest);

    EGenAgentType GenAgentType() const override { return EGenAgentType::GenStateTransitionAgent; } //!< Return type of the GenAgent to process GenStateTransitionRequest.
    const char* RequestType() const override { return "GenStateTransitionRequest"; } //!< Return GenStateTransitionRequest type.

    const State* TargetState() const { return mpTargetState; } //!< Return a collection of one or more StateElements that describe the target State.
    EStateTransitionType StateTransitionType() const { return mStateTransType; } //!< Return the StateTransition type.
    EStateTransitionOrderMode OrderMode() const { return mOrderMode; } //!< Return the order mode for processing StateElements.
    std::vector<EStateElementType> StateElementTypeOrder() const { return mStateElemTypeOrder; } //!< Return the order for processing StateElements in ByStateElementType order mode.
  protected:
    GenStateTransitionRequest(const GenStateTransitionRequest& rOther);
  protected:
    const State* mpTargetState; //!< A collection of one or more StateElements that describe the target State
    const EStateTransitionType mStateTransType; //!< StateTransition type
    const EStateTransitionOrderMode mOrderMode; //!< The order mode for processing StateElements
    const std::vector<EStateElementType> mStateElemTypeOrder; //!< The order for processing StateElements in ByStateElementType order mode
  };

  /*!
    \class GenLoadRegister
    \brief A request to the test generator thread to generate a sequence to load a register to a certain value.
  */
  class GenLoadRegister : public GenSequenceRequest {
  public:
    GenLoadRegister(const std::string& regName, uint64 value, const std::string& inter_reg_name=""); //!< Constructor with register name and value given.
    const char* RequestType() const override { return "GenLoadRegister"; } //!< Return GenLoadRegister type.
    const std::string& RegisterName() const { return mRegisterName; } //!< Return the name of the register to load.
    uint64 RegisterValue() const { return mRegisterValue; } //!< Return the value of the register to load.
    const std::string& InterRegisterName() const { return mInterRegName; }  //!< Return the name of the intermediate register
  protected:
    GenLoadRegister(const GenLoadRegister& rOther); //!< Copy constructor.
  protected:
    std::string mRegisterName; //!< Name of the register to be loaded.
    uint64 mRegisterValue; //!< Value to be loaded.
    std::string mInterRegName;  //!< intermediate register name
  };

  /*!
    \class GenLoadLargeRegister
    \brief A request to the test generator thread to generate a sequence to load a register to a certain value.
  */
  class GenLoadLargeRegister : public GenSequenceRequest {
  public:
    GenLoadLargeRegister(const std::string& rRegName, const std::vector<uint64>& rValues, const std::string& rInterRegName="", uint32 immOffset=-1); //!< Constructor with register name and value given.
    const char* RequestType() const override { return "GenLoadLargeRegister"; } //!< Return GenLoadLargeRegister type.
    const std::string& RegisterName() const { return mRegisterName; } //!< Return the name of the register to load.
    const std::vector<uint64>& RegisterValues() const { return mRegisterValues; } //!< Return the value of the register to load.
    const std::string& InterRegisterName() const { return mInterRegName; }  //!< Return the name of the intermediate register
    uint32 ImmOffset() const { return mImmOffset; } //!< Return the imm offset value.
  protected:
    GenLoadLargeRegister(const GenLoadLargeRegister& rOther); //!< Copy constructor.
  protected:
    std::string mRegisterName; //!< Name of the register to be loaded.
    std::vector<uint64> mRegisterValues; //!< Value to be loaded.
    std::string mInterRegName;  //!< intermediate register name.
    uint32 mImmOffset; //!< Offset of loading address.
  };

  /*!
    \class GenReloadRegister
    \brief A request to the test generator thread to generate a sequence to reload registers to certain values.
  */
  class GenReloadRegister : public GenSequenceRequest {
  public:
    GenReloadRegister(); //!< Constructor.
    const char* RequestType() const override { return "GenReloadRegister"; } //!< Return GenReloadRegister type.

    void AddReloadRegister(const std::string& regName, uint64 value); //!< Add reloading register with value request.
    const std::map<std::string, uint64>& ReloadRequests() const { return mReloadMap; } //!< Return reloading registers.

    void SetInterRegisterName(const std::string& interRegName) { mInterRegName = interRegName; }  //!< Set the name of the intermediate register.
    const std::string& InterRegisterName() const { return mInterRegName; }  //!< Return the name of the intermediate register.

    void SetRegisterType(ERegisterType type) { mType = type; } //!< Set reloading register type.
    ERegisterType RegisterType() const { return mType; } //<! Return reloading register type.

    void SetReloadMethod(EReloadingMethodType method) { mReloadMethod = method; mReloadMethodForced = true; }  //<! Set reloading register method
    bool ReloadMethodForced() const { return mReloadMethodForced; } //!< Return whether the reloading method forced
    EReloadingMethodType ReloadMethod() const { return mReloadMethod; }  //!< Return the reloading method.
  protected:
    GenReloadRegister(const GenReloadRegister& rOther); //!< Copy constructor.
  protected:
    std::string mInterRegName;  //!< Intermediate register name
    std::map<std::string, uint64> mReloadMap;  //!< Reloading registers map.
    EReloadingMethodType mReloadMethod; //!< Register reloading method.
    bool mReloadMethodForced; //!< Whether register reloading method forced.
    ERegisterType mType; //!< Reloading register type.
  };

  /*!
    \class GenBatchReloadRegisters
    \brief A request to the test generator thread to generate a sequence to reload batch of registers.
  */
  class GenBatchReloadRegisters : public GenSequenceRequest {
  public:
    explicit GenBatchReloadRegisters(const std::string& inter_reg_name=""); //!< Constructor
    void AddReloadRegister(const std::string& regName); //!< Add individual Register request.
    const char* RequestType() const override { return "GenBatchReloadRegisters"; } //!< Return GenBatchReloadRegisters type.
    const std::string& InterRegisterName() const { return mInterRegName; }  //!< Return the name of the intermediate register.
    const std::vector<std::string >& ReloadRegisters() const { return mRegisters; } //!< Return reload registers.
  protected:
    GenBatchReloadRegisters(const GenBatchReloadRegisters& rOther); //!< Copy constructor.
  protected:
    std::string mInterRegName;  //!< intermediate register name
    std::vector<std::string> mRegisters;
  };

  /*!
    \class GenSetRegister
    \brief A request to the test generator thread to set a register to a certain value.
  */
  class GenSetRegister : public GenSequenceRequest {
  public:
    GenSetRegister(Register* pReg, uint64 value); //!< Constructor with register name and value given.
    ASSIGNMENT_OPERATOR_ABSENT(GenSetRegister);
    const char* RequestType() const override { return "GenSetRegister"; } //!< Return GenSetRegister type.
    Register* GetRegister() const { return mpRegister; } //!< Return the pointer to the register to set.
    uint64 RegisterValue() const { return mRegisterValue; } //!< Return the value of the register to set.
  protected:
    GenSetRegister(const GenSetRegister& rOther); //!< Copy constructor.
  protected:
    Register* mpRegister; //!< Pointer to the register to be set.
    uint64 mRegisterValue; //!< Value to be set.
  };

  /*!
    \class GenBranchToTarget
    \brief A request to the test generator thread to generate a sequence to load a register to a certain value.
  */
  class GenBranchToTarget : public GenSequenceRequest {
  public:
    explicit GenBranchToTarget(uint64 target, bool near=false, bool noBnt = false); //!< Constructor with branch target value given.

    // AddingInstruction() needs to return true for GenBranchToTarget to ensure we check for an
    // instruction collision before processing GenBranchToTarget and creating the relevant
    // GenInstructionRequests. Otherwise, the branch instructions are generated based on the
    // pre-escape PC value and will miss the intended target.
    bool AddingInstruction() const override { return true; } //!< Indicates whether the GenRequest will result in adding instruction to the instruction stream.

    const char* RequestType() const override { return "GenBranchToTarget"; } //!< Return GenBranchToTarget type.
    uint64 BranchTarget() const { return mTargetValue; } //!< Return the value of the branch target
    bool NearBranch() const { return mNear; } //!< Return whether a near branch need to be used.
    inline bool NoBnt() const { return mNoBnt; } //!< return whether noBnt request
  protected:
    GenBranchToTarget(const GenBranchToTarget& rOther); //!< Copy constructor.
  protected:
    uint64 mTargetValue; //!< Value of the branch target.
    bool mNear; //!< Indicate if must use near branch.
    bool mNoBnt; //!< Indicate if NoBnt request
  };

  class Instruction;

  /*!
    \class GenCommitInstruction
    \brief A request to the test generator thread to generate a sequence to load a register to a certain value.
  */
  class GenCommitInstruction : public GenSequenceRequest {
  public:
    GenCommitInstruction(Instruction* instr, GenInstructionRequest* instrReq); //!< Constructor with pointer to Instruction object given.
    ~GenCommitInstruction(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenCommitInstruction);

    const char* RequestType() const override { return "GenCommitInstruction"; } //!< Return GenCommitInstruction type.
    bool AddingInstruction() const override { return true; } //!< Indicates GenCommitInstruction will add instruction to the instruction stream.
    Instruction* GiveInstruction(); //!< Return a pointer to the instruction to be committed and give up its ownership.
    GenInstructionRequest* GiveInstructionRequest(); //!< Return a pointer to the instruction request object to be processed and give up its ownership.
    void CleanUp() override; //!< do some clean up
  protected:
    GenCommitInstruction(const GenCommitInstruction& rOther); //!< Copy constructor.
  protected:
    Instruction* mpInstruction; //!< Instruction to be committed.
    GenInstructionRequest* mpInstructionRequest; //!< Instruction request object.
  };

  /*!
    \class GenRegisterReservation
    \brief A request to the test generator thread to generate a sequence to load a register to a certain value.
  */
  class GenRegisterReservation : public GenSequenceRequest {
  public:
    GenRegisterReservation(const std::string& regName, bool doReserve, ERegAttrType regAttr); //!< Constructor with register name ect parameters given.
    const char* RequestType() const override { return "GenRegisterReservation"; } //!< Return GenRegisterReservation type.
    const std::string& RegisterName() const { return mRegisterName; } //!< Return the name of the register to load.
    bool DoReserve() const { return mDoReserve; } //!< Return whether we are doing reserve or not.
    ERegAttrType ReservationAttributes() const { return mReservationAttributes; }  //!< Return the reservation attributes.
  protected:
    GenRegisterReservation(const GenRegisterReservation& rOther); //!< Copy constructor.
  protected:
    std::string mRegisterName; //!< Name of the register to be loaded.
    bool mDoReserve; //!< Whether doing reserve or unreserve.
    ERegAttrType mReservationAttributes; //!< Attributes involved in the reservation.
  };

  /*!
    \class GenEscapeCollision
    \brief A request to the test generator thread to generate a sequence to load a register to a certain value.
  */
  class GenEscapeCollision : public GenSequenceRequest {
  public:
    GenEscapeCollision(); //!< Default constructor.
    const char* RequestType() const override { return "GenEscapeCollision"; } //!< Return GenEscapeCollision type.
    bool DelayHandle() const override { return false; } //!< Indicates whether the GenRequest can be insert by other request.
  protected:
    GenEscapeCollision(const GenEscapeCollision& rOther); //!< Copy constructor.
  };

   /*!
    \class GenBntRequest
    \brief A request to the test generator thread to generate a sequence to load a register to a certain value.
  */
  class GenBntRequest : public GenSequenceRequest {
  public:
    explicit GenBntRequest(BntNode* pBntNode); //!< Constructor with pointer to BntNode given.
    ~GenBntRequest(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenBntRequest);

    const char* RequestType() const override { return "GenBntRequest"; } //!< Return GenBntRequest type.
    virtual BntNode* GiveBntNode(); //!< Give up ownership of BntNode object.
  protected:
    GenBntRequest(); //!< Default constructor.
    GenBntRequest(const GenBntRequest& rOther); //!< Copy constructor.
  protected:
    BntNode* mpBntNode; //!< Pointer to BntNode object.
  };

  /*!
    \class GenSpeculativeBntRequest
    \brief A request to the test generator thread to handle speculative Bnt
  */
  class GenSpeculativeBntRequest : public GenSequenceRequest {
  public:
    GenSpeculativeBntRequest(BntNode* pBntNode, ESpeculativeBntActionType actionType); //!< Constructor with pointer to BntNode given.
    ~GenSpeculativeBntRequest(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenSpeculativeBntRequest);

    const char* RequestType() const override { return "GenSpeculativeBntRequest"; } //!< Return GenBntRequest type.
    bool DelayHandle() const override { return false; } //!< The request handle can not been delayed.
    inline BntNode* GetBntNode() const { return mpBntNode; } //!< Get pointer to the Bnt node
    inline ESpeculativeBntActionType GetActionType() const { return mActionType; }
  protected:
    GenSpeculativeBntRequest(); //!< Default constructor.
    GenSpeculativeBntRequest(const GenSpeculativeBntRequest& rOther); //!< Copy constructor.
  protected:
    BntNode* mpBntNode; //!< Pointer to BntNode object.
    ESpeculativeBntActionType mActionType; //!< action type
  };

  /*!
    \class GenReExecutionRequest
    \brief A request to the test generator thread to re-execute existing code.
  */
  class GenReExecutionRequest : public GenSequenceRequest {
  public:
    explicit GenReExecutionRequest(uint64 pc); //!< Constructor with PC given.
    GenReExecutionRequest(); //!< Default constructor.
    ~GenReExecutionRequest(); //!< Destructor.
    const char* RequestType() const override { return "GenReExecutionRequest"; } //!< Return GenReExecutionRequest type.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    bool DelayHandle() const override { return false; } //!< Indicates whether the GenRequest can be insert by other request.
    uint64 ReExecutionAddress() const { return mReExecutionAddress; } //!< Return re-execution address.
    uint32 MaxReExecutionInstructions() const { return mMaxReExecutionInstructions; } //!< Return maximum number of instructions to step.
  protected:
    GenReExecutionRequest(const GenReExecutionRequest& rOther); //!< Copy constructor.
  protected:
    uint64 mReExecutionAddress; //!< Re-execution address.
  private:
    uint32 mMaxReExecutionInstructions; //!< Maximum number of instructions to step.
  };

  /*!
    \class GenPeStateUpdateRequest
    \brief A request to the test generator thread to update the PeState
  */
  class GenPeStateUpdateRequest : public GenSequenceRequest {
  public:
    explicit GenPeStateUpdateRequest(uint32 id); //!< Constructor with record id given.
    GenPeStateUpdateRequest(); //!< Default constructor.
    ~GenPeStateUpdateRequest(); //!< Destructor.
    const char* RequestType() const override { return "GenPeStateUpdateRequest"; } //!< Return GenPeStateUpdateRequest type.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    uint64 RecordId() const { return mRecordId; } //!< Return PeState update record id
  protected:
    GenPeStateUpdateRequest(const GenPeStateUpdateRequest& rOther); //!< Copy constructor.
  protected:
    uint64 mRecordId; //!< PeState update record id
  };

  /*!
    \class GenRegisterFieldUpdateRequest
    \brief A request to the test generator thread to update register
  */
  class GenRegisterFieldUpdateRequest : public GenSequenceRequest {
  public:
    GenRegisterFieldUpdateRequest(const std::string& reg_name, const std::string& field_name, uint64 field_value); //!< Constructor with given register information to be updated
    GenRegisterFieldUpdateRequest(); //!< Default constructor.
    ~GenRegisterFieldUpdateRequest(); //!< Destructor.
    const char* RequestType() const override { return "GenRegisterFieldUpdateRequest"; } //!< Return GenRegisterFieldUpdateRequest type.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add request detail, with value string parameter.
    const std::string& RegisterName() const { return mRegName; }
    const std::string& FieldName() const { return mFieldName; }
    uint64 FieldValue() const { return mFieldValue; }
    uint64 Mask() const { return mMask; }
    uint64 Value() const { return mValue; }
  protected:
    GenRegisterFieldUpdateRequest(const GenRegisterFieldUpdateRequest& rOther); //!< Copy constructor.
  protected:
    std::string mRegName;     //!< register name
    std::string mFieldName;   //!< register field name
    uint64 mFieldValue;       //!< register field value
    uint64 mMask;             //!< register mask
    uint64 mValue;            //!< register value
  };

  /*!
    \class GenInitializeAddrTablesRequest
    \brief A request to initialize address tables
  */
  class GenInitializeAddrTablesRequest : public GenSequenceRequest {
  public:
    GenInitializeAddrTablesRequest(); //!< Default constructor.
    ~GenInitializeAddrTablesRequest() { } //!< Destructor.
    const char* RequestType() const override { return "GenInitializeAddrTablesRequest"; } //!< Return GenInitializeAddrTablesRequest type.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    uint32 TableIndex() const { return mAddrTableIndex; } //!< Return address table register index.
    uint64 FastMode() const { return mFastMode; } //!< Return Mode.
  protected:
    GenInitializeAddrTablesRequest(const GenInitializeAddrTablesRequest& rOther); //!< Copy constructor.
  protected:
    uint32 mAddrTableIndex; //!< Address table register index.
    uint64 mFastMode; //!< Exception handler mode.
  };

  /*!
    \class GenRestoreRequest
    \brief A request to manage automatic state restoration
  */
  class GenRestoreRequest : public GenSequenceRequest {
  public:
    explicit GenRestoreRequest(const ESequenceType seqType); //!< Constructor
    GenRestoreRequest(const ESequenceType seqType, cuint32 loopRegIndex, cuint32 simCount, cuint32 restoreCount, const std::set<ERestoreExclusionGroup>& restoreExclusions); //!< Constructor
    SUBCLASS_DESTRUCTOR_DEFAULT(GenRestoreRequest); //!< Destructor
    const char* RequestType() const override { return "GenRestoreRequest"; } //!< Return GenRestoreRequest type.
    uint32 LoopRegisterIndex() const { return mLoopRegIndex; } //!< Return loop register index.
    uint32 SimulationCount() const { return mSimCount; } //!< Return number of iterations to execute loop.
    uint32 RestoreCount() const { return mRestoreCount; } //!< Return number of iterations to generate restore instructions.
    std::set<ERestoreExclusionGroup> RestoreExclusions() const { return mRestoreExclusions; } //!< Return groups to exclude from restore.
    uint32 LoopId() const { return mLoopId; } //!< Return loop ID.
    void SetLoopId(cuint32 loopId); //!< Set loop ID.
  protected:
    GenRestoreRequest(const GenRestoreRequest& rOther); //!< Copy constructor.
  protected:
    uint32 mLoopRegIndex; //!< Loop register index
    uint32 mSimCount; //!< Number of iterations to execute loop
    uint32 mRestoreCount; //!< Number of iterations to generate restore instructions
    std::set<ERestoreExclusionGroup> mRestoreExclusions; //!< Groups to exclude from restore
    uint32 mLoopId; //!< Loop ID
  };

  /*!
    \class GenLoopReconvergeRequest
    \brief A sequence request to the test generator thread, will generate a sequence of instructions to reconverge into existing loop body.
  */
  class GenLoopReconvergeRequest : public GenSequenceRequest {
  public:
    GenLoopReconvergeRequest(uint64 currentPC, uint64 lastPC); //!< Constructor.
    SUBCLASS_DESTRUCTOR_DEFAULT(GenLoopReconvergeRequest); //!< Destructor.
    const char* RequestType() const override { return "GenLoopReconvergeRequest"; } //!< Return GenLoopReconvergeRequest type.
    uint64 CurrentPC() const { return mCurrentPC; } //!< Return current PC from where to start reconverge.
    uint64 LastPC() const { return mLastPC; } //!< Return last PC to identify Loop body.
  protected:
    GenLoopReconvergeRequest(const GenLoopReconvergeRequest& rOther); //!< Copy constructor.
  protected:
    uint64 mCurrentPC; //!< Current PC virtual address.
    uint64 mLastPC; //!< Last PC virtual address.
  };

  /*!
    \class GenOneTimeStateTransitionRequest
    \brief A request to initiate a StateTransition in which the State object is deleted after the request is processed
  */
  class GenOneTimeStateTransitionRequest : public GenStateTransitionRequest {
  public:
    GenOneTimeStateTransitionRequest(const State* pTargetState, const EStateTransitionType stateTransType, const EStateTransitionOrderMode orderMode, const std::vector<EStateElementType>& rStateElemTypeOrder);
    SUBCLASS_DESTRUCTOR_DEFAULT(GenOneTimeStateTransitionRequest);
    ASSIGNMENT_OPERATOR_ABSENT(GenOneTimeStateTransitionRequest);

    const char* RequestType() const override { return "GenOneTimeStateTransitionRequest"; } //!< Return GenOneTimeStateTransitionRequest type.
  protected:
    GenOneTimeStateTransitionRequest(const GenOneTimeStateTransitionRequest& rOther);
  };

  /*!
    \class GenRequestWithResult
    \brief A request to the test generator thread to generate a sequence, in most cases are a sequence of instructions.
  */
  class GenRequestWithResult : public GenRequest {
  public:
    GenRequestWithResult() : GenRequest() { } //!< Constructor
    const char* RequestType() const override { return "GenRequestWithResult"; } //!< Return GenRequestWithResult type.
    virtual void GetResults(py::object& rPyObject) const { } //!< Return request results in the passed in rPyObject if applicable.
  protected:
    GenRequestWithResult(const GenRequestWithResult& rOther) : GenRequest(rOther) { } //!< Copy constructor.
  };

  /*!
    \class GenVirtualMemoryRequest
    \brief A request to the test generator thread to process a virtual memory request.
  */
  class GenVirtualMemoryRequest : public GenRequestWithResult {
  public:
    explicit GenVirtualMemoryRequest(EVmRequestType seqType); //!< Constructor with id given.
    ~GenVirtualMemoryRequest(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(GenVirtualMemoryRequest);

    const std::string ToString() const override; //!< Return a string describing the current state of the GenVirtualMemoryRequest object.
    const char* RequestType() const override { return "GenVirtualMemoryRequest"; } //!< Return GenVirtualMemoryRequest type.
    EGenAgentType GenAgentType() const override { return EGenAgentType::GenVirtualMemoryAgent; } //!< Return type of the GenAgent to process GenVirtualMemoryRequest.

    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add request detail, with value string parameter.

    EVmRequestType VmRequestType() const { return mVmRequestType; } //!< Return sequence type.
    EMemDataType DataType() const { return mDataType; } //!< Return data type of the request.
    EMemBankType BankType() const { return mBankType; } //!< Return the bank type of the request.
    uint64 Size() const { return mSize; } //!< Return size of the virtual memory request.
    void SetSize(uint64 size) { mSize = size; } //!< Set virtual memory access size.
    uint64 Align() const { return mAlign; } //!< Return the alignment of the request.
    uint64 Tag() const { return mTag;  } //!<Return the request tag
    bool ForceAlias() const { return mForceAlias; } //!< Return the flag on whether or not to attempt to allocate VA to aliased page.
    uint64 PhysPageId() const { return mPhysPageId; } //!< Return the physical page ID for aliasing
    uint64 PhysAddr() const { return mPhysAddr; } //!< Return the Physical Address the request is targeting (for GenVAforPA)
    bool FlatMap() const { return mFlatMap; } //!< Return the flat mapped flag
    bool ForceMemAttrs() const { return mForceMemAttrs; } //!< return the force mem attribute checking flag
    bool CanAlias() const { return mCanAlias; } //!< Return the flag corresponding to whether the page allocated can be aliased in the future
    bool ForceNewAddr() const { return mForceNewAddr; } //!< Return the flag which forces the request to return a new mapping instead of an existing mapping
    bool SharedMemory() const { return mSharedMemory; } //!< Return the flag to indicate whether the returned memory range should be marked shared.
    void SetImplementationMemoryAttributes(const std::vector<std::string>& rImplMemAttributes); //!< Set implementation memory attributes.
    void SetArchitectureMemoryAttributes(const std::vector<EMemoryAttributeType>& rArchMemAttributes); //!< Set architecture memory attributes.
    void SetAliasImplementationMemoryAttributes(const std::vector<std::string>& rAliasImplMemAttributes); //!< Set alias implementation memory attributes.
    const std::vector<std::string>& ImplementationMemoryAttributes() const { return mImplMemAttributes; } //!< Return implementation memory attributes.
    const std::vector<EMemoryAttributeType>& ArchitectureMemoryAttributes() const { return mArchMemAttributes; } //!< Return architecture memory attributes.
    const std::vector<std::string>& AliasImplementationMemoryAttributes() const { return mAliasImplMemAttributes; } //!< Return alias implementation memory attributes.
    const ConstraintSet* MemoryRangesConstraint() const { return mpMemoryRangesConstraint; } //!< Return memory ranges constraint.
    void SetVmInfoBoolType(EVmInfoBoolType reqType, bool isSet); //!< Set VmInfo attributes.
    bool VmSpecified() const; //!< Return whether target VM is specified.
    inline EPrivilegeLevelType PrivilegeLevel() const { return EPrivilegeLevelType(mPrivilegeLevel); } //!< Return the EL value.
    inline bool PrivilegeLevelSpecified() const { return mPrivilegeLevelSpecified; } //!< Return whether exception level is specified.

    inline bool BankSpecified(EMemBankType& rMemBank) const //!< Return whether target bank was set in request.
    {
      if (mBankSpecified) rMemBank = mBankType;
      return mBankSpecified;
    }

    void SetPrivilegeLevel(EPrivilegeLevelType priv); //!< Set exception level.
    void SetBankType(EMemBankType bank); //!< Set memory bank type
    static GenVirtualMemoryRequest* GenVirtualMemoryRequestInstance(const std::string& reqName); //!< Return a GenVirtualMemoryRequest instance based on the reqName given.
  protected:
    GenVirtualMemoryRequest(const GenVirtualMemoryRequest& rOther); //!< Copy constructor.
  protected:
    EVmRequestType mVmRequestType; //!< Virtual memory request type of the GenVirtualMemoryRequest object.
    EMemDataType mDataType; //!< Memory data type.
    EMemBankType mBankType; //!< Memory bank type.
    EPrivilegeLevelType mPrivilegeLevel; //!< Exception Level.
    uint64 mSize; //!< Size of the virtual memory request.
    uint64 mAlign; //!< Alignment of the request.
    uint64 mTag; //!<Tag of the request
    uint64 mPhysAddr; //!< Target Physical Address, used in GenVaForPa
    uint64 mPhysPageId; //!< Page ID to attempt to alias VA to.
    bool mPrivilegeLevelSpecified; //!< Target VM specified or not.
    bool mBankSpecified; //!< Target mem bank specified or not.
    bool mForceAlias; //!< Flag for allocating VA in aliased page.
    bool mFlatMap; //!< Flag for a flat mapped allocation.
    bool mForceMemAttrs; //!< Flag to ignore mem attribute checking when allocating/aliasing.
    bool mCanAlias; //!< Flag to indicate if allocated/aliased page can be aliasable in the future.
    bool mForceNewAddr; //!< Flag to force request to return a newly mapped address, not an existing mapping.
    bool mSharedMemory; //!< Flag to indicate whether the returned memory range should be marked shared.
    std::vector<std::string> mImplMemAttributes; //!< Implementation memory attributes.
    std::vector<EMemoryAttributeType> mArchMemAttributes; //!< Architecture memory attributes.
    std::vector<std::string> mAliasImplMemAttributes; //!< Alias implementation memory attributes.
    ConstraintSet* mpMemoryRangesConstraint; //!< Constraint Set for Range
    std::map<EVmContextParamType,uint64> mVmContextParams;
    uint64 mVmInfoBoolTypes; //!< VmInfo boolean types attributes.
    uint64 mVmInfoBoolTypeMask; //!< VmInfo boolean type set mask.
  private:
    void AddImplementationMemoryAttributes(const std::string& rValueStr, std::vector<std::string>& rMemAttributes); //!< Add implementation memory attributes. rValueStr should be a comma-separated list of memory attribute names.
    void AddArchitectureMemoryAttributes(const std::string& rValueStr, std::vector<EMemoryAttributeType>& rMemAttributes); //! Add architecture memory attributes. rValueStr should be either a comma-separated list of memory attribute names or a comma-separated list of integers.
  };

  /*!
    \class GenVaRequest
    \brief A request to the test generator thread to generate a valid virtual address.
  */
  class GenVaRequest : public GenVirtualMemoryRequest {
  public:
    GenVaRequest(); //!< Constructor.
    ~GenVaRequest(); //!< Destructor.
    const char* RequestType() const override { return "GenVaRequest"; } //!< Return GenVaRequest type.

    uint64 VA() const { return mVA; } //!< Return the resulting VA.
  protected:
    GenVaRequest(const GenVaRequest& rOther); //!< Copy constructor.
  protected:
    uint64 mVA; //!< Resulting virtual address.

    friend class GenVirtualMemoryAgent;
  };

  /*!
    \class GenVmVaRequest
    \brief A request to the test generator thread to generate a valid virtual address.
  */
  class GenVmVaRequest : public GenVirtualMemoryRequest {
  public:
    GenVmVaRequest(); //!< Constructor.
    ~GenVmVaRequest(); //!< Destructor.
    const char* RequestType() const override { return "GenVmVaRequest"; } //!< Return GenVaRequest type.

    uint64 VA() const { return mVA; } //!< Return the resulting VA.
  protected:
    GenVmVaRequest(const GenVmVaRequest& rOther); //!< Copy constructor.
  protected:
    uint64 mVA; //!< Resulting virtual address.

    friend class GenVirtualMemoryAgent;
  };

  /*!
    \class GenPaRequest
    \brief A request to the test generator thread to generate a valid physical address.
  */
  class GenPaRequest : public GenVirtualMemoryRequest {
  public:
    GenPaRequest(); //!< Constructor.
    ~GenPaRequest(); //!< Destructor.
    const char* RequestType() const override { return "GenPaRequest"; } //!< Return GenPaRequest type.

    uint64 PA() const { return mPA; } //!< Return the resulting PA.
  protected:
    GenPaRequest(const GenPaRequest& rOther); //!< Copy constructor.
  protected:
    uint64 mPA; //!< Resulting virtual address.

    friend class GenVirtualMemoryAgent;
  };

  class GenVaForPaRequest : public GenVirtualMemoryRequest {
    public:
      GenVaForPaRequest(); //!< Constructor.
      ~GenVaForPaRequest(); //!< Destructor.
      const char* RequestType() const override { return "GenVaForPaRequest"; } //!< Return GenVaForPaRequest type

      uint64 VA() const { return mVA; } //!< Return the resulting VA.
    protected:
      GenVaForPaRequest(const GenVaForPaRequest& rOther); //!< Copy Constructor.
    protected:
      uint64 mVA; // Resultive virtual address.

      friend class GenVirtualMemoryAgent;
  };

  class GenVmContextRequest : public GenVirtualMemoryRequest {
    public:
      GenVmContextRequest(); //!< Constructor.
      ~GenVmContextRequest(); //!< Destructor.
      const char* RequestType() const override { return "GenVmContextRequest"; } //!< Return GenVmContextRequest type
      void GetResults(py::object& rPyObject) const override; //!< Return request results in the passed in rPyObject if applicable.
      void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
      const std::map<std::string, uint64> & GetInputArgs() const { return mInputArgs;} //!< return variable value of argName.
      bool FindInputArgs(const std::string& argName) const { return mInputArgs.find(argName) != mInputArgs.end();}  //!< find if argName exists
      void SetId(uint32 id) { mId = id; } //!< Return the resulting Id.
    protected:
      GenVmContextRequest(const GenVmContextRequest& rOther); //!< Copy Constructor.
    protected:
      uint64 mId; //!< vmContextId.
      std::map<std::string, uint64> mInputArgs; //!< store input arguments
      friend class GenVirtualMemoryAgent;
  };


  /*!
    \class GenPhysicalRegionRequest
    \brief A request to the test generator thread to generate a base address for handler memory.
  */
  class GenPhysicalRegionRequest : public GenVirtualMemoryRequest {
  public:
    GenPhysicalRegionRequest(); //!< Constructor.
    ~GenPhysicalRegionRequest(); //!< Destructor.
    const char* RequestType() const override { return "GenPhysicalRegionRequest"; } //!< Return GenPhysicalRegionRequest type.

    void GetResults(py::object& rPyObject) const override; //!< Return request results in the passed in rPyObject if applicable.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add request detail, with value string parameter.
    void SetBaseAddress(uint64 addr) { mBaseAddress = addr; } //!< Set return base address.
    EPhysicalRegionType RegionType() const { return mRegionType; } //!< Return the region type.
    EMemBankType MemoryBank() const { return mMemoryBank; } //!< Return memory bank ID.
    uint64 FastMode() const {return mFastMode;} //!< Return Mode.
  protected:
    GenPhysicalRegionRequest(const GenPhysicalRegionRequest& rOther); //!< Copy constructor.
  protected:
    EPhysicalRegionType mRegionType; //!< Type of the physical region.
    EMemBankType mMemoryBank; //!< Memory bank type.
    mutable uint64 mBaseAddress; //!< Resulting base address.
    uint64 mFastMode;
  };

  struct PageSizeInfo;
  class ConstraintSet;

  /*!
    \class GenPageRequest
    \brief A request to the test generator thread to generate a valid virtual address.
  */
  class GenPageRequest : public GenVirtualMemoryRequest {
  public:
    ~GenPageRequest(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenPageRequest);
    const char* RequestType() const override { return "GenPageRequest"; } //!< Return GenPageRequest type.

    virtual GenPageRequest* Clone() const { return new GenPageRequest(*this); } //!< Return a clone of the GenPageRequest object.

    const std::string ToString() const override; //!< Return a string representation of the GenPageRequest object.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add request detail, with value string parameter.
    void SetAttributeValue(EPageRequestAttributeType, uint64 value); //!< Set page request attribute value.
    void SetGenBoolAttribute(EPageGenBoolAttrType attrType, bool isSet); //!< Set boolean type generation value.
    void SetPteAttributeConstraint(EPteAttributeType attrType, ConstraintSet* constrSet); //!< Set constraint for a PTE attribute type.
    void SetPteAttribute(EPteAttributeType attrType, uint64 value); //!< Set value for a pte gen attribute type.
    void SetGenAttributeConstraint(EPageGenAttributeType attrType, ConstraintSet* constrSet); //!< Set constraint for a page gen attribute type.
    void SetGenAttributeValue(EPageGenAttributeType attrType, uint64 value); //!< Set value for a page gen attribute type.
    const ConstraintSet* PteAttributeConstraint(EPteAttributeType attrType) const; //!< Return PTE attribute constraint if available.
    const ConstraintSet* GenAttributeConstraint(EPageGenAttributeType attrType) const; //!< Return gen attribute constraint if available.
    bool GetAttributeValue(EPageRequestAttributeType attrType, uint64& value) const; //!< Return page request value and whether it is set.
    bool GetGenBoolAttribute(EPageGenBoolAttrType attrType, bool& value) const; //!< Return whether a generation boolean value is set as well as whether it's value.
    bool GenBoolAttribute(EPageGenBoolAttrType attrType) const; //!< Return value of the specified gen-bool attribute.  Fail if the value is not specified.
    bool InstructionRequest() const; //!< Return whether this request is for instruction.

    inline bool GenBoolAttributeDefaultFalse(EPageGenBoolAttrType attrType) const //!< Return value of the gen-bool attribute, if not set, return false as default value.
    {
      return bool (((mGenBoolAttributes & mGenBoolAttrMask) >> (uint32)(attrType)) & 1);
    }

    inline bool GenBoolAttributeDefaultTrue(EPageGenBoolAttrType attrType) const //!< Return value of the gen-bool attribute, if not set, return true as default value.
    {
      return bool (((mGenBoolAttributes & mGenBoolAttrMask) >> (uint32)(attrType)) & 1) or bool (((mGenBoolAttrMask >> (uint32)(attrType)) & 1) == 0);
    }

    inline EMemAccessType MemoryAccessType() const { return mMemAccessType; } //!< Memory access type.
    void SetMemoryAccessType(EMemAccessType memAccessType) { mMemAccessType = memAccessType; } //!< Set memory access type.
    void SetExceptionConstraint(EPagingExceptionType exceptType, EExceptionConstraintType constrType); //!< Set paging exception constraint type.
    EExceptionConstraintType GetExceptionConstraint(EPagingExceptionType exceptType) const; //!< Return exception constraint type, if specified.
    inline const std::map<EPagingExceptionType, EExceptionConstraintType>& GetExceptionConstraints() const { return mExceptionConstraints; } //!< Return all exception constraints.
  protected:
    GenPageRequest(); //!< Default constructor, protected to be not directly called.
    GenPageRequest(const GenPageRequest& rOther); //!< Copy constructor.
    void SetAttributeMask(EPageRequestAttributeType attrType); //!< Set page request attribute mask to indicate that attribute is specified.
  protected:
    uint64 mVA; //!< VA to be mapped.
    uint64 mIPA; //!< IPA to be mapped.
    uint64 mPA; //!< PA to be mapped.
    uint64 mPageId; //!< Page ID to alias mapping to.
    uint64 mAttributeMask; //!< Attribute set mask.
    uint64 mGenBoolAttributes; //!< Generation boolean type attributes.
    uint64 mGenBoolAttrMask; //!< Generation boolean type attributes set mask.
    EMemAccessType mMemAccessType; //!< Memory access type.
    std::vector<PageSizeInfo* > mPageSizes; //!< Requested page sizes
    std::map<EPteAttributeType, ConstraintSet* > mPteAttributes; //!< PTE attribute constraints.
    std::map<EPageGenAttributeType, ConstraintSet* > mGenAttributes; //!< Page gen attribute constraints.
    std::map<EPagingExceptionType, EExceptionConstraintType> mExceptionConstraints; //!< Paging exception constraints.
    friend class Generator;
    friend class GenVirtualMemoryRequest;
  };

   /*!
    \class GenFreePageRequest
    \brief A request to generate free page.
  */
  class GenFreePageRequest : public GenVirtualMemoryRequest {
  public:
    GenFreePageRequest(); //!< Constructor.
    ~GenFreePageRequest(); //!< Destructor.
    const char* RequestType() const override { return "GenFreePageRequest"; } //!< Return GenFreePage type.

    void GetResults(py::object& rPyObject) const override; //!< Return request results in the passed in rPyObject if applicable.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add request detail, with value string parameter.
  protected:
    void SplitToPageSize(const std::string& pageSizeStr); //!< split page size string
    void RegulateRequest(); //!< Do some regulation on the request
    ASSIGNMENT_OPERATOR_ABSENT(GenFreePageRequest) ;
    COPY_CONSTRUCTOR_ABSENT(GenFreePageRequest);
  protected:
    uint32 mPageNum; //!< page numbers to request
    std::vector<uint64> mRequestPageSizes; //!< page size to request
    ConstraintSet* mpRequestRanges; //!< address range to request

    bool mValid; //!< whether the request is valid or not
    uint64 mStartAddr; //!< start address for the request pages
    ConstraintSet* mpResolvedRanges; //!< address range requested.
    std::vector<uint64> mResolvedPageSizes; //!< page sizes resolved.

    friend class GenVirtualMemoryAgent;
  };

  /*!
    \class GenUpdateVmRequest
    \brief A request to activate the VM
  */
  class GenUpdateVmRequest: public GenVirtualMemoryRequest {
    public:
      GenUpdateVmRequest(); //!< Constructor.
      ~GenUpdateVmRequest(); //!< Destructor.
      const char* RequestType() const override { return "GenUpdateVmRequest"; } //!< Return GenUpdateVmRequest type
      void GetResults(py::object& rPyObject) const override; //!< Return request results in the passed in rPyObject if applicable.
      const std::map<std::string, uint64> & GetInputArgs() const { return mInputArgs;} //!< return variable value of argName.
    protected:
      GenUpdateVmRequest(const GenUpdateVmRequest& rOther); //!< Copy Constructor.
    protected:
      std::map<std::string, uint64> mInputArgs; //!< store input arguments
      friend class GenVirtualMemoryAgent;
  };

  /*!
    \class GenStateRequest
    \brief A request to the test generator thread to change a generator state.
  */
  class GenStateRequest : public GenRequestWithResult {
  public:
    explicit GenStateRequest(EGenStateType stateType); //!< Constructor with state type given.
    GenStateRequest(EGenStateActionType actType, EGenStateType stateType, uint64 value); //!< Constructor with value details given.
    const char* RequestType() const override { return "GenStateRequest"; } //!< Return GenStateRequest type.
    void SetPrimaryValue(uint64 value) override; //!< Set primary value, with integer value parameter.
    void SetPrimaryString(const std::string& valueStr) override; //!< Set primary string.
    const std::string ToString() const override; //!< Return a string describing the current state of the GenStateRequest object.
    EGenAgentType GenAgentType() const override { return EGenAgentType::GenStateAgent; } //!< Return type of the GenAgent to process GenStateRequest.
    void SetAction(const std::string& actionName); //!< Set action for the generator state.
    EGenStateActionType ActionType() const { return mActionType; } //!< Return action type.
    EGenStateType StateType() const { return mStateType; } //!< Return state type.
    const std::string& StringVariable() const { return mString; } //!< Return string variable.
    uint64 ValueVariable() const { return mValue; } //!< Return value variable.
    bool IsValue() const { return mIsValue; } //!< Return whether the variable is in value form.
    bool DelayHandle() const override { return false; } //!< Indicates whether the GenRequest can be insert by other request.
    static GenStateRequest* GenStateRequestInstance(const std::string& rStateType); //!< Return a GenStateRequest instance based on the rStateType given.
  protected:
    GenStateRequest(const GenStateRequest& rOther); //!< Copy constructor.
  protected:
    EGenStateActionType mActionType; //!< Action type of the GenStateRequest object.
    EGenStateType mStateType; //!< State type of the GenStateRequest object.
    bool mIsValue; //!< Indicate the state is a value type.
    uint64 mValue; //!< State value if applicable.
    std::string mString; //!< State string if applicable.
  };

  /*!
    \class GenLoopRequest
    \brief A request to the test generator thread to enter or exit loop.
  */
  class GenLoopRequest : public GenStateRequest {
  public:
    explicit GenLoopRequest(EGenStateType stateType); //!< Constructor.
    const char* RequestType() const override { return "GenLoopRequest"; } //!< Return GenLoopRequest type.
    void SetPrimaryString(const std::string& valueStr) override; //!< Set primary string.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    void SetLoopId(uint32 loopId) { mLoopId = loopId; } //!< Set loop ID.
    void SetLoopBackAddress(uint64 addr) { mLoopBackAddress = addr; } //!< Set loop back address.
    uint32 LoopRegIndex() const { return mLoopRegIndex; } //!< Return loop register index value.
    uint32 LoopId() const { return mLoopId; } //!< Return Loop ID.
    void GetResults(py::object& rPyObject) const override; //!< Return request results in the passed in rPyObject if applicable.
  protected:
    GenLoopRequest(const GenLoopRequest& rOther); //!< Copy constructor.
  protected:
    uint32 mLoopRegIndex; //!< Loop register index.
    uint32 mLoopId; //!< Loop ID.
    uint64 mLoopBackAddress; //!< Loop back address.
  };

 /*!
    \class GenLinearBlockRequest
    \brief A request to the test generator thread to start or end linear block.
  */
  class GenLinearBlockRequest : public GenStateRequest {
  public:
    GenLinearBlockRequest(); //!< Constructor.
    const char* RequestType() const override { return "GenLinearBlockRequest"; } //!< Return GenLinearBlockRequest type.
    void SetPrimaryString(const std::string& valueStr) override; //!< Set primary string.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    void SetBlockId(uint32 loopId) { mBlockId = loopId; } //!< Set linear block ID.
    void SetEmpty() {mEmpty = true;}
    void SetBlockEndAddress(cuint64 blockEndAddr) { mBlockEndAddr = blockEndAddr; } //!< Set end address for the linear block.
    uint32 BlockId() const { return mBlockId; } //!< Return linear block ID.
    bool Execute() const { return mExecute; } //!< Return whether to execute after finished generating the linear block.
    void GetResults(py::object& rPyObject) const override; //!< Return request results in the passed in rPyObject if applicable.
  protected:
    GenLinearBlockRequest(const GenLinearBlockRequest& rOther); //!< Copy constructor.
  protected:
    uint32 mBlockId; //!< linear block ID.
    bool mExecute; //!< Whether to execute after finished generating the linear block.
    mutable bool mEmpty; //!< If start and end address are same for the linear block.
  private:
    uint64 mBlockEndAddr; //!< End address for the linear block.
  };

  /*!
    \class GenBntHookRequest
    \brief A request to the test generator thread to start or end Bnt hook request.
  */
  class GenBntHookRequest : public GenStateRequest {
  public:
    GenBntHookRequest(); //!< Constructor.
    const char* RequestType() const override { return "GenBntHookRequest"; } //!< Return GenLinearBlockRequest type.
    const std::string ToString() const override; //!< Return a string describing the current state of the GenStateRequest object.
    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add request detail, with value string parameter.
    inline const std::string& BntSequence() const { return mBntSequence; } //!< return bnt sequence name
    inline const std::string& BntFunction() const { return mBntFunction; } //!< return bnt function name
    inline uint64 BntId() const { return mBntId; } //!< return Bnt Id
    inline void SetBntId(uint64 bntId) {mBntId = bntId; } //!< set Bnt Id
    void GetResults(py::object& rPyObject) const override; //!< Return request results in the passed in rPyObject if applicable.
  protected:
    GenBntHookRequest(const GenBntHookRequest& rOther); //!< Copy constructor.
  protected:
    std::string mBntSequence; //!< the name for bnt sequence
    std::string mBntFunction; //!< the name for bnt function
    uint64 mBntId; //!< bnt id
  };

  /*!
    \class GenExceptionRequest
    \brief A request to the test generator thread to handle an exception related request.
  */
  class GenExceptionRequest : public GenRequestWithResult {
  public:
    explicit GenExceptionRequest(EExceptionRequestType reqType); //!< Constructor with request type given.
    explicit GenExceptionRequest(const std::string& rSeqTypeStr); //!< Constructor with id in string form given.
    const char* RequestType() const override { return "GenExceptionRequest"; } //!< Return GenExceptionRequest type.
    const std::string ToString() const override; //!< Return a string describing the current state of the GenExceptionRequest object.
    EGenAgentType GenAgentType() const override { return EGenAgentType::GenExceptionAgent; } //!< Return type of the GenAgent to process GenExceptionRequest.
    EExceptionRequestType ExceptionRequestType() const { return mExceptionRequestType; } //!< Return exception request type.
    static GenExceptionRequest* GenExceptionRequestInstance(const std::string& rReqName); //!< Return a GenExceptionRequest instance based on the rReqName given.
  protected:
    GenExceptionRequest(const GenExceptionRequest& rOther); //!< Copy constructor.
  protected:
    EExceptionRequestType mExceptionRequestType; //!< Exception type of the GenExceptionRequest object.
  };

  /*!
    \class GenHandleException
    \brief A request to the test generator thread to handle exception.
  */
  class GenHandleException : public GenExceptionRequest {
  public:
    GenHandleException(uint32 excepId, const std::string& rDesc); //!< Constructor with exception infomation given.
    GenHandleException() : GenExceptionRequest(EExceptionRequestType::HandleException), mId(0), mDescription() { } //!< Default constructor.
    const char* RequestType() const override { return "GenHandleException"; } //!< Return GenHandleException type.

    ~GenHandleException(); //!< Destructor.
    uint32 Id() const { return mId; } //!< Return exception ID.
    const std::string& Description() const { return mDescription; } //!< Return exception description.
    bool DelayHandle() const override { return false; } //!< Indicates whether the GenRequest can be insert by other request.
  protected:
    GenHandleException(const GenHandleException& rOther); //!< Copy constructor.
  protected:
    uint32 mId;
    std::string mDescription;
  };

  /*!
    \class GenSystemCall
    \brief A request to the test generator thread to handle system call.
  */
  class GenSystemCall : public GenExceptionRequest {
  public:
    GenSystemCall() : GenExceptionRequest(EExceptionRequestType::SystemCall), mSysCallParms(), mSysCallResult(0), mSysCallResults() { } //!< Default constructor.
    ~GenSystemCall(); //!< Destructor.
    const char* RequestType() const override { return "GenSystemCall"; } //!< Return GenSystemCall type.

    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add request detail, with value string parameter.

    void SetResults(const std::map<std::string, uint64>& results) const { mSysCallResults = results; } //!< set up the system call results
    void GetResults(py::object& rPyObject) const override; //!< Return request results in the passed in rPyObject if applicable.

    const std::map<std::string, std::string>& SystemCallParams() const { return mSysCallParms; } //!< Return given system call params

  protected:
    GenSystemCall(const GenSystemCall& rOther); //!< Copy constructor.
  protected:
    std::map<std::string, std::string> mSysCallParms;  //!< given system call params
    mutable int mSysCallResult;  //!< system call result
    mutable std::map<std::string, uint64> mSysCallResults;
  };

  /*!
    \class GenUpdateHandlerInfo
    \brief A request to the test generator thread to unpdate exception handler information
  */
  class GenUpdateHandlerInfo : public GenExceptionRequest {
  public:
    GenUpdateHandlerInfo() : GenExceptionRequest(EExceptionRequestType::UpdateHandlerInfo), mUpdateHandlerParams(), mUpdateHandlerResult(0) { } //!< Default constructor.
    ~GenUpdateHandlerInfo(); //!< Destructor.
    const char* RequestType() const override { return "GenUpdateHandlerInfo"; } //!< Return GenUpdateHandlerInfo type.

    void AddDetail(const std::string& attrName, uint64 value) override; //!< Add request detail, with integer value parameter.
    void AddDetail(const std::string& attrName, const std::string& valueStr) override; //!< Add request detail, with value string parameter.

    void SetResults(int result) const { mUpdateHandlerResult = result; } //!< set up the handler update result
    void GetResults(py::object& rPyObject) const override; //!< Return request results in the passed in rPyObject if applicable.

    const std::map<std::string, std::string>& UpdaterHandlerParams() const { return mUpdateHandlerParams; } //!< Return given system call params

  protected:
    GenUpdateHandlerInfo(const GenUpdateHandlerInfo& rOther); //!< Copy constructor.
  protected:
    std::map<std::string, std::string> mUpdateHandlerParams;  //!< given handler update params
    mutable int mUpdateHandlerResult;  //!< system call result
  };

}

#endif
