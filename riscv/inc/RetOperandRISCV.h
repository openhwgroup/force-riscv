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
#ifndef Force_RetOperandRISCV_H
#define Force_RetOperandRISCV_H

#include <map>
#include <string>

#include "Defines.h"
#include "Operand.h"
#include "OperandConstraint.h"

namespace Force {

  class VmMapper;

  /*!
    \class RetOperandConstraint
    \brief The class carries dynamic constraint properties for RetOperand.
  */
  class RetOperandConstraint : public ChoicesOperandConstraint {
  public:
    RetOperandConstraint(); //!< Constructor.
    ~RetOperandConstraint() { } //!< Destructor.
    void Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct) override; //!< Setup dynamic operand constraints for ChoicesOperand

    void SetReturnStates(uint32 targetPriv); //!< update the return core state.

    void SetPEUpdateId(uint64 peUpdateId) { mPEUpdateId = peUpdateId; } //!< Set the pe update id.
    void AddReloadRegister(const std::string& name, uint64 value) { mReloadRegisters[name] = value; } //Add Reload register value

    uint32 PrivilegeLevel() const { return mPrivilegeLevel; } //!< Exception Level
    uint64 PEUpdateId() const { return mPEUpdateId; } //!< return the PE update id.
    uint32 TargetPrivilege() const { return mTargetPrivilege; } //!< target exception level.
    bool Undefined() const { return mUndefined; } //!< ret is undefined.
    bool IllegalReturn() const { return mIllegalReturn; } //!< illegial state.
    bool HasPreambleSequence() const { return mPreambleSequence != ""; } //!< return true if custom eret preamble defined
    const ConstraintSet* TargetAddressConstraint() const { return mpTargetAddressConstraint; } //!< Return the target address constraint.
    const ConstraintSet* TargetStatesConstraint() const { return mpTargetStatesConstraint; } //!< Return the target state constraint.
    const std::string& PreambleSequence() const { return mPreambleSequence; } //!< return preamble sequence named defined on front-end
    const std::string& EpcName() const { return mEpcName; } //!< return *epc register name
    const std::string& StatusName() const { return mStatusName; } //!< return *status register name
    const std::string& PpName() const { return mPpName; } //!< return *PP register field name
    const std::string& PieName() const { return mPieName; } //!< return *PIE register field name
    const std::map<std::string, uint64>& ReloadRegisters() const { return mReloadRegisters; }
  protected:
    ASSIGNMENT_OPERATOR_ABSENT(RetOperandConstraint); //!< operator "=" no used.
    COPY_CONSTRUCTOR_ABSENT(RetOperandConstraint); //!< Copy constructor, not meant to be used.
  private:
    uint32 mPrivilegeLevel; //!< exception level.
    bool mUndefined; //!< whether ret is undefined.
    bool mIllegalReturn; //!< illegal return.
    const ConstraintSet* mpTargetAddressConstraint; // Target address constraint.
    const ConstraintSet* mpTargetStatesConstraint; // Target state constraint.
    std::string mPreambleSequence; //!< the name of ret preamble sequence class
    std::string mEpcName; //!< name of the *epc register with priv prefix
    std::string mStatusName; //!< name of the *status register with priv prefix
    std::string mPpName; //!< name of the *PP register field with priv prefix
    std::string mPieName; //!< name of the *PIE register field with priv prefix
    uint64 mPEUpdateId; //!< PE update id.
    uint32 mTargetPrivilege; //!< target exception level.
    std::map<std::string, uint64> mReloadRegisters; //!< need update system register values.
  };

  /*!
    \class RetOperand
    \brief special operand to handle RET additional instructions
  */
  class RetOperand : public ChoicesOperand {
  public:
    Object* Clone() const override { return new RetOperand(*this); } //!< Return a cloned RetOperand object of the same type and same contents of the object.

    const char* Type() const override { return "RetOperand"; } //!< Return the type of the Imms32MaskOperand object in C string.

    RetOperand() : ChoicesOperand() { } //!< Constructor.
    ~RetOperand() { } //!< Destructor

    void Setup(Generator& gen, Instruction& instr) override; //!< Setup for generation of RetOperand object.
    void Generate(Generator& gen, Instruction& instr) override; //!< Generate RetOperand details.
    void Commit(Generator& gen, Instruction& instr) override; //!< Commit generated RetOperand.

    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary pre/post amble requests, if any.
  protected:
    RetOperand(const RetOperand& rOther) : ChoicesOperand(rOther) { } //!< Copy constructor.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstaint object.

    bool ProcessPreambleSequence(Generator& gen) const; //!< Get ret callback sequence and append to preamble
    bool ValidateTargetVm(const VmMapper* target_mapper); //!< validate target privilege VM information and context.

    void GenerateTargetStates(const Generator& rGen, Instruction& instr); //!< Generate return states.
    void GenerateTargetAddress(Generator& rGen, Instruction& instr); //!< Generate return branch target.
    void GenerateOthers(const Generator& rGen, Instruction& instr); //!< Generate other return state information.
  protected:
    ASSIGNMENT_OPERATOR_ABSENT(RetOperand);
  private:
    uint64 GetEffectiveTargetPc(const Generator& rGen, cuint64 epcVal) const; //!< Get the effective target PC value. This may differ from the value of the xepc register depending on the value of IALIGN.
  };

}
#endif //Force_RetOperandRISCV_H
