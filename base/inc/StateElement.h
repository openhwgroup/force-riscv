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
#ifndef Force_StateElement_H
#define Force_StateElement_H

#include <Object.h>

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

#include <vector>

namespace Force {

  /*!
    \class StateElement
    \brief A value relevant to the state of the simulation.
  */
  class StateElement : public Object {
  public:
    StateElement(const EStateElementType stateElemType, const std::vector<uint64>& rValues, const std::vector<uint64>& rMasks, cuint32 priority);
    SUBCLASS_DESTRUCTOR_DEFAULT(StateElement);
    ASSIGNMENT_OPERATOR_ABSENT(StateElement);

    const std::string ToString() const override { return GetName(); } //!< Return a string describing the current state of the Object.

    virtual std::string GetName() const = 0; //!< Return the name of the StateElement.
    EStateElementType GetStateElementType() const { return mStateElemType; } //!< Return the type of this StateElement.
    const std::vector<uint64>& GetValues() const { return mValues; } //!< Return a list of values specified by the StateElement.
    const std::vector<uint64>& GetMasks() const { return mMasks; } //!< Return a list of masks indicating which bits of the StateElement's values are valid.
    uint32 GetPriority() const { return mPriority; } //!< Return the processing priority of the StateElement.
    virtual bool IsDuplicate(const StateElement& rOther) const; //!< Return true if this StateElement represents the same underlying State information as the other StateElement.
    virtual bool CanMerge(const StateElement& rOther) const; //!< Return true if a call to Merge() with the specified StateElement would succeed.
    virtual void Merge(const StateElement& rOther); //!< Merge another StateElement's values and masks into this one. The other StateElement must not have masks that overlap with this StateElement's masks; otherwise, this method will fail.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(StateElement);
  private:
    const EStateElementType mStateElemType; //!< Type of StateElement
    std::vector<uint64> mValues; //!< List of values specified by the StateElement
    std::vector<uint64> mMasks; //!< List of masks indicating which bits of the StateElement's values are valid
    uint32 mPriority; //!< Processing priority of the StateElement
  };

  /*!
    \class MemoryStateElement
    \brief A memory value relevant to the state of the simulation.
  */
  class MemoryStateElement : public StateElement {
  public:
    MemoryStateElement(cuint64 startAddr, cuint64 value, cuint64 mask, cuint32 priority);
    SUBCLASS_DESTRUCTOR_DEFAULT(MemoryStateElement);
    ASSIGNMENT_OPERATOR_ABSENT(MemoryStateElement);

    Object* Clone() const override { return new MemoryStateElement(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "MemoryStateElement"; } //!< Return a string describing the actual type of the Object.

    std::string GetName() const override; //!< Return the name of the StateElement.
    bool IsDuplicate(const StateElement& rOther) const override; //!< Return true if this StateElement represents the same underlying State information as the other StateElement.

    uint64 GetStartAddress() const { return mStartAddr; } //!< Return the starting virtual address for the memory chunk.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(MemoryStateElement);
  private:
    cuint64 mStartAddr; //!< Starting virtual address for the memory chunk
  };

  /*!
    \class RegisterStateElement
    \brief A register value relevant to the state of the simulation.
  */
  class RegisterStateElement : public StateElement {
  public:
    RegisterStateElement(const EStateElementType stateElemType, const std::string& rRegName, cuint32 regIndex, const std::vector<uint64>& rValues, const std::vector<uint64>& rMasks, cuint32 priority);
    SUBCLASS_DESTRUCTOR_DEFAULT(RegisterStateElement);
    ASSIGNMENT_OPERATOR_ABSENT(RegisterStateElement);

    Object* Clone() const override { return new RegisterStateElement(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "RegisterStateElement"; } //!< Return a string describing the actual type of the Object.

    std::string GetName() const override { return mRegName; } //!< Return the name of the StateElement.

    uint32 GetRegisterIndex() const { return mRegIndex; } //!< Return the register index.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(RegisterStateElement);
  private:
    const std::string mRegName; //!< Register name
    cuint32 mRegIndex; //!< Register index
  };

  /*!
    \class VmContextStateElement
    \brief A VM context value relevant to the state of the simulation.
  */
  class VmContextStateElement : public StateElement {
  public:
    VmContextStateElement(const std::string& rRegName, const std::string& rRegFieldName, cuint64 fieldVal, cuint32 priority);
    SUBCLASS_DESTRUCTOR_DEFAULT(VmContextStateElement);
    ASSIGNMENT_OPERATOR_ABSENT(VmContextStateElement);

    Object* Clone() const override { return new VmContextStateElement(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "VmContextStateElement"; } //!< Return a string describing the actual type of the Object.

    std::string GetName() const override { return (mRegName + "." + mRegFieldName); } //!< Return the name of the StateElement.

    std::string GetRegisterName() const { return mRegName; } //!< Return the register name.
    std::string GetRegisterFieldName() const { return mRegFieldName; } //!< Return the register field name.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(VmContextStateElement);
  private:
    const std::string mRegName; //!< Register name
    const std::string mRegFieldName; //!< Register field name
  };

  /*!
    \class PrivilegeLevelStateElement
    \brief A privilege level value relevant to the state of the simulation.
  */
  class PrivilegeLevelStateElement : public StateElement {
  public:
    PrivilegeLevelStateElement(const EPrivilegeLevelType privLevel, cuint32 priority);
    SUBCLASS_DESTRUCTOR_DEFAULT(PrivilegeLevelStateElement);
    ASSIGNMENT_OPERATOR_ABSENT(PrivilegeLevelStateElement);

    Object* Clone() const override { return new PrivilegeLevelStateElement(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "PrivilegeLevelStateElement"; } //!< Return a string describing the actual type of the Object.

    std::string GetName() const override { return (EPrivilegeLevelType_to_string(mPrivLevel)); } //!< Return the name of the StateElement.
    bool IsDuplicate(const StateElement& rOther) const override; //!< Return true if this StateElement represents the same underlying State information as the other StateElement.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(PrivilegeLevelStateElement);
  private:
    EPrivilegeLevelType mPrivLevel; //!< Privilege level
  };

  /*!
    \class PcStateElement
    \brief A PC value relevant to the state of the simulation.
  */
  class PcStateElement : public StateElement {
  public:
    PcStateElement(cuint64 pcVal, cuint32 priority);
    SUBCLASS_DESTRUCTOR_DEFAULT(PcStateElement);
    ASSIGNMENT_OPERATOR_ABSENT(PcStateElement);

    Object* Clone() const override { return new PcStateElement(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "PcStateElement"; } //!< Return a string describing the actual type of the Object.

    std::string GetName() const override { return "PC"; } //!< Return the name of the StateElement.
  protected:
    COPY_CONSTRUCTOR_DEFAULT(PcStateElement);
  };

}

#endif  // Force_StateElement_H
