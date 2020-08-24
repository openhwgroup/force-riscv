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
#include <Operand.h>
#include <InstructionStructure.h>
#include <Random.h>
#include <OperandConstraint.h>
#include <Choices.h>
#include <ChoicesModerator.h>
#include <GenException.h>
#include <Constraint.h>
#include <Instruction.h>
#include <Register.h>
#include <Generator.h>
#include <ObjectRegistry.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <VaGenerator.h>
#include <GenRequest.h>
#include <InstructionConstraint.h>
#include <ChoicesFilter.h>
#include <MemoryInitData.h>
#include <AddressTagging.h>
#include <BntNode.h>
#include <OperandDataRequest.h>
#include <RegisterInitData.h>
#include <AddressSolver.h>
#include <ResourceDependence.h>
#include <VmUtils.h>
#include <GenPC.h>
#include <MemoryManager.h>
#include <BaseOffsetConstraint.h>
#include <OperandRequest.h>
#include <Log.h>
#include <VirtualMemoryInitializer.h>
#include <VectorLayout.h>
#include <DataBlock.h>
#include <Config.h>

#include <memory>
// C++UP accumulate defined in numeric
#include <numeric>

/*!
  \file Operand.cc
  \brief Code supporting operand generation
*/

using namespace std;

namespace Force {

  Operand::Operand()
    : Object(), mpStructure(nullptr), mpOperandConstraint(nullptr), mValue(0)
  {

  }

  Operand::~Operand()
  {
    if (nullptr != mpOperandConstraint) {
      LOG(fail) << "Expect operand constraint to be delete by now." << endl;
      FAIL("dangling-operand-constraint-pointer");
    }
  }

  Operand::Operand(const Operand& rOther)
    : Object(rOther), mpStructure(rOther.mpStructure), mpOperandConstraint(nullptr), mValue(rOther.mValue)
  {

  }

  Object* Operand::Clone() const
  {
    return new Operand(*this);
  }

  const string  Operand::ToString() const
  {
    return "Operand : " + mpStructure->Name();
  }

  const string& Operand::Name() const
  {
    return mpStructure->Name();
  }

  uint32 Operand::Size() const
  {
    return mpStructure->mSize;
  }

  uint32 Operand::Mask() const
  {
    return mpStructure->mMask;
  }

  EOperandType Operand::OperandType() const
  {
    return mpStructure->Type();
  }

  void Operand::Initialize(const OperandStructure* oprStructure)
  {
    mpStructure = oprStructure;
  }

  void Operand::Setup(Generator& gen, Instruction& instr)
  {
    if (nullptr != mpOperandConstraint) {
      LOG(fail) << "{Operand::Setup} expecting mpOperandConstraint to be nullptr at this point." << endl;
      FAIL("operand-constraint-not-null");
    }
    mpOperandConstraint = SetupOperandConstraint(gen, instr);
    mValue = 0;
  }

  OperandConstraint* Operand::SetupOperandConstraint(const Generator& gen, const Instruction& instr) const
  {
    auto new_constraint = InstantiateOperandConstraint();
    auto instr_req = instr.GetInstructionConstraint()->InstructionRequest();
    auto opr_req = instr_req->FindOperandRequest(Name());
    if (nullptr != opr_req) {
      if (this->mpStructure->IsSlave()) {
        LOG(notice) << "{Operand::SetupOperandConstraint} Operand: " << Name() << " is slave, " << opr_req->ToString() << " is ignored" << endl;
        opr_req->SetIgnored();
      }
      else{
        new_constraint->ApplyUserRequest(*opr_req);
      }
    }
    new_constraint->Setup(gen, instr, *(this->mpStructure));
    return new_constraint;
  }

  OperandConstraint* Operand::InstantiateOperandConstraint() const
  {
    return new OperandConstraint();
  }

  void  Operand::Generate(Generator& gen, Instruction& instr)
  {
    if (mpOperandConstraint->HasConstraint()) {
      ConstraintSet opr_constr(0, mpStructure->mMask);
      opr_constr.ApplyConstraintSet(*mpOperandConstraint->GetConstraint());
      if (not opr_constr.IsEmpty()) {
        mValue = (uint32)(opr_constr.ChooseValue());
        LOG(notice) << "{OperandGenerate} Operand : " << Name() << " Constraint: " << opr_constr.ToSimpleString() << " value 0x" << mValue << endl;
        return;
      } else {
        LOG(warn) << "{Operand::Generate} operand: " << Name() << ", invalid override constraint: " << opr_constr.ToSimpleString() << endl;
      }
    }

    mValue = Random::Instance()->Random32() & mpStructure->mMask;
    LOG(notice) << "Operand : " << Name() << " mask 0x" << hex << mpStructure->mMask << " value 0x" << mValue << endl;
  }

  uint32 Operand::Encoding() const
  {
    return mpStructure->Encoding(mValue);
  }

  void  Operand::Commit(Generator& gen, Instruction& instr)
  {

  }

  void  Operand::CleanUp()
  {
    delete mpOperandConstraint;
    mpOperandConstraint = nullptr;
  }

  OperandConstraint* ImmediateOperand::InstantiateOperandConstraint() const
  {
    return new ImmediateOperandConstraint();
  }

  const string ImmediateOperand::AssemblyText() const
  {
    char print_buffer[32];
    snprintf(print_buffer, 32, "0x%x", mValue);
    return print_buffer;
  }

  void ImmediatePartialOperand::Generate(Generator& gen, Instruction& instr)
  {
    // TODO investigate why test fail when using base class Generate method.
    if (mpOperandConstraint->HasConstraint()) {
      auto opr_constr = mpOperandConstraint->GetConstraint();
      mValue = (uint32)(opr_constr->ChooseValue());
    } else {
      LOG(fail) << "{ImmediatePartialOperand::Generate} expecting constrainted operand generation." << endl;
      FAIL("expecting-constraint");
    }
  }

  OperandConstraint* ImmediatePartialOperand::InstantiateOperandConstraint() const
  {
    return new ImmediatePartialOperandConstraint();
  }

  OperandConstraint* ImmediateGe1Operand::InstantiateOperandConstraint() const
  {
    return new ImmediateGe1OperandConstraint();
  }

  OperandConstraint* ImmediateExcludeOperand::InstantiateOperandConstraint() const
  {
    return new ImmediateExcludeOperandConstraint();
  }

  OperandConstraint* ChoicesOperand::InstantiateOperandConstraint() const
  {
    return new ChoicesOperandConstraint();
  }

  ChoicesFilter * ChoicesOperand::GetChoicesFilter(const ConstraintSet* pConstrSet) const
  {
    return new ConstraintChoicesFilter(pConstrSet);
  }

  void ChoicesOperand::GenerateChoice(Generator& gen, Instruction& instr)
  {
    auto choices_constraint = mpOperandConstraint->CastInstance<ChoicesOperandConstraint>();
    auto choices_tree = choices_constraint->GetChoiceTree();

    if (choices_constraint->HasConstraint()) {
      auto constr_set = choices_constraint->GetConstraint();
      if (choices_constraint->ConstraintForced()) {
        uint64 single_choice = constr_set->OnlyValue();
        SetChoiceResult(gen, instr, choices_tree->FindChoiceByValue(uint32(single_choice)));
      }
      else {
        auto choices_filter_raw = GetChoicesFilter(constr_set);
        std::unique_ptr<ChoicesFilter> choices_filter(choices_filter_raw);
        auto clone_raw = dynamic_cast<ChoiceTree*>(choices_tree->Clone());
        std::unique_ptr<ChoiceTree> clone_choices_tree(clone_raw);
        clone_choices_tree->ApplyFilter(*choices_filter);
        SetChooseResultWithConstraint(gen, instr, clone_choices_tree.get());
      }
    } else {
      SetChooseResultWithConstraint(gen, instr, choices_tree);
    }
  }

  void ChoicesOperand::SetChooseResultWithConstraint(Generator& gen, Instruction& instr, const Choice *pChoiceTree)
  {
    auto choice = pChoiceTree->Choose();
    SetChoiceResult(gen, instr, choice);
  }

  void ChoicesOperand::SetChoiceResult(Generator& gen, Instruction& instr, const Choice* choice)
  {
    // << "ChoicesOperand::SetChoiceResult value=" << choice->Value() << " text=" << choice->Name() << endl;
    mValue = choice->Value();
    mChoiceText = choice->Name();
  }

  void ChoicesOperand::SetChoiceResultDirect(Generator& gen, Instruction& instr, const string& choiceText)
  {
    mChoiceText = choiceText;
    const RegisterFile* reg_file = gen.GetRegisterFile();
    Register* reg = reg_file->RegisterLookup(choiceText);
    mValue = reg->IndexValue();
  }

  void ChoicesOperand::Generate(Generator& gen, Instruction& instr)
  {
    try {
      GenerateChoice(gen, instr);
    }
    catch (const ChoicesError& choices_error) {
      stringstream err_stream;
      err_stream << "Operand \"" << Name() << "\" failed to generate; " << choices_error.what();
      throw OperandError(err_stream.str());
    }
    catch (const ConstraintError& constraint_error) {
      stringstream err_stream;
      err_stream << "Operand \"" << Name() << "\" failed to generate; " << constraint_error.what();
      throw OperandError(err_stream.str());
    }
    catch (...) {
      stringstream err_stream;
      err_stream << "Operand \"" << Name() << "\" failed to generate; unknown reason";
      throw OperandError(err_stream.str());
    }

    //LOG(notice) << "ChoicesOperand : " << Name() << hex << " value 0x" << mValue << " text " << mChoiceText << endl;
  }

  void ChoicesOperand::GetAvailableChoices(vector<const Choice*>& rChoicesList) const
  {
    auto choices_constraint = mpOperandConstraint->CastInstance<ChoicesOperandConstraint>();
    auto choices_tree = choices_constraint->GetChoiceTree();

    if (choices_constraint->HasConstraint()) {
      auto constr_set = choices_constraint->GetConstraint();
      if (choices_constraint->ConstraintForced()) {
        uint64 single_choice = constr_set->OnlyValue();
        rChoicesList.push_back(choices_tree->FindChoiceByValue(uint32(single_choice)));
      }
      else {
        auto choices_filter = GetChoicesFilter(constr_set);
        std::unique_ptr<ChoicesFilter> choices_storage(choices_filter);
        choices_tree->GetAvailableChoices(*choices_filter, rChoicesList);
      }
    } else {
      choices_tree->GetAvailableChoices(rChoicesList);
    }
  }

  OperandConstraint* RegisterOperand::InstantiateOperandConstraint() const
  {
    return new RegisterOperandConstraint();
  }

  void RegisterOperand::SetChooseResultWithConstraint(Generator& gen, Instruction& instr, const Choice *pChoicesTree)
  {
    const ConstraintSet *dep_constr = nullptr;
    EResourceType res_type = EResourceType(0);
    if (gen.OperandTypeToResourceType(mpStructure->mType, res_type)) {
      dep_constr = gen.GetDependenceInstance()->GetDependenceConstraint(mpStructure->mAccess, res_type, instr.GetInstructionConstraint()->GetHotResource());
    }

    if (nullptr != dep_constr && not dep_constr->IsEmpty()) {
      LOG(info) << "Operand " << Name() << " Filtering dependency constraint : \"" << dep_constr->ToSimpleString() << "\"" << endl;
      std::unique_ptr<ChoicesFilter> choices_filter(GetChoicesFilter(dep_constr));
      std::unique_ptr<ChoiceTree> clone_choices_tree(dynamic_cast<ChoiceTree*>(pChoicesTree->Clone()));
      clone_choices_tree->ApplyFilter(*choices_filter);
      try {
        auto choice = clone_choices_tree->Choose();
        SetChoiceResult(gen, instr, choice);
      }
      catch (const ChoicesError& choices_error) {
        LOG(info) <<  "Operand " << Name() << " Filtering dependency constraint, reverted" << endl;
        auto choice = pChoicesTree->Choose();
        SetChoiceResult(gen, instr, choice);
      }
    }
    else {
      LOG(info)  << "Operand " << Name() << " No Dependency constraint" << endl;
      auto choice = pChoicesTree->Choose();
      SetChoiceResult(gen, instr, choice);
    }

  }

  void RegisterOperand::SaveResource(const Generator& gen, const Instruction& rInstr) const
  {
    auto hot_resource = rInstr.GetInstructionConstraint()->GetHotResource();
    EResourceType res_type = EResourceType(0);
    if (gen.OperandTypeToResourceType(mpStructure->mType, res_type)) {
      ConstraintSet* res_constr = new ConstraintSet();
      GetChosenRegisterIndices(gen, *res_constr);
      hot_resource->RecordAccess(mpStructure->mAccess, res_type, res_constr);
    }
  }

  void RegisterOperand::SetChoiceResult(Generator& gen, Instruction& instr, const Choice* choice)
  {
    ChoicesOperand::SetChoiceResult(gen, instr, choice);
    SetUnpredict(gen, instr);
    SaveResource(gen, instr);
  }

  void RegisterOperand::SetChoiceResultDirect(Generator& gen, Instruction& instr, const std::string& choiceText)
  {
    ChoicesOperand::SetChoiceResultDirect(gen, instr, choiceText);
    SetUnpredict(gen, instr);
    SaveResource(gen, instr);
  }

  void RegisterOperand::SetUnpredict(const Generator&rGen, Instruction&rInstr) const
  {
    auto reg = rGen.GetRegisterFile()->RegisterLookup(mChoiceText);
    bool unpredict = (reg->HasAttribute(ERegAttrType::Unpredictable) or rGen.IsRegisterReserved(reg->Name(), ERegAttrType::Read, ERegReserveType::Unpredictable));
    bool has_read_access = GetOperandStructure()->HasReadAccess();

    if (unpredict and has_read_access) {
      rInstr.SetUnpredictable(true);
    }

  }
  void RegisterOperand::SubConstraintValue(uint32 value) const
  {
    auto register_constraint = mpOperandConstraint->CastInstance<RegisterOperandConstraint>();
    register_constraint->SubConstraintValue(value,*mpStructure);
  }

  static inline void clear_register_value(Register* reg_ptr, bool clear_it)
  {
    if (clear_it) {
      reg_ptr->ClearAttribute(ERegAttrType::HasValue);
    }
  }

  void RegisterOperand::Commit(Generator& gen, Instruction& instr)
  {
    ChoicesOperand::Commit(gen, instr);

    // << "RegisterOperand::Commit " << mChoiceText << endl;
    const RegisterFile* reg_file = gen.GetRegisterFile();
    Register* reg_ptr = reg_file->RegisterLookup(mChoiceText);
    bool clear_it = (not gen.HasISS()) and mpStructure->HasWriteAccess();

    vector<string> reg_names;
    reg_ptr->GetRelyUponRegisters(reg_names);

    if (!reg_names.empty())
    {
      for (const auto & name_ref : reg_names)
      {
        // << "RegisterOperand::Commit rely upon reg name=" << name_ref << endl;
        Register* rely_upon_reg_ptr = reg_file->RegisterLookup(name_ref);
        if (not rely_upon_reg_ptr->IsInitialized())
        {
          RegisterInitData rely_init_data;
          rely_init_data.Setup(gen, rely_upon_reg_ptr, nullptr);
          rely_init_data.Commit(gen, rely_upon_reg_ptr);
          rely_init_data.Cleanup();

          clear_register_value(rely_upon_reg_ptr, clear_it);
        }
      }
    }

    // << "RegisterOperand::Commit reg_ptr name=" << reg_ptr->Name() << reg_ptr->Type() << endl;
    if (reg_ptr->IsInitialized()) {
      //<< "Register \"" << reg_ptr->Name() << "\" was initialized, ignore" << endl;
      return clear_register_value(reg_ptr, clear_it);
    }

    if (gen.DelayInit()) {
      return clear_register_value(reg_ptr, clear_it);
    }

    RegisterInitData init_data;
    auto register_constraint = mpOperandConstraint->CastInstance<RegisterOperandConstraint>();
    auto data_req = register_constraint->GetOperandDataRequest(instr, Name());
    init_data.Setup(gen, reg_ptr, data_req);
    init_data.Commit(gen, reg_ptr);
    init_data.Cleanup();

    return clear_register_value(reg_ptr, clear_it);
  }

  void RegisterOperand::AddWriteConstraint(Generator& gen) const
  {
    auto reg_constr = mpOperandConstraint->CastInstance<RegisterOperandConstraint>();
    reg_constr->AddWriteConstraint(gen, *mpStructure);
  }

  void RegisterOperand::GetRegisterIndices(uint32 regIndex, ConstraintSet& rRegIndices) const
  {
    rRegIndices.AddValue(regIndex);
  }

  void RegisterOperand::GetChosenRegisterIndices(const Generator& gen, ConstraintSet& rRegIndices) const
  {
    const RegisterFile* reg_file = gen.GetRegisterFile();
    Register* reg = reg_file->RegisterLookup(mChoiceText);
    rRegIndices.AddValue(reg->IndexValue());
  }

  RegisterOperand:: ~RegisterOperand()
  {

  }

  OperandConstraint* FpRegisterOperand::InstantiateOperandConstraint() const
  {
    return new FpRegisterOperandConstraint();
  }

  void VectorRegisterOperand::Generate(Generator& gen, Instruction& instr)
  {
    SetupDataTraits(gen, instr);
    RegisterOperand::Generate(gen, instr);
  }

  OperandConstraint* VectorRegisterOperand::InstantiateOperandConstraint() const
  {
    return new VectorRegisterOperandConstraint();
  }

  void SameValueOperand::Generate(Generator& gen, Instruction& instr)
  {
    const string& sameName = Name();
    size_t pos = sameName.find("Same");
    if (pos == string::npos)
      FAIL("In-correct operand name");
    string originalName = sameName.substr(0, pos);
    auto original_opr = instr.FindOperand(originalName, true);
    mValue = original_opr->Value();
  }

  void Minus1ValueOperand::Generate(Generator& gen, Instruction& instr)
  {
    const string& MinusName = Name();
    size_t pos = MinusName.find("Minus1");
    if (pos == string::npos)
      FAIL("In-correct operand name");
    string originalName = MinusName.substr(0, pos);
    auto original_opr = instr.FindOperand(originalName, true);
    if (original_opr->Value() < 1)
      FAIL("in-correct operand value to minus 1");
    mValue = original_opr->Value() - 1;
  }

  GroupOperand::GroupOperand(const GroupOperand& rOther)
    : Operand(rOther), mOperands()
  {
    transform(rOther.mOperands.cbegin(), rOther.mOperands.cend(), back_inserter(mOperands),
      [](const Operand* pOpr) { return dynamic_cast<Operand*>(pOpr->Clone()); });
  }

  GroupOperand::~GroupOperand()
  {
    for (auto opr_ptr : mOperands) {
      delete opr_ptr;
    }
  }

  const Operand* GroupOperand::MatchOperand(const std::string& oprName) const
  {
    if (Name() == oprName) return this;

    for (auto opr_ptr : mOperands) {
      auto match_opr = opr_ptr->MatchOperand(oprName);
      if (nullptr != match_opr) return match_opr;
    }

    return nullptr;
  }

  Operand* GroupOperand::MatchOperandMutable(const std::string& oprName)
  {
    if (Name() == oprName) return this;

    for (auto opr_ptr : mOperands) {
      auto match_opr = opr_ptr->MatchOperandMutable(oprName);
      if (nullptr != match_opr) return match_opr;
    }

    return nullptr;
  }

  Operand* GroupOperand::GetSubOperand(const std::string& oprName)
  {
    Operand* opr = nullptr;

    auto itr = find_if(mOperands.cbegin(), mOperands.cend(),
      [&oprName](const Operand* pOpr) { return (pOpr->Name() == oprName); });

    if (itr != mOperands.end()) {
      opr = *itr;
    }
    else {
      LOG(fail) << "{GroupOperand::GetSubOperand} sub operand: " << oprName << " not found." << endl;
      FAIL("sub-operand-not-found");
    }

    return opr;
  }

  vector<Operand*> GroupOperand::GetSubOperands() const
  {
    return vector<Operand*>(mOperands);
  }

  uint32 GroupOperand::Encoding() const
  {
    uint32 grp_encoding = accumulate(mOperands.cbegin(), mOperands.cend(), uint32(0),
      [](cuint32 partialEncoding, const Operand* pOpr) { return (partialEncoding | pOpr->Encoding()); });

    //LOG(notice) << "GroupOperand::Encoding} " << Name() << "=0x" << hex << grp_encoding << endl;

    return grp_encoding;
  }

  void GroupOperand::Initialize(const OperandStructure* oprStructure)
  {
    Operand::Initialize(oprStructure);

    auto grp_struct = oprStructure->CastOperandStructure<GroupOperandStructure>();

    const vector<OperandStructure* >& opr_vec = grp_struct->mOperandStructures;

    ObjectRegistry* obj_registry = ObjectRegistry::Instance();
    for (auto opr_struct_ptr : opr_vec) {
      Operand* opr = obj_registry->TypeInstance<Operand>(opr_struct_ptr->mClass);
      opr->Initialize(opr_struct_ptr);
      mOperands.push_back(opr);
    }
  }

  void GroupOperand::Setup(Generator& gen, Instruction& instr)
  {
    Operand::Setup(gen, instr);

    for (auto opr_ptr : mOperands) {
      opr_ptr->Setup(gen, instr);
    }
  }

  void GroupOperand::Generate(Generator& gen, Instruction& instr)
  {
    Operand::Generate(gen, instr);

    for (auto opr_ptr : mOperands) {
      opr_ptr->Generate(gen, instr);
    }
  }

  void GroupOperand::Commit(Generator& gen, Instruction& instr)
  {
    Operand::Commit(gen, instr);

    for (auto opr_ptr : mOperands) {
      opr_ptr->Commit(gen, instr);
    }
  }

  void GroupOperand::CleanUp()
  {
    Operand::CleanUp();

    for (auto opr_ptr : mOperands) {
      opr_ptr->CleanUp();
    }
  }

  OperandConstraint* GroupOperand::InstantiateOperandConstraint() const
  {
    return new GroupOperandConstraint();
  }

  void VectorLayoutOperand::Setup(Generator& gen, Instruction& instr)
  {
    Operand::Setup(gen, instr);

    SetupVectorLayout(gen, instr);
  }

  void AddressingOperand::Generate(Generator& gen, Instruction& instr)
  {
    if (instr.NoRestriction()) {
      // front end has setup the details, no need to generate target address etc.
      BaseGenerate(gen, instr, true);
      return;
    }

    auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    if (not MustGeneratePreamble(gen)) {
      if (GenerateNoPreamble(gen, instr)) {
        LOG(info) << "{AddressingOperand::Generate} generated without preamble" << endl;
        return; // generate no preamble successful.
      }
      else if (addr_constr->NoPreamble()) {
        stringstream err_stream;
        err_stream << "Operand \"" << Name() << "\" failed to generate no-preamble";
        throw OperandError(err_stream.str());
      }
      else {
        LOG(info) << "{AddressingOperand::Generate} switch from no-preamble to preamble" << endl;
      }
    }

    addr_constr->SetUsePreamble(gen);
    GenerateWithPreamble(gen, instr);
    LOG(info) << "{AddressingOperand::Generate} generated with preamble" << endl;
  }

  OperandConstraint* AddressingOperand::InstantiateOperandConstraint() const
  {
    return new AddressingOperandConstraint();
  }

  bool AddressingOperand::BaseGenerate(Generator& gen, Instruction& instr, bool noRestrict)
  {
    GroupOperand::Generate(gen, instr);

    if (noRestrict) {
      UpdateNoRestrictionTarget(instr);
    }

    return false;
  }

  void AddressingOperand::UpdateNoRestrictionTarget(const Instruction& instr)
  {
    auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    auto target_constr =  addr_constr->TargetConstraint();
    if ((nullptr != target_constr) && (target_constr->Size() == 1)) {
      mTargetAddress = target_constr->OnlyValue();
    }
  }

  AddressSolver* AddressingOperand::GetAddressSolver(AddressingMode* pAddrMode, uint64 alignment)
  {
    return new AddressSolver(this, pAddrMode, alignment);
  }

  bool AddressingOperand::VerifyVirtualAddress(uint64 va, uint64 size, bool isInstr) const
  {
    auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    auto vm_mapper = addr_constr->GetVmMapper();
    auto page_req = addr_constr->GetPageRequest();
    bool verif_okay = vm_mapper->VerifyVirtualAddress(va, size, isInstr, page_req);
    LOG(info) << "{AddressingOperand::VerifyVirtualAddress} VA=0x" << hex << va << " size 0x" << size << " is-instr: " << isInstr << " verified okay? " << verif_okay << endl;
    return verif_okay;
  }

  bool AddressingOperand::MustGeneratePreamble(const Generator& rGen)
  {
    auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    if (addr_constr->UsePreamble()) {
      return true;
    }

    return false;
  }

  OperandConstraint* BranchOperand::InstantiateOperandConstraint() const
  {
    return new BranchOperandConstraint();
  }

  BntNode* BranchOperand::GetBntNode(const Instruction& instr) const
  {
    auto branch_constr = mpOperandConstraint->CastInstance<BranchOperandConstraint>();
    if (instr.SpeculativeBnt() and branch_constr->SimulationEnabled())
      return new SpeculativeBntNode(mTargetAddress, not mEscapeTaken);
    else
      return new BntNode(mTargetAddress, not mEscapeTaken);
  }

  void BranchOperand::Commit(Generator& gen, Instruction& instr)
  {
    AddressingOperand::Commit(gen, instr);

    auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    VmMapper* vm_mapper = addr_constr->GetVmMapper();
    const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
    uint64 untagged_target_address = addr_tagging->UntagAddress(mTargetAddress, true);

    auto initializer = gen.GetVirtualMemoryInitializer();
    bool mem_init = initializer->InstructionVaInitialized(untagged_target_address);
    bool sim_on = gen.SimulationEnabled();
    if (mem_init && sim_on && IsBranchTaken(instr)) {
      LOG(notice) << "{BranchOperand::Commit} branch to PC 0x" << hex << mTargetAddress << " re-execution."<< endl;
      auto re_exe_req = new GenReExecutionRequest(untagged_target_address);
      gen.PrependRequest(re_exe_req);
    }
    else if (mem_init && not sim_on) {
      LOG(info) << "{BranchOperand::Commit} escape taking PC to 0x" << hex << mTargetAddress << endl;
      mEscapeTaken = true;
    }
  }

  OperandConstraint* RegisterBranchOperand::InstantiateOperandConstraint() const
  {
    return new RegisterBranchOperandConstraint();
  }

  void RegisterBranchOperand::GenerateWithPreamble(Generator& gen, Instruction& instr)
  {
    auto rb_constr = mpOperandConstraint->CastInstance<RegisterBranchOperandConstraint>();
    auto vm_mapper = rb_constr->GetVmMapper();

    if (BaseGenerate(gen, instr)) {
      vm_mapper->MapAddressRange(mTargetAddress, gen.InstructionSpace(), true);
      return;
    }
    auto br_target_constr = rb_constr->TargetConstraint();
    auto page_req = rb_constr->GetPageRequest();
    VaGenerator va_gen(vm_mapper, page_req, br_target_constr);
    mTargetAddress = va_gen.GenerateAddress(gen.InstructionAlignment(), gen.InstructionSpace(), true, page_req->MemoryAccessType());
    LOG(notice) << "Register-branch generated target address 0x" << hex << mTargetAddress << endl;
  }

  uint64 RegisterBranchOperand::GetAddressingAlignment(uint64 alignment, uint64 dataSize) const
  {
    auto rb_constr =  mpOperandConstraint->CastInstance<RegisterBranchOperandConstraint>();
    if (rb_constr->UnalignedPC() or rb_constr->TargetConstraintForced()) {
      alignment = 1;
    }

    return alignment;
  }

  AddressingMode* RegisterBranchOperand::GetAddressingMode(uint64 alignment) const
  {
    auto rb_constr =  mpOperandConstraint->CastInstance<RegisterBranchOperandConstraint>();
    if (rb_constr->UnalignedPC()) {
      return new UnalignedRegisterBranchMode(alignment);
    }
    return new RegisterBranchMode(alignment);
  }

  bool RegisterBranchOperand::GenerateNoPreamble(Generator& gen, Instruction& instr)
  {
    auto instr_align = gen.InstructionAlignment();
    AddressingMode* template_ptr = GetAddressingMode(GetAddressingAlignment(instr_align));
    auto addr_solver = GetAddressSolver(template_ptr, instr_align);
    std::unique_ptr<AddressSolver> addr_solver_storage(addr_solver);
    auto addr_mode = addr_solver->Solve(gen, instr, gen.InstructionSpace(), true);

    if (nullptr == addr_mode)
      return false;

    mTargetAddress = addr_mode->TargetAddress();
    LOG(notice) << "{RegisterBranchOperand::GenerateNoPreamble} instruction: " << instr.FullName() << " addressing-mode: " << template_ptr->Type() << " target address: 0x" << hex << mTargetAddress << " base value: 0x" << addr_mode->BaseValue() << " base free? " << addr_mode->IsFree() << endl;
    addr_solver->SetOperandResults();
    return true;
  }

  bool RegisterBranchOperand::GetPrePostAmbleRequests(Generator& gen) const
  {
    auto rb_constr = mpOperandConstraint->CastInstance<RegisterBranchOperandConstraint>();
    if (rb_constr->UsePreamble()) {
      gen.AddLoadRegisterAmbleRequests(rb_constr->GetRegisterOperand()->ChoiceText(), mTargetAddress);
      return true;
    }

    return false;
  }

  bool RegisterBranchOperand::MustGeneratePreamble(const Generator& rGen)
  {
    auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    if (not rGen.HasISS() /* TODO temporary */ or addr_constr->UsePreamble()) {
      return true;
    }

    return false;
  }

  void PcRelativeBranchOperand::Generate(Generator& gen, Instruction& instr)
  {
    GroupOperand::Generate(gen, instr);
    if (instr.NoRestriction()) {
      // front end has setup the details, no need to generate target address etc.
      UpdateNoRestrictionTarget(instr);
      return;
    }

    auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    auto offset_opr = addr_constr->OffsetOperand();
    auto addr_struct = mpStructure->CastOperandStructure<AddressingOperandStructure>();
    auto offset_constr = offset_opr->GetOperandConstraint()->GetConstraint();
    uint64 pc_value = addr_constr->PC();
    ConstraintSet pc_offset_constr;
    uint32 offset_shift = addr_struct->OffsetScale();
    uint32 access_size = gen.InstructionSpace();
    BaseOffsetConstraint base_offset_constr(offset_opr->BaseValue(), offset_opr->Size(),  offset_shift, MAX_UINT64, true);
    base_offset_constr.GetConstraint(pc_value, access_size, offset_constr, pc_offset_constr);
    LOG(notice) << "PC-relative-branch PC offset constraint: " << pc_offset_constr.ToSimpleString() << endl;
    // In linear mode and user has setup the details, no need to generate target address using VaGenerator.
    if (gen.NoJump() and offset_constr != nullptr) {
      if (offset_constr->Size() == 1) {
        uint64 offset_value = offset_constr->OnlyValue();
        mTargetAddress = pc_value + (offset_value << offset_shift);
        offset_opr->SetValue(offset_value);
        LOG(notice) << "{PcRelativeBranchOperand::Generate} target address 0x" << hex << mTargetAddress << " PC value 0x"<< pc_value << " offset value 0x" << offset_value << endl;
        return;
      }
    }

    auto branch_target_constr = addr_constr->TargetConstraint();
    auto page_req = addr_constr->GetPageRequest();
    VaGenerator va_gen(addr_constr->GetVmMapper(), page_req, branch_target_constr);
    if (gen.HasISS() or (not IsConditional())) {
      va_gen.SetAccurateBranch(instr.ByteSize());
    }
    // << "PC-relative-branch branch target constraint: " << branch_target_constr->ToSimpleString() << endl;
    if (nullptr != branch_target_constr) {
      ConstraintSet * reach_constr = new ConstraintSet();
      base_offset_constr.GetConstraint(pc_value, 1, nullptr, *reach_constr);
      va_gen.SetReachConstraint(this->Name(), reach_constr);
    }
    else
    {
      va_gen.SetReachConstraint(this->Name(), nullptr);
    }

    mTargetAddress = va_gen.GenerateAddress(gen.InstructionAlignment(), access_size, true, page_req->MemoryAccessType(), &pc_offset_constr);

    uint64 offset_value = ((mTargetAddress - pc_value) >> offset_shift) & offset_opr->Mask();
    offset_opr->SetValue(offset_value);
    LOG(notice) << "PC-relative-branch generated target address 0x" << hex << mTargetAddress << " PC value 0x"<< pc_value << " offset value 0x" << offset_value << endl;
  }

  void PcRelativeBranchOperand::UpdateNoRestrictionTarget(const Instruction& instr)
  {
    auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    auto addr_struct = mpStructure->CastOperandStructure<AddressingOperandStructure>();
    auto offset_opr = addr_constr->OffsetOperand();
    uint64 pc_value = addr_constr->PC();
    uint64 offset_value = sign_extend64(offset_opr->Value(), offset_opr->Size()) << addr_struct->OffsetScale();
    mTargetAddress = pc_value + offset_value;
  }

  OperandConstraint* PcRelativeBranchOperand::InstantiateOperandConstraint() const
  {
    return new PcRelativeBranchOperandConstraint();
  }

  void MultiVectorRegisterOperand::Generate(Generator& gen, Instruction& instr)
  {
    VectorRegisterOperand::Generate(gen, instr);
    mExtraRegisters.clear();
    GetExtraRegisterNames(mValue, mExtraRegisters);
  }

  void MultiRegisterOperand::Generate(Generator& gen, Instruction& instr)
  {
    RegisterOperand::Generate(gen, instr);
    mExtraRegisters.clear();
    GetExtraRegisterNames(mValue, mExtraRegisters);
  }

  void MultiRegisterOperand::Commit(Generator& gen, Instruction& instr)
  {
    RegisterOperand::Commit(gen, instr);

    RegisterInitData init_data;
    auto register_constraint = mpOperandConstraint->CastInstance<RegisterOperandConstraint>();
    auto data_req = register_constraint->GetOperandDataRequest(instr, Name());

    const RegisterFile* reg_file = gen.GetRegisterFile();
    bool updating_reg = (not gen.HasISS()) and mpStructure->HasWriteAccess();

    for (auto reg_name : mExtraRegisters) {
      Register* reg_ptr = reg_file->RegisterLookup(reg_name);
      if (updating_reg) {
        reg_ptr->ClearAttribute(ERegAttrType::HasValue);
      }

      if (reg_ptr->IsInitialized())
        continue;

      init_data.Setup(gen, reg_ptr, data_req);
      init_data.Commit(gen, reg_ptr);
      init_data.Cleanup();
    }

    // LOG(notice) << "commited extra registers" << endl;
  }

  void MultiRegisterOperand::GetExtraRegisterNames(uint32 regIndex, std::vector<std::string>& nameVec) const
  {
    uint32 extra_num = NumberRegisters() - 1;

    uint32 index_var = regIndex;
    for (uint32 i = 0; i < extra_num; ++ i) {
      nameVec.push_back(GetNextRegisterName(index_var));
    }
  }

  ChoicesFilter * MultiRegisterOperand::GetChoicesFilter(const ConstraintSet* pConstrSet) const
  {
    return new MultiRegisterChoicesFilter(pConstrSet, this);
  }

  OperandConstraint* LoadStoreOperand::InstantiateOperandConstraint() const
  {
    return new LoadStoreOperandConstraint();
  }

  void LoadStoreOperand::Commit(Generator& gen, Instruction& instr)
  {
    AddressingOperand::Commit(gen, instr);

    auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    VmMapper* vm_mapper = addr_constr->GetVmMapper();
    const AddressTagging* addr_tagging = vm_mapper->GetAddressTagging();
    uint64 untagged_target_address = addr_tagging->UntagAddress(mTargetAddress, false);

    vector<uint64> target_addresses;
    GetTargetAddresses(instr, untagged_target_address, target_addresses);

    bool no_init = false;
    if (instr.NoRestriction() and (not addr_constr->HasDataConstraints())) {
      no_init = true;
    }

    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    auto initializer = gen.GetVirtualMemoryInitializer();
    for (uint64 target_address : target_addresses) {
      if (IsTargetSharedRead(lsop_struct->MemAccessType(), target_address, lsop_struct->DataSize())) {
        instr.SetUnpredictable(true);
      }

      bool addr_needs_init = initializer->VaNeedInitialization(target_address, lsop_struct->DataSize());
      if (no_init or (not addr_needs_init)) {
        continue;
      }

      MemoryInitData init_data(target_address, lsop_struct->DataSize(), lsop_struct->ElementSize(), lsop_struct->MemAccessType());
      init_data.Setup(gen, this);
      init_data.Commit(gen);
    }
  }

  void LoadStoreOperand::GetDataTargetConstraint(ConstraintSet& dataConstr) const
  {
    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    uint32 element_size = lsop_struct->ElementSize();
    uint32 sign_extension = lsop_struct->SignExtension();
    uint32 element_bits =  element_size << 3;

    if (sign_extension > element_size) { // do sign extension
      uint64 base_value = 1ull << (element_bits - 1);
      dataConstr.AddRange(0, base_value - 1);

      if (sign_extension == 8u) {
        uint64 ext_value = sign_extend64(base_value, element_bits);
        dataConstr.AddRange(ext_value, -1ull);
      }
      else if (sign_extension == 4u) {
        uint32 ext_value = sign_extend32(base_value, element_bits);
        dataConstr.AddRange(ext_value, -1u);
      }
      else {
        LOG(fail) << "Invalid sign exension value:" << sign_extension << endl;
        FAIL("invalid-sign-extension");
      }
    }
    else {
      uint64 max_data = -1ull;
      if (element_size < 8u)
        max_data =  (1ull << element_bits) - 1;

      dataConstr.AddRange(0, max_data);
    }

    LOG(notice) << "Data target constraint:" << dataConstr.ToSimpleString() << endl;
  }

  uint64 LoadStoreOperand::GetAddressingAlignment(uint64 alignment, uint64 dataSize) const
  {
    auto ls_constr =  mpOperandConstraint->CastInstance<LoadStoreOperandConstraint>();
    if (ls_constr->TargetConstraintForced()) {
      alignment = 1;
      // << "{LoadStoreOperand::GetAddressingAlignment} Target constraint forced unaligned." << endl;
      return alignment;
    }
    if (ls_constr->BaseOperandSpAligned()) {
      return (nullptr == ls_constr->OffsetOperand()) ? ls_constr->SpAlignment() : alignment;
    }

    auto ls_aligned = ls_constr->AlignedData();
    switch (ls_aligned) {
    case EDataAlignedType::Unaligned:
      alignment = GetUnalignment(dataSize);
      break;
    case EDataAlignedType::SingleDataAligned:
      break;
    case EDataAlignedType::WholeDataAligned:
      if (is_power_of_2(dataSize))
         alignment = dataSize;
      else {
         alignment = round_up_power2(dataSize);
      }
    break;
    default:
      LOG(fail) << "{LoadStoreOperand::GetAddressingAlignment} unimplemented data aligned type:" << EDataAlignedType_to_string(ls_aligned) << endl;
      FAIL("unimplemented-data-aligned-type");
    }
    // << "{LoadStoreOperand::GetAddressingAlignment} data aligned type: " <<  EDataAlignedType_to_string(ls_aligned) << ", alignment: 0x" << hex << alignment << endl;
    return alignment;
  }

  void LoadStoreOperand::GetTargetAddresses(const Instruction& rInstr, cuint64 baseTargetAddr, vector<uint64>& rTargetAddresses) const
  {
    rTargetAddresses.push_back(baseTargetAddr);
  }

  bool LoadStoreOperand::IsTargetSharedRead(const EMemAccessType memAccessType, cuint64 targetAddr, cuint64 dataSize) const
  {
    if (memAccessType == EMemAccessType::Read) {
      auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
      VmMapper* vm_mapper = addr_constr->GetVmMapper();
      const ConstraintSet* shared_constr = vm_mapper->VirtualSharedConstraintSet();
      ConstraintSet target_constr(targetAddr, (targetAddr + dataSize - 1));

      if (shared_constr->Intersects(target_constr)) {
        return true;
      }
    }

    return false;
  }

  OperandConstraint* BaseOffsetLoadStoreOperand::InstantiateOperandConstraint() const
  {
    return new BaseOffsetLoadStoreOperandConstraint();
  }

  static void trim_offset_value(uint64 target_addr, const LoadStoreOperandStructure* lsop_struct, BaseOffsetLoadStoreOperandConstraint * bols_constr)
  {
    auto offset_opr = bols_constr->OffsetOperand();
    if (nullptr == offset_opr) {
      return;
    }
    uint64 offset_value = offset_opr->Value();
    uint64 offset_value_old = offset_value;
    uint32 offset_scale = lsop_struct->OffsetScale();
    uint64 trim_mask = (bols_constr->SpAlignment() - 1) >> offset_scale;
    if (offset_opr->IsSigned()) {
      offset_value = sign_extend64(offset_value, offset_opr->Size());
    }
    offset_value = (offset_value & ~trim_mask) + ((target_addr >> offset_scale) & trim_mask);
    offset_value &= offset_opr->Mask();
    if ((offset_opr->GetOperandConstraint()->HasConstraint()) and (offset_value != offset_value_old))
    {
      LOG(fail) << "{trim_offset_value} User set immediate("<<offset_value_old<<"), which can't satisfy alignment of SP and target!"<<endl;
      FAIL("unimplemented-imm-value");
    }
    offset_opr->SetValue(offset_value);
  }

  static void calculate_base_value(uint64 target_addr, const LoadStoreOperandStructure* lsop_struct, BaseOffsetLoadStoreOperandConstraint * bols_constr)
  {
    auto offset_opr = bols_constr->OffsetOperand();
    if (nullptr == offset_opr) {
      bols_constr->SetBaseValue(target_addr);
      return;
    }

    uint64 offset_value = offset_opr->Value();

    if (offset_opr->IsSigned()) {
      offset_value = sign_extend64(offset_value, offset_opr->Size());
    }

    offset_value = lsop_struct->AdjustOffset(offset_value);
    bols_constr->SetBaseValue(target_addr - offset_value);
  }

  void BaseOffsetLoadStoreOperand::GenerateWithPreamble(Generator& gen, Instruction& instr)
  {
    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    auto bols_constr = mpOperandConstraint->CastInstance<BaseOffsetLoadStoreOperandConstraint>();
    auto vm_mapper =  bols_constr->GetVmMapper();
    auto data_size = lsop_struct->DataSize();
    if (BaseGenerate(gen, instr)) {
      vm_mapper->MapAddressRange(mTargetAddress, data_size, false);
      return;
    }

    auto target_constr = bols_constr->TargetConstraint();
    auto ls_alignment = GetAddressingAlignment(lsop_struct->Alignment(), data_size);
    auto page_req = bols_constr->GetPageRequest();
    VaGenerator va_gen(vm_mapper, page_req, target_constr);
    mTargetAddress = va_gen.GenerateAddress(ls_alignment, data_size, false, page_req->MemoryAccessType());

    if (bols_constr->BaseOperandSpAligned()) {
      trim_offset_value(mTargetAddress, lsop_struct, bols_constr);
    }
    calculate_base_value(mTargetAddress, lsop_struct, bols_constr);
    LOG(notice) << "{BaseOffsetLoadStoreOperand::GenerateWithPreamble} generated target address 0x" << hex << mTargetAddress << " alignment " << dec << ls_alignment << " data size "
                << data_size << " base value 0x"<< hex << bols_constr->BaseValue() << endl;
  }

  AddressingMode* BaseOffsetLoadStoreOperand::GetAddressingMode(uint64 alignment) const
  {
    auto bols_constr = mpOperandConstraint->CastInstance<BaseOffsetLoadStoreOperandConstraint>();
    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    AddressingMode* addr_mode_ptr = nullptr;
    if (nullptr == bols_constr->OffsetOperand()) {
      addr_mode_ptr = new BaseOnlyMode();
    }
    else if (lsop_struct->OffsetScale() > 0) {
      if (lsop_struct->GetIsOffsetShift()){
        addr_mode_ptr = new BaseOffsetShiftMode(lsop_struct->OffsetScale());
      } else{
        addr_mode_ptr = new BaseOffsetMulMode(lsop_struct->OffsetScale());
      }
    }
    else {
      addr_mode_ptr = new BaseOffsetMode();
    }
    return addr_mode_ptr;
  }

  bool BaseOffsetLoadStoreOperand::GenerateNoPreamble(Generator& gen, Instruction& instr)
  {
    auto bols_constr = mpOperandConstraint->CastInstance<BaseOffsetLoadStoreOperandConstraint>();
    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    AddressingMode* template_ptr = GetAddressingMode();
    auto data_size = lsop_struct->DataSize();
    auto ls_alignment = GetAddressingAlignment(lsop_struct->Alignment(), data_size);

    auto addr_solver = GetAddressSolver(template_ptr, ls_alignment);
    std::unique_ptr<AddressSolver> addr_solver_storage(addr_solver);

    auto addr_mode = addr_solver->Solve(gen, instr, data_size, false, lsop_struct->MemAccessType());

    if (nullptr == addr_mode)
      return false;

    bols_constr->SetBaseValue(addr_mode->BaseValue());
    mTargetAddress = addr_mode->TargetAddress();
    LOG(notice) << "{BaseOffsetLoadStoreOperand::GenerateNoPreamble} instruction: " << instr.FullName() << " addressing-mode: " << template_ptr->Type() << " target address: 0x" << hex << mTargetAddress << endl;
    addr_solver->SetOperandResults();
    return true;
  }

  bool BaseOffsetLoadStoreOperand::GetPrePostAmbleRequests(Generator& gen) const
  {
    auto bols_constr = mpOperandConstraint->CastInstance<BaseOffsetLoadStoreOperandConstraint>();
    if (bols_constr->UsePreamble()) {
      uint64 baseAddr =  bols_constr->BaseValue();
      gen.AddLoadRegisterAmbleRequests(bols_constr->BaseOperand()->ChoiceText(), baseAddr);
      return true;
    }

    return false;
  }

  static void trim_index_value(uint64 target_addr, const LoadStoreOperandStructure* lsop_struct, BaseIndexLoadStoreOperandConstraint * bils_constr,Generator& gen, Instruction& instr)
  {
    auto index_opr = bils_constr->IndexOperand();
    if (nullptr == index_opr) {
      return;
    }
    //if target_addr is not aligned with spalignement,and index is xzr/wzr, we need to generate a new non-zero index.
    if (0 != (target_addr & (bils_constr->SpAlignment() - 1)) and (31 == bils_constr->IndexOperand()->Value())){
      LOG(info)<<"spalignment="<<bils_constr->SpAlignment()<<endl;
      vector<const Choice*> rChoicesList ;
      bils_constr->IndexOperand()->GetAvailableChoices(rChoicesList);
      if (1 == rChoicesList.size() and 31 == rChoicesList[0]->Value()){
        LOG(fail) << "{trim_index_value} target_addr is 0x"<<hex<<target_addr<<", base is sp and aligned, but index is XZR/WZR!" << endl;
        FAIL("index-forced-wrong");
      }else{
        bils_constr->IndexOperand()->SubConstraintValue(31);
        bils_constr->IndexOperand()->Generate(gen,instr);
        LOG(info)<<"{trim_index_value}: index is changed from XZR/WZR to "<<bils_constr->IndexOperand()->ChoiceText()<<endl;
      }
    }

    uint32 ext_amount = 0;
    const string& ea_name = lsop_struct->ExtendAmount();
    get_extend_type_amount(ea_name, ext_amount);
    auto extend_amount_opr = bils_constr->ExtendAmountOperand();
    if (nullptr != extend_amount_opr){
      ext_amount *= extend_amount_opr->Value();
    }
    uint64 index_value = bils_constr->IndexValue();
    uint64 trim_mask = (bils_constr->SpAlignment() - 1) >> ext_amount;
    index_value = (index_value & ~trim_mask) + ((target_addr >> ext_amount) & trim_mask);

    if (bils_constr->IndexValue() != index_value){
      LOG(trace) << "{trim_index_value} index value before " << bils_constr->IndexValue() << " after " << index_value << endl;
      bils_constr->SetIndexValue(index_value);
    }
  }

  static void calculate_base_value(uint64 target_addr, const LoadStoreOperandStructure* lsop_struct, BaseIndexLoadStoreOperandConstraint * bils_constr)
  {
    auto index_opr = bils_constr->IndexOperand();
    if (nullptr == index_opr) {
      bils_constr->SetBaseValue(target_addr);
      return;
    }
    uint64 index_value = bils_constr->IndexValue();
    const string& ea_name = lsop_struct->ExtendAmount();
    uint32 ext_amount = 0;
    EExtendType ext_type = get_extend_type_amount(ea_name, ext_amount);

    auto extend_amount_opr = bils_constr->ExtendAmountOperand();
    if (nullptr != extend_amount_opr){
      ext_amount *= extend_amount_opr->Value();
    }

    uint64 offset_value = extend_regval(index_value, ext_type, ext_amount);

    bils_constr->SetBaseValue(target_addr - offset_value);
  }

  void BaseIndexLoadStoreOperand::GenerateWithPreamble(Generator& gen, Instruction& instr)
  {
    GroupOperand::Generate(gen, instr); // generate after SetUsePreamble call

    auto bils_constr = mpOperandConstraint->CastInstance<BaseIndexLoadStoreOperandConstraint>();
    auto ls_target_constr = bils_constr->TargetConstraint();
    auto page_req = bils_constr->GetPageRequest();
    VaGenerator va_gen(bils_constr->GetVmMapper(), page_req, ls_target_constr);
    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    const RegisterFile* reg_file = gen.GetRegisterFile();
    auto data_size = lsop_struct->DataSize();
    Register* index_ptr = reg_file->RegisterLookup(bils_constr->IndexOperand()->ChoiceText());
    auto ls_alignment = GetAddressingAlignment(lsop_struct->Alignment(), data_size);
    if (bils_constr->BaseOperandSpAligned() and (31 == bils_constr->IndexOperand()->Value())){
      ls_alignment = 16;
    }

    mTargetAddress = va_gen.GenerateAddress(ls_alignment, data_size, false, page_req->MemoryAccessType());

    if (bils_constr->IndexChoiceValueDuplicate()) { // If has same choice value before index operand, it should be using preamble to make sure the index value always usable.
      bils_constr->SetIndexUsePreamble();
    }

    uint64 index_value = index_ptr->ReloadValue();
    bils_constr->SetIndexValue(index_value);

    if (bils_constr->BaseOperandSpAligned()) {
      bils_constr->SetIndexUsePreamble();
      trim_index_value(mTargetAddress, lsop_struct, bils_constr, gen, instr);
    }

    if (not bils_constr->IndexUsePreamble()) {
      if (not index_ptr->IsInitialized()) {
        instr.SetOperandDataValue(bils_constr->IndexOperand()->Name(), index_value, index_ptr->Size());
      }
      else {
        index_value = index_ptr->Value();
        bils_constr->SetIndexValue(index_value);
      }
    }

    calculate_base_value(mTargetAddress, lsop_struct, bils_constr);

    LOG(notice) << "Base-index-load-store generated target address 0x" << hex << mTargetAddress << " alignment " << dec << ls_alignment << " data size " << data_size << " base value 0x" << hex << bils_constr->BaseValue() << " index value 0x" << hex <<  bils_constr->IndexValue() << endl;
  }

  AddressingMode* BaseIndexLoadStoreOperand::GetAddressingMode(uint64 alignment) const
  {
    auto bils_constr = mpOperandConstraint->CastInstance<BaseIndexLoadStoreOperandConstraint>();
    AddressingMode* addr_mode_ptr = nullptr;
    if (nullptr == bils_constr->ExtendAmountOperand()){
      addr_mode_ptr = new BaseIndexExtendMode();
    } else {
      addr_mode_ptr = new BaseIndexAmountBitExtendMode();
    }
    return addr_mode_ptr;
  }
  bool BaseIndexLoadStoreOperand::GenerateNoPreamble(Generator& gen, Instruction& instr)
  {
    auto bils_constr = mpOperandConstraint->CastInstance<BaseIndexLoadStoreOperandConstraint>();
    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    AddressingMode* template_ptr = GetAddressingMode();
    auto data_size = lsop_struct->DataSize();
    auto ls_alignment = GetAddressingAlignment(lsop_struct->Alignment(), data_size);
    auto addr_solver = GetAddressSolver(template_ptr, ls_alignment);
    std::unique_ptr<AddressSolver> addr_solver_storage(addr_solver);

    auto addr_mode = addr_solver->Solve(gen, instr, data_size, false, lsop_struct->MemAccessType());

    if (nullptr == addr_mode)
      return false;

    bils_constr->SetBaseValue(addr_mode->BaseValue());
    bils_constr->SetIndexValue(addr_mode->IndexValue());
    mTargetAddress = addr_mode->TargetAddress();
    LOG(notice) << "{BaseIndexLoadStoreOperand::GenerateNoPreamble} instruction: " << instr.FullName() << " addressing-mode: " << template_ptr->Type() << " target address: 0x" << hex << mTargetAddress << endl;
    addr_solver->SetOperandResults();
    return true;
  }

  bool BaseIndexLoadStoreOperand::GetPrePostAmbleRequests(Generator& gen) const
  {
    auto bils_constr = mpOperandConstraint->CastInstance<BaseIndexLoadStoreOperandConstraint>();
    if (bils_constr->UsePreamble()) {
      uint64 baseAddr =  bils_constr->BaseValue();

      auto index_reg_name = bils_constr->IndexOperand()->ChoiceText();
      if (bils_constr->IndexUsePreamble()) {
        gen.AddLoadRegisterAmbleRequests(index_reg_name, bils_constr->IndexValue());
      }
      else { //Reserve the index register to avoid overwrite by the preamble instruction.
        ERegAttrType reserv_acc = ERegAttrType::Write;

        if (not gen.IsRegisterReserved(index_reg_name, reserv_acc)) {
          gen.ReserveRegister(index_reg_name, reserv_acc);
          gen.PrependRequest(new GenRegisterReservation(index_reg_name, false, reserv_acc));
        }
      }
      gen.AddLoadRegisterAmbleRequests(bils_constr->BaseOperand()->ChoiceText(), baseAddr);
      return true;
    }

    return false;
  }

  OperandConstraint* BaseIndexLoadStoreOperand::InstantiateOperandConstraint() const
  {
    return new BaseIndexLoadStoreOperandConstraint();
  }

  bool BaseIndexLoadStoreOperand::MustGeneratePreamble(const Generator& rGen)
  {
    auto addr_constr = mpOperandConstraint->CastInstance<AddressingOperandConstraint>();
    if (not rGen.HasISS() /* TODO temporary */ or addr_constr->UsePreamble()) {
      return true;
    }

    return false;
  }

  void PcOffsetLoadStoreOperand::Generate(Generator& gen, Instruction& instr)
  {
    GroupOperand::Generate(gen, instr);
    if (instr.NoRestriction()) {
      // front end has setup the details, no need to generate target address etc.
      return;
    }

    auto pols_constr = mpOperandConstraint->CastInstance<PcOffsetLoadStoreOperandConstraint>();
    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    auto offset_opr = pols_constr->OffsetOperand();

    uint64 pc_value = pols_constr->PC();
    ConstraintSet pc_offset_constr;
    // uint32 offset_shift = addr_struct->OffsetScale(); // investigate to see if the scale info is needed.
    const unsigned offset_shift = 2;
    auto data_size = lsop_struct->DataSize();
    auto offset_constr = offset_opr->GetOperandConstraint()->GetConstraint();
    BaseOffsetConstraint base_offset_constr(offset_opr->BaseValue(), offset_opr->Size(),  offset_shift, MAX_UINT64, true);
    base_offset_constr.GetConstraint(pc_value, data_size, offset_constr, pc_offset_constr);
    // << "PC-relative-load-store PC offset constraint: " << pc_offset_constr.ToSimpleString() << endl;

    auto ls_target_constr = pols_constr->TargetConstraint();

    // << "PC-relative-load-store load-store target constraint: " << ls_target_constr->ToSimpleString() << endl;
    auto page_req = pols_constr->GetPageRequest();
    VaGenerator va_gen(pols_constr->GetVmMapper(), page_req, ls_target_constr);
    if (nullptr != ls_target_constr) {
      ConstraintSet * reach_constr = new ConstraintSet();
      base_offset_constr.GetConstraint(pc_value, 1, nullptr, *reach_constr); // reach constraint, access size is 1.
      va_gen.SetReachConstraint(this->Name(), reach_constr);
    }
    auto ls_alignment = GetAddressingAlignment(lsop_struct->Alignment(), data_size);
    mTargetAddress = va_gen.GenerateAddress(ls_alignment, data_size, false, page_req->MemoryAccessType(), &pc_offset_constr);

    uint64 offset_value = ((mTargetAddress - pc_value) >> offset_shift) & offset_opr->Mask();
    offset_opr->SetValue(offset_value);
    LOG(notice) << "PC-Offset-load-store generated target address 0x" << hex << mTargetAddress << " alignment " << dec << ls_alignment<< " data size " << data_size << " PC value 0x"<< hex << pc_value << " Offset 0x" << offset_opr->Value() << endl;
  }

  OperandConstraint* PcOffsetLoadStoreOperand::InstantiateOperandConstraint() const
  {
    return new PcOffsetLoadStoreOperandConstraint();
  }

  bool VectorStridedLoadStoreOperand::GetPrePostAmbleRequests(Generator& gen) const
  {
    auto strided_opr_constr = mpOperandConstraint->CastInstance<VectorStridedLoadStoreOperandConstraint>();
    if (strided_opr_constr->UsePreamble()) {
      RegisterOperand* stride_opr = strided_opr_constr->StrideOperand();
      gen.AddLoadRegisterAmbleRequests(stride_opr->ChoiceText(), strided_opr_constr->StrideValue());

      RegisterOperand* base_opr = strided_opr_constr->BaseOperand();
      gen.AddLoadRegisterAmbleRequests(base_opr->ChoiceText(), strided_opr_constr->BaseValue());

      return true;
    }

    return false;
  }

  OperandConstraint* VectorStridedLoadStoreOperand::InstantiateOperandConstraint() const
  {
    return new VectorStridedLoadStoreOperandConstraint();
  }

  void VectorStridedLoadStoreOperand::GenerateWithPreamble(Generator& gen, Instruction& instr)
  {
    GroupOperand::Generate(gen, instr);

    // TODO(Noah): Implement solving for the case when the base and index registers are the same
    // register when a solution for this case can be devised.
    DifferStrideOperand(gen, instr);

    auto strided_opr_constr = mpOperandConstraint->CastInstance<VectorStridedLoadStoreOperandConstraint>();
    const GenPageRequest* page_req = strided_opr_constr->GetPageRequest();
    VaGenerator va_gen(strided_opr_constr->GetVmMapper(), page_req, strided_opr_constr->TargetConstraint());

    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    uint64 alignment = GetAddressingAlignment(lsop_struct->Alignment(), lsop_struct->DataSize());

    // TODO(Noah): Implement a more robust solution method when one can be devised. The difficulty
    // is finding a pattern of equidistant compliant address ranges. We have the facility to find
    // one compliant address range, but then we have to find a series of address ranges that are
    // stride length apart that also comply with the relevant constraints. A brute force approach to
    // this would likely be very costly. The suboptimal solution constrains all of the addressses to
    // be in one large block. This is relatively fast and reliable, but severely limits the possible
    // stride values that can be used.
    uint32 addr_range_size = 0x4000;
    uint64 stride_value = CalculateStrideValue(instr, alignment, addr_range_size);
    strided_opr_constr->SetStrideValue(stride_value);

    uint64 base_addr = va_gen.GenerateAddress(alignment, addr_range_size, false, page_req->MemoryAccessType());
    mTargetAddress = CalculateBaseValue(base_addr, alignment, addr_range_size, stride_value);
    strided_opr_constr->SetBaseValue(mTargetAddress);

    LOG(notice) << "Vector-strided-load-store generated target address 0x" << hex << mTargetAddress << " alignment " << dec << alignment << " data size " << lsop_struct->DataSize() << " base value 0x" << hex << strided_opr_constr->BaseValue() << " stride value 0x" << hex <<  strided_opr_constr->StrideValue() << endl;
  }

  bool VectorStridedLoadStoreOperand::GenerateNoPreamble(Generator& gen, Instruction& instr)
  {
    // TODO(Noah): Implement this method before finishing the vector extension project.
    return false;
  }

  AddressingMode* VectorStridedLoadStoreOperand::GetAddressingMode(uint64 alignment) const
  {
    // TODO(Noah): Implement this method before finishing the vector extension project.
    return nullptr;
  }

  void VectorStridedLoadStoreOperand::GetTargetAddresses(const Instruction& rInstr, cuint64 baseTargetAddr, vector<uint64>& rTargetAddresses) const
  {
    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    const VectorLayout* vec_layout = instr_constr->GetVectorLayout();
    auto strided_opr_constr = mpOperandConstraint->CastInstance<VectorStridedLoadStoreOperandConstraint>();
    for (uint32 elem_index = 0; elem_index < vec_layout->mElemCount; elem_index++) {
      uint64 elem_target_addr = baseTargetAddr + strided_opr_constr->StrideValue() * elem_index;
      rTargetAddresses.push_back(elem_target_addr);
    }
  }

  void VectorStridedLoadStoreOperand::DifferStrideOperand(Generator& rGen, Instruction& rInstr) {
    auto strided_opr_constr = mpOperandConstraint->CastInstance<VectorStridedLoadStoreOperandConstraint>();
    RegisterOperand* base_opr = strided_opr_constr->BaseOperand();
    RegisterOperand* stride_opr = strided_opr_constr->StrideOperand();
    if (base_opr->Value() == stride_opr->Value()) {
      stride_opr->SubConstraintValue(base_opr->Value());
      stride_opr->Generate(rGen, rInstr);
    }
  }

  uint64 VectorStridedLoadStoreOperand::CalculateStrideValue(const Instruction& rInstr, cuint32 alignment, cuint32 addrRangeSize) const
  {
    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    const VectorLayout* vec_layout = instr_constr->GetVectorLayout();

    uint64 max_stride_total = addrRangeSize - lsop_struct->ElementSize();
    uint64 max_stride = max_stride_total / (vec_layout->mElemCount - 1);

    // Randomly select a stride value between plus or minus the maximum stride value
    ConstraintSet stride_constr(0, max_stride);
    stride_constr.AddRange(-max_stride, MAX_UINT64);

    uint32 align_shift = get_align_shift(alignment);
    stride_constr.AlignWithSize(get_align_mask(alignment), lsop_struct->DataSize());
    stride_constr.ShiftRight(align_shift);
    uint64 stride_val = stride_constr.ChooseValue() << align_shift;

    return stride_val;
  }

  uint64 VectorStridedLoadStoreOperand::CalculateBaseValue(cuint64 baseAddr, cuint32 alignment, cuint32 addrRangeSize, cuint64 strideVal) const
  {
    uint64 base_val = 0;
    if (static_cast<int64>(strideVal) >= 0) {
      base_val = baseAddr;
    }
    else {
      uint64 end_addr = baseAddr + addrRangeSize - 1;
      base_val = end_addr & get_align_mask(alignment);
    }

    return base_val;
  }

  bool VectorIndexedLoadStoreOperand::GetPrePostAmbleRequests(Generator& gen) const
  {
    auto indexed_opr_constr = mpOperandConstraint->CastInstance<VectorIndexedLoadStoreOperandConstraint>();
    if (indexed_opr_constr->UsePreamble()) {
      // TODO(Noah): Handle the case where multiple vector registers are used as the index operand
      // when there is time to do so.
      RegisterOperand* index_opr = indexed_opr_constr->IndexOperand();
      uint64 index_opr_data_block_addr = AllocateIndexOperandDataBlock(gen);
      gen.AddLoadRegisterAmbleRequests(index_opr->ChoiceText(), index_opr_data_block_addr);

      RegisterOperand* base_opr = indexed_opr_constr->BaseOperand();
      gen.AddLoadRegisterAmbleRequests(base_opr->ChoiceText(), indexed_opr_constr->BaseValue());

      return true;
    }

    return false;
  }

  OperandConstraint* VectorIndexedLoadStoreOperand::InstantiateOperandConstraint() const
  {
    return new VectorIndexedLoadStoreOperandConstraint();
  }

  void VectorIndexedLoadStoreOperand::GenerateWithPreamble(Generator& gen, Instruction& instr)
  {
    GroupOperand::Generate(gen, instr);

    RecordIndexElementByteSize(instr);

    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    uint64 alignment = GetAddressingAlignment(lsop_struct->Alignment(), lsop_struct->DataSize());

    vector<uint64> index_elem_values;
    uint64 base_val = CalculateBaseAndFirstIndexValues(instr, alignment, index_elem_values);
    auto indexed_opr_constr = mpOperandConstraint->CastInstance<VectorIndexedLoadStoreOperandConstraint>();
    indexed_opr_constr->SetBaseValue(base_val);
    mTargetAddress = base_val + index_elem_values[0];

    CalculateIndexValues(instr, alignment, base_val, index_elem_values);
    indexed_opr_constr->SetIndexValues(index_elem_values);

    LOG(notice) << "Vector-indexed-load-store generated target address 0x" << hex << mTargetAddress << " alignment " << dec << alignment << " data size " << lsop_struct->DataSize() << " base value 0x" << hex << indexed_opr_constr->BaseValue() << endl;
  }

  bool VectorIndexedLoadStoreOperand::GenerateNoPreamble(Generator& gen, Instruction& instr)
  {
    // TODO(Noah): Implement this method before finishing the vector extension project.
    return false;
  }

  AddressingMode* VectorIndexedLoadStoreOperand::GetAddressingMode(uint64 alignment) const
  {
    // TODO(Noah): Implement this method before finishing the vector extension project.
    return nullptr;
  }

  void VectorIndexedLoadStoreOperand::GetTargetAddresses(const Instruction& rInstr, cuint64 baseTargetAddr, vector<uint64>& rTargetAddresses) const
  {
    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    const VectorLayout* vec_layout = instr_constr->GetVectorLayout();
    auto indexed_opr_constr = mpOperandConstraint->CastInstance<VectorIndexedLoadStoreOperandConstraint>();
    const vector<uint64>& index_elem_values = indexed_opr_constr->IndexValues();
    for (uint32 elem_index = 0; elem_index < vec_layout->mElemCount; elem_index++) {
      uint64 elem_target_addr = indexed_opr_constr->BaseValue() + index_elem_values[elem_index];
      rTargetAddresses.push_back(elem_target_addr);
    }
  }

  uint64 VectorIndexedLoadStoreOperand::AllocateIndexOperandDataBlock(Generator& rGen) const
  {
    Config* config = Config::Instance();
    uint64 reg_size = config->LimitValue(ELimitType::MaxPhysicalVectorLen) / 8;
    DataBlock data_block(reg_size);

    auto indexed_opr_constr = mpOperandConstraint->CastInstance<VectorIndexedLoadStoreOperandConstraint>();
    const vector<uint64>& index_elem_values = indexed_opr_constr->IndexValues();
    if (rGen.IsDataBigEndian()) {
      for (auto itr = index_elem_values.crbegin(); itr != index_elem_values.crend(); ++itr) {
        data_block.AddUnit(*itr, indexed_opr_constr->IndexElementSize(), rGen.IsDataBigEndian());
      }
    }
    else {
      for (uint64 index_elem_val : index_elem_values) {
        data_block.AddUnit(index_elem_val, indexed_opr_constr->IndexElementSize(), rGen.IsDataBigEndian());
      }
    }

    // The index operand format may only occupy part of the register. The preamble instruction loads
    // the entire register, so fill in the remaining elements with random values.
    uint64 reg_elem_count = reg_size / indexed_opr_constr->IndexElementSize();
    ConstraintSet random_elem_constr(0, get_mask64(indexed_opr_constr->IndexElementSize() * 8));
    for (uint64 elem_index = index_elem_values.size(); elem_index < reg_elem_count; elem_index++) {
      data_block.AddUnit(random_elem_constr.ChooseValue(), indexed_opr_constr->IndexElementSize(), rGen.IsDataBigEndian());
    }

    return data_block.Allocate(&rGen, indexed_opr_constr->GetVmMapper());
  }

  void VectorIndexedLoadStoreOperand::RecordIndexElementByteSize(const Instruction& rInstr)
  {
    auto indexed_opr_constr = mpOperandConstraint->CastInstance<VectorIndexedLoadStoreOperandConstraint>();
    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    const VectorLayout* vec_layout = instr_constr->GetVectorLayout();
    indexed_opr_constr->SetIndexElementSize(vec_layout->mElemSize / 8);
  }

  uint64 VectorIndexedLoadStoreOperand::CalculateBaseAndFirstIndexValues(const Instruction& rInstr, cuint32 alignment, vector<uint64>& rIndexElemValues) const
  {
    auto indexed_opr_constr = mpOperandConstraint->CastInstance<VectorIndexedLoadStoreOperandConstraint>();
    const GenPageRequest* page_req = indexed_opr_constr->GetPageRequest();
    VaGenerator va_gen(indexed_opr_constr->GetVmMapper(), page_req, indexed_opr_constr->TargetConstraint());

    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();
    uint64 target_addr = va_gen.GenerateAddress(alignment, lsop_struct->DataSize(), false, page_req->MemoryAccessType());

    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    const VectorLayout* vec_layout = instr_constr->GetVectorLayout();

    ConstraintSet index_elem_constr(0, get_mask64(vec_layout->mElemSize));
    uint64 index_elem_val = sign_extend64(index_elem_constr.ChooseValue(), vec_layout->mElemSize);

    rIndexElemValues.push_back(index_elem_val);
    return (target_addr - index_elem_val);
  }

  void VectorIndexedLoadStoreOperand::CalculateIndexValues(const Instruction& rInstr, cuint32 alignment, cuint64 baseVal, vector<uint64>& rIndexElemValues) const
  {
    auto indexed_opr_constr = mpOperandConstraint->CastInstance<VectorIndexedLoadStoreOperandConstraint>();
    const GenPageRequest* page_req = indexed_opr_constr->GetPageRequest();
    VaGenerator va_gen(indexed_opr_constr->GetVmMapper(), page_req, indexed_opr_constr->TargetConstraint());

    auto instr_constr = dynamic_cast<const VectorInstructionConstraint*>(rInstr.GetInstructionConstraint());
    const VectorLayout* vec_layout = instr_constr->GetVectorLayout();
    auto lsop_struct = mpStructure->CastOperandStructure<LoadStoreOperandStructure>();

    ConstraintSet target_addr_constr;
    uint32 offset_base = 1 << (vec_layout->mElemSize - 1);
    BaseOffsetConstraint base_offset_constr(offset_base, vec_layout->mElemSize, 0, MAX_UINT64, true);
    base_offset_constr.GetConstraint(baseVal, lsop_struct->DataSize(), nullptr, target_addr_constr);

    for (uint32 elem_index = 1; elem_index < vec_layout->mElemCount; elem_index++) {
      uint64 target_addr = va_gen.GenerateAddress(alignment, lsop_struct->DataSize(), false, page_req->MemoryAccessType(), &target_addr_constr);
      rIndexElemValues.push_back(target_addr - baseVal);
    }
  }

  void SignedImmediateOperand::SetValue(uint64 val)
  {
    mValue = val & mpStructure->mMask;
  }

  const string SignedImmediateOperand::AssemblyText() const
  {
    char print_buffer[32];
    uint32 value_extend = sign_extend32(mValue,Size());
    snprintf(print_buffer, 32, "%d", int32(value_extend));
    return print_buffer;
  }

  void ImpliedRegisterOperand::Generate(Generator& gen, Instruction& instr)
  {
    // check if register has necessary access attributes
    // Allowing for NoRestriction case
    if (not instr.NoRestriction()) {
      if (mpOperandConstraint->HasConstraint()) {
        auto constr_set = mpOperandConstraint->GetConstraint();
        if (not constr_set->IsEmpty()) {
          auto implied_constr = mpOperandConstraint->CastInstance<ImpliedRegisterOperandConstraint>();
          uint64 only_value = constr_set->OnlyValue();
          if (only_value != implied_constr->RegisterIndex()) {
            LOG(fail) << "{ImpliedRegisterOperand::Generate} only value in constraint-set: 0x" << hex << only_value << " not expected value: 0x" << implied_constr->RegisterIndex() << endl;
            FAIL("incorrect-constraint-for-implied-register");
          }
        }
        else {
          stringstream err_stream;
          err_stream << "Operand \"" << Name() << "\" failed to generate; since the implied register is reserved.";
          throw OperandError(err_stream.str());
        }
      }
    }
    mChoiceText = Name();
    // TODO, also set value to register index.
  }

  OperandConstraint* ImpliedRegisterOperand::InstantiateOperandConstraint() const
  {
    return new ImpliedRegisterOperandConstraint();
  }

  void AluImmediateOperand::Generate(Generator& gen, Instruction& instr)
  {
    auto alu_opr_constr = mpOperandConstraint->CastInstance<AluImmediateOperandConstraint>();

    if (alu_opr_constr->ResultType() == 0 && !instr.NoRestriction()) {
      // Choice is to generate load/store address.

      // Alignment is not critical for generating the ALU instruction, but specifying an alignment value that is usable
      // by most load/store instructions will improve dependency generation.
      uint64 alignment = 16;

      AddressingMode* addr_mode_template = GetAddressingMode(alignment);
      std::unique_ptr<AddressSolver> addr_solver(GetAddressSolver(addr_mode_template, alignment));
      const AddressingMode* addr_mode = addr_solver->Solve(gen, instr, alignment, false);
      if (addr_mode) {
        mTargetAddress = addr_mode->TargetAddress();
        addr_solver->SetOperandResults();

        LOG(notice) << "{AluImmediateOperand::Generate} instruction: " << instr.FullName() << " addressing-mode: " << addr_mode_template->Type() << " target address: 0x" << hex << mTargetAddress << endl;
      }
      else {
        // If no solution is possible, we can fall back to generating random data.
        GroupOperand::Generate(gen, instr);
      }
    }
    else {
      // Choice is to generate random data.
      GroupOperand::Generate(gen, instr);
    }
  }

  AddressingMode* AluImmediateOperand::GetAddressingMode(uint64 alignment) const
  {
    auto alu_opr_struct = mpStructure->CastOperandStructure<AluOperandStructure>();
    return new AluImmediateMode(alu_opr_struct->OperationType());
  }

  OperandConstraint* AluImmediateOperand::InstantiateOperandConstraint() const
  {
    return new AluImmediateOperandConstraint();
  }

  void DataProcessingOperand::Generate(Generator& gen, Instruction& instr)
  {
    auto data_proc_opr_constr = mpOperandConstraint->CastInstance<DataProcessingOperandConstraint>();

    if (data_proc_opr_constr->ResultType() == 0 && !instr.NoRestriction()) {
      // Choice is to generate load/store address.

      // Alignment is not critical for generating the data processing instruction, but specifying an alignment value
      // that is usable by most load/store instructions will improve dependency generation.
      uint64 alignment = 16;

      AddressingMode* addr_mode_template = GetAddressingMode(alignment);
      unique_ptr<AddressSolver> addr_solver(GetAddressSolver(addr_mode_template, alignment));

      const AddressingMode* addr_mode = addr_solver->SolveMultiRegister(gen, instr, alignment, false);

      if (addr_mode) {
        mTargetAddress = addr_mode->TargetAddress();
        addr_solver->SetOperandResults();

        LOG(notice) << "{DataProcessingOperand::Generate} instruction: " << instr.FullName() << " addressing-mode: " << addr_mode_template->Type() << " target address: 0x" << hex << mTargetAddress << endl;
      }
      else {
        // If no solution is possible, we can fall back to generating random data.
        GroupOperand::Generate(gen, instr);
      }
    }
    else {
      // Choice is to generate random data.
      GroupOperand::Generate(gen, instr);
    }
  }

  AddressingMode* DataProcessingOperand::GetAddressingMode(uint64 alignment) const
  {
    auto data_proc_opr_struct = mpStructure->CastOperandStructure<DataProcessingOperandStructure>();
    return new DataProcessingMode(*data_proc_opr_struct, GetSubOperands());
  }

  OperandConstraint* DataProcessingOperand::InstantiateOperandConstraint() const
  {
    return new DataProcessingOperandConstraint();
  }

}
