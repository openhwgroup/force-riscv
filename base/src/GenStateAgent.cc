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
#include "GenStateAgent.h"

#include "BntHookManager.h"
#include "Defines.h"
#include "GenMode.h"
#include "GenRequest.h"
#include "Generator.h"
#include "Log.h"
#include "ReExecutionManager.h"
#include "Register.h"
#include "StringUtils.h"

PICKY_IGNORE_BLOCK_START
#include "SimAPI.h"
PICKY_IGNORE_BLOCK_END

using namespace std;

/*!
  \file GenStateAgent.cc
  \brief Code handling all generator state related actions.
*/

namespace Force {
  const EGenModeTypeBaseType LINEAR_MODE = EGenModeTypeBaseType(EGenModeType::SimOff) | EGenModeTypeBaseType(EGenModeType::NoEscape) | EGenModeTypeBaseType(EGenModeType::NoJump);

  Object* GenStateAgent::Clone() const
  {
    return new GenStateAgent(*this);
  }

  void GenStateAgent::SetGenRequest(GenRequest* genRequest)
  {
    mpStateRequest = dynamic_cast<GenStateRequest* >(genRequest);
  }

  void GenStateAgent::HandleRequest()
  {
    switch (mpStateRequest->ActionType()) {
    case EGenStateActionType::Push:
      PushState();
      break;
    case EGenStateActionType::Pop:
      PopState();
      break;
    case EGenStateActionType::Set:
      SetState();
      break;
    case EGenStateActionType::Enable:
      EnableState();
      break;
    case EGenStateActionType::Disable:
      DisableState();
      break;
    default:
      LOG(fail) << "{GenStateAgent::HandleRequest} unsupported state action: " << EGenStateActionType_to_string(mpStateRequest->ActionType()) << endl;
      FAIL("unsupported-state-action");
    }
  }

  void GenStateAgent::CleanUpRequest()
  {
    delete mpStateRequest;
    mpStateRequest = nullptr;
  }

  void GenStateAgent::ResetRequest()
  {
    mpStateRequest = nullptr;
  }

  void GenStateAgent::PushState()
  {
    switch (mpStateRequest->StateType()) {
    case EGenStateType::GenMode:
      {
        if (mpGenerator->HasISS() && IsRequestSpeculative() && not mpGenerator->InSpeculative()) {
          SimAPI *sim_api = mpGenerator->GetSimAPI(); // Get handle to simulator.
          sim_api->EnterSpeculativeMode(mpGenerator->ThreadId()); // enter speculative mode.
        }

        auto gen_mode = mpGenerator->GetGenMode();
        if (mpStateRequest->IsValue()) {
          gen_mode->PushGenMode(mpStateRequest->ValueVariable());
        }
        else {
          gen_mode->PushGenMode(mpStateRequest->StringVariable());
        }
      }
      break;
    case EGenStateType::Loop:
      BeginLoop();
      break;
    case EGenStateType::LinearBlock:
      BeginLinearBlock();
      break;
    case EGenStateType::BntHook:
      PushBntHook();
      break;
    default:
      LOG(fail) << "{GenStateAgent::PushState} unsupported state type: " << EGenStateType_to_string(mpStateRequest->StateType()) << endl;
      FAIL("unsupported-state-for-push");
    }
  }

  void GenStateAgent::PopState()
  {
    switch (mpStateRequest->StateType()) {
    case EGenStateType::GenMode:
      {
        auto gen_mode = mpGenerator->GetGenMode();
        if (mpStateRequest->IsValue()) {
          gen_mode->PopGenMode(mpStateRequest->ValueVariable());
        }
        else {
          gen_mode->PopGenMode(mpStateRequest->StringVariable());
        }

        if (mpGenerator->HasISS() && IsRequestSpeculative() && not mpGenerator->InSpeculative()) {
          SimAPI *sim_api = mpGenerator->GetSimAPI(); // Get handle to simulator.
          sim_api->LeaveSpeculativeMode(mpGenerator->ThreadId()); // enter speculative mode.
        }
      }
      break;
    case EGenStateType::Loop:
      EndLoop();
      break;
    case EGenStateType::LinearBlock:
      EndLinearBlock();
      break;
    case EGenStateType::BntHook:
      RevertBntHook();
      break;
    default:
      LOG(fail) << "{GenStateAgent::PopState} unsupported state type: " << EGenStateType_to_string(mpStateRequest->StateType()) << endl;
      FAIL("unsupported-state-for-pop");
    }
  }

  void GenStateAgent::SetState()
  {
    switch (mpStateRequest->StateType()) {
    case EGenStateType::PC:
      {
        uint64 set_pc = mpStateRequest->ValueVariable();
        if (not mpStateRequest->IsValue()) {
          set_pc = parse_uint64(mpStateRequest->StringVariable());
        }
        mpGenerator->SetPC(set_pc);
        UpdatePcOnISS(set_pc);
        LOG(notice) << "{GenStateAgent::SetState} setting PC to 0x" << hex << set_pc << endl;
      }
      break;
    case EGenStateType::InitialPC:
      {
        bool partial;
        uint64 set_value = mpStateRequest->ValueVariable();
        auto reg_file = mpGenerator->GetRegisterFile();
        auto reg = reg_file->RegisterLookup("PC");
        if (!reg->IsInitialized(&partial)) {
          mpGenerator->InitializeRegister("PC", "",  set_value);
          mpGenerator->SetStateValue(EGenStateType::InitialPC, set_value);
        }
        else
          LOG(warn) << "PC has been previously initialized." << endl;
      }
      break;
    case EGenStateType::BootPC:
      {
        uint64 set_value = mpStateRequest->ValueVariable();
        if (not mpStateRequest->IsValue()) {
          set_value = parse_uint64(mpStateRequest->StringVariable());
        }
        mpGenerator->SetStateValue(mpStateRequest->StateType(), set_value);
      }
      break;
    case EGenStateType::EL:
    case EGenStateType::PrivilegeLevel:
      {
        uint64 set_priv = mpStateRequest->ValueVariable();
        if (not mpStateRequest->IsValue()) {
          set_priv = parse_uint64(mpStateRequest->StringVariable());
        }
        mpGenerator->SetPrivilegeLevel(set_priv);
      }
      break;
    case EGenStateType::GenMode:
      {
        uint64 set_mode = mpStateRequest->ValueVariable();
        if (not mpStateRequest->IsValue()) {
          set_mode = parse_uint64(mpStateRequest->StringVariable());
        }
        mpGenerator->GetGenMode()->SetGenMode(set_mode);
      }
      break;
    case EGenStateType::PostLoopAddress:
      {
        auto loop_req = mpStateRequest->CastInstance<GenLoopRequest>();
        uint64 post_loop_addr = loop_req->ValueVariable();
        auto loop_id = loop_req->LoopId();
        mpGenerator->GetReExecutionManager()->ReportPostLoopAddress(loop_id, post_loop_addr);
      }
      break;
    case EGenStateType::LoopReconvergeAddress:
      {
        auto loop_req = mpStateRequest->CastInstance<GenLoopRequest>();
        uint64 loop_reconverge_addr = loop_req->ValueVariable();
        auto loop_id = loop_req->LoopId();
        mpGenerator->GetReExecutionManager()->ReportLoopReconvergeAddress(loop_id, loop_reconverge_addr);
      }
      break;
    default:
      LOG(fail) << "Unsupported generator state type: " << EGenStateType_to_string(mpStateRequest->StateType()) << endl;
      FAIL("unsupported-gen-state-type");
    }
  }

  void GenStateAgent::EnableState()
  {
    switch (mpStateRequest->StateType()) {
    case EGenStateType::GenMode:
      {
        auto gen_mode = mpGenerator->GetGenMode();
        gen_mode->EnableGenMode(mpStateRequest->ValueVariable());
      }
      break;
    default:
      LOG(fail) << "{GenStateAgent::EnableState} Unsupported generator state type: " << EGenStateType_to_string(mpStateRequest->StateType()) << endl;
      FAIL("unsupported-gen-state-type");
    }
  }

  void GenStateAgent::DisableState()
  {
    switch (mpStateRequest->StateType()) {
    case EGenStateType::GenMode:
      {
        auto gen_mode = mpGenerator->GetGenMode();
        gen_mode->DisableGenMode(mpStateRequest->ValueVariable());
      }
      break;
    default:
      LOG(fail) << "{GenStateAgent::DisableState} Unsupported generator state type: " << EGenStateType_to_string(mpStateRequest->StateType()) << endl;
      FAIL("unsupported-gen-state-type");
    }
  }

  void GenStateAgent::BeginLoop()
  {
    auto loop_req = mpStateRequest->CastInstance<GenLoopRequest>();
    uint64 loop_begin = mpGenerator->PC();
    loop_req->SetLoopBackAddress(loop_begin);
    auto loop_id = mpGenerator->GetReExecutionManager()->StartLoop(loop_begin);
    loop_req->SetLoopId(loop_id);
    LOG(notice) << "{GenStateAgent::BeginLoop} begin loop ID=" << dec << loop_id << " at 0x" << hex << loop_begin << endl;
    auto gen_mode = mpGenerator->GetGenMode();
    gen_mode->PushGenMode(EGenModeTypeBaseType(EGenModeType::InLoop));
  }

  void GenStateAgent::EndLoop()
  {
    auto loop_req = mpStateRequest->CastInstance<GenLoopRequest>();
    auto loop_id = loop_req->LoopId();
    uint64 end_addr = mpGenerator->PC();
    mpGenerator->GetReExecutionManager()->EndLoop(loop_id, end_addr);
    LOG(notice) << "{GenStateAgent::EndLoop} end loop ID=" << dec << loop_id << " at 0x" << hex << end_addr << endl;
    auto gen_mode = mpGenerator->GetGenMode();
    gen_mode->PopGenMode(EGenModeTypeBaseType(EGenModeType::InLoop));
  }

  void GenStateAgent::BeginLinearBlock()
  {
    auto lb_req = mpStateRequest->CastInstance<GenLinearBlockRequest>();
    uint64 lb_begin = mpGenerator->PC();
    auto lb_id = mpGenerator->GetReExecutionManager()->StartLinearBlock(lb_begin);
    lb_req->SetBlockId(lb_id);
    LOG(notice) << "{GenStateAgent::BeginLinearBlock} begin block ID=" << dec << lb_id << " at 0x" << hex << lb_begin << endl;
    auto gen_mode = mpGenerator->GetGenMode();
    gen_mode->EnableGenMode(LINEAR_MODE);
  }

  void GenStateAgent::EndLinearBlock()
  {
    auto lb_req = mpStateRequest->CastInstance<GenLinearBlockRequest>();
    auto lb_id = lb_req->BlockId();
    uint64 end_addr = mpGenerator->PC();
    uint64 block_start = mpGenerator->GetReExecutionManager()->EndLinearBlock(lb_id, end_addr);

    if (end_addr == block_start)
      lb_req->SetEmpty();

    lb_req->SetPrimaryValue(block_start);
    lb_req->SetBlockEndAddress(end_addr);
    LOG(notice) << "{GenStateAgent::EndLinear} end block ID=" << dec << lb_id << " at 0x" << hex << end_addr << " execute=" << lb_req->Execute() << endl;
    if (not lb_req->Execute())
      UpdatePcOnISS(end_addr);

    auto gen_mode = mpGenerator->GetGenMode();
    gen_mode->DisableGenMode(LINEAR_MODE);
  }

  void GenStateAgent::UpdatePcOnISS(uint64 pc)
  {
    if (mpGenerator->HasISS()) {
      SimAPI *sim_ptr = mpGenerator->GetSimAPI(); // get handle to simulator...
      sim_ptr->WriteRegister(mpGenerator->ThreadId(), "PC", pc, -1ull);
    }
  }

  void GenStateAgent::PushBntHook()
  {
    auto bh_req = mpStateRequest->CastInstance<GenBntHookRequest>();
    auto bh_manager = mpGenerator->GetBntHookManager();
    auto bh_id = bh_manager->AllocateId();
    BntHook bntHook(bh_id, bh_req->BntSequence(), bh_req->BntFunction());
    bh_manager->PushBntHook(bntHook);
    bh_req->SetBntId(bh_id);
  }

  void GenStateAgent::RevertBntHook()
  {
    auto bh_req = mpStateRequest->CastInstance<GenBntHookRequest>();
    auto bh_manager = mpGenerator->GetBntHookManager();
    bh_manager->RevertBntHook(bh_req->BntId());
  }

  bool GenStateAgent::IsRequestSpeculative() const
  {
    bool spec = false;
    if (mpStateRequest->IsValue())
      spec = mpStateRequest->ValueVariable() & EGenModeTypeBaseType(EGenModeType::Speculative);
    else {
      auto mode_str = mpStateRequest->StringVariable();
      spec = mode_str.find(EGenModeType_to_string(EGenModeType::Speculative)) != string::npos;
    }
    return spec;
  }
}
