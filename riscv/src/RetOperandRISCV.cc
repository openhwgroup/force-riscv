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
#include <RetOperandRISCV.h>
#include <InstructionConstraintRISCV.h>
#include <UtilityFunctionsRISCV.h>
#include <InstructionConstraint.h>
#include <Instruction.h>
#include <Choices.h>
#include <UtilityFunctions.h>
#include <InstructionStructure.h>
#include <GenExceptionAgent.h>
#include <Generator.h>
#include <AddressTable.h>
#include <AddressTableManager.h>
#include <VaGenerator.h>
#include <VmManager.h>
#include <GenRequest.h>
#include <Constraint.h>
#include <Register.h>
#include <AddressTagging.h>
#include <ChoicesFilter.h>
#include <Random.h>
#include <DataStation.h>
#include <PeStateUpdate.h>
#include <GenException.h>
#include <VmInfo.h>
#include <VmMapper.h>
#include <ChoicesModerator.h>
#include <OperandRequest.h>
#include <VmUtils.h>
#include <MemoryManager.h>
#include <RegisterReload.h>
#include <ExceptionManager.h>
#include <PageRequestRegulator.h>
#include <Log.h>

#include <memory>

using namespace std;

/*!
  \file RetOperandRISCV.cc
  \brief Code supporting RISCV ret operand generation
*/

namespace Force {

  RetOperandConstraint::RetOperandConstraint()
      : ChoicesOperandConstraint(), mPrivilegeLevel(3), mUndefined(false), mIllegalReturn(false),
        mpTargetAddressConstraint(nullptr), mpTargetStatesConstraint(nullptr), mPreambleSequence(""),
        mEpcName(""), mStatusName(""), mPpName(""), mPieName(""), mPEUpdateId(0), mTargetPrivilege(0),
        mReloadRegisters()
  { }

  void RetOperandConstraint::Setup(const Generator& gen, const Instruction& instr, const OperandStructure& operandStruct)
  {
    ChoicesOperandConstraint::Setup(gen, instr, operandStruct);

    mPreambleSequence = gen.GetVariable("Ret Preamble Sequence Class", EVariableType::String);
    if (HasPreambleSequence()) {
      LOG(notice) << "{RetOperandConstraint::Setup} Preamble sequence is specified on front-end, ignore all overrides" << endl;
      return;
    }

    auto instr_req = instr.GetInstructionConstraint()->InstructionRequest();

    auto priv_req = instr_req->FindOperandRequest("priv");
    if (nullptr != priv_req)
    {
      mpTargetStatesConstraint = priv_req->GetValueConstraint();
      LOG(trace) << "{RetOperandConstraint::Setup} priv constraint provided: " << mpTargetStatesConstraint->ToSimpleString() << endl;
      priv_req->SetApplied();
    }

    auto epc_req = instr_req->FindOperandRequest("epc");
    if (nullptr != epc_req)
    {
      mpTargetAddressConstraint = epc_req->GetValueConstraint();
      LOG(trace) << "{RetOperandConstraint::Setup} epc constraint provided: " << mpTargetAddressConstraint->ToSimpleString() << endl;
      epc_req->SetApplied();
    }

    auto record_req = instr_req->FindOperandRequest("PeUpdateId");
    if (nullptr != record_req)
    {
      auto constr_set = record_req->GetValueConstraint();
      uint64 value = constr_set->ChooseValue();
      LOG(trace) << "{RetOperandConstraint:Setup} PeUpdateId value:0x" << hex << value << endl;
      mPEUpdateId = value;
      record_req->SetApplied();
    }

    mPrivilegeLevel = gen.PrivilegeLevel();
    std::string priv_prefix_lower, priv_prefix_upper;
    privilege_prefix(priv_prefix_lower, mPrivilegeLevel, false);
    privilege_prefix(priv_prefix_upper, mPrivilegeLevel, true);

    mEpcName = priv_prefix_lower + "epc";
    mStatusName = priv_prefix_lower + "status";
    mPpName = priv_prefix_upper + "PP";
    mPieName = priv_prefix_upper + "PIE";

    LOG(debug) << "{RetOperandConstraint::Setup} priv level=" << mPrivilegeLevel << " prefix=" << priv_prefix_lower << endl;

    if (priv_req == nullptr)
    {
      uint32 value = mpChoiceTree->Choose()->Value();
      std::string name = mpChoiceTree->Choose()->Name();
      LOG(trace) << "{RetOperandConstraint::Setup} choose name: " << name << " with value:" << value << endl;
      mIllegalReturn = (value == 1) ? true : false;
    }
  }

  void RetOperandConstraint::SetReturnStates(uint32 targetPriv)
  {
    bool is_illegal_return = illegal_exception_return(targetPriv, mPrivilegeLevel);

    if (mIllegalReturn != is_illegal_return)
    {
      LOG(notice) << "{RetOperandRISCV::SetReturnStates} Using user override reset illegal state:" << is_illegal_return << endl;
      mIllegalReturn = is_illegal_return;
    }

    mTargetPrivilege = (mIllegalReturn) ? mPrivilegeLevel : targetPriv;
  }

  OperandConstraint* RetOperand::InstantiateOperandConstraint() const
  {
    return new RetOperandConstraint();
  }

  void RetOperand::Setup(Generator& gen, Instruction& instr)
  {
    if (instr.NoRestriction()) {
      return;
    }

    ChoicesOperand::Setup(gen, instr);
  }

  void RetOperand::Generate(Generator& gen, Instruction& instr)
  {
    if (instr.NoRestriction()) return;

    auto ret_constr = mpOperandConstraint->CastInstance<RetOperandConstraint>();
    if (ret_constr->HasPreambleSequence())
    {
      LOG(info) << "{RetOperandRISCV::Generate} has preamble sequence, skipped" << endl;
      return;
    }

    ChoicesOperand::Generate(gen, instr);

    if (ret_constr->Undefined()) return;

    GenerateTargetStates(gen, instr);
    GenerateTargetAddress(gen, instr);
    GenerateOthers(gen, instr);
  }

  void RetOperand::GenerateTargetStates(const Generator& gen, Instruction& instr)
  {
    auto ret_constr = mpOperandConstraint->CastInstance<RetOperandConstraint>();
    auto priv_constr = ret_constr->TargetStatesConstraint();

    uint32 pp_val = 0;

    if (nullptr != priv_constr)
    {
      pp_val = priv_constr->ChooseValue();
      LOG(debug) << "{RetOperandRISCV::GenerateTargetStates} Using user override privilege target: 0x" << hex << pp_val << endl;
    }
    else
    {
      //generate *pp value
      if (ret_constr->IllegalReturn())
      {
        LOG(fail) << "{RetOperandRISCV::GenerateTargetStates} illegal return generation not implemented" << endl;
        FAIL("gen_target_states_illegal_return_unimpl");
      }
      else
      {
        auto general_choices_ptr = gen.GetChoicesModerator(EChoicesType::GeneralChoices);
        std::unique_ptr<ChoiceTree> choices_tree(general_choices_ptr->CloneChoiceTree("Privilege Switch - Target Privilege"));

        uint32 curr_priv = ret_constr->PrivilegeLevel();
        ConstraintSet constr_set(0, curr_priv);
        const ConstraintChoicesFilter choices_filter(&constr_set);
        choices_tree.get()->ApplyFilter(choices_filter);
        pp_val = (uint32)choices_tree.get()->Choose()->Value();
        LOG(notice) << "{RetOperandRISCV::GenerateTargetStates} random privilege target: 0x" << hex << pp_val << endl;
      }
    }

    auto reg_file = gen.GetRegisterFile();
    auto status_reg = reg_file->RegisterLookup(ret_constr->StatusName());
    auto pp_field = status_reg->RegisterFieldLookup(ret_constr->PpName());

    uint64 status_val = status_reg->Value();
    uint64 pp_mask = pp_field->GetPhysicalRegisterMask();
    uint32 pp_shift = pp_field->Lsb();
    LOG(trace) << "{RetOperandRISCV::GenerateTargetStates} pp_mask=0x" << hex << pp_mask << " status_val=0x" << status_val << endl;
    status_val = (status_val & ~pp_mask) | (pp_val << pp_shift);
    ret_constr->SetReturnStates(pp_val);
    ret_constr->AddReloadRegister(ret_constr->StatusName(), status_val);
  }

  bool RetOperand::ValidateTargetVm(const VmMapper* target_mapper)
  {
    // check whether the target VmMapper has a good register context
    string err_msg;
    if (not target_mapper->ValidateContext(err_msg)) {
      stringstream err_stream;
      err_stream << "{RetOperandRISCV::ValidateTargetVm} ret has bad context register initialization " << err_msg << endl;
      throw OperandError(err_stream.str());
    }

    auto regs_reload = target_mapper->GetRegisterReload();
    if (nullptr != regs_reload)
    {
      bool res = regs_reload->Validate();
      LOG(notice) << "{RetOperandRISCV::ValidateTargetVm} validation result " << res << endl;
      if (res) return true;
    }
    else
    {
      LOG(notice) << "{RetOperandRISCV::ValidateTargetVm} Register reload pointer not available!" << endl;
      return true;
    }

    return false;
  }

  void RetOperand::GenerateTargetAddress(Generator& gen, Instruction& instr)
  {
    RetOperandConstraint* ret_constr = mpOperandConstraint->CastInstance<RetOperandConstraint>();

    auto vm_manager = gen.GetVmManager();
    std::unique_ptr<VmInfo> vm_info_storage(vm_manager->VmInfoInstance());
    vm_info_storage.get()->SetPrivilegeLevel(ret_constr->TargetPrivilege());
    vm_info_storage.get()->GetOtherStates(gen);

    LOG(info) << "{RetOperandRISCV::GenerateTargetAddress} target VM info: " << vm_info_storage.get()->ToString() << endl;
    auto target_mapper = vm_manager->GetVmMapper(*vm_info_storage.get());
    target_mapper->Initialize();
    ValidateTargetVm(target_mapper);

    uint64 branch_addr = 0;
    auto epc_constr = ret_constr->TargetAddressConstraint();
    if (nullptr != epc_constr)
    {
      branch_addr = epc_constr->ChooseValue();
      LOG(info) << "{RetOperandRISCV::GenerateTargetAddress} using user branch address: 0x" << hex << branch_addr << endl;
      ret_constr->AddReloadRegister(ret_constr->EpcName(), branch_addr);
      return;
    }

    std::unique_ptr<GenPageRequest> page_req_storage(gen.GenPageRequestInstance(true, EMemAccessType::Unknown));
    page_req_storage.get()->SetPrivilegeLevel(EPrivilegeLevelType(ret_constr->TargetPrivilege()));
    gen.GetPageRequestRegulator()->RegulateBranchPageRequest(target_mapper, nullptr, page_req_storage.get());
    VaGenerator va_gen(target_mapper, page_req_storage.get());
    branch_addr = va_gen.GenerateAddress(4, 64, true, EMemAccessType::Unknown);

    LOG(info) << "{RetOperandRISCV::GenerateTargetAddress} random branch address: 0x" << hex << branch_addr << endl;
    ret_constr->AddReloadRegister(ret_constr->EpcName(), branch_addr);
  }

  void RetOperand::GenerateOthers(const Generator& gen, Instruction& instr)
  {
    RetOperandConstraint* ret_constr = mpOperandConstraint->CastInstance<RetOperandConstraint>();

    // update pe state.
    if (ret_constr->PEUpdateId() == 0) {
      PeStateUpdate* pe_state = new PeStateUpdate();
      if (not gen.HasISS()) {
        pe_state->UpdateState("PrivilegeLevel", "Set", (uint64)(ret_constr->TargetPrivilege()));
        pe_state->UpdateRegisterField(ret_constr->StatusName(), ret_constr->PpName(), (uint64)(ret_constr->TargetPrivilege()));
      }
      auto reload_registers = ret_constr->ReloadRegisters();
      auto epc_iter = reload_registers.find(ret_constr->EpcName());
      pe_state->UpdateState("PC", "Set", GetEffectiveTargetPc(gen, epc_iter->second));
      uint64 update_id = DataStation::Instance()->Add(pe_state);
      ret_constr->SetPEUpdateId(update_id);
      LOG(info) << "{RetOperandRISCV::GenerateOthers} set PeStateUpdate Update ID :" << update_id << endl;
    }

    // RET preamble
    // access the address table value to load into the register along with the register index
    // add the necessary request to generate the instructions during the RET preamble to change the value inside that register
    map<string, uint64> reload_regs;
    auto addr_table_manager = gen.GetAddressTableManager();
    addr_table_manager->GetReloadRegisters(0, ret_constr->TargetPrivilege(), reload_regs); //mem bank as uint32 - default == 0
    for (auto item : reload_regs) {
      ret_constr->AddReloadRegister(item.first, item.second);
    }
  }

  void RetOperand::Commit(Generator& gen, Instruction& instr)
  {
    LOG(notice) << "{RetOperandRISCV::Committing...}" << endl;
    if (instr.NoRestriction()) {
      return;
    }

    LOG(notice) << "{RetOperandRISCV::Commit}" << endl;
    ChoicesOperand::Commit(gen, instr);

    auto ret_constr = mpOperandConstraint->CastInstance<RetOperandConstraint>();

    if (ret_constr->Undefined() or ret_constr->HasPreambleSequence()) {
      return;   // no further process needed
    }

    // Update PEState
    uint64 pe_rec_id = ret_constr->PEUpdateId();
    if (pe_rec_id > 0) {
      GenRequest* update_request = new GenPeStateUpdateRequest(pe_rec_id);
      gen.PrependRequest(update_request);
    }
  }

  bool RetOperand::GetPrePostAmbleRequests(Generator& gen) const
  {
    auto ret_constr = mpOperandConstraint->CastInstance<RetOperandConstraint>();

    if (ret_constr->Undefined()) { // undefined RET when priv = U, no extra request need to be issued
      return false;
    }
    if (ret_constr->HasPreambleSequence()) {
      return ProcessPreambleSequence(gen);
    }

    auto reload_registers = ret_constr->ReloadRegisters();
    gen.AddLoadSysRegistersAmbleRequests(reload_registers);
    return true;
  }

  bool RetOperand::ProcessPreambleSequence(Generator& gen) const
  {
    auto ret_constr = mpOperandConstraint->CastInstance<RetOperandConstraint>();
    GenRequest* eret_request = new GenCallBackEretRequest(ret_constr->PreambleSequence());
    gen.AddPreambleRequest(eret_request);
    return true;
  }

  uint64 RetOperand::GetEffectiveTargetPc(const Generator& rGen, cuint64 epcVal) const
  {
    uint64 effective_target_pc = epcVal;

    const RegisterFile* reg_file = rGen.GetRegisterFile();
    Register* misa_reg = reg_file->RegisterLookup("misa");
    RegisterField* c_field = misa_reg->RegisterFieldLookup("C");
    if (c_field->FieldValue() == 0x1) {
      effective_target_pc = get_aligned_value(effective_target_pc, 2);
    }
    else {
      effective_target_pc = get_aligned_value(effective_target_pc, 4);
    }

    return effective_target_pc;
  }

}
