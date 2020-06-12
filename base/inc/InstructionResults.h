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
#ifndef Force_InstructionResults_H
#define Force_InstructionResults_H

#include <Defines.h>
#include <Object.h>
#include <map>
#include <vector>

namespace Force {

  class Instruction;
  class Generator;

  class InstructionResultsBank : public Object  {
  public:
    explicit InstructionResultsBank(uint32 bank) : Object(), mBank(bank), mResults() {} //!< Constructor with bank ID given.
    InstructionResultsBank() : Object(), mBank(0), mResults() {} //!< Default constructor.
    Object* Clone() const override;  //!< Return a cloned InstructionResultsBank object of the same type
    const std::string ToString() const override; //!< Return a string describing the current state of the InstructionResultsBank object.
    const char* Type() const override { return "InstructionResultsBank"; } //!< Return a string describing the actual type of the InstructionResultsBank object
    ~InstructionResultsBank(); //!< Destructor.

    const uint32 Bank() { return mBank; }
    const Instruction* LookupInstruction(uint64 address) const; //!< Look up instruction by its program address.
    void AddInstruction(uint64 pa, Instruction* instr); //!< Add an instruction.
    const std::map<uint64, Instruction* >& GetInstructions() const { return mResults; } //!< Return instruction results in the memory bank.
    uint32 GetInstructionCount() const; //!< Return number of instructions.
  protected:
    InstructionResultsBank(const InstructionResultsBank& rOther); //!< Copy constructor.
  private:
    uint32 mBank; //!< Memory bank ID.
    std::map<uint64, Instruction* > mResults; //!< Container of all generated instructions in a memory bank.
  };

  /*!
    \class ThreadInstructionResults
    \brief Container for all the generated instructions in the current PE.
  */

  class ThreadInstructionResults : public Object {
  public:
    explicit ThreadInstructionResults(uint32 numBanks); //!< Constructor with bank parameter.
    Object* Clone() const override;  //!< Return a cloned ThreadInstructionResults object of the same type
    const std::string ToString() const override; //!< Return a string describing the current state of the ThreadInstructionResults object.
    const char* Type() const override { return "ThreadInstructionResults"; } //!< Return a string describing the actual type of the ThreadInstructionResults object
    ~ThreadInstructionResults(); //!< Destructor.

    bool Commit(Generator* gen, Instruction* instr); //!< Commit a generated instruction.
    void GenSummary(); // !< Generate the Instruction summary
    const Instruction* LookupInstruction(uint32 bank, uint64 address) const; //!< Look up instruction by its physical address.
    const std::map<uint64, Instruction* >& GetInstructions(uint32 bank) const; //!< Return instruction results in a memory bank.
    uint32 GetInstructionCount(cuint32 bank) const; //!< Return number of instructions for the specified bank.
    uint32 GetBankCount() const; //!< Return number of instruction results banks.
    void InvalidCurrentBankAddress(); //!< Invalidate current bank and address.
    void GetCurrentInstructionRecordId (std::string& rec_id); //!< Return current isntruction record id
  protected:
    ThreadInstructionResults() : Object(), mBanks(), mCurBank(0), mCurAddr(0), mCurAddrValid(false), mRecordId() { } //!< Default constructor.
    ThreadInstructionResults(const ThreadInstructionResults& rOther); //!< Copy constructor.
    void AddInstruction(uint32 bank, uint64 pa, Instruction* instr); //!< Add an instruction.
  private:
    std::vector<InstructionResultsBank* > mBanks; //!< Container of all generated instructions.
    uint32 mCurBank; //!< Current instruction bank
    uint64 mCurAddr; //!< Current instruction address
    bool mCurAddrValid; //!< Flag indicating whether the current instruction address value is valid
    std::string mRecordId; //!< current record id
  };

  /*!
    \class InstructionResults
    \brief Container for all the generated instructions.
  */

  class InstructionResults {
  public:
    DESTRUCTOR_DEFAULT(InstructionResults); //!< Destructor
    COPY_CONSTRUCTOR_ABSENT(InstructionResults);
    ASSIGNMENT_OPERATOR_ABSENT(InstructionResults);

    static void Initialize(); //!< Initialize interface.
    static void Destroy(); //!< Clean up interface.
    static InstructionResults* Instance() { return mspInstructionResults; } //!< Access instruction results instance.
    void AddThreadResults(const ThreadInstructionResults* pThreadInstructionResults); //!< Add reference to thread's results.
    void GenSummary(); //!< Generate the instruction summary.
  private:
    InstructionResults(); //!< Default constructor
  private:
    static InstructionResults* mspInstructionResults; //!< Static pointer to instruction results instance.
    std::vector<const ThreadInstructionResults*> mThreadInstructionResults;
  };

}

#endif
