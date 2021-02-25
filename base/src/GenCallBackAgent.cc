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
#include <GenCallBackAgent.h>
#include <GenRequest.h>
#include <SimplePeState.h>
#include <BntNode.h>
#include <Generator.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <MemoryManager.h>
#include <Constraint.h>
#include <FrontEndCall.h>
#include <GenMode.h>
#include <GenPC.h>
#include <Log.h>

using namespace std;

namespace Force {
  GenCallBackAgent::GenCallBackAgent(const GenCallBackAgent& rOther) : GenAgent(rOther),mpCallBackRequest(nullptr), mLastSequenceName()
  {

  }

  Object* GenCallBackAgent::Clone() const
  {
    return new GenCallBackAgent(*this);
  }

  void  GenCallBackAgent::SetGenRequest(GenRequest* genRequest)
  {
    mpCallBackRequest = dynamic_cast<GenCallBackRequest* >(genRequest);
  }

  void GenCallBackAgent::HandleRequest()
  {
    auto cb_type = mpCallBackRequest->CallBackType();
    LOG(notice) << "{GenCallBackAgent::HandleRequest} HandleRequest is called with : " << ECallBackType_to_string(cb_type) << endl;
    switch (cb_type) {
    case ECallBackType::Bnt:
      ProcessAccurateBntNode();
      break;
    case ECallBackType::Eret:
      ProcessEretPreambleSequence();
      break;
    default:
      LOG(fail) << "Unhandle callback type:" << ECallBackType_to_string(cb_type) << endl;
      FAIL("unhandle callback type");
    }

    delete mpCallBackRequest;
    mpCallBackRequest = nullptr;
  }

  void  GenCallBackAgent::ProcessAccurateBntNode()
  {
    auto cb_bnt =  mpCallBackRequest->CastInstance<GenCallBackBntRequest>();
    auto pBntNode = cb_bnt->GetBntNode();

    if (not pBntNode->IsSpeculative()) {
      // restore PE state.
      SimplePeState* pe_state = pBntNode->GetPeState();
      if (pe_state->RestoreState()) {
        mpGenerator->UpdateVm();
      }
    }

    uint64 inter_start = 0;
    uint64 inter_size = 0;
    uint64 not_taken_path = pBntNode->NotTakenPath();

    auto vm_mapper = mpGenerator->GetVmManager()->CurrentVmMapper();
    auto usable_constr = vm_mapper->VirtualUsableConstraintSet(true);

    // << "{GenCallBackAgent::HandleRequest} usable_constr=" << usable_constr->ToSimpleString() << endl;
    inter_start = usable_constr->LeadingIntersectingRange(not_taken_path, -1ull, inter_size);

    LOG(notice) << "{GenCallBackAgent::HandleRequest} generating not-taken-path starting from 0x" << hex << not_taken_path << " intersection start 0x" << inter_start << " size " << dec << inter_size << endl;

    if ((not_taken_path == inter_start) && (inter_size >= mpGenerator->BntMinSpace())) {
      auto gen_pc = mpGenerator->GetGenPC();
      uint32 saved_ispace = gen_pc->InstructionSpace();
      gen_pc->SetInstructionSpace(mpGenerator->DefaultInstructionSize());
      mpGenerator->SetPC(not_taken_path);
      auto seq_name = pBntNode->GetSequenceName();
      auto bnt_func = pBntNode->GetBntFunction();
      if (seq_name != mLastSequenceName) {
        CallBackTemplate(ECallBackTemplateType::SetBntSeq, seq_name);
        mLastSequenceName = seq_name;
      }
      CallBackTemplate(ECallBackTemplateType::RunBntSeq, bnt_func);
      gen_pc->SetInstructionSpace(saved_ispace);
    }
  }

  void GenCallBackAgent::ProcessEretPreambleSequence()
  {
    auto cb_eret =  mpCallBackRequest->CastInstance<GenCallBackEretRequest>();
    auto seq_name = cb_eret->GetPreambleSequence();
    CallBackTemplate(ECallBackTemplateType::SetEretPreambleSeq, seq_name);
    CallBackTemplate(ECallBackTemplateType::RunEretPreambleSeq, "");
  }

  uint32 GenCallBackAgent::CallBackTemplate(ECallBackTemplateType callBackType, const std::string& primaryValue, const std::map<std::string, uint64>& callBackValues)
  {
    FrontEndCall* front_ptr = FrontEndCall::Instance();
    return front_ptr->CallBackTemplate(mpGenerator->ThreadId(), callBackType, primaryValue, callBackValues);
  }

}
