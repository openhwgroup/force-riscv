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
#ifndef Force_Instruction_H
#define Force_Instruction_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <Object.h>
#include <vector>
#include <map>

namespace Force {

  class InstructionStructure;
  class Operand;
  class Generator;
  class InstructionConstraint;
  class GenInstructionRequest;
  class ConstraintSet;
  class BntNode;
  class ResourceAccessStage;

  /*!
    \class Instruction
    \brief Base class for instructions.
  */
  class Instruction : public Object {
  public:
    Object* Clone() const override;  //!< Return a cloned Instruction object of the same type and same contents of the object.
    const std::string ToString() const override; //!< Return a string describing the current state of the Instruction object.
    const char* Type() const override { return "Instruction"; } //!< Return the type of the Instruction object in C string.

    Instruction(); //!< Default constructor.
    ~Instruction(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(Instruction);

    const std::string FullName() const; //!< Return instruction full name.
    const std::string& Name() const; //!< Return instruction name.
    const std::string AssemblyText() const; //!< return instruction assembly code.
    uint32 Opcode() const { return mOpcode; } //!< Return instruction final opcode.
    uint32 Size() const; //!< Return instruction size in number of bits.
    uint32 ByteSize() const; //!< Return instruction size in number of bytes.
    uint32 ElementSize() const; //!< Return instruction element size in number of bytes.
    const Operand* FindOperand(const std::string& oprName, bool failNotFound=false) const; //!< Find operand by name.
    Operand* FindOperandMutable(const std::string& oprName, bool failNotFound=false) const; //!< Find operand by name, return a non-const pointer.
    const InstructionConstraint* GetInstructionConstraint() const { return mpInstructionConstraint; } //!< Return const pointer to instruction constraint object.
    void SetOperandDataValue(const std::string& oprName, uint64 oprValue, uint32 valueSize) const; //!< Set operand data value.
    void SetOperandDataValue(const std::string& oprName, const std::string& oprValue, uint32 valueSize) const; //!< Set operand data value
    void SetOperandDataValue(const std::string& oprName, std::vector<uint64> oprValues, uint32 valueSize) const; //!< Set operand data values for LargeRegister(Z).
    ResourceAccessStage* GiveHotResource(); //!< give hot resource
    virtual void Initialize(const InstructionStructure* instrStructure); //!< Initialize instruction object.
    virtual void Setup(const GenInstructionRequest& instrReq, Generator& gen); //!< Setup conditions, constraining mechanisms before generating instruction.
    virtual void Generate(Generator& gen); //!< Generate instruction details.
    virtual void Commit(Generator& gen); //!< Commit generated instruction.
    virtual void CleanUp(); //!< Clean up resources that can be released.
    virtual bool GetPrePostAmbleRequests(Generator& gen) const { return false; } //!< Return preamble requests if there is any.
    virtual bool Validate(Generator& gen, std::string& error) const { return true; } //!< Validate generation control.
    /*!
      Templated function to find Operand by its class type.
      For example, const VectorRegisterOperand* vec_opr = instr.FindOperand<VectorRegisterOperand>();
     */
    template<typename T>
      const T* FindOperandType() const
      {
        for (auto opr_ptr : mOperands) {
          auto cast_ptr = dynamic_cast<const T*>(opr_ptr);
          if (nullptr != cast_ptr) return cast_ptr;
        }
        return nullptr;
      }

    virtual bool IsBranch() const { return false; } //!< Return whether the instruction is a branch instruction.
    virtual bool IsBranchTaken() const { return false; } //!< Return whether the instruction is a branch and has taken the branch.
    virtual uint64 BranchTarget() const { return 0; } //!< Return the branch target if the instruction is a branch instruction.
    virtual BntNode* GetBntNode() const { return nullptr; } //!< Return branch information in a BntNode object, if applicable.
    virtual bool IsLoadStore() const { return false; } //!< Return whether the instruction is a load-store instruction.
    virtual bool IsSystemCall() const { return false; } //!< Return whether the instruction is a system-call instruction.
    virtual bool IsCacheOp() const {return false;} //!< Return whether the instruction is a cache mantainance instruction.
    virtual bool IsAtOp() const {return false;} //!< Return whether the instruction is a AT instruction
    virtual bool IsPartialWriter() const { return false; } //!< Return whether an instruction is partial write instruction
    bool NoRestriction() const; //!< Return whether the instruction request to not be restricted.
    bool NoSkip() const; //!< Return whether the instruction can be skipped or not.
    bool UnalignedPC() const; //!< Return whether the instruction should add PC miss-alignment.
    bool AlignedData() const; //!< Return whether the instruction should require data alignment.
    bool AlignedSP() const; //!< Return whether the instruction should require SP alignment.
    bool NoBnt() const; //! Return whether an branch instruction has branch not taken node.
    bool NoPreamble() const; //!< Return true when preamble instructions are not allowed.
    bool SpeculativeBnt() const; //!< Return whether a branch instruction has speculative Bnt.
    bool NoDataAbort() const; //!< Return whether a loadstore instruction has nodataabort attribute.
    bool SharedTarget() const; //!< Return whether a load or store instruction has the SharedTarget attribute.
    inline bool Unpredictable() const { return mUnpredictable; } //!< Return whether instruction is unpredictable
    inline void SetUnpredictable(bool unpredict) {mUnpredictable = unpredict; } //!< set unpredict
    const ConstraintSet* BranchTargetConstraint() const; //!< Return branch target constraint if available.
    const ConstraintSet* LoadStoreTargetConstraint() const;  //!< Return load store target constraint if available.
    const std::vector<ConstraintSet* >& LoadStoreDataConstraints() const;  //!< Return the data constraint if available.
    const ConstraintSet* ConditionTakenConstraint() const; //!< return branch taken or not constraint if available.
    //\ section - instruction record
    EInstructionGroupType Group() const;
    const std::vector<Operand* > GetOperands() const { return mOperands; }
    const std::vector<ConstraintSet* >& LoadStoreGatherScatterTargetListConstraints() const;  //!< Return the targetlist constraint if available.
  protected:
    Instruction(const Instruction& rOther); //!< Copy constructor.
    void Assemble(); //!< Assembly instruction opcode.
    virtual InstructionConstraint* InstantiateInstructionConstraint() const; //!< Return an instance of appropriate InstructionConstraint object.
  protected:
    const InstructionStructure* mpStructure; //!< Pointer to InstructionStructure object that described the structure of the instruction.
    InstructionConstraint* mpInstructionConstraint; //!< Dynamic constraints for the instruction generation.
    uint32 mOpcode; //!< Instruction opcode.
    std::vector<Operand* > mOperands; //!< Container holding pointer to all operands.
    bool mUnpredictable; //!< whether instruction result is unpredictable
  };

  /*!
    \class PartialWriteInstruction
    \brief Base class for partial write instructions.
  */
  class PartialWriteInstruction : public Instruction {
  public:
    Object* Clone() const override { return new PartialWriteInstruction(*this); }  //!< Return a cloned PartialWriteInstruction object of the same type and same contents of the object.
    const char* Type() const override { return "PartialWriteInstruction"; } //!< Return the type of the PartialWriteInstruction object in C string.

    PartialWriteInstruction() : Instruction() { } //!< Constructor
    ~PartialWriteInstruction() { } //!< Destructor
    bool IsPartialWriter() const override { return true; } //!< Return true to indicate that this is a partial write instruction
  protected:
    PartialWriteInstruction(const PartialWriteInstruction& rOther) : Instruction(rOther) { } //!< Copy constructor.

  };

  /*!
    \class BranchInstruction
    \brief Base class for branch instructions.
  */
  class BranchInstruction : public Instruction {
  public:
    Object* Clone() const override { return new BranchInstruction(*this); }  //!< Return a cloned BranchInstruction object of the same type and same contents of the object.
    const char* Type() const override { return "BranchInstruction"; } //!< Return the type of the BranchInstruction object in C string.

    BranchInstruction() : Instruction() { } //!< Constructor
    ~BranchInstruction() { } //!< Destructor
    bool IsBranch() const override { return true; } //!< Return true to indicate that this is a branch instruction.
    bool IsBranchTaken() const override; //!< Return true to indicate that this branch is taken.
    uint64 BranchTarget() const override; //!< Return the branch target.
    BntNode* GetBntNode() const override; //!< Return branch information in a BntNode object.
    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary preamble requests, if any.
  protected:
    BranchInstruction(const BranchInstruction& rOther) : Instruction(rOther) { } //!< Copy constructor.

    InstructionConstraint* InstantiateInstructionConstraint() const override; //!< Return an instance of BranchInstructionConstraint object.
  };

  /*!
    \class LoadStoreInstruction
    \brief Base class for branch instructions.
  */
  class LoadStoreInstruction : public virtual Instruction {
  public:
    Object* Clone() const override { return new LoadStoreInstruction(*this); }  //!< Return a cloned LoadStoreInstruction object of the same type and same contents of the object.
    const char* Type() const override { return "LoadStoreInstruction"; } //!< Return the type of the LoadStoreInstruction object in C string.

    LoadStoreInstruction() : Instruction() { } //!< Constructor
    ~LoadStoreInstruction() { } //!< Destructor
    bool IsLoadStore() const override { return true; } //!< Return true to indicate that this is a branch instruction.
    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary preamble requests, if any.
  protected:
    LoadStoreInstruction(const LoadStoreInstruction& rOther) : Instruction(rOther) { } //!< Copy constructor.

    InstructionConstraint* InstantiateInstructionConstraint() const override; //!< Return an instance of LoadStoreInstructionConstraint object.
  };

  /*!
    \class UnpredictStoreInstruction
    \brief class for unpredictable store instructions.
  */
  class UnpredictStoreInstruction : public LoadStoreInstruction {
  public:
    Object* Clone() const override { return new UnpredictStoreInstruction(*this); }  //!< Return a cloned UnpredictStoreInstruction object of the same type and same contents of the object.
    const char* Type() const override { return "UnpredictStoreInstruction"; } //!< Return the type of UnpredictStoreInstruction object in C string.
    void Generate(Generator& gen) override; //!< Generate instruction details.
    UnpredictStoreInstruction() : LoadStoreInstruction() { } //!< Constructor
    ~UnpredictStoreInstruction() { } //!< Destructor
  protected:
    UnpredictStoreInstruction(const UnpredictStoreInstruction& rOther) : LoadStoreInstruction(rOther) { } //!< Copy constructor.
  };


  /*!
    \class SystemCallInstruction
    \brief Base class for system call instructions.
  */
  class SystemCallInstruction : public virtual Instruction {
  public:
    Object* Clone() const override { return new SystemCallInstruction(*this); }  //!< Return a cloned SystemCallInstruction object of the same type and same contents of the object.
    const char* Type() const override { return "SystemCallInstruction"; } //!< Return the type of the SystemCallInstruction object in C string.

    SystemCallInstruction() : Instruction() { } //!< Constructor
    ~SystemCallInstruction() { } //!< Destructor
    bool IsSystemCall() const override { return true; } //!< Return true to indicate that this is a system call instruction.
    //bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary preamble requests, if any.
  protected:
    SystemCallInstruction(const SystemCallInstruction& rOther) : Instruction(rOther) { } //!< Copy constructor.

    //InstructionConstraint* InstantiateInstructionConstraint() const override; //!< Return an instance of SystemCallInstructionConstraint object.
  };

  /*!
    \class VectorInstruction
    \brief Class for vector instructions.
  */
  class VectorInstruction : public virtual Instruction {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VectorInstruction);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorInstruction);
    ASSIGNMENT_OPERATOR_ABSENT(VectorInstruction);

    Object* Clone() const override { return new VectorInstruction(*this); } //!< Return a cloned VectorInstruction object of the same type and same contents of the object.
    const char* Type() const override { return "VectorInstruction"; } //!< Return the type of the VectorInstruction object in C string.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VectorInstruction);

    InstructionConstraint* InstantiateInstructionConstraint() const override; //!< Return an instance of appropriate InstructionConstraint object.
  };

  /*!
    \class VectorLoadStoreInstruction
    \brief Class for vector load store instructions.
  */
  class VectorLoadStoreInstruction : public VectorInstruction, public LoadStoreInstruction {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VectorLoadStoreInstruction);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorLoadStoreInstruction);
    ASSIGNMENT_OPERATOR_ABSENT(VectorLoadStoreInstruction);

    Object* Clone() const override { return new VectorLoadStoreInstruction(*this); } //!< Return a cloned object of the same type and same contents of the object.
    const char* Type() const override { return "VectorLoadStoreInstruction"; } //!< Return the type of the VectorLoadStoreInstruction object in C string.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VectorLoadStoreInstruction);

    InstructionConstraint* InstantiateInstructionConstraint() const override; //!< Return an instance of appropriate InstructionConstraint object.
  };

}

#endif
