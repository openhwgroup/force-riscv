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
#ifndef Force_GenSequenceAgentRISCV_H
#define Force_GenSequenceAgentRISCV_H

#include <GenSequenceAgent.h>

namespace Force {

  /*!
    \class GenSequenceAgentRISCV
    \brief A generator agent class to handle RISC-V sequence generation.
  */
  class GenSequenceAgentRISCV : public GenSequenceAgent {
  public:
    GenSequenceAgentRISCV() : GenSequenceAgent() { } //!< Constructor.
    Object* Clone() const override;  //!< Return a cloned GenSequenceAgentRISCV object of the same type and content.
    const char* Type() const override { return "GenSequenceAgentRISCV"; } //!< Return type of the GenSequenceAgentRISCV object.
  protected:
    GenSequenceAgentRISCV(const GenSequenceAgentRISCV& rOther) : GenSequenceAgent(rOther) { } //!< Copy constructor.
    void GetBootLoadRegisterRequests(BootOrder& rBootOrder, std::vector<GenRequest*>& rLoadRegisterRequests) const override; //!< Get requests to generate instructions for loading registers in the boot sequence.
    void GetReloadGPRSequence(const GenReloadRegister* reqPtr, std::vector<GenRequest* >& reqSeq, const Register* interRegPtr=nullptr) override; //!< Provide an instruction sequence to load GPR.
    void GetReloadBaseAddressSequence(const Register* interRegPtr, uint32 size, std::vector<GenRequest* >& reqSeq) override; //!< Provide instruction sequence to get reload base address.
    void GetReloadGPRUsingMoveSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq); //!< Provide instruction sequence to reload GPR.
    void GetReloadGPRUsingLoadSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq, const Register* interRegPtr); //!< Provide instruction sequence to reload GPR.
    void GetReloadGPRsUsingLoadPairSequence(const Register* regPtr1, uint64 loadValue1, const Register* regPtr2, uint64 loadValue2, std::vector<GenRequest* >& reqSeq, const Register* interRegPtr); //!< Provide instruction sequence to reload GPRs.

    void GetLoadArchRegisterSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq, const Register* interRegPtr=nullptr) override; //!< Provide an instruction sequence to load RISC-V architecture depended register type.
    void GetLoadGPRSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq) override; //!< Provide an instruction sequence to load GPR.
    void GetLoadSysRegSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq, const Register* gprPtr = nullptr); //!< Provide an instruction sequence to load RISC-V SysReg registers.
    void GetLoadFPRSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq, const Register* gprPtr = nullptr); //!< Provide an instruction sequence to load RISC-V FPR registers.
    void GetLoadVecRegSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq, const Register* gprPtr = nullptr); //!< Provide an instruction sequence to load RISC-V VECREG registers.

    void GetBranchToSelfSequence(std::vector<GenRequest* >& req_seq) const; //!< Provide an instruction sequence to branch to self.
    void GetStore64BitRegisterSequence(const Register* pSrcRegister, Register* pBaseRegister, std::vector<GenRequest* >& reqSeq) override; //!< Provide an instruction sequence to store GPR registers
    void BranchToTarget() override; //!< Override base class for BranchToTarget sequence.
    void InitialSetup() override; //!< Setup initial states for the generator thread.
    void JumpToStart() override; //!< Handle JumpToStart request.
    void InitializeAddrTables() override; //!<  Initial address table of the Generator.
    void RestoreArchBootStates() override; //!< Restore RISC-V boot states for boot loading.
    Register* GetBootLoadingGPR() const override; //!< Get a GPR for boot loading.
    bool GetRandomReloadRegisters(uint32 number, std::vector<std::string >& regIndices) const override; //!< Get the reload regsiters index.
    void RegulateInitRegisters(std::list<Register*>& registers) const override; //!<Regulate initial registers

    bool ValidateEretStartOption(uint64 target_pc);  //!< check choice to determine whether to use Eret or branch
    void EretStart(uint64 init_pc); //!< use Eret to jump to the start of the main test.
    bool UseBranchSelfToEndTest(void); //!< use branch to self sequence to end of test
    Register* GetRandomGPR(char regPrefix) const; //!< Obtain a pointer to a random GPR register object.
    void GetShortBranchConstraint(uint64 pcValue, ConstraintSet& rPcOffsetConstr) override; //!< Get the Branch constraint when space is limited.
  private:
    void GetEndOfTestSequence(std::vector<GenRequest*>& rReqSeq) const override; //!< Provide instruction sequence to end the test.
    uint32 GetEndOfTestInstructionCount() const override; //!< Return the number of instructions generated by the end of test sequence.
    void GetLoadGPRTop53BitsSetSequence(const Register* regPtr, uint32 loadValue, std::vector<GenRequest* >& reqSeq); //!< Provide an instruction sequence to load a GPR with a value that has the top 53 bits set. This method takes advantage of the sign extension for LUI and ADDI to perform the operation in two instructions.
    void GetLoadGPR32BitSequence(const Register* regPtr, uint32 loadValue, std::vector<GenRequest* >& reqSeq); //!< Provide an instruction sequence to load a GPR with a 32-bit value. If Bit 31 is set in loadValue, the value loaded into the register will be sign-extended to 64 bits.
  };

}

#endif
