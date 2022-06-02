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
#ifndef Force_GenSequenceAgent_H
#define Force_GenSequenceAgent_H

#include <list>
#include <vector>

#include "Constraint.h"
#include "Defines.h"
#include "GenAgent.h"

namespace Force {

  class GenSequenceRequest;
  class GenReloadRegister;
  class BootOrder;
  class Register;
  class BntNode;

  /*!
    \class GenSequenceAgent
    \brief A generator agent class to handle sequence generation.
  */
  class GenSequenceAgent : public GenAgent {
  public:
    GenSequenceAgent() : GenAgent(), mpSequenceRequest(nullptr), mBntLevel(0), mSpeculativeBntLevel(0), mHasGenEndOfTest(false) { } //!< Constructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenSequenceAgent);

    EGenAgentType GenAgentType() const override { return EGenAgentType::GenSequenceAgent; } //!< Return type of the generator agent.
    void SetGenRequest(GenRequest* genRequest) override; //!< Set pointer to GenRequest object.
  protected:
    GenSequenceAgent(const GenSequenceAgent& rOther) : GenAgent(rOther), mpSequenceRequest(nullptr), mBntLevel(0), mSpeculativeBntLevel(0), mHasGenEndOfTest(rOther.mHasGenEndOfTest) { } //!< Copy constructor, do not copy the request pointer.
    void HandleRequest() override; //!< Handle GenSequenceRequest transaction.

    void CommitInstruction(); //!< Handle GenCommitInstruction request.
    void LoadRegister(); //!< Handle GenLoadRegister request.
    void BootLoading(); //!< Handle BootLoading request
    void ThreadSummary(); //!< Generate instruction summary for a single thread
    void Summary(); //!< Generate instruction summary for all threads
    void RestoreBootStates(); //!< Restore boot states to start boot loading.
    void RegisterReservation(); //!< Handles register reservation related actions.
    void EscapeCollision(); //!< Handles escape from instruction collision.
    void BranchNotTaken(); //!< Handles branch-not-taken code generation.
    void ProcessBntNode(); //!< Handles BntNode code generation.
    void ReExecution(); //!< Handles code re-execution.
    void UpdatePeState(); //!< Handles PE state update.
    void UpdateRegisterField(); //!< Handles register field update.
    void SetRegister(); //!< Handles request to set register value.
    void SequenceUtility(); //!< Handle Sequence Utility
    void ReloadRegister(); //!< Handle reload register request.
    void BatchReloadRegisters(); //!< Handles request to batch reload registers.
    void ProcessSpeculativeBntNode(); //!< Handle speculative Bnt node
    void ExecuteSpeculativeBntNode(BntNode* pBntNode); //!< Execute speculative Bnt node
    void RestoreSpeculativeBntNode(BntNode* pBntNode); //!< Restore speculative Bnt node
    void PopSpeculativeBntNode(BntNode* pBntNode); //!< Pop speculative Bnt node
    void LoadLargeRegister(); //!< Handles Load large regsiter request.
    void BeginRestoreLoop(); //!< Handles begin restore loop request.
    void EndRestoreLoop(); //!< Handles end restore loop request.
    void RestoreLoopState(); //!< Handles restore loop state request.
    void LoopReconverge(); //!< Handles loop reconverge request.

    void ReserveBntConstraint(BntNode* pBntNode); //!< reserve some constraints from Bnt
    void UnreserveBntConstraint(BntNode* pBntNode); //!< unreserve some constraints from Bnt
    void InsertToRegisterListByBoot(std::list<Register*>& registerList, Register* element) const; //!< insert element to register list
    virtual void GetBootLoadRegisterRequests(BootOrder& rBootOrder, std::vector<GenRequest*>& rLoadRegisterRequests) const { } //!< Get requests to generate instructions for loading registers in the boot sequence.
    void EndOfTest(); //!< Handle EndOfTest sequenceRequest.
    virtual void BranchToTarget() { } //!< Handle BranchToTarget request.
    virtual void InitialSetup() { } //!< Initial setup of the Generator thread states.
    virtual void JumpToStart(); //!< Handle JumpToStart request.
    virtual void InitializeAddrTables() { } //!< Initialize address tables.
    virtual void RestoreArchBootStates() { } //!< Restore architectural boot states to start boot loading.
    virtual Register* GetBootLoadingGPR() const { return nullptr; } //!< Get a GPR for boot loading.
    virtual uint64 GetBootLoadingBaseAddress(uint32 size) const; //!< Get address for boot loading.
    virtual bool GetRandomReloadRegisters(uint32 number, std::vector<std::string >& regIndices) const { return false; } //!< Get the reload regsiters index.
    virtual void RegulateInitRegisters(std::list<Register*>& registers) const { } //!< Regulate initial registers

    void GetLoadRegisterSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq, const Register* interRegPtr=nullptr); //!< Obtain general get register loading sequence.
    void GetReloadRegisterSequence(const GenReloadRegister* requestPtr, std::vector<GenRequest* >& reqSeq); //!< Obtain general get register reloading sequence.
    EReloadingMethodType ChooseReloadingMethod() const; //!< Choose register reloading method.
    uint32 ChooseBatchReloadingNumber() const; //!< Choose register reloading number.
    uint64 GenerateAvailableAddress() const; //!< Return available address to using reload register.
    virtual void GetReloadBaseAddressSequence(const Register* interRegPtr, uint32 size, std::vector<GenRequest* >& reqSeq) { }//!< Provide instruction sequence to get reload base address.
    virtual void GetReloadGPRSequence(const GenReloadRegister* reqPtr, std::vector<GenRequest* >& reqSeq, const Register* interRegPtr=nullptr) { } //!< Provide an instruction sequence to load GPR.
    virtual void GetLoadGPRSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq) { } //!< To be overrided in Arch layer to load a GPR.
    virtual void GetLoadArchRegisterSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq, const Register* interRegPtr=nullptr); //!< Load architecture depended register type.
    virtual void GetStore64BitRegisterSequence(const Register* pSrcRegister, Register* pDestRegister , std::vector<GenRequest* >& reqSeq) {}
                //!< Provide an instruction sequence to store GPR registers
    virtual void GetShortBranchConstraint(uint64 pcValue, ConstraintSet& rPcOffsetConstr);//!< Get the Branch constraint when space is limited
    virtual void GetLoadLargeRegisterSequence(const Register* regPtr, const std::vector<uint64>& loadValue, std::vector<GenRequest* >& reqSeq, const Register* interRegPtr, uint32 immOffset) {}; //!< Provide instruction sequence to get load large register.
  protected:
    GenSequenceRequest* mpSequenceRequest; //!< Pointer to GenSequenceRequest object.
    uint32 mBntLevel; //!< the current level for un-speculative branch not taken level
    uint32 mSpeculativeBntLevel; //!< the current level for speculative branch not taken level
  private:
    virtual void GetEndOfTestSequence(std::vector<GenRequest*>& rReqSeq) const = 0; //!< Provide instruction sequence to end the test.
    virtual uint32 GetEndOfTestInstructionCount() const = 0; //!< Return the number of instructions generated by the end of test sequence.
  private:
    bool mHasGenEndOfTest; //!< flag indicating whether EndOfTest sequence has been generated
  };

}

#endif
