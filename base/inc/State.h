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
#ifndef Force_State_H
#define Force_State_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

#include <list>
#include <vector>

namespace Force {

  class Generator;
  class StateElement;

  /*!
    \class State
    \brief A collection of values relevant to the state of the simulation.
  */
  class State {
  public:
    explicit State(const Generator* pGenerator); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(State);
    ~State(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(State);

    static constexpr uint64 GetMaxMemoryStateElementSize() { return smMaxMemStateElemSize; } //!< Return maximum size of a MemoryStateElement in bytes.
    void AddMemoryStateElement(cuint64 memStartAddr, cuint64 memSize, cuint64 memVal, cuint32 priority); //!< Create a MemoryStateElement and add it to the State. The maximum of bytes that can be specified at a time is equal to the value returned by GetMaxMemoryStateElementSize(). The starting address must be aligned to this maximum number of bytes.
    void AddMemoryStateElementsAsBytes(cuint64 memStartAddr, const std::vector<uint8>& rMemValues, cuint32 priority); //!< Create one or more MemoryStateElements and add them to the State. An unlimited number of bytes may be specified. The memory values will be divided into MemoryStateElements of no more than the number of bytes specified by GetMaxMemoryStateElementSize() with starting addresses aligned to this maximum number of bytes.
    void AddRegisterStateElement(const std::string& rRegName, const std::vector<uint64>& rRegValues, cuint32 priority); //!< Create a RegisterStateElement and add it to the State. Don't use this method to specify VM context registers; use AddVmContextStateElement() instead.
    void AddSystemRegisterStateElementByField(const std::string& rRegName, const std::string& rRegFieldName, cuint64 regFieldVal, cuint32 priority); //!< Create a RegisterStateElement and add it to the State by specifying a field of the register and an associated value for the field. The RegisterStateElement's mask will only be set for the bits represented by the specified field. Don't use this method to specify VM context parameter fields; use AddVmContextStateElement() instead.
    void AddVmContextStateElement(const std::string& rRegName, const std::string& rRegFieldName, cuint64 regFieldVal, cuint32 priority); //!< Create a VmContextStateElement and add it to the State.
    void AddPrivilegeLevelStateElement(const EPrivilegeLevelType privLevel, cuint32 priority); //!< Create a PrivilegeLevelStateElement and add it to the State.
    void AddPrivilegeLevelStateElementByName(const std::string& rPrivLevelName, cuint32 priority); //!< Create a PrivilegeLevelStateElement by specifying the privilege level name and add it to the State.
    void AddPcStateElement(cuint64 pcVal, cuint32 priority); //!< Create a PcStateElement and add it to the state.
    const std::list<StateElement*>& GetStateElements() const { return mStateElems; } //!< Return the StateElements comprising the State.
    void SetDuplicateMode(const EStateElementDuplicateMode duplicateMode) { mDuplicateMode = duplicateMode; } //!< Specify how SateElements are handled when two or more StateElements are specified that describe the same State information.
  private:
    void AddStateElement(StateElement* pStateElem); //!< Add a StateElement to the State.
    EStateElementType GetRegisterStateElementType(const ERegisterType regType) const; //!< Determine the RegisterStateElement type from the register type.
  private:
    static constexpr uint64 smMaxMemStateElemSize = sizeof(uint64); //!< Maximum size of a MemoryStateElement in bytes
    std::list<StateElement*> mStateElems; //!< StateElements comprising the State
    EStateElementDuplicateMode mDuplicateMode; //!< Mode that determines how duplicate StateElements are handled
    const Generator* mpGenerator; //!< Pointer to the Generator object
  };

}

#endif  // Force_State_H
