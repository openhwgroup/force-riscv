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
#ifndef Force_StateTransition_H
#define Force_StateTransition_H

#include <Defines.h>
#include <Enums.h>

#include <list>
#include <map>
#include <pybind11/pybind11.h>
#include <vector>

namespace py = pybind11;

namespace Force {

  // StateTransitionAssignmentSet maps a StateElement type to the StateTransitionHandler registered
  // to handle that StateElement type.
  using StateTransitionAssignmentSet = std::map<EStateElementType, py::object>;

  /*!
    \struct StateTransitionOrderAssignment
    \brief A structure for storing information about the order in which StateElements are processed.
  */
  struct StateTransitionOrderAssignment {
    StateTransitionOrderAssignment()
      : mOrderMode(EStateTransitionOrderMode::AsSpecified), mStateElemTypeOrder()
    {
    }

    EStateTransitionOrderMode mOrderMode; //!< The order mode for processing StateElements
    std::vector<EStateElementType> mStateElemTypeOrder; //!< The order for processing StateElements in ByStateElementType order mode
  };

  class Generator;
  class MemoryStateElement;
  class RegisterStateElement;
  class State;
  class StateElement;

  /*!
    \class StateTransitionManager
    \brief A class to coordinate execution of StateTransitions.
  */
  class StateTransitionManager {
  public:
    explicit StateTransitionManager(Generator* pGenerator);
    COPY_CONSTRUCTOR_ABSENT(StateTransitionManager);
    DESTRUCTOR_DEFAULT(StateTransitionManager);
    ASSIGNMENT_OPERATOR_ABSENT(StateTransitionManager);

    void RegisterStateTransitionHandler(const py::object stateTransHandler, const EStateTransitionType stateTransType, const std::vector<EStateElementType>& rStateElemTypes); //!< Register a StateTransitionHandler to process StateElements of the specified type during StateTransitions of the specified type. Registering a StateTransitionHandler for a StateTransition type and StateElement type that already has an assigned StateTransitionHandler will replace the existing one.
    void SetDefaultStateTransitionHandler(const py::object stateTransHandler, const EStateElementType stateElemType); //!< Set the default StateTransitionHandler to process StateElements of the specified type. Attempting to register a default StateTransitionHandler for a StateElement type that already has an assigned default StateTransitionHandler will fail.
    void SetDefaultStateTransitionOrderMode(const EStateTransitionType stateTransType, const EStateTransitionOrderMode orderMode, const std::vector<EStateElementType>& rStateElemTypeOrder); //!< Set the default order mode for the specified StateTransition type.
    void TransitionToState(const State& rTargetState, const EStateTransitionType stateTransType, const EStateTransitionOrderMode orderMode, const std::vector<EStateElementType>& rStateElemTypeOrder) const; //!< Execute the appropriate StateTransitionHandlers for the StateElements in the specified State.
  private:
    void ResolveOrderMode(const EStateTransitionType stateTransType, EStateTransitionOrderMode& orderMode, std::vector<EStateElementType>& rStateElemTypeOrder) const; //!< Determine whether or not to use the default order mode.
    void GetArbitraryGprs(const std::list<StateElement*>& rStateElems, std::vector<uint64>& rArbitraryGprIndices) const; //!< Get indices of GPRs that have no assigned values in the specified list of StateElements. Exclude reserved GPRs.
    void SetArbitraryGprs(const StateTransitionAssignmentSet& rStateTransAssignSet, const std::vector<uint64>& rArbitraryGprIndices) const; //!< Pass list of arbitrary GPR indices to the StateTransition handlers.
    void SetStateTransitionType(const StateTransitionAssignmentSet& rStateTransAssignSet, const EStateTransitionType stateTransType) const; //!< Set the StateTransition type in the StateTransition handlers.
    void ClearStateTransitionType(const StateTransitionAssignmentSet& rStateTransAssignSet) const; //!< Unset the StateTransition type in the StateTransition handlers.
    void ProcessStateElements(const std::list<StateElement*>& rStateElems, const StateTransitionAssignmentSet& rStateTransAssignSet, const EStateTransitionOrderMode orderMode, const std::vector<EStateElementType>& rStateElemTypeOrder) const; //!< Use the specified StateTransitionHandlers to process the specified StateElements.
    StateElement* CreateCurrentMergedStateElement(const StateElement& rStateElem) const; //!< Create a copy of a StateElement with any unspecified parts filled in with the current simulation values.
    void ProcessStateElementsAsSpecified(const StateTransitionAssignmentSet& rStateTransAssignSet, const std::vector<StateElement*>& rStateElems) const; //!< Use the specified StateTransitionHandlers to process the specified StateElements in index order.
    void ProcessStateElementsByStateElementType(const StateTransitionAssignmentSet& rStateTransAssignSet, const std::vector<StateElement*>& rStateElems, const std::vector<EStateElementType>& rStateElemTypeOrder) const; //!< Use the specified StateTransitionHandlers to process the specified StateElements grouped by StateElemnet type.
    void ProcessStateElementsByPriority(const StateTransitionAssignmentSet& rStateTransAssignSet, std::vector<StateElement*>& rStateElems) const; //!< Use the specified StateTransitionHandlers to process the specified StateElements in order of ascending priority value. StateElements of equal priority are processed in random order. This method assumes rStateElems is not empty.
    MemoryStateElement* CreateCurrentMergedMemoryStateElement(const MemoryStateElement& rMemStateElem) const; //!< Create a copy of a MemoryStateElement with any unspecified parts filled in with the current simulation values.
    RegisterStateElement* CreateCurrentMergedRegisterStateElement(const RegisterStateElement& rRegStateElem) const; //!< Create a copy of a RegisterStateElement with any unspecified parts filled in with the current simulation values.
    void ProcessStateElement(const StateTransitionAssignmentSet& rStateTransAssignSet, const StateElement& rStateElem) const; //!< Process a StateElement.
    void ProcessStateElementsOfType(const StateTransitionAssignmentSet& rStateTransAssignSet, const EStateElementType stateElemType, const std::vector<StateElement*>& rStateElems) const; //!< Process a list of StateElements of a given StateElement type.
    void InitializeMemory(const MemoryStateElement& rMemStateElem) const; //!< Initialize the memory location indicated by the specified MemoryStateElement.
  private:
    std::map<EStateTransitionType, StateTransitionAssignmentSet> mStateTransAssignments; //!< StateTransitionHandler assignments for each StateTransition type
    StateTransitionAssignmentSet mDefaultStateTransAssignments; //!< Default StateTransitionHandler assignment for each StateElement type
    std::map<EStateTransitionType, StateTransitionOrderAssignment> mDefaultOrderModes; //!< Default order mode for each StateTransition type
    Generator* mpGenerator; //!< Pointer to the Generator object
  };

  /*!
    \class StateTransitionManagerRepository
    \brief A repository of StateTransitionManagers.
  */
  class StateTransitionManagerRepository {
  public:
    static void Initialize(); //!< Create StateTransitionManagerRepository instance.
    static void Destroy(); //!< Destroy StateTransitionManagerRepository instance.
    static StateTransitionManagerRepository* Instance() { return mspStateTransManagerRepo; } //!< Access StateTransitionManagerRepository instance.

    void AddStateTransitionManager(cuint32 threadId, StateTransitionManager* pStateTransManager);
    StateTransitionManager* GetStateTransitionManager(cuint32 threadId) const;
  private:
    StateTransitionManagerRepository();
    COPY_CONSTRUCTOR_ABSENT(StateTransitionManagerRepository);
    ~StateTransitionManagerRepository();
    ASSIGNMENT_OPERATOR_ABSENT(StateTransitionManagerRepository);
  private:
    static StateTransitionManagerRepository* mspStateTransManagerRepo; //!< StateTransitionManagerRepository instance
    std::map<uint32, StateTransitionManager*> mStateTransManagers; //!< StateTransitionManagers stored by thread ID
  };

}

#endif  // Force_StateTransition_H
