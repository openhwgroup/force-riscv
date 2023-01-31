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
#ifndef Force_GeneratorRISCV_H
#define Force_GeneratorRISCV_H

#include "Generator.h"

namespace Force {

  class RegisterField;

  /*!
    \class GeneratorRISCV
    \brief Base generator class for RISC-V 64-bit architecture.
  */
  class GeneratorRISCV : public Generator {
  public:
    GeneratorRISCV() : Generator(-1ull), mpPrivilegeField(nullptr) { } //!< Constructor.
    GeneratorRISCV(const GeneratorRISCV& rOther) : Generator(rOther), mpPrivilegeField(nullptr) { } //!< Copy constructor.
    ~GeneratorRISCV(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GeneratorRISCV);

    Object* Clone() const override; //!< Clone a object of the GeneratorRISCV type.
    const char* Type() const override { return "GeneratorRISCV"; }

    void Setup(uint32 threadId) override; //!< Setup RISC-V generator.
    uint32 InstructionSpace() const override; //!< Return necessary space between instruction streams for RISC-V.
    uint32 BntReserveSpace() const override; //!< Return the number of memory bytes to reserve for a BNT path.
    uint32 BntMinSpace() const override; //!< Return the minimum number of memory bytes to generate a BNT path.
    void SetPrivilegeLevel(uint32 priv) override; //!< Set RISC-V exception level.
    uint32 PrivilegeLevel() const override; //!< Return RISC-V exception level
    ConditionFlags GetConditionFlags() const override; //!< Return current condition flag values.
    std::string GetGPRName(uint32 index) const override; //!< Return GPR name from index value
    std::string GetGPRExcludes() const override; //!< Return gpr exclude indices in string format
    bool RegisterUpdateSystem(const std::string& name, const std::string& field) override;
    void UpdateSystemWithRegister(const std::string& name, const std::string& field, uint64 value) override; //!< API that checks and update system according to given register name, field, and value
    void SetupInstructionGroup(EInstructionGroupType iGrpType) override; //!< Setup necessary resources for instruction group.
    bool OperandTypeCompatible(ERegisterType regType, EOperandType oprType) const override; //!< Check if the operand type is compatible with the register type.
    bool OperandTypeToResourceType(EOperandType opType, EResourceType& rResType) const override; //!< Convert operand type to resource type.
    uint32 GetResourceCount(EResourceType rResType) const override; //!< Return resource count for a certain resource type.
    void AdvancePC() override; //!< Advance Generator PC by default value.
  protected:
    void AddArchImageThreadInfo(std::map<std::string, uint64>& rThreadInfo) const override; //!< Add architectural specific image thread info.
  protected:
    RegisterField* mpPrivilegeField; //!<Privilege level internal register field pointer
  };

}

#endif
