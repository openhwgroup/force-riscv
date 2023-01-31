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
#include "StateTransition.h"

#include <algorithm>

#include "pybind11/stl.h"

#include "Constraint.h"
#include "Generator.h"
#include "Log.h"
#include "MemoryInitData.h"
#include "Random.h"
#include "Register.h"
#include "RegisterReserver.h"
#include "State.h"
#include "StateElement.h"
#include "UtilityFunctions.h"
#include "VirtualMemoryInitializer.h"

/*!
  \file StateTransitionManagerRepository.cc
  \brief Code supporting coordinating execution of StateTransitions.
*/

using namespace std;

namespace Force {

  StateTransitionManager::StateTransitionManager(Generator* pGenerator)
    : mStateTransAssignments(), mDefaultStateTransAssignments(), mDefaultOrderModes(), mpGenerator(pGenerator)
  {
    for (EStateTransitionTypeBaseType i = 0; i < EStateTransitionTypeSize; i++) {
      mStateTransAssignments.emplace(EStateTransitionType(i), StateTransitionAssignmentSet());
      mDefaultOrderModes.emplace(EStateTransitionType(i), StateTransitionOrderAssignment());
    }
  }

  void StateTransitionManager::RegisterStateTransitionHandler(const py::object stateTransHandler, const EStateTransitionType stateTransType, const vector<EStateElementType>& rStateElemTypes)
  {
    auto assign_itr = mStateTransAssignments.find(stateTransType);
    if (assign_itr != mStateTransAssignments.end()) {
      StateTransitionAssignmentSet& state_trans_assign_set = assign_itr->second;

      for (EStateElementType state_elem_type : rStateElemTypes) {
        auto handler_itr = state_trans_assign_set.find(state_elem_type);

        if (handler_itr != state_trans_assign_set.end()) {
          handler_itr->second = stateTransHandler;
        }
        else {
          state_trans_assign_set.emplace(state_elem_type, stateTransHandler);
        }
      }
    }
    else {
      LOG(fail) << "{StateTransitionManager::RegisterStateTransitionHandler} unexpected StateTransition type " << EStateTransitionType_to_string(stateTransType) << endl;
      FAIL("unexpected-state-transition-type");
    }
  }

  void StateTransitionManager::SetDefaultStateTransitionHandler(const py::object stateTransHandler, const EStateElementType stateElemType)
  {
    auto result = mDefaultStateTransAssignments.emplace(stateElemType, stateTransHandler);
    if (not result.second) {
      LOG(fail) << "{StateTransitionManager::SetDefaultStateTransitionHandler} a default StateTransitionHandler has already been assigned to handle " << EStateElementType_to_string(stateElemType) << " StateElements" << endl;
      FAIL("duplicate-state-transition-handler");
    }
  }

  void StateTransitionManager::SetDefaultStateTransitionOrderMode(const EStateTransitionType stateTransType, const EStateTransitionOrderMode orderMode, const std::vector<EStateElementType>& rStateElemTypeOrder)
  {
    if (orderMode == EStateTransitionOrderMode::UseDefault) {
      LOG(fail) << "{StateTransitionManager::SetDefaultStateTransitionOrderMode} " << EStateTransitionOrderMode_to_string(orderMode) << " is not a valid default order mode" << endl;
      FAIL("invalid-default-order-mode");
    }

    if ((orderMode == EStateTransitionOrderMode::ByStateElementType) and (rStateElemTypeOrder.size() != EStateElementTypeSize)) {
      LOG(fail) << "{StateTransitionManager::SetDefaultStateTransitionOrderMode} unexpected number of StateElement types specified: Expected=" << dec << EStateElementTypeSize << ", Actual=" << rStateElemTypeOrder.size() << endl;
      FAIL("invalid-state-element-type-order");
    }

    auto itr = mDefaultOrderModes.find(stateTransType);
    if (itr != mDefaultOrderModes.end()) {
      StateTransitionOrderAssignment& state_trans_order_assignment = itr->second;
      state_trans_order_assignment.mOrderMode = orderMode;
      state_trans_order_assignment.mStateElemTypeOrder = rStateElemTypeOrder;
    }
    else {
      LOG(fail) << "{StateTransitionManager::SetDefaultStateTransitionOrderMode} unexpected StateTransition type " << EStateTransitionType_to_string(stateTransType) << endl;
      FAIL("unexpected-state-transition-type");
    }
  }

  void StateTransitionManager::TransitionToState(const State& rTargetState, const EStateTransitionType stateTransType, const EStateTransitionOrderMode orderMode, const vector<EStateElementType>& rStateElemTypeOrder) const
  {
    const list<StateElement*>& state_elems = rTargetState.GetStateElements();
    if (state_elems.empty()) {
      return;
    }

    auto itr = mStateTransAssignments.find(stateTransType);
    if (itr == mStateTransAssignments.end()) {
      LOG(fail) << "{StateTransitionManager::TransitionToState} no StateTransitionHandlers were assigned for " << EStateTransitionType_to_string(stateTransType) << endl;
      FAIL("no-state-transition-handlers");
    }

    const StateTransitionAssignmentSet& state_trans_assign_set = itr->second;

    // Need to import the StateElement module in order to pass StateElement objects to Python
    // methods.
    py::module::import("StateElement");

    std::vector<uint64> arbitrary_gpr_indices;
    GetArbitraryGprs(state_elems, arbitrary_gpr_indices);

    SetArbitraryGprs(state_trans_assign_set, arbitrary_gpr_indices);
    SetArbitraryGprs(mDefaultStateTransAssignments, arbitrary_gpr_indices);
    SetStateTransitionType(state_trans_assign_set, stateTransType);
    SetStateTransitionType(mDefaultStateTransAssignments, stateTransType);

    EStateTransitionOrderMode order_mode = orderMode;
    vector<EStateElementType> state_elem_type_order = rStateElemTypeOrder;
    ResolveOrderMode(stateTransType, order_mode, state_elem_type_order);

    ProcessStateElements(state_elems, state_trans_assign_set, order_mode, state_elem_type_order);

    // The StateTransition type and arbitrary GPR list are only valid for the given StateTransition,
    // so clear them after processing the StateElements
    ClearStateTransitionType(mDefaultStateTransAssignments);
    ClearStateTransitionType(state_trans_assign_set);
    arbitrary_gpr_indices.clear();
    SetArbitraryGprs(mDefaultStateTransAssignments, arbitrary_gpr_indices);
    SetArbitraryGprs(state_trans_assign_set, arbitrary_gpr_indices);
  }

  void StateTransitionManager::ResolveOrderMode(const EStateTransitionType stateTransType, EStateTransitionOrderMode& orderMode, vector<EStateElementType>& rStateElemTypeOrder) const
  {
    if (orderMode == EStateTransitionOrderMode::UseDefault) {
      auto itr = mDefaultOrderModes.find(stateTransType);

      if (itr != mDefaultOrderModes.end()) {
        const StateTransitionOrderAssignment& state_trans_order_assignment = itr->second;
        orderMode = state_trans_order_assignment.mOrderMode;
        rStateElemTypeOrder = state_trans_order_assignment.mStateElemTypeOrder;
      }
      else {
        LOG(fail) << "{StateTransitionManager::ResolveOrderMode} no default order mode was set for " << EStateTransitionType_to_string(stateTransType) << endl;
        FAIL("no-default-order-mode");
      }
    }
    else if ((orderMode == EStateTransitionOrderMode::ByStateElementType) and (rStateElemTypeOrder.size() != EStateElementTypeSize)) {
      LOG(fail) << "{StateTransitionManager::ResolveOrderMode} unexpected number of StateElement types specified: Expected=" << dec << EStateElementTypeSize << ", Actual=" << rStateElemTypeOrder.size() << endl;
      FAIL("invalid-state-element-type-order");
    }
  }

  void StateTransitionManager::GetArbitraryGprs(const list<StateElement*>& rStateElems, vector<uint64>& rArbitraryGprIndices) const
  {
    ConstraintSet arbitrary_gpr_indices;
    const RegisterFile* reg_file = mpGenerator->GetRegisterFile();
    RegisterReserver* reg_reserver = reg_file->GetRegisterReserver();
    reg_reserver->UsableIndexConstraint(ERegisterType::GPR, ERegAttrType::ReadWrite, &arbitrary_gpr_indices);

    for (StateElement* state_elem : rStateElems) {
      if (state_elem->GetStateElementType() == EStateElementType::GPR) {
        auto reg_state_elem = dynamic_cast<RegisterStateElement*>(state_elem);
        arbitrary_gpr_indices.SubValue(reg_state_elem->GetRegisterIndex());
      }
    }

    arbitrary_gpr_indices.GetValues(rArbitraryGprIndices);
  }

  void StateTransitionManager::SetArbitraryGprs(const StateTransitionAssignmentSet& rStateTransAssignSet, const vector<uint64>& rArbitraryGprIndices) const
  {
    for (auto& entry : rStateTransAssignSet) {
      py::object state_trans_handler = entry.second;
      state_trans_handler.attr("setArbitraryGprs")(rArbitraryGprIndices);
    }
  }

  void StateTransitionManager::SetStateTransitionType(const StateTransitionAssignmentSet& rStateTransAssignSet, const EStateTransitionType stateTransType) const
  {
    for (auto& entry : rStateTransAssignSet) {
      py::object state_trans_handler = entry.second;
      state_trans_handler.attr("mStateTransType") = stateTransType;
    }
  }

  void StateTransitionManager::ClearStateTransitionType(const StateTransitionAssignmentSet& rStateTransAssignSet) const
  {
    for (auto& entry : rStateTransAssignSet) {
      py::object state_trans_handler = entry.second;
      state_trans_handler.attr("mStateTransType") = py::none();
    }
  }

  void StateTransitionManager::ProcessStateElements(const list<StateElement*>& rStateElems, const StateTransitionAssignmentSet& rStateTransAssignSet, const EStateTransitionOrderMode orderMode, const vector<EStateElementType>& rStateElemTypeOrder) const
  {
    // We need to fill in any unspecified portions of StateElements. Create a new list of
    // StateElements by merging the given list from the target State with the current State.
    vector<StateElement*> state_elems;
    transform(rStateElems.cbegin(), rStateElems.cend(), back_inserter(state_elems),
      [this](const StateElement* pStateElem) { return CreateCurrentMergedStateElement(*pStateElem); });

    switch(orderMode) {
    case EStateTransitionOrderMode::AsSpecified:
      ProcessStateElementsAsSpecified(rStateTransAssignSet, state_elems);
      break;
    case EStateTransitionOrderMode::ByStateElementType:
      ProcessStateElementsByStateElementType(rStateTransAssignSet, state_elems, rStateElemTypeOrder);
      break;
    case EStateTransitionOrderMode::ByPriority:
      ProcessStateElementsByPriority(rStateTransAssignSet, state_elems);
      break;
    default:
      LOG(fail) << "{StateTransitionManager::ProcessStateElements} unknown StateTransition order mode " << EStateTransitionOrderMode_to_string(orderMode) << endl;
      FAIL("unknown-order-mode");
    }

    for (StateElement* state_elem : state_elems) {
      delete state_elem;
    }
  }

  StateElement* StateTransitionManager::CreateCurrentMergedStateElement(const StateElement& rStateElem) const
  {
    StateElement* merged_state_elem = nullptr;

    const vector<uint64>& masks = rStateElem.GetMasks();
    bool partial_mask = any_of(masks.cbegin(), masks.cend(),
      [](cuint64 mask) { return (mask != MAX_UINT64); });

    if (partial_mask) {
      switch (rStateElem.GetStateElementType()) {
      case EStateElementType::Memory:
        merged_state_elem = CreateCurrentMergedMemoryStateElement(dynamic_cast<const MemoryStateElement&>(rStateElem));
        break;
      case EStateElementType::SystemRegister:
      case EStateElementType::VectorRegister:
      case EStateElementType::GPR:
      case EStateElementType::FloatingPointRegister:
      case EStateElementType::PredicateRegister:
        merged_state_elem = CreateCurrentMergedRegisterStateElement(dynamic_cast<const RegisterStateElement&>(rStateElem));
        break;
      default:
        LOG(fail) << "{StateTransitionManager::CreateCurrentMergedStateElement} StateElements of type " << EStateElementType_to_string(rStateElem.GetStateElementType()) << " should always be fully specified"  << endl;
        FAIL("state-element-processing-failure");
      }
    }
    else {
      if (rStateElem.GetStateElementType() == EStateElementType::Memory) {
        InitializeMemory(dynamic_cast<const MemoryStateElement&>(rStateElem));
      }

      // If the masks are fully set, there is no need to merge anything
      merged_state_elem = dynamic_cast<StateElement*>(rStateElem.Clone());
    }

    return merged_state_elem;
  }

  void StateTransitionManager::ProcessStateElementsAsSpecified(const StateTransitionAssignmentSet& rStateTransAssignSet, const vector<StateElement*>& rStateElems) const
  {
    for (StateElement* state_elem : rStateElems) {
      ProcessStateElement(rStateTransAssignSet, *state_elem);
    }
  }

  void StateTransitionManager::ProcessStateElementsByStateElementType(const StateTransitionAssignmentSet& rStateTransAssignSet, const vector<StateElement*>& rStateElems, const vector<EStateElementType>& rStateElemTypeOrder) const
  {
    // Group the StateElements by StateElement type
    map<EStateElementType, vector<StateElement*>> state_elems_by_type;
    for (StateElement* state_elem : rStateElems) {
      auto itr = state_elems_by_type.find(state_elem->GetStateElementType());

      if (itr != state_elems_by_type.end()) {
        itr->second.push_back(state_elem);
      }
      else {
        state_elems_by_type.emplace(state_elem->GetStateElementType(), vector<StateElement*>({state_elem}));
      }
    }

    for (EStateElementType state_elem_type : rStateElemTypeOrder) {
      auto itr = state_elems_by_type.find(state_elem_type);

      if (itr != state_elems_by_type.end()) {
        ProcessStateElementsOfType(rStateTransAssignSet, state_elem_type, itr->second);
      }
      // Else there are no StateElements of the current StateElement type, so nothing needs to be
      // done
    }
  }

  void StateTransitionManager::ProcessStateElementsByPriority(const StateTransitionAssignmentSet& rStateTransAssignSet, vector<StateElement*>& rStateElems) const
  {
    auto compare_priority = [](const StateElement* pStateElemA, const StateElement* pStateElemB) {
      return (pStateElemA->GetPriority() < pStateElemB->GetPriority());
    };

    sort(rStateElems.begin(), rStateElems.end(), compare_priority);

    // We process the StateElements in batches that have equal priority. The processing order is
    // randomized for each batch.

    // Get iterators pointing to the first equal-priority batch
    auto iterators = equal_range(rStateElems.begin(), rStateElems.end(), rStateElems[0], compare_priority);

    RandomURBG32 urbg32(Random::Instance());
    while (iterators.first != rStateElems.end()) {
      shuffle(iterators.first, iterators.second, urbg32);

      for_each(iterators.first, iterators.second,
        [this, &rStateTransAssignSet](const StateElement* pStateElem) { ProcessStateElement(rStateTransAssignSet, *pStateElem); });

      // Get iterators pointing to the boundaries of the next equal-priority batch
      if (iterators.second != rStateElems.end()) {
        iterators = equal_range(iterators.second, rStateElems.end(), *(iterators.second), compare_priority);
      }
      else {
        iterators.first = rStateElems.end();
      }
    }
  }

  MemoryStateElement* StateTransitionManager::CreateCurrentMergedMemoryStateElement(const MemoryStateElement& rMemStateElem) const
  {
    InitializeMemory(rMemStateElem);

    uint64 start_addr = rMemStateElem.GetStartAddress();
    uint32 chunk_size = State::GetMaxMemoryStateElementSize();

    // Get the current value for the chunk of memory at the start address
    vector<uint8> data_buffer(chunk_size, 0);
    VirtualMemoryInitializer* virt_mem_initializer = mpGenerator->GetVirtualMemoryInitializer();
    virt_mem_initializer->ReadMemory(start_addr, chunk_size, data_buffer.data());

    uint64 mem_val = 0;
    if (mpGenerator->IsDataBigEndian()) {
      mem_val = data_array_to_value_big_endian(data_buffer.data(), chunk_size);
    }
    else {
      mem_val = data_array_to_value_little_endian(data_buffer.data(), chunk_size);
    }

    uint64 complement_mask = ~(rMemStateElem.GetMasks()[0]);
    auto merged_mem_state_elem = new MemoryStateElement(start_addr, mem_val, complement_mask, rMemStateElem.GetPriority());
    merged_mem_state_elem->Merge(rMemStateElem);

    return merged_mem_state_elem;
  }

  RegisterStateElement* StateTransitionManager::CreateCurrentMergedRegisterStateElement(const RegisterStateElement& rRegStateElem) const
  {
    string reg_name = rRegStateElem.GetName();

    // Initialize any uninitialized parts of the register
    const RegisterFile* reg_file = mpGenerator->GetRegisterFile();
    Register* reg = reg_file->RegisterLookup(reg_name);
    reg_file->InitializeRegisterRandomly(reg);

    // Get the register's current value
    vector<uint64> reg_values;
    if (reg->IsLargeRegister()) {
      auto large_reg = dynamic_cast<LargeRegister*>(reg);
      reg_values = large_reg->Values();
    }
    else {
      reg_values.push_back(reg->Value());
    }

    const vector<uint64>& masks = rRegStateElem.GetMasks();
    vector<uint64> complement_masks;
    transform(masks.cbegin(), masks.cend(), back_inserter(complement_masks),
      [](cuint64 mask) { return ~mask; });

    auto merged_reg_state_elem = new RegisterStateElement(rRegStateElem.GetStateElementType(), reg_name, rRegStateElem.GetRegisterIndex(), reg_values, complement_masks, rRegStateElem.GetPriority());
    merged_reg_state_elem->Merge(rRegStateElem);

    return merged_reg_state_elem;
  }

  void StateTransitionManager::ProcessStateElement(const StateTransitionAssignmentSet& rStateTransAssignSet, const StateElement& rStateElem) const
  {
    bool element_processed = false;
    auto handler_itr = rStateTransAssignSet.find(rStateElem.GetStateElementType());
    if (handler_itr != rStateTransAssignSet.end()) {
      py::object state_trans_handler = handler_itr->second;

      // Need to pass rStateElem as a pointer; otherwise, pybind will attempt to copy the object.
      py::object success = state_trans_handler.attr("processStateElement")(&rStateElem);

      element_processed = success.cast<bool>();
    }

    if (!element_processed) {
      auto default_handler_itr = mDefaultStateTransAssignments.find(rStateElem.GetStateElementType());

      if (default_handler_itr != mDefaultStateTransAssignments.end()) {
        py::object default_state_trans_handler = default_handler_itr->second;

        // Need to pass rStateElem as a pointer; otherwise, pybind will attempt to copy the object.
        py::object success = default_state_trans_handler.attr("processStateElement")(&rStateElem);

        element_processed = success.cast<bool>();
      }
    }

    if (!element_processed) {
      LOG(fail) << "{StateTransitionManager::ProcessStateElement} unable to process StateElement " << rStateElem.ToString() << endl;
      FAIL("state-element-processing-failure");
    }
  }

  void StateTransitionManager::ProcessStateElementsOfType(const StateTransitionAssignmentSet& rStateTransAssignSet, const EStateElementType stateElemType, const vector<StateElement*>& rStateElems) const
  {
    bool elements_processed = false;
    auto handler_itr = rStateTransAssignSet.find(stateElemType);
    if (handler_itr != rStateTransAssignSet.end()) {
      py::object state_trans_handler = handler_itr->second;
      state_trans_handler.attr("processStateElements")(rStateElems);
      elements_processed = true;
    }

    if (!elements_processed) {
      auto default_handler_itr = mDefaultStateTransAssignments.find(stateElemType);

      if (default_handler_itr != mDefaultStateTransAssignments.end()) {
        py::object default_state_trans_handler = default_handler_itr->second;
        default_state_trans_handler.attr("processStateElements")(rStateElems);
        elements_processed = true;
      }
    }

    if (!elements_processed) {
      LOG(fail) << "{StateTransitionManager::ProcessStateElementsOfType} unable to process " << EStateElementType_to_string(stateElemType) << " StateElements" << endl;
      FAIL("state-element-processing-failure");
    }
  }

  void StateTransitionManager::InitializeMemory(const MemoryStateElement& rMemStateElem) const
  {
    MemoryInitData init_data(rMemStateElem.GetStartAddress(), State::GetMaxMemoryStateElementSize(), 1, EMemAccessType::Write);
    init_data.Setup(*mpGenerator, new ConstraintSet(0, MAX_UINT64));
    init_data.Commit(*mpGenerator);
  }

  StateTransitionManagerRepository* StateTransitionManagerRepository::mspStateTransManagerRepo = nullptr;

  void StateTransitionManagerRepository::Initialize()
  {
    if (mspStateTransManagerRepo == nullptr) {
      mspStateTransManagerRepo = new StateTransitionManagerRepository();
    }
  }

  void StateTransitionManagerRepository::Destroy()
  {
    delete mspStateTransManagerRepo;
    mspStateTransManagerRepo = nullptr;
  }

  StateTransitionManagerRepository::StateTransitionManagerRepository()
    : mStateTransManagers()
  {
  }

  StateTransitionManagerRepository::~StateTransitionManagerRepository()
  {
    for (const auto& entry : mStateTransManagers) {
      delete entry.second;
    }
  }

  void StateTransitionManagerRepository::AddStateTransitionManager(cuint32 threadId, StateTransitionManager* pStateTransManager)
  {
    auto result = mStateTransManagers.emplace(threadId, pStateTransManager);
    if (not result.second) {
      LOG(fail) << "{StateTransitionManagerRepository::GetStateTransitionManager} a StateTransitionManager has already been added for thread " << hex << threadId << endl;
      FAIL("unknown-state-transition-manager");
    }
  }

  StateTransitionManager* StateTransitionManagerRepository::GetStateTransitionManager(cuint32 threadId) const
  {
    StateTransitionManager* state_trans_manager = nullptr;
    auto itr = mStateTransManagers.find(threadId);
    if (itr != mStateTransManagers.end()) {
      state_trans_manager = itr->second;
    }
    else {
      LOG(fail) << "{StateTransitionManagerRepository::GetStateTransitionManager} no StateTransitionManager has been added for thread " << hex << threadId << endl;
      FAIL("unknown-state-transition-manager");
    }

    return state_trans_manager;
  }

}
