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
#include "GenStateTransitionAgent.h"

#include "GenRequest.h"
#include "Generator.h"
#include "Log.h"
#include "State.h"
#include "StateTransition.h"

using namespace std;

namespace Force {

  GenStateTransitionAgent::GenStateTransitionAgent()
    : GenAgent(), mpStateTransReq(nullptr)
  {
  }

  GenStateTransitionAgent::GenStateTransitionAgent(const GenStateTransitionAgent& rOther)
    : GenAgent(rOther), mpStateTransReq(rOther.mpStateTransReq)
  {
  }

  void GenStateTransitionAgent::SetGenRequest(GenRequest* genRequest)
  {
    mpStateTransReq = dynamic_cast<GenStateTransitionRequest*>(genRequest);
  }

  void GenStateTransitionAgent::HandleRequest()
  {
    LOG(notice) << "{GenStateTransitionAgent::HandleRequest} called" << endl;

    StateTransitionManagerRepository* state_trans_manager_repo = StateTransitionManagerRepository::Instance();
    StateTransitionManager* state_trans_manager = state_trans_manager_repo->GetStateTransitionManager(mpGenerator->ThreadId());
    const State* state = mpStateTransReq->TargetState();
    state_trans_manager->TransitionToState(*state, mpStateTransReq->StateTransitionType(), mpStateTransReq->OrderMode(), mpStateTransReq->StateElementTypeOrder());
  }

  void GenStateTransitionAgent::CleanUpRequest()
  {
    // If this is a one-time request, delete the State object
    auto one_time_state_trans_req = dynamic_cast<GenOneTimeStateTransitionRequest*>(mpStateTransReq);
    if (one_time_state_trans_req != nullptr) {
      delete mpStateTransReq->TargetState();
    }

    delete mpStateTransReq;
    mpStateTransReq = nullptr;
  }

}
