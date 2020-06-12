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
#ifndef Force_GenQueryAgent_H
#define Force_GenQueryAgent_H

#include <GenAgent.h>

namespace Force {

  class GenQuery;
  class Operand;
  class GenInstructionRecordQuery;

  /*!
    \class GenQueryAgent
    \brief A generator agent class to handle instruction generation.
  */
  class GenQueryAgent : public GenAgent {
  public:
    explicit GenQueryAgent(Generator* gen) : GenAgent(gen), mpQuery(nullptr) { } //!< Constructor with Generator pointer parameter.
    GenQueryAgent() : GenAgent(), mpQuery(nullptr) { } //!< Constructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenQueryAgent);

    Object* Clone() const override; //!< Return a cloned GenQueryAgent object of the same type and content.
    const char* Type() const override { return "GenQueryAgent"; } //!< Return type of the GenQueryAgent object.

    EGenAgentType GenAgentType() const override { return EGenAgentType::GenQueryAgent; } //!< Return type of the generator agent.
    void SetGenQuery(const GenQuery* genQuery) const override { mpQuery = genQuery;} //!< Set pointer to GenQuery object.
  protected:
    GenQueryAgent(const GenQueryAgent& rOther) : GenAgent(rOther), mpQuery(nullptr) { } //!< Copy constructor, do not copy the request pointer.
    void HandleQuery() const override; //!< Handle GenQuery transaction.

    void RegisterIndex() const; //!< Handle RegisterIndex query.
    void RegisterReloadValue() const; //!< Handle RegisterReloadValue query.
    void RegisterFieldInfo() const;  //!> Handle RegisterFieldInfo query
    void RegisterInfo() const;  //!< Handle RegisterInfo query.
    void UpdateTargetQueryWithOperandInformation(bool check_reg_status, const GenInstructionRecordQuery* pTargetQuery, Operand* pOperand) const; //!< Update query with operand information according to logic in InstructionRecord(), so that GroupOperand's can be processed correctly as well.
    void InstructionRecord() const; //!< Handle InstructionRecord query.
    void GenState() const; //!< Handle GenState query.
    void ChoicesTreeInfo() const; //!< handle ChoicesTreeInfo query.
    void ExceptionsHistory(int last_only_flag) const; //!< handle the ExceptionHistory query.
    void GetVmContextDelta() const; //!< handle the GetVmContextDelta query.
    void GetVmCurrentContext() const; //!< handle the GetVmCurrentContext query.
    /* Allows specific architectures to implement arch-specific queries */
    virtual bool GenStateArch() const { return false; }
    virtual bool UtilityArch() const { return false; }

    void PageInfo() const; //!< Handle PageInfo query
    void BranchOffset() const; //!< Handle BranchOffset query.
    void Utility() const; //!< Handle Utility query
    void HandlerSetMemory() const; //!< handle exception handler memory set query.
    void ExceptionVectorBaseAddress() const; //!< handle exception vector base address query.
    void ResourceEntropy() const; //!< handle resource entropy
    void RestoreLoopContext() const;  //!< Handle RestoreLoopContext query.
  protected:
    mutable const GenQuery* mpQuery; //!< Const pointer to GenQuery object.
  };

}

#endif
