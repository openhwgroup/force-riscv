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
#ifndef Force_PyStateTransition_H
#define Force_PyStateTransition_H

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "GenRequest.h"
#include "Generator.h"
#include "Scheduler.h"
#include "StateTransition.h"
#include "ThreadContext.h"

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(StateTransition, mod) {
    // Need to import the Enums module in order to specify a default state transition order mode.
    py::module::import("Enums");

    std::vector<EStateElementType> empty_elem_type_order;

    mod
      .def("registerStateTransitionHandler",
        [](const py::object stateTransHandler, const EStateTransitionType stateTransType, const std::vector<EStateElementType>& rStateElemTypes) {
          ThreadContext thread_context;

          StateTransitionManagerRepository* state_trans_manager_repo = StateTransitionManagerRepository::Instance();
          StateTransitionManager* state_trans_manager = state_trans_manager_repo->GetStateTransitionManager(thread_context.GetThreadId());
          state_trans_manager->RegisterStateTransitionHandler(stateTransHandler, stateTransType, rStateElemTypes);
        })
      .def("setDefaultStateTransitionHandler",
        [](const py::object stateTransHandler, const EStateElementType stateElemType) {
          ThreadContext thread_context;

          StateTransitionManagerRepository* state_trans_manager_repo = StateTransitionManagerRepository::Instance();
          StateTransitionManager* state_trans_manager = state_trans_manager_repo->GetStateTransitionManager(thread_context.GetThreadId());
          state_trans_manager->SetDefaultStateTransitionHandler(stateTransHandler, stateElemType);
        })
      .def("setDefaultStateTransitionOrderMode",
        [](const EStateTransitionType stateTransType, const EStateTransitionOrderMode orderMode, const std::vector<EStateElementType>& rStateElemTypeOrder) {
          ThreadContext thread_context;

          StateTransitionManagerRepository* state_trans_manager_repo = StateTransitionManagerRepository::Instance();
          StateTransitionManager* state_trans_manager = state_trans_manager_repo->GetStateTransitionManager(thread_context.GetThreadId());
          state_trans_manager->SetDefaultStateTransitionOrderMode(stateTransType, orderMode, rStateElemTypeOrder);
        },
        py::arg(), py::arg(), py::arg("aStateElemTypeOrder") = empty_elem_type_order)
      .def("transitionToState",
        [](const State& rState, const EStateTransitionOrderMode orderMode, const std::vector<EStateElementType>& rStateElemTypeOrder) {
          ThreadContext thread_context;

          Scheduler* scheduler = Scheduler::Instance();
          Generator* generator = scheduler->LookUpGenerator(thread_context.GetThreadId());
          generator->GenSequence(new GenStateTransitionRequest(&rState, EStateTransitionType::Explicit, orderMode, rStateElemTypeOrder));
        },
        py::arg(), py::arg("aOrderMode") = EStateTransitionOrderMode::UseDefault, py::arg("aStateElemTypeOrder") = empty_elem_type_order)
      ;
  }

}

#endif  // Force_PyStateTransition_H
