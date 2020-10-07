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
#include <GenSequenceAgentRISCV.h>
#include <Register.h>
#include <GenRequest.h>
#include <Generator.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <UtilityFunctions.h>
#include <BootOrder.h>
#include <InitialSetupRISCV.h>
#include <Log.h>

#include <Config.h>
#include <Choices.h>
#include <ChoicesModerator.h>
#include <memory>
#include <algorithm>
#include <BaseOffsetConstraint.h>
#include <GenExceptionAgent.h>
#include <AddressTableManager.h>
#include <State.h>
#include <RecoveryAddressGeneratorRISCV.h>

using namespace std;

namespace Force {

  Object* GenSequenceAgentRISCV::Clone() const
  {
    return new GenSequenceAgentRISCV(*this);
  }

  void GenSequenceAgentRISCV::GetBootLoadRegisterRequests(BootOrder& rBootOrder, vector<GenRequest*>& rLoadRegisterRequests) const
  {
    auto state = new State(mpGenerator);
    rBootOrder.CreateStateElements(state);

    rLoadRegisterRequests.push_back(new GenOneTimeStateTransitionRequest(state, EStateTransitionType::Boot, EStateTransitionOrderMode::UseDefault, {}));
  }

  // This method first computes the parameter values required to load the lower bits, if necessary,
  // then generates instructions to load the top 32 bits. After that, it uses the parameter values
  // initially computed to generate the instructions to load the remaining bits. The maximum number
  // of instructions this method generates should be 8. The method attempts use larger shift amounts
  // and avoids adding 0 to reduce the number of instructions required to load values with few set
  // bits.
  void GenSequenceAgentRISCV::GetLoadGPRSequence(const Register* regPtr, uint64 loadValue, vector<GenRequest* >& reqSeq)
  {
    // << "Loading register " << regPtr->Name() << " with 0x" << hex << loadValue << " size=" << dec << regPtr->Size() << " index=" << regPtr->IndexValue() << endl;

    if (regPtr->Size() == 32) {
      GetLoadGPR32BitSequence(regPtr, loadValue, reqSeq);
      return;
    }
    
    if ((loadValue & 0xFFFFFFFFFFFFF800) == 0xFFFFFFFFFFFFF800) {
        return GetLoadGPRTop53BitsSetSequence(regPtr, loadValue, reqSeq);
    }

    const char* addi_name = "ADDI##RISCV";
    const char* slli_name = "SLLI#RV64I#RISCV";
    const char* dest_opr = "rd";

    // Initialize list of shift amounts and ADDI immediate parameters
    vector<pair<uint32, uint32>> imm_params;

    uint64 value = loadValue;
    while (value > MAX_UINT32) {
      uint32 bottom_12_bits = value & 0xFFF;
      uint64 top_52_bits = (value + 0x800) >> 12;

      uint32 lsb = lowest_bit_set(top_52_bits);
      uint32 shift_amount = lsb + 12;
      imm_params.emplace_back(shift_amount, bottom_12_bits);

      // Set value equal to the remaining significant bits
      value = top_52_bits >> lsb;
    }

    GetLoadGPR32BitSequence(regPtr, value, reqSeq);

    for (auto itr = imm_params.rbegin(); itr != imm_params.rend(); ++itr) {
      auto slli_req = new GenInstructionRequest(slli_name);
      slli_req->AddOperandRequest(dest_opr, regPtr->IndexValue());
      slli_req->AddOperandRequest("rs1", regPtr->IndexValue());
      slli_req->AddOperandRequest("shamt", itr->first);
      reqSeq.push_back(slli_req);

      if (itr->second) {
        auto addi_req = new GenInstructionRequest(addi_name);
        addi_req->AddOperandRequest(dest_opr, regPtr->IndexValue());
        addi_req->AddOperandRequest("rs1", regPtr->IndexValue());
        addi_req->AddOperandRequest("simm12", itr->second);
        reqSeq.push_back(addi_req);
      }
    }

    // Due to limitations in the RISC-V architecture, GetLoadGPR32BitSequence() is forced to use
    // instructions that sign-extend the result to 64 bits. This yields the wrong value when the
    // argument passed to GetLoadGPR32BitSequence() has Bit 31 as the most significant bit. We
    // remove the sign extension by shifting.
    if ((value != 0) and (highest_bit_set(value) == 31) and (highest_bit_set(loadValue) != 63)) {
      uint32 shift_amount = 63 - highest_bit_set(loadValue);
      auto slli_req = new GenInstructionRequest(slli_name);
      slli_req->AddOperandRequest(dest_opr, regPtr->IndexValue());
      slli_req->AddOperandRequest("rs1", regPtr->IndexValue());
      slli_req->AddOperandRequest("shamt", shift_amount);
      reqSeq.push_back(slli_req);

      const char* srli_name = "SRLI#RV64I#RISCV";
      auto srli_req = new GenInstructionRequest(srli_name);
      srli_req->AddOperandRequest(dest_opr, regPtr->IndexValue());
      srli_req->AddOperandRequest("rs1", regPtr->IndexValue());
      srli_req->AddOperandRequest("shamt", shift_amount);
      reqSeq.push_back(srli_req);
    }
  }

  Register* GenSequenceAgentRISCV::GetBootLoadingGPR() const
  {
    return GetRandomGPR('x');
  }

  Register* GenSequenceAgentRISCV::GetRandomGPR(char regPrefix) const
  {
    uint32 random_index = mpGenerator->GetRandomRegister(ERegisterType::GPR, "0,1,2"); // getting a random GPR, excluding register indices 0, 1 and 2.
    char print_buffer[16];
    snprintf(print_buffer, 16, "%c%d", regPrefix, random_index);
    auto reg_file = mpGenerator->GetRegisterFile();
    auto gpr_ptr = reg_file->RegisterLookup(print_buffer);

    return gpr_ptr;
  }

  void GenSequenceAgentRISCV::GetLoadSysRegSequence(const Register* regPtr, uint64 loadValue, vector<GenRequest* >& reqSeq, const Register* gprPtr)
  {
    const char* instr_name = "CSRRW#register#RISCV";
    const char* src_opr = "rs1";
    const char* dest_opr = "rd";
    const char* sysreg_opr = "csr";

    uint32 gpr_index;
    if (gprPtr == nullptr)
    {
      gpr_index = mpGenerator->GetRandomRegister(ERegisterType::GPR, "0");
      char reg_prefix = 'x';
      char print_buffer[16];
      snprintf(print_buffer, 16, "%c%d", reg_prefix, gpr_index);

      auto reg_file = mpGenerator->GetRegisterFile();
      auto gpr_ptr = reg_file->RegisterLookup(print_buffer);

      GetLoadGPRSequence(gpr_ptr, loadValue, reqSeq);
    }
    else
    {
      GetLoadGPRSequence(gprPtr, loadValue, reqSeq);
      gpr_index = gprPtr->IndexValue();
    }

    auto mov_sysreg_req = new GenInstructionRequest(instr_name);
    mov_sysreg_req->AddOperandRequest(src_opr, gpr_index);
    mov_sysreg_req->AddOperandRequest(dest_opr, 0); // invokes a write but not read to CSR when rd==x0
    mov_sysreg_req->AddOperandRequest(sysreg_opr, regPtr->IndexValue());
    reqSeq.push_back(mov_sysreg_req);
  }

  void GenSequenceAgentRISCV::GetLoadFPRSequence(const Register* regPtr, uint64 loadValue, vector<GenRequest* >& reqSeq, const Register* gprPtr)
  {
    char reg_precision = regPtr->Name().at(0);
    char reg_prec_instr_char = 'W'; // TODO may want default to be D
    switch (reg_precision)
    {
      case 'S':
        reg_prec_instr_char = 'W';
        break;
      case 'D':
        reg_prec_instr_char = 'D';
        break;
      case 'Q':
        //TODO no FMV.Q.X in RV64 - need way to load lower/upper halves of 128 bit register via 1 or more instructions
        LOG(notice) << "{GenSequenceAgentRISCV::GetLoadFPRSequence} using FMV.D.X for Quad Precision instr" << endl;
        reg_prec_instr_char = 'D';
        break;
      default:
        LOG(fail) << "{GenSequenceAgentRISCV::GetLoadFPRSequence} no valid FPR prefix detected for reg=" << regPtr->Name() << endl;
        FAIL("load-fpr-invalid-dest-reg");

    }

    char instr_name[15];
    snprintf(instr_name, 15, "%s%c%s", "FMV.", reg_prec_instr_char, ".X##RISCV");
    const char* src_opr = "rs1";
    const char* dest_opr = "rd";

    uint32 gpr_index;
    if (gprPtr == nullptr)
    {
      gpr_index = mpGenerator->GetRandomRegister(ERegisterType::GPR, "0");
      char reg_prefix = 'x';
      char print_buffer[16];
      snprintf(print_buffer, 16, "%c%d", reg_prefix, gpr_index);

      auto reg_file = mpGenerator->GetRegisterFile();
      auto gpr_ptr = reg_file->RegisterLookup(print_buffer);

      GetLoadGPRSequence(gpr_ptr, loadValue, reqSeq);
    }
    else
    {
      GetLoadGPRSequence(gprPtr, loadValue, reqSeq);
      gpr_index = gprPtr->IndexValue();
    }

    auto fmv_req = new GenInstructionRequest(instr_name);
    fmv_req->AddOperandRequest(src_opr, gpr_index);
    fmv_req->AddOperandRequest(dest_opr, regPtr->IndexValue());
    reqSeq.push_back(fmv_req);
  }

  void GenSequenceAgentRISCV::GetLoadVecRegSequence(const Register* regPtr, uint64 loadValue, vector<GenRequest*>& reqSeq, const Register* gprPtr)
  {
    const char* instr_name = "VL1R.V##RISCV"; //TODO: build each instruction -> "%s%d%s", "VL", nf, "R.V##RISCV"
    const char* src_opr = "rs1";
    const char* dest_opr = "vd";

    uint32 gpr_index;
    if (gprPtr == nullptr)
    {
      gpr_index = mpGenerator->GetRandomRegister(ERegisterType::GPR, "0");
      char reg_prefix = 'v';
      char print_buffer[16];
      snprintf(print_buffer, 16, "%c%d", reg_prefix, gpr_index);

      auto reg_file = mpGenerator->GetRegisterFile();
      auto gpr_ptr = reg_file->RegisterLookup(print_buffer);

      GetLoadGPRSequence(gpr_ptr, loadValue, reqSeq);
    }
    else
    {
      GetLoadGPRSequence(gprPtr, loadValue, reqSeq);
      gpr_index = gprPtr->IndexValue();
    }

    auto vl_req = new GenInstructionRequest(instr_name);
    vl_req->AddOperandRequest(src_opr, gpr_index);
    vl_req->AddOperandRequest(dest_opr, regPtr->IndexValue());
    vl_req->AddDetail("NoRestriction", 1);
    reqSeq.push_back(vl_req);
  }

  void GenSequenceAgentRISCV::GetLoadArchRegisterSequence(const Register* regPtr, uint64 loadValue, vector<GenRequest* >& reqSeq, const Register* interRegPtr)
  {
    ERegisterType reg_type = regPtr->RegisterType();
    switch (reg_type)
    {
      case ERegisterType::SysReg:
        GetLoadSysRegSequence(regPtr, loadValue, reqSeq, interRegPtr);
        break;
      case ERegisterType::ZR:
        break;
      case ERegisterType::FPR:
        GetLoadFPRSequence(regPtr, loadValue, reqSeq, interRegPtr);
        break;
      case ERegisterType::VECREG:
        GetLoadVecRegSequence(regPtr, loadValue, reqSeq, interRegPtr);
        break;
      default:
        LOG(fail) << "{GenSequenceAgentRISCV::GetLoadArchRegisterSequence} not yet supported register type: " << ERegisterType_to_string(reg_type) << endl;
        FAIL("unsupported-arch-register-type");
    }
  }

  void GenSequenceAgentRISCV::GetStore64BitRegisterSequence(const Register* pSrcRegister, Register* pBaseRegister , vector<GenRequest* >& reqSeq)
  {
    const char* instr_name = "SD##RISCV";
    const char* src_opr = "rs2";
    const char* base_opr = "rs1";

    auto str_req = new GenInstructionRequest(instr_name);
    str_req->AddDetail("NoRestriction", 1);
    str_req->AddOperandRequest(src_opr, pSrcRegister->IndexValue());
    str_req->AddOperandRequest(base_opr, pBaseRegister->IndexValue());
    str_req->AddOperandRequest("simm12", 0);
    reqSeq.push_back(str_req);
  }

  void GenSequenceAgentRISCV::GetBranchToSelfSequence(vector<GenRequest* >& req_seq)
  {
    const char* j_instr = "JAL##RISCV";
    auto j_req = new GenInstructionRequest(j_instr);
    j_req->AddDetail("NoRestriction", 1);
    j_req->AddDetail("NoBnt", 1);
    j_req->AddOperandRequest("rd", 0);
    j_req->AddOperandRequest("simm20", 0);
    req_seq.push_back(j_req);
  }

  void GenSequenceAgentRISCV::EndOfTest()
  {
    vector<GenRequest* > req_seq;
    GetBranchToSelfSequence(req_seq);
    mpGenerator->PrependRequests(req_seq);
  }

  void GenSequenceAgentRISCV::BranchToTarget()
  {
    auto br_req = dynamic_cast<GenBranchToTarget* >(mpSequenceRequest);
    uint64 current_pc = mpGenerator->PC();
    uint64 target_pc = br_req->BranchTarget();
    bool near_valid = false;
    uint64 offset_value = int64(target_pc - current_pc) >> 1; // use signed number so the sign extension works after shifting.
    uint64 opr_offset = get_offset_field(offset_value, 20, &near_valid); // get the 20-bit offset and check if the offset is valid form.
    // << "{BranchToTarget} current PC 0x" << hex << current_pc << " target PC 0x" << target_pc << " offset 0x" << offset_value << " near_valid? " << near_valid << endl;
    if ((not near_valid) && br_req->NearBranch()) {
      LOG(fail) << "{GenSequenceAgentRISCV::BranchToTarget} need to use a near branch to jump from 0x" << hex << current_pc << " to 0x" << target_pc << " but cannot reach it." << endl;
      FAIL("failed-needing-near-branch");
    }
    bool noBnt = br_req->NoBnt();
    if (near_valid) {
      const char* b_instr = "JAL##RISCV";
      auto b_req = new GenInstructionRequest(b_instr);
      b_req->AddDetail("NoRestriction", 1);
      if (noBnt)
        b_req->AddDetail("NoBnt", 1);
      b_req->AddOperandRequest("rd", 0); //Need to set rd because this instruction modifies a register, so that update needs to be discarded.
      b_req->AddOperandRequest("simm20", opr_offset);
      mpGenerator->PrependRequest(b_req);
    }
    else {
      auto gpr_ptr = GetRandomGPR('x'); // getting a random GPR

      vector<GenRequest* > branch_seq;
      GetLoadGPRSequence(gpr_ptr, target_pc, branch_seq);

      ERegAttrType reserv_acc = ERegAttrType::Write;
      mpGenerator->ReserveRegister(gpr_ptr->Name(), reserv_acc); // reserve the GPR to avoid being corrupted by others.

      const char* jalr_instr = "JALR##RISCV";
      auto instr_req = new GenInstructionRequest(jalr_instr);
      instr_req->AddOperandRequest("rd", 0); //Need to set rd because this instruction modifies a register, so that update needs to be discarded.
      instr_req->AddOperandRequest("rs1", gpr_ptr->IndexValue());
      instr_req->AddOperandRequest("simm12", 0);
      instr_req->AddDetail("NoRestriction", 1);
      instr_req->AddDetail("BRTarget", target_pc);
      if (noBnt) {
        instr_req->AddDetail("NoBnt", 1);
      }

      branch_seq.push_back(instr_req);

      branch_seq.push_back(new GenRegisterReservation(gpr_ptr->Name(), false, reserv_acc)); // restore the register reservation.
      mpGenerator->PrependRequests(branch_seq);
    }
  }

  void GenSequenceAgentRISCV::InitialSetup()
  {
    InitialSetupRISCV initial_setup(mpGenerator);
    initial_setup.Process();
  }

  void GenSequenceAgentRISCV::RestoreArchBootStates()
  {
    auto reg_file = mpGenerator->GetRegisterFile();

    auto misa_reg = reg_file->RegisterLookup("misa");
    uint64 misa_reg_val = misa_reg->InitialValue();
    misa_reg->SetValue(misa_reg_val);

    auto mstatus_reg = reg_file->RegisterLookup("mstatus");
    uint64 mstatus_reg_val = mstatus_reg->InitialValue();
    mstatus_reg->SetValue(mstatus_reg_val);

    auto fcsr_reg = reg_file->RegisterLookup("fcsr");
    uint64 fcsr_reg_val = fcsr_reg->InitialValue();
    fcsr_reg->SetValue(fcsr_reg_val);
    LOG(notice) << "{GenSequenceAgentRISCV::RestoreArchBootStates} misa=0x" << hex << misa_reg_val << " mstatus=0x" << mstatus_reg_val << " fcsr=0x" << fcsr_reg_val << endl;
  }

  void GenSequenceAgentRISCV::JumpToStart()
  {
    // obtain InitialPC value, issue sequence to jump to the target.
    uint64 init_pc = 0;
    if (!mpGenerator->GetStateValue(EGenStateType::InitialPC, init_pc)) 
    {
      LOG(fail) << "{GenSequenceAgentRISCV::JumpToStart} InitialPC not set." << endl;
      FAIL("no-initial-pc-specified");
    }

    bool opt_valid = false;
    uint64 priv_opt = Config::Instance()->GetOptionValue("PrivilegeLevel", opt_valid);

    ChoicesModerator* pChoicesModerator = mpGenerator->GetChoicesModerator(EChoicesType::GeneralChoices);
    if (nullptr == pChoicesModerator)
    {
      LOG(fail) << "{GenSequenceAgentRISCV::JumpToStart} General choices moderator not found" << endl;
      FAIL("choice-moderator-not-found");
    }

    std::unique_ptr<Choice> choices_tree(pChoicesModerator->CloneChoiceTree("Starting jump"));
    auto chosen_ptr = choices_tree->Choose();
    std::string name = chosen_ptr->Name();

    bool use_ret_jts = true;

    if (name == "Branch")
    {
      //can only branch if priv opt not set, or set to M
      if (!opt_valid || priv_opt == 0x3ull) use_ret_jts = false;
    }

    vector<GenRequest*> jump_start_req;

    if (use_ret_jts)
    {
      auto reg_file = mpGenerator->GetRegisterFile();

      //Note: MPP should be configured in initial setup based on priv option, no need to modify here.
      // load mepc
      auto mepc_ptr = reg_file->RegisterLookup("mepc");
      GetLoadSysRegSequence(mepc_ptr, init_pc, jump_start_req, mpGenerator->GetBootOrder()->LastGPRElement()->GetRegister());

      // load MPP to ensure cosim and standalone match each other.
      cuint64 mpp_field_value = opt_valid ? priv_opt : 0x3;
      auto mstatus_reg = reg_file->RegisterLookup("mstatus");
      RegisterField* mpp_rf = mstatus_reg->RegisterFieldLookup({"MPP"});
      cuint64 mpp_masklet = mpp_rf->FieldMask(); 
      cuint32 mpp_offset = mpp_rf->Lsb();
      cuint64 delete_mask = ~(mpp_masklet << mpp_offset);
      cuint64 add_mask = mpp_field_value << mpp_offset;

      uint64 mstatus_reg_val = mstatus_reg->InitialValue() & delete_mask;
      mstatus_reg_val |= add_mask;
      GetLoadSysRegSequence(mstatus_reg, mstatus_reg_val, jump_start_req, mpGenerator->GetBootOrder()->LastGPRElement()->GetRegister());

      // load last register
      mpGenerator->GetBootOrder()->LastRegistersLoadingRequest(jump_start_req, false);

      // RET
      const char* mret_instr = "MRET##RISCV";
      auto mret_req = new GenInstructionRequest(mret_instr);
      mret_req->AddDetail("NoRestriction", 1);
      jump_start_req.push_back(mret_req);

    }
    else
    {
      // load last register(s)
      mpGenerator->GetBootOrder()->LastRegistersLoadingRequest(jump_start_req, true);

      auto br_req = new GenBranchToTarget(init_pc, true, true); // indicate a near branch need to be used, No Bnt
      jump_start_req.push_back(br_req);
    }

    mpGenerator->PrependRequests(jump_start_req);
  }

  void GenSequenceAgentRISCV::GetShortBranchConstraint(uint64 pcValue, ConstraintSet& rPcOffsetConstr)
  {
    uint32 access_size = mpGenerator->InstructionSpace();
    uint32 offset_size = 20;
    uint32 offset_base = (1 << (20 - 1));
    BaseOffsetConstraint base_offset_constr(offset_base, offset_size,  1, MAX_UINT64, true);
    base_offset_constr.GetConstraint(pcValue, access_size, nullptr, rPcOffsetConstr);
    LOG(info) << "{GenSequenceAgentRISCV::GetShortBranchConstraint} PC-relative-branch PC offset constraint: " << rPcOffsetConstr.ToSimpleString() << endl;
  }

  bool GenSequenceAgentRISCV::GetRandomReloadRegisters(uint32 number, std::vector<string >& regsNames) const
  {
    bool status = false;
    vector<uint64> reg_indices;

    do {
      status = mpGenerator->GetRandomRegisters(number--, ERegisterType::GPR, "0", reg_indices);
    } while (status == false and number > 0);

    if (number == 0) {
      LOG(notice) << "{GenSequenceAgentRISCV::GetRandomReloadRegisters} not enough GPR random registers available" << endl;
      return false;
    }

    transform(reg_indices.begin(), reg_indices.end(), back_inserter(regsNames), [](uint64 reg_index) { return "x" + to_string(reg_index); });

    return status;
  }

  void GenSequenceAgentRISCV::GetReloadGPRSequence(const GenReloadRegister* reqPtr, vector<GenRequest* >& reqSeq, const Register* interRegPtr)
  {
    auto reg_file = mpGenerator->GetRegisterFile();

    EReloadingMethodType reload_method = reqPtr->ReloadMethod();
    if (not reqPtr->ReloadMethodForced())
    {
      reload_method = ChooseReloadingMethod();
    }

    auto reload_reqs = reqPtr->ReloadRequests();
    if (reload_reqs.size() > 2)
    {
      LOG(fail) << "{GenSequenceAgenRISCV::GetReloadGPRSequence} Unsupported number of register reloads: " << reload_reqs.size() << endl;
      FAIL("unsupported_reloading_register_number_fail");
    }

    auto reload_reg_iter = reload_reqs.begin();
    switch (reload_method)
    {
      case EReloadingMethodType::Move:
      {
        for ( ; reload_reg_iter != reload_reqs.end(); reload_reg_iter++)
        {
          auto reg_ptr = reg_file->RegisterLookup(reload_reg_iter->first);
          GetReloadGPRUsingMoveSequence(reg_ptr, reload_reg_iter->second, reqSeq);
          if (not mpGenerator->HasISS()) reqSeq.push_back(new GenSetRegister(reg_ptr, reload_reg_iter->second));
        }
        break;
      }
      case EReloadingMethodType::Load:
      {
        auto reg_ptr = reg_file->RegisterLookup(reload_reg_iter->first);
        GetReloadGPRUsingLoadSequence(reg_ptr, reload_reg_iter->second, reqSeq, interRegPtr);
        if (not mpGenerator->HasISS()) reqSeq.push_back(new GenSetRegister(reg_ptr, reload_reg_iter->second));
        break;
      }
      default:
      {
        LOG(fail) << "{GenSequenceAgentRISCV::GetReloadGPRSequence} Unsupported reloading register methods:" << EReloadingMethodType_to_string(reload_method) << endl;
        FAIL("unsupported_reloading_register_methods_fail");
      }
    }
  }

  void GenSequenceAgentRISCV::GetReloadBaseAddressSequence(const Register* interRegPtr, uint32 size, vector<GenRequest* >& reqSeq)
  {
    RecoveryAddressGeneratorRISCV recovery_addr_generator(mpGenerator);
    uint64 addr = recovery_addr_generator.GenerateAddress(16, size, false, EMemAccessType::ReadWrite);
    GetLoadGPRSequence(interRegPtr, addr, reqSeq);
  }

  void GenSequenceAgentRISCV::GetReloadGPRUsingLoadSequence(const Register* regPtr, uint64 loadValue, vector<GenRequest* >& reqSeq, const Register* interRegPtr)
  {
    LOG(notice) << "{GenSequenceAgentRISCV::GetReloadGPRUsingLoadSequence} reload_data=0x" << hex << loadValue << endl;
    const Register* inter_reg_ptr = interRegPtr;

    if (inter_reg_ptr == nullptr) {
      inter_reg_ptr = GetRandomGPR('x'); // getting a random GPR
      GetReloadBaseAddressSequence(inter_reg_ptr, 8, reqSeq);
    }

    const char* ldr_name = "LD##RISCV";
    auto ldr_req = new GenInstructionRequest(ldr_name);
    ldr_req->AddOperandRequest("rd", regPtr->IndexValue());
    ldr_req->AddOperandRequest("rs1", inter_reg_ptr->IndexValue());
    ldr_req->AddDetail("LSData", to_string(loadValue));
    //ldr_req->AddDetail("NoPreamble", 1);

    reqSeq.push_back(ldr_req);
    return;
  }

  void GenSequenceAgentRISCV::GetReloadGPRUsingMoveSequence(const Register* regPtr, uint64 loadValue, std::vector<GenRequest* >& reqSeq)
  {
    GetLoadGPRSequence(regPtr, loadValue, reqSeq);
  }

  void GenSequenceAgentRISCV::InitializeAddrTables()
  {
    auto cast_req = mpSequenceRequest->CastInstance<GenInitializeAddrTablesRequest>();
    uint32 table_index = cast_req->TableIndex();
    auto address_table_manager = mpGenerator->GetAddressTableManager();
    if (cast_req->FastMode())
    {
      address_table_manager->SetFastMode(true);
    }
    address_table_manager->Initialize(table_index);
  }

  void GenSequenceAgentRISCV::RegulateInitRegisters(std::list<Register*>& registers) const
  {
  }

  void GenSequenceAgentRISCV::GetLoadGPRTop53BitsSetSequence(const Register* regPtr, uint32 loadValue, std::vector<GenRequest* >& reqSeq)
  {
    const char* lui_name = "LUI##RISCV";
    const char* dest_opr = "rd";
    auto lui_req = new GenInstructionRequest(lui_name);
    lui_req->AddOperandRequest(dest_opr, regPtr->IndexValue());
    lui_req->AddOperandRequest("simm20", 0);
    reqSeq.push_back(lui_req);

    uint32 imm12_to_load = loadValue & 0xFFF;
    const char* addi_name = "ADDI##RISCV";
    auto addi_req = new GenInstructionRequest(addi_name);
    addi_req->AddOperandRequest(dest_opr, regPtr->IndexValue());
    addi_req->AddOperandRequest("rs1", regPtr->IndexValue());
    addi_req->AddOperandRequest("simm12", imm12_to_load);
    reqSeq.push_back(addi_req);
  }

  void GenSequenceAgentRISCV::GetLoadGPR32BitSequence(const Register* regPtr, uint32 loadValue, std::vector<GenRequest* >& reqSeq)
  {
    const char* lui_name = "LUI##RISCV";
    const char* dest_opr = "rd";

    uint32 lui_value = ((loadValue + 0x800) >> 12) & 0xFFFFF;

    // Always execute the LUI, even when top 20 bits are 0, because it will clear the remaining
    // register bits, as desired.
    auto lui_req = new GenInstructionRequest(lui_name);
    lui_req->AddOperandRequest(dest_opr, regPtr->IndexValue());
    lui_req->AddOperandRequest("simm20", lui_value);
    reqSeq.push_back(lui_req);

    uint32 imm12_to_load = loadValue & 0xFFF;
    if (imm12_to_load) {
      const char* addiw_name = (regPtr->Size() == 32) ? "ADDI##RISCV" : "ADDIW##RISCV";
      auto addiw_req = new GenInstructionRequest(addiw_name);
      addiw_req->AddOperandRequest(dest_opr, regPtr->IndexValue());
      addiw_req->AddOperandRequest("rs1", regPtr->IndexValue());
      addiw_req->AddOperandRequest("simm12", imm12_to_load);
      reqSeq.push_back(addiw_req);
    }
  }

}
