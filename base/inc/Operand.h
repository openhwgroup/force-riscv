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
#ifndef Force_Operand_H
#define Force_Operand_H

#include <Defines.h>
#include <Object.h>
#include <vector>
#include <UtilityFunctions.h>

namespace Force {

  class Generator;
  class Instruction;
  class OperandStructure;
  class OperandConstraint;
  class ImmediateOperandConstraint;
  class ChoiceOperandConstraint;
  class RegisterOperandConstraint;
  class BntNode;
  class Data;
  class ChoicesFilter;
  class ConstraintSet;

  /*!
    \class Operand
    \brief Base class for operands.
  */
  class Operand : public Object {
  public:
    Object* Clone() const override;  //!< Return a cloned Operand object of the same type and same contents of the object.
    const std::string ToString() const override; //!< Return a string describing the current state of the Operand object.
    const char* Type() const override { return "Operand"; } //!< Return the type of the Operand object in C string.

    Operand(); //!< Default constructor.
    ~Operand(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(Operand);

    const std::string& Name() const; //!< Return Operand name.
    uint32 Size() const; //!< Return operand size.
    uint32 Value() const { return mValue; } //!< Return the selected operand field value.
    void SetValue(uint32 value) const { mValue = value; } //!< Set operand value, used by controlling party to set operand value directly.
    uint32 Mask() const; //!< Return mask for operand value.
    EOperandType OperandType() const; //!< Return operand type.
    virtual uint32 Encoding() const; //!< Return the operand encoding.
    virtual void Initialize(const OperandStructure* oprStructure); //!< Initialize operand object.
    virtual void Setup(Generator& gen, Instruction& instr); //!< Setup conditions, constraining mechanisms before generating operand.
    virtual void Generate(Generator& gen, Instruction& instr); //!< Generate operand details.
    virtual void Commit(Generator& gen, Instruction& instr); //!< Commit generated operand.
    virtual void CleanUp(); //!< Clean up resources that can be released.
    virtual const std::string AssemblyText() const { return ""; } //!< return operand assembly text.

    virtual const Operand* MatchOperand(const std::string& oprName) const //!< Return operand pointer if it matches the parameter.
    {
      if (Name() == oprName) return this;
      return nullptr;
    }

    virtual Operand* MatchOperandMutable(const std::string& oprName) //!< Return mutable operand pointer if it matches the parameter.
    {
      if (Name() == oprName) return this;
      return nullptr;
    }

    virtual const Operand* MatchOperandStartWith(const std::string& oprName) const //!< Return operand pointer if it matches the parameter by start with
    {
      if (Name().find(oprName) == 0) return this;
      return nullptr;
    }

    virtual bool GetPrePostAmbleRequests(Generator& gen) const { return false; } //!< Return necessary pre/post amble requests, if any.
    const OperandStructure* GetOperandStructure() const { return mpStructure; } //!< Return a const pointer to the operand structure object.
    OperandConstraint* GetOperandConstraint() const { return mpOperandConstraint; } //!< Return a pointer to the OperandConstraint object.

    virtual bool IsImmediateOperand() const { return false; }  //!< Indicate if it is an immediate operand.
    virtual bool IsRegisterOperand() const { return false; }   //!< indicate if it is a register operand.
    virtual bool IsConditional() const { return false; } //!< Indicate if it is a conditional operand.
  protected:
    Operand(const Operand& rOther); //!< Copy constructor.
    OperandConstraint* SetupOperandConstraint(const Generator& gen, const Instruction& instr) const; //!< Setup an OperandConstraint object for this type of Operand.
    virtual OperandConstraint* InstantiateOperandConstraint() const; //!< Return an instance of appropriate OperandConstraint object.
  protected:
    const OperandStructure* mpStructure; //!< Pointer to OperandStructure object that described the structure of the operand.
    mutable OperandConstraint* mpOperandConstraint; //!< Dynamic constraints for the operand generation.
    mutable uint32 mValue; //!< Un-shifted operand encoding value.
  };

  /*!
    \class ImmediateOperand
    \brief Class handling normal immediate operands.
  */
  class ImmediateOperand : public Operand {
  public:
    Object* Clone() const override  //!< Return a cloned ImmediateOperand object of the same type and same contents of the object.
    {
      return new ImmediateOperand(*this);
    }

    const char* Type() const override { return "ImmediateOperand"; } //!< Return the type of the ImmediateOperand object in C string.

    ImmediateOperand() : Operand() { } //!< Constructor.
    ~ImmediateOperand() { } //!< Destructor
    const std::string AssemblyText() const override; //!< return ImmediateOperand assembly text.
    virtual bool IsSigned() const { return false; } //!< Return whether the immeidate value is signed.
    virtual uint64 MaxValue() const { return (1ull << Size()) - 1;} //!< max value
    virtual uint64 MinValue() const {return 0ull;} //!< minimal value
    virtual uint64 BaseValue() const { return 0ull; } //!< Base value for the value range.

    bool IsImmediateOperand() const override { return true; }  //!< Indicate if it is an immediate operand

  protected:
    ImmediateOperand(const ImmediateOperand& rOther) //!< Copy constructor.
      : Operand(rOther)
    {
    }

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for ImmediateOperand.
  };

  /*!
    \class ImmediatePartialOperand
    \brief Class handling immediate operands that only allow partial of the whole value range, i.e. some values are exluded.
  */
  class ImmediatePartialOperand : public ImmediateOperand {
  public:
    Object* Clone() const override  //!< Return a cloned ImmediatePartialOperand object of the same type and same contents of the object.
    {
      return new ImmediatePartialOperand(*this);
    }

    const char* Type() const override { return "ImmediatePartialOperand"; } //!< Return the type of the ImmediatePartialOperand object in C string.

    ImmediatePartialOperand() : ImmediateOperand() { } //!< Constructor.
    ~ImmediatePartialOperand() { } //!< Destructor

    void Generate(Generator& gen, Instruction& instr) override; //!< Generate ImmediatePartialOperand details.
  protected:
    ImmediatePartialOperand(const ImmediatePartialOperand& rOther) //!< Copy constructor.
      : ImmediateOperand(rOther)
    {
    }

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for ImmediatePartialOperand.
  };

  /*!
    \class ImmediateExcludeOperand
    \brief Class handling immediate operands that has value exclusion.
  */
  class ImmediateExcludeOperand : public ImmediatePartialOperand {
  public:
    Object* Clone() const override  //!< Return a cloned ImmediateExcludeOperand object of the same type and same contents of the object.
    {
      return new ImmediateExcludeOperand(*this);
    }

    const char* Type() const override { return "ImmediateExcludeOperand"; } //!< Return the type of the ImmediateExcludeOperand object in C string.

    ImmediateExcludeOperand() : ImmediatePartialOperand() { } //!< Constructor.
    ~ImmediateExcludeOperand() { } //!< Destructor
  protected:
    ImmediateExcludeOperand(const ImmediateExcludeOperand& rOther) //!< Copy constructor.
      : ImmediatePartialOperand(rOther)
    {
    }

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for ImmediateExcludeOperand.
  };

   /*!
    \class ImmediateGe1Operand
    \brief Class handling immediate operands that has Ge1.
  */
  class ImmediateGe1Operand : public ImmediatePartialOperand {
  public:
    Object* Clone() const override  //!< Return a cloned ImmediateGe1Operand object of the same type and same contents of the object.
    {
      return new ImmediateGe1Operand(*this);
    }

    const char* Type() const override { return "ImmediateGe1Operand"; } //!< Return the type of the ImmediateGe1Operand object in C string.

    ImmediateGe1Operand() : ImmediatePartialOperand() { } //!< Constructor.
    ~ImmediateGe1Operand() { } //!< Destructor
  protected:
    ImmediateGe1Operand(const ImmediateGe1Operand& rOther) //!< Copy constructor.
      : ImmediatePartialOperand(rOther)
    {
    }

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for ImmediateGe1Operand.
  };

  class Choice;

  /*!
    \class ChoicesOperand
    \brief Class handling operands with weighted choices.
  */
  class ChoicesOperand : public Operand {
  public:
    Object* Clone() const override  //!< Return a cloned ChoicesOperand object of the same type and same contents of the object.
    {
      return new ChoicesOperand(*this);
    }

    const char* Type() const override { return "ChoicesOperand"; } //!< Return the type of the ChoicesOperand object in C string.

    ChoicesOperand() : Operand(), mChoiceText() { } //!< Constructor.
    ~ChoicesOperand() { } //!< Destructor

    void Generate(Generator& gen, Instruction& instr) override; //!< Generate ChoicesOperand details.
    const std::string AssemblyText() const override { return mChoiceText; } //!< return ChoicesOperand assembly text.
    const std::string ChoiceText() const { return mChoiceText; } //!< Return choosen options text.

    bool IsImmediateOperand() const override { return true; }  //!< Indicate if it is an immediate operand
    void GetAvailableChoices(std::vector<const Choice*>& rChoicesList) const; //!< Return available choices.
    virtual void SetChoiceResultDirect(Generator& gen, Instruction& instr, const std::string& choiceText); //!< Set choice result.
  protected:
    ChoicesOperand(const ChoicesOperand& rOther) //!< Copy constructor.
      : Operand(rOther), mChoiceText()
    {
    }

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for ChoicesOperand.
    void GenerateChoice(Generator& gen, Instruction& instr); //!< Generate a Choice.
    virtual ChoicesFilter * GetChoicesFilter(const ConstraintSet* pConstrSet) const; //!< Return choices filter
    virtual void SetChooseResultWithConstraint(Generator&gen, Instruction& instr, const Choice *pChoiceTree); //!< set choose result with constraint
    virtual void SetChoiceResult(Generator& gen, Instruction& instr, const Choice* choice);
  protected:
    std::string mChoiceText; //!< Text associated with the choice value.
  };

  /*!
    \class RegisterOperand
    \brief Class handling operands with weighted choices.
  */
  class RegisterOperand : public ChoicesOperand {
  public:
    Object* Clone() const override  //!< Return a cloned RegisterOperand object of the same type and same contents of the object.
    {
      return new RegisterOperand(*this);
    }

    const char* Type() const override { return "RegisterOperand"; } //!< Return the type of the RegisterOperand object in C string.

    void Commit(Generator& gen, Instruction& instr) override;

    RegisterOperand() : ChoicesOperand() { } //!< Constructor.
    ~RegisterOperand(); //!< Destructor

    bool IsImmediateOperand() const override { return false; }  //!< Indicate if it is an immediate operand
    bool IsRegisterOperand() const override { return true; }   //!< indicate if it is a register operand
    void AddWriteConstraint(Generator& gen) const; //!< Add write related constraint.
    virtual void GetRegisterIndices(uint32 regIndex, ConstraintSet& rRegIndices) const; //!< Return the register indices in a ConstraintSet, assuming the specified register is chosen.
    virtual void GetChosenRegisterIndices(const Generator& gen, ConstraintSet& rRegIndices) const; //!< Return the chosen register indices in a ConstraintSet.
    void SetChoiceResultDirect(Generator& gen, Instruction& instr, const std::string& choiceText) override; //!< Set choice result direct overrided.
    void SubConstraintValue(uint32 value) const;
  protected:
    explicit RegisterOperand(const ChoicesOperand& rOther) //!< Copy constructor.
      : ChoicesOperand(rOther)
    {
    }
    void SetChooseResultWithConstraint(Generator& gen, Instruction& instr, const Choice *pChoiceTree) override; //!< set choose result with constraint
    void SetChoiceResult(Generator& gen, Instruction& instr, const Choice* choice) override; //!< set Choice result
    void SaveResource(const Generator& gen, const Instruction& rInstr) const; //!< Save register resource.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for RegisterOperand.
    void SetUnpredict(const Generator&rGen, Instruction&rInstr) const; //!< save constraints as register is unpredictable
  };

  /*!
    \class MultiRegisterOperand
    \brief Class handling operands with weighted choices.
  */
  class MultiRegisterOperand : public virtual RegisterOperand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(MultiRegisterOperand);
    SUBCLASS_DESTRUCTOR_DEFAULT(MultiRegisterOperand);
    ASSIGNMENT_OPERATOR_ABSENT(MultiRegisterOperand);

    Object* Clone() const override = 0; //!< Return a cloned MultiRegisterOperand object of the same type and same contents of the object.
    const char* Type() const override { return "MultiRegisterOperand"; } //!< Return the type of the MultiRegisterOperand object in C string.

    void Commit(Generator& gen, Instruction& instr) override; //!< Commit generated MultiRegisterOperand.
    virtual uint32 NumberRegisters() const = 0; //!< Return number of registers.
    void GetExtraRegisterNames(uint32 regIndex, std::vector<std::string>& nameVec) const; //!< Return a list of the extra register names.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(MultiRegisterOperand);

    virtual const std::string GetNextRegisterName(uint32& indexVar) const = 0; //!< Return the name of the next register.
    ChoicesFilter * GetChoicesFilter(const ConstraintSet* pConstrSet) const override; //!< Return choices filter for MultiRegisterOperand.
  };

  /*!
    \class FpRegisterOperand
    \brief Class handling operands with weighted choices.
  */
  class FpRegisterOperand : public RegisterOperand {
  public:
    Object* Clone() const override  //!< Return a cloned FpRegisterOperand object of the same type and same contents of the object.
    {
      return new FpRegisterOperand(*this);
    }

    const char* Type() const override { return "FpRegisterOperand"; } //!< Return the type of the FpRegisterOperand object in C string.

    FpRegisterOperand() : RegisterOperand() { } //!< Constructor.
    ~FpRegisterOperand() { } //!< Destructor
  protected:
    explicit FpRegisterOperand(const RegisterOperand& rOther) //!< Copy constructor.
      : RegisterOperand(rOther)
    {
    }

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for FpRegisterOperand.
  };

  /*!
    \class VectorRegisterOperand
    \brief Class handling operands with weighted choices.
  */
  class VectorRegisterOperand : public virtual RegisterOperand {
  public:
    Object* Clone() const override  //!< Return a cloned VectorRegisterOperand object of the same type and same contents of the object.
    {
      return new VectorRegisterOperand(*this);
    }

    const char* Type() const override { return "VectorRegisterOperand"; } //!< Return the type of the VectorRegisterOperand object in C string.

    VectorRegisterOperand() : RegisterOperand() { } //!< Constructor.
    ~VectorRegisterOperand() { } //!< Destructor

    void Generate(Generator& gen, Instruction& instr) override; //!< Generate VectorRegisterOperand details.
  protected:
    explicit VectorRegisterOperand(const RegisterOperand& rOther) //!< Copy constructor.
      : RegisterOperand(rOther)
    {
    }

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for VectorRegisterOperand.
    virtual void SetupDataTraits(Generator& gen, Instruction& instr) { } //!< Setup VectorRegisterOperand data traits.
  };

  /*!
    \class SameValueOperand
    \brief Class handling the same value operands.
  */
  class SameValueOperand : public Operand {
  public:
    Object* Clone() const override  //!< Return a cloned SameValueOperand object of the same type and same contents of the object.
    {
      return new SameValueOperand(*this);
    }

    const char* Type() const override { return "SameValueOperand"; } //!< Return the type of the SameValueOperand object in C string.

    SameValueOperand() : Operand() { } //!< Constructor.
    ~SameValueOperand() { } //!< Destructor
    void Generate(Generator& gen, Instruction& instr) override; //!< Generate operand details.
  protected:
    SameValueOperand(const SameValueOperand& rOther) //!< Copy constructor.
      : Operand(rOther)
    {
    }

  };

/*!
    \class Minus1ValueOperand
    \brief Class handling Minus one value operands.
  */
  class Minus1ValueOperand : public Operand {
  public:
    Object* Clone() const override  //!< Return a cloned Minus1ValueOperand object of the same type and same contents of the object.
    {
      return new Minus1ValueOperand(*this);
    }

    const char* Type() const override { return "Minus1ValueOperand"; } //!< Return the type of the Minus1ValueOperand object in C string.

    Minus1ValueOperand() : Operand() { } //!< Constructor.
    ~Minus1ValueOperand() { } //!< Destructor
    void Generate(Generator& gen, Instruction& instr) override; //!< Generate operand details.
  protected:
    Minus1ValueOperand(const Minus1ValueOperand& rOther) //!< Copy constructor.
      : Operand(rOther)
    {
    }
  };

  /*!
    \class GroupOperand
    \brief Base class for various operands that have sub-operands.
  */
  class GroupOperand : public Operand {
  public:
    Object* Clone() const override  //!< Return a cloned GroupOperand object of the same type and same contents of the object.
    {
      return new GroupOperand(*this);
    }

    const char* Type() const override { return "GroupOperand"; } //!< Return the type of the GroupOperand object in C string.

    GroupOperand() : Operand(), mOperands() { } //!< Constructor.
    ~GroupOperand(); //!< Destructor

    uint32 Encoding() const override; //!< Return the GroupOperand encoding.
    void Initialize(const OperandStructure* oprStructure) override; //!< Initialize GroupOperand object.
    void Setup(Generator& gen, Instruction& instr) override; //!< Setup conditions, constraining mechanisms before generating GroupOperand.
    void Generate(Generator& gen, Instruction& instr) override; //!< Generate GroupOperand details.
    void Commit(Generator& gen, Instruction& instr) override; //!< Commit generated GroupOperand.
    void CleanUp() override; //!< Clean up resources that can be released.
    const std::string AssemblyText() const override { return Name(); } //!< return GroupOperand assembly text.
    const Operand* MatchOperand(const std::string& oprName) const override; //!< Return operand pointer if it matches the parameter.
    Operand* MatchOperandMutable(const std::string& oprName) override; //!< Return mutable operand pointer if it matches the parameter.
    Operand* GetSubOperand(const std::string& oprName); //!< Return sub operand by the specified name.
    std::vector<Operand*> GetSubOperands() const; //!< Return all sub operands.
  protected:
    GroupOperand(const GroupOperand& rOther); //!< Copy constructor.

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for GroupOperand.
  protected:
    std::vector<Operand* > mOperands; //!< Container holding pointer to all sub operands.
  };

  /*!
    \class VectorLayoutOperand
    \brief Class for vector layout operands.
  */
  class VectorLayoutOperand : public Operand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VectorLayoutOperand);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorLayoutOperand);
    ASSIGNMENT_OPERATOR_ABSENT(VectorLayoutOperand);

    void Setup(Generator& gen, Instruction& instr) override; //!< Setup conditions, constraining mechanisms before generating operand.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VectorLayoutOperand);
  private:
    virtual void SetupVectorLayout(const Generator& rGen, const Instruction& rInstr) = 0; //!< Determine and set the vector layout attributes.
  };

  class AddressSolver;
  class AddressingMode;

  /*!
    \class AddressingOperand
    \brief Base class of addressing-mode operands
  */
  class AddressingOperand : public GroupOperand {
  public:
    Object* Clone() const override  //!< Return a cloned AddressingOperand object of the same type and same contents of the object.
    {
      return new AddressingOperand(*this);
    }

    const char* Type() const override { return "AddressingOperand"; } //!< Return the type of the AddressingOperand object in C string.

    AddressingOperand() : GroupOperand(), mTargetAddress(0) { } //!< Constructor.
    ~AddressingOperand() { } //!< Destructor

    void Generate(Generator& gen, Instruction& instr) override; //!< Generate operand details.
    uint64 TargetAddress() const { return mTargetAddress; } //!< Return the target address of the addressingOperand.
  protected:
    AddressingOperand(const AddressingOperand& rOther) : GroupOperand(rOther), mTargetAddress(0) { } //!< Copy constructor.
    virtual bool BaseGenerate(Generator& gen, Instruction& instr, bool noRestrict=false); //!< Call base class generate to generate the basic structure of the operand.
    virtual void UpdateNoRestrictionTarget(const Instruction&); //!< Update target address when no-restriction is specified.
    virtual void GenerateWithPreamble(Generator& gen, Instruction& instr) { } //!< Generate with preamble.
    virtual bool GenerateNoPreamble(Generator& gen, Instruction& instr) { return false; } //!< Generate the AddressingOperand using no-preamble approach.
    virtual AddressingMode* GetAddressingMode(uint64 alignment=1) const { return nullptr; } //!< Return an AddressingMode instance.
    virtual AddressSolver* GetAddressSolver(AddressingMode* pAddrMode, uint64 alignment); //!< Return an AddressSolver instance.
    bool VerifyVirtualAddress(uint64 va, uint64 size, bool isInstr) const; //!< Verify if the virtual address range is usable.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for AddressingOperand.
    virtual uint64 GetAddressingAlignment(uint64 alignment, uint64 dataSize = 0) const { return alignment; } //!< Get addressing alignment.
  protected:
    uint64 mTargetAddress; //!< Recording the target virtual address,
  private:
    virtual void AdjustMemoryElementLayout(const Generator& rGen) { } //!< Finalize memory access dimensions based on runtime state.
    virtual bool MustGeneratePreamble(const Generator& rGen) const; //!< Return true if the operand is required to be generated using preamble.
  };

  /*!
    \class BranchOperand
    \brief Base class of branch operands
  */
  class BranchOperand : public AddressingOperand {
  public:
    Object* Clone() const override  //!< Return a cloned BranchOperand object of the same type and same contents of the object.
    {
      return new BranchOperand(*this);
    }

    const char* Type() const override { return "BranchOperand"; } //!< Return the type of the BranchOperand object in C string.

    BranchOperand() : AddressingOperand(), mEscapeTaken(false) { } //!< Constructor.
    ~BranchOperand() { } //!< Destructor

    uint64 BranchTarget() const { return mTargetAddress; } //!< Return the branch target address.
    virtual bool IsBranchTaken(const Instruction& instr) const { return true; } //!< Return true to indicate that this branch is taken.
    virtual BntNode* GetBntNode(const Instruction& instr) const; //!< Return branch information in a BntNode object.
    void Commit(Generator& gen, Instruction& instr) override; //!< Commit generated GroupOperand.
  protected:
    BranchOperand(const BranchOperand& rOther) : AddressingOperand(rOther), mEscapeTaken(rOther.mEscapeTaken) { } //!< Copy constructor.

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for BranchOperand.
    bool mEscapeTaken;
  };

  /*!
    \class RegisterBranchOperand
    \brief Base class of branch-to-register operands
  */
  class RegisterBranchOperand : public BranchOperand {
  public:
    Object* Clone() const override  //!< Return a cloned RegisterBranchOperand object of the same type and same contents of the object.
    {
      return new RegisterBranchOperand(*this);
    }

    const char* Type() const override { return "RegisterBranchOperand"; } //!< Return the type of the RegisterBranchOperand object in C string.

    RegisterBranchOperand() : BranchOperand() { } //!< Constructor.
    ~RegisterBranchOperand() { } //!< Destructor

    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary pre/post amble requests for the RegisterBranchOperand, if any.
  protected:
    RegisterBranchOperand(const RegisterBranchOperand& rOther) : BranchOperand(rOther) { } //!< Copy constructor.
    void GenerateWithPreamble(Generator& gen, Instruction& instr) override; //!< Generate the RegisterBranchOperand using preamble approach.
    bool GenerateNoPreamble(Generator& gen, Instruction& instr) override; //!< Generate the RegisterBranchOperand using no-preamble approach.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for RegisterBranchOperand.
    uint64 GetAddressingAlignment(uint64 alignment, uint64 dataSize = 0) const override; //!< Get register branch addressing alignment.
    AddressingMode* GetAddressingMode(uint64 alignment) const override; //!< Return suitable addressing mode object.
  private:
    bool MustGeneratePreamble(const Generator& rGen) const override; //!< Return true if the operand is required to be generated using preamble.
  };

  /*!
    \class PcRelativeBranchOperand
    \brief Base class of PC-relative-branch operands
  */
  class PcRelativeBranchOperand : public BranchOperand {
  public:
    Object* Clone() const override  //!< Return a cloned PcRelativeBranchOperand object of the same type and same contents of the object.
    {
      return new PcRelativeBranchOperand(*this);
    }

    const char* Type() const override { return "PcRelativeBranchOperand"; } //!< Return the type of the PcRelativeBranchOperand object in C string.

    PcRelativeBranchOperand() : BranchOperand() { } //!< Constructor.
    ~PcRelativeBranchOperand() { } //!< Destructor

    void Generate(Generator& gen, Instruction& instr) override; //!< Generate PcRelativeBranchOperand details.
  protected:
    PcRelativeBranchOperand(const PcRelativeBranchOperand& rOther) : BranchOperand(rOther) { } //!< Copy constructor.
    void UpdateNoRestrictionTarget(const Instruction& instr) override; //!< Update branch target when no-restriction is specified.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for PcRelativeBranchOperand.
  };

  /*!
    \class MultiVectorRegisterOperand
    \brief Class handling operands with weighted choices.
  */
  class MultiVectorRegisterOperand : public VectorRegisterOperand, public MultiRegisterOperand {
  public:
    Object* Clone() const override = 0; //!< Return a cloned MultiVectorRegisterOperand object of the same type and same contents of the object.
    const char* Type() const override { return "MultiVectorRegisterOperand"; } //!< Return the type of the MultiVectorRegisterOperand object in C string.

    MultiVectorRegisterOperand() : VectorRegisterOperand(), MultiRegisterOperand() { } //!< Constructor.
    ~MultiVectorRegisterOperand() { } //!< Destructor

    void Generate(Generator& gen, Instruction& instr) override; //!< Generate MultiVectorRegisterOperand details.

  protected:
    explicit MultiVectorRegisterOperand(const MultiVectorRegisterOperand& rOther) //!< Copy constructor.
      : VectorRegisterOperand(rOther), MultiRegisterOperand(rOther)
    {
    }
  };

  /*!
    \class ImpliedRegisterOperand
    \brief Class handling operands with weighted choices.
  */
  class ImpliedRegisterOperand : public RegisterOperand {
  public:
    Object* Clone() const override  //!< Return a cloned ImpliedRegisterOperand object of the same type and same contents of the object.
    {
      return new ImpliedRegisterOperand(*this);
    }

    const char* Type() const override { return "ImpliedRegisterOperand"; } //!< Return the type of the ImpliedRegisterOperand object in C string.

    ImpliedRegisterOperand() : RegisterOperand() { } //!< Constructor.
    ~ImpliedRegisterOperand() { } //!< Destructor

    void Generate(Generator& gen, Instruction& instr) override; //!< Generate ImpliedRegisterOperand details.
  protected:
    explicit ImpliedRegisterOperand(const RegisterOperand& rOther) //!< Copy constructor.
      : RegisterOperand(rOther)
    {
    }

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for ImpliedRegisterOperand.
  };

  /*!
    \class LoadStoreOperand
    \brief Base class of branch operands
  */
  class LoadStoreOperand : public AddressingOperand {
  public:
    Object* Clone() const override  //!< Return a cloned LoadStoreOperand object of the same type and same contents of the object.
    {
      return new LoadStoreOperand(*this);
    }

    const char* Type() const override { return "LoadStoreOperand"; } //!< Return the type of the LoadStoreOperand object in C string.

    LoadStoreOperand() : AddressingOperand() { } //!< Constructor.
    ~LoadStoreOperand() { } //!< Destructor
    void Commit(Generator& gen, Instruction& instr) override; //!< Commit generated MultiVectorRegisterOperand.
    virtual void GetDataTargetConstraint(ConstraintSet& dataConstr) const; //!< get data target constraint.
    uint64 GetAddressingAlignment(uint64 alignment, uint64 dataSize) const override; //!< Get load/store addressing alignment.
  protected:
    virtual uint64 GetUnalignment(uint64 dataSize) const { return 1;} //!< Get unalignment.
    LoadStoreOperand(const LoadStoreOperand& rOther) : AddressingOperand(rOther) { } //!< Copy constructor.

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for LoadStoreOperand.
  private:
    virtual void GetTargetAddresses(const Instruction& rInstr, cuint64 baseTargetAddr, std::vector<uint64>& rTargetAddresses) const; //!< Return a list of target addresses the instruction will access.
    bool IsTargetSharedRead(const EMemAccessType memAccessType, cuint64 targetAddr, cuint64 dataSize) const; //!< Returns true if read access is specified and the target address range intersects shared memory.
  };

  /*!
    \class BaseOffsetLoadStoreOperand
    \brief Base class of base-offset load-store operands
  */
  class BaseOffsetLoadStoreOperand : public LoadStoreOperand {
  public:
    Object* Clone() const override  //!< Return a cloned BaseOffsetLoadStoreOperand object of the same type and same contents of the object.
    {
      return new BaseOffsetLoadStoreOperand(*this);
    }

    const char* Type() const override { return "BaseOffsetLoadStoreOperand"; } //!< Return the type of the BaseOffsetLoadStoreOperand object in C string.

    BaseOffsetLoadStoreOperand() : LoadStoreOperand() { } //!< Constructor.
    ~BaseOffsetLoadStoreOperand() { } //!< Destructor

    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary pre/post amble requests for the BaseOffsetLoadStoreOperand, if any.
    AddressingMode* GetAddressingMode(uint64 alignment=1) const override; //!< Return an AddressingMode instance for the BaseOffsetLoadStoreOperand.
  protected:
    BaseOffsetLoadStoreOperand(const BaseOffsetLoadStoreOperand& rOther) : LoadStoreOperand(rOther) { } //!< Copy constructor.
    void GenerateWithPreamble(Generator& gen, Instruction& instr) override; //!< Generate the BaseOffsetLoadStoreOperand using preamble approach.
    bool GenerateNoPreamble(Generator& gen, Instruction& instr) override; //!< Generate the BaseOffsetLoadStoreOperand using no-preamble approach.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for BaseOffsetLoadStoreOperand.
  };

  /*!
    \class SignedImmediateOperand
    \brief Class handling immediate operands that only allow partial of the whole value range, i.e. some values are exluded.
  */
  class SignedImmediateOperand : public ImmediateOperand {
  public:
    Object* Clone() const override  //!< Return a cloned SignedImmediateOperand object of the same type and same contents of the object.
    {
      return new SignedImmediateOperand(*this);
    }

    const char* Type() const override { return "SignedImmediateOperand"; } //!< Return the type of the SignedImmediateOperand object in C string.

    SignedImmediateOperand() : ImmediateOperand() { } //!< Constructor.
    ~SignedImmediateOperand() { } //!< Destructor
    const std::string AssemblyText() const override; //!< return SignedImmediateOperand assembly text.
    bool IsSigned() const override { return true; } //!< Return true to indicate the immeidate value is signed.
    void SetValue(uint64 val); //!<  set value
    uint64 MaxValue() const override { return (1ull << (Size() - 1)) - 1;}  //!< max value
    uint64 MinValue() const override {return sign_extend64(1ull << (Size() - 1), Size());} //!< minimal value
    uint64 BaseValue() const override { return (1ull << (Size() - 1)); } //!< Base value for the value range.
  protected:
    SignedImmediateOperand(const SignedImmediateOperand& rOther) //!< Copy constructor.
      : ImmediateOperand(rOther)
    {
    }
  };

 /*!
    \class BaseIndexLoadStoreOperand
    \brief Base class of base-index load-store operands
  */
  class BaseIndexLoadStoreOperand : public LoadStoreOperand {
  public:
    Object* Clone() const override  //!< Return a cloned BaseIndexLoadStoreOperand object of the same type and same contents of the object.
    {
      return new BaseIndexLoadStoreOperand(*this);
    }

    const char* Type() const override { return "BaseIndexLoadStoreOperand"; } //!< Return the type of the BaseIndexLoadStoreOperand object in C string.

    BaseIndexLoadStoreOperand() : LoadStoreOperand() { } //!< Constructor.
    ~BaseIndexLoadStoreOperand() { } //!< Destructor

    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary pre/post amble requests for the BaseIndexLoadStoreOperand, if any.
    AddressingMode* GetAddressingMode(uint64 alignment=1) const override; //!< Return an AddressingMode instance for the BaseIndexLoadStoreOperand.
  protected:
    BaseIndexLoadStoreOperand(const BaseIndexLoadStoreOperand& rOther) : LoadStoreOperand(rOther) { } //!< Copy constructor.
    void GenerateWithPreamble(Generator& gen, Instruction& instr) override; //!< Generate base index addressing mode with preamble.
    bool GenerateNoPreamble(Generator& gen, Instruction& instr) override; //!< Generate base index addressing mode with no preamble.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for BaseIndexLoadStoreOperand.
  private:
    bool MustGeneratePreamble(const Generator& rGen) const override; //!< Return true if the operand is required to be generated using preamble.
  };

  /*!
    \class PcOffsetLoadStoreOperand
    \brief Base class of PC Offset (label) load store operands
  */
  class PcOffsetLoadStoreOperand : public LoadStoreOperand {
  public:
    Object* Clone() const override  //!< Return a cloned PcOffsetLoadStoreOperand object of the same type and same contents of the object.
    {
      return new PcOffsetLoadStoreOperand(*this);
    }

    const char* Type() const override { return "PcOffsetLoadStoreOperand"; } //!< Return the type of the PcOffsetLoadStoreOperand object in C string.

    PcOffsetLoadStoreOperand() : LoadStoreOperand() { } //!< Constructor.
    ~PcOffsetLoadStoreOperand() { } //!< Destructor

    void Generate(Generator& gen, Instruction& instr) override; //!< Generate PcOffsetLoadStoreOperand details.
   protected:
    PcOffsetLoadStoreOperand(const PcOffsetLoadStoreOperand& rOther) : LoadStoreOperand(rOther) { } //!< Copy constructor.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for PcOffsetLoadStoreOperand.
  };

  class LoadStoreOperandStructure;

  /*!
    \class VectorStridedLoadStoreOperand
    \brief Operand for vector strided load/store operations.
  */
  class VectorStridedLoadStoreOperand : public LoadStoreOperand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VectorStridedLoadStoreOperand);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorStridedLoadStoreOperand);
    ASSIGNMENT_OPERATOR_ABSENT(VectorStridedLoadStoreOperand);

    Object* Clone() const override { return new VectorStridedLoadStoreOperand(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VectorStridedLoadStoreOperand"; } //!< Return a string describing the actual type of the Object.
    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary pre/post amble requests, if any.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VectorStridedLoadStoreOperand);

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object.
    void GenerateWithPreamble(Generator& gen, Instruction& instr) override; //!< Generate with preamble.
    bool GenerateNoPreamble(Generator& gen, Instruction& instr) override; //!< Generate the AddressingOperand using no-preamble approach.
    AddressingMode* GetAddressingMode(uint64 alignment=1) const override; //!< Return an AddressingMode instance.
  private:
    void GetTargetAddresses(const Instruction& rInstr, cuint64 baseTargetAddr, std::vector<uint64>& rTargetAddresses) const override; //!< Return a list of target addresses the instruction will access.
    void DifferStrideOperand(Generator& rGen, Instruction& rInstr); //!< Ensure the stride operand uses a different register from the base operand.
    uint64 CalculateStrideValue(const Instruction& rInstr, cuint32 alignment, cuint32 addrRangeSize) const; //!< Calculate the value of the stride operand.
    uint64 CalculateBaseValue(cuint64 baseAddr, cuint32 alignment, cuint32 addrRangeSize, cuint64 strideVal) const; //!< Calculate the value of the base operand.
  };

  /*!
    \class VectorIndexedLoadStoreOperand
    \brief Operand for vector indexed load/store operations.
  */
  class VectorIndexedLoadStoreOperand : public LoadStoreOperand {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(VectorIndexedLoadStoreOperand);
    SUBCLASS_DESTRUCTOR_DEFAULT(VectorIndexedLoadStoreOperand);
    ASSIGNMENT_OPERATOR_ABSENT(VectorIndexedLoadStoreOperand);

    bool GetPrePostAmbleRequests(Generator& gen) const override; //!< Return necessary pre/post amble requests, if any.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VectorIndexedLoadStoreOperand);

    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object.
    void GenerateWithPreamble(Generator& gen, Instruction& instr) override; //!< Generate with preamble.
    bool GenerateNoPreamble(Generator& gen, Instruction& instr) override; //!< Generate the AddressingOperand using no-preamble approach.
    AddressingMode* GetAddressingMode(uint64 alignment=1) const override; //!< Return an AddressingMode instance.
  private:
    void GetTargetAddresses(const Instruction& rInstr, cuint64 baseTargetAddr, std::vector<uint64>& rTargetAddresses) const override; //!< Return a list of target addresses the instruction will access.
    virtual void GetIndexRegisterNames(std::vector<std::string>& rIndexRegNames) const = 0; //!< Get the names of the index registers.
    uint64 AllocateIndexOperandDataBlock(Generator& rGen, cuint32 relativeRegIndex) const; //!< Allocate and initialize a block of memory to use for preamble loading of an index operand register.
    void RecordIndexElementByteSize(const Instruction& rInstr); //!< Compute and capture the index vector element size in bytes.
    uint64 CalculateBaseAndFirstIndexValues(const Instruction& rInstr, cuint32 alignment, std::vector<uint64>& rIndexElemValues) const; //!< Calculate the values of the base operand and first index operand element.
    void CalculateIndexValues(const Instruction& rInstr, cuint32 alignment, cuint64 baseVal, std::vector<uint64>& rIndexElemValues) const; //!< Calculate the values of the index operand elements after the first one.
  };

  /*!
    \class AluImmediateOperand
    \brief Base class for ALU immediate operand
  */
  class AluImmediateOperand : public AddressingOperand {
  public:
    Object* Clone() const override  //!< Return a cloned AluImmediateOperand object of the same type and same contents of the object.
    {
      return new AluImmediateOperand(*this);
    }

    const char* Type() const override { return "AluImmediateOperand"; } //!< Return the type of AluImmediateOperand object in C string.

    AluImmediateOperand() : AddressingOperand() { } //!< Constructor.
    ~AluImmediateOperand() { } //!< Destructor
    void Generate(Generator& gen, Instruction& instr) override; //!< Generate  details.
    AddressingMode* GetAddressingMode(uint64 alignment) const override; //!< Return an AddressingMode instance for the AluImmediateOperand.
  protected:
    AluImmediateOperand(const AluImmediateOperand& rOther) : AddressingOperand(rOther) { } //!< Copy constructor.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for AluImmediateOperand.
  };

  /*!
    \class DataProcessingOperand
    \brief Base class for data processing operand
  */
  class DataProcessingOperand : public AddressingOperand {
  public:
    Object* Clone() const override  //!< Return a cloned DataProcessingOperand object of the same type and same contents of the object.
    {
      return new DataProcessingOperand(*this);
    }

    const char* Type() const override { return "DataProcessingOperand"; } //!< Return the type of DataProcessingOperand object in C string.

    DataProcessingOperand() : AddressingOperand() { } //!< Constructor.
    SUBCLASS_DESTRUCTOR_DEFAULT(DataProcessingOperand); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(DataProcessingOperand);
    void Generate(Generator& gen, Instruction& instr) override; //!< Generate details.
    AddressingMode* GetAddressingMode(uint64 alignment) const override; //!< Return an AddressingMode instance for the DataProcessingOperand.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(DataProcessingOperand); //!< Copy constructor.
    OperandConstraint* InstantiateOperandConstraint() const override; //!< Return an instance of appropriate OperandConstraint object for DataProcessingOperand.
  };

}

#endif
