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
#ifndef Force_GenInstructionAgent_H
#define Force_GenInstructionAgent_H

#include <Defines.h>
#include <GenAgent.h>
#include <Notify.h>
#include <NotifyDefines.h>
#include <vector>

namespace Force {

  class GenInstructionRequest;
  class Instruction;
  class SimAPI;
  class Register;
  class ReadOnlyRegister;
  class ReadOnlyRegisterField;
  class BntNode;
  struct RegUpdate;
  struct MemUpdate;
  struct ExceptionUpdate;

  /*!
    \class GenInstructionAgent
    \brief A generator agent class to handle instruction generation.
  */
  class GenInstructionAgent : public GenAgent, public NotificationSender, public NotificationReceiver {
  public:
    GenInstructionAgent() : GenAgent(), Receiver(), mInstrSimulated(0), mpInstructionRequest(nullptr), mRegisterInitializations() { } //!< Constructor.
    ~GenInstructionAgent(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenInstructionAgent);

    Object* Clone() const override;  //!< Return a cloned GenInstructionAgent object of the same type and content.
    const char* Type() const override { return "GenInstructionAgent"; } //!< Return type of the GenInstructionAgent object.

    EGenAgentType GenAgentType() const override { return EGenAgentType::GenInstructionAgent; } //!< Return type of the generator agent.
    void SetGenRequest(GenRequest* genRequest) override; //!< Set pointer to GenRequest object.
    void StepInstruction(const Instruction* pInstr); //!< Step commited instruction.
    void ExecuteHandler(); //!< Step through exception handler.
    void ExceptionReturn(); //!< Handle exception return.
    void ReExecute(cuint64 addr, cuint32 maxReExeInstr); //!< Handles re-execution.
    void HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* payload) override; //!< Handle a notification.
    void InitializeReadOnlyRegistersWithISS(const std::vector<ReadOnlyRegister* >& rRegs); //!< Initialize readonly register by reading values from ISS.
    void ResetReadOnlyRegisterFieldsWithISS(const std::vector<ReadOnlyRegisterField* >& rRegFields); //!< initialize readonly register fields by reading values from ISS
  protected:
    GenInstructionAgent(const GenInstructionAgent& rOther); //!< Copy constructor.
    void HandleRequest() override; //!< Handle GenRequest transaction.
    void StepInstructionNoSimulation(const Instruction* pInstr); //!< Step commited instruction with no ISS.
    bool StepInstructionWithSimulation(const Instruction* pInstr=nullptr); //!< simulate generated instruction with ISS, return true if there is an event.
    void UpdateRegisterFromSimulation(const std::vector<RegUpdate>& regUpdates, bool hasExceptEvent, uint64& targetPC); //!< update register from iss updates.
    void UpdateMemoryFromSimulation(const std::vector<MemUpdate>& memUpdates); //!< update memory from iss updates.
    void SendInitsToISS(); //!< Send initializations to ISS before stepping.
    void ReleaseInits(); //!< Release initializations when ISS is not available.
    bool HasExceptionEvent(const std::vector<ExceptionUpdate> & rExcepEvents, bool& hasEretEvent) const; //!< Check if there is exception event, return true if there is an event.
    bool UpdateExceptionEvent(std::vector<ExceptionUpdate> & rExcepEvents); //!< Update exception event.
    void UpdateInstructionCount(); //!< Update simulated instruction count.
    void UpdateAccurateBnt(const Instruction* pInstr, uint64 targetPC = 0); //!< Update BNT information when branch direction is accurate.
    virtual bool IsLowPower(uint32 exceptionId) const { return false; } //!< whether the exception is low power
    virtual bool IsSimExit(uint32 exceptionId) const { return false; } //!< whether the exception is sim exit
    virtual bool IsEret(uint32 exceptionId) const { return false; } //!< whether the exception is eret
    void SkipRequest(Instruction* pInstr); //!< skip request and delete resource

    void RecoverExceptionBeforeUpdate(const std::vector<ExceptionUpdate>& exceptUpdates, const std::vector<RegUpdate>& regUpdates, SimAPI* pSimAPI); //!< Recover exception
    void SaveRegisterBeforeUpdate(const std::vector<RegUpdate>& regUpdates, BntNode* pBntNode); //!< Save register states before update.
    void SaveLoopRegisterBeforeUpdate(const std::vector<RegUpdate>& regUpdates); //!< Save loop register states before update.
    void SaveMemoryBeforeUpdate(const std::vector<MemUpdate>& memUpdates, BntNode* pBntNode); //!< Save memory states before update.
    void SaveLoopMemoryBeforeUpdate(const std::vector<MemUpdate>& memUpdates); //!< Save loop memory states before update.
    void UpdateUnpredictedConstraint(const Instruction* pInstr); //!< Does some uppredicted constraint on operand registers
  protected:
    uint64 mInstrSimulated; //!< Number of instructions simulated.
    GenInstructionRequest* mpInstructionRequest; //!< Pointer to GenInstructionRequest object.
    std::vector<Register* > mRegisterInitializations; //!< Vector of initialized registers.
  private:
    void InitializeLoopMemory(cuint64 startVa, cuint64 memRangeSize) const; //!< Initialize any uninitialized memory that will be recorded by a restore loop.
 };
}

#endif
