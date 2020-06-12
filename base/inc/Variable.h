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
#ifndef Force_Data_H
#define Force_Data_H

#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <string>
#include <Defines.h>
#include <Object.h>
#include <Notify.h>
#include <NotifyDefines.h>

#include <vector>
#include <map>

namespace Force {

  class ChoiceTree;

  /*!
    \class Variable
    \brief an interface for variable
  */
  class Variable : public NotificationSender, public Object {
  public:
    Object* Clone() const override;  //!< Return a cloned object of the same type and same contents of the Variable object.
    const std::string ToString() const override; //!< Return a string describing the current state of the Variable object.
    const char* Type() const override { return "Variable"; } //!< Return a string describing the actual type of the Variable Object

    Variable() : Sender(), Object(), mName(), mStrValue() { }  //!< constructor
    virtual ~Variable() {} //!< virtual destructor

    virtual void SetUp(const std::string& value); //!< Set up variable into specific object.
    virtual void CleanUp()  { mStrValue = ""; }; //!< Clean up object built.

    virtual  uint64 Value() const;

    void SetName(const std::string& rName) { mName = rName; } //!< Set variable name.
    const std::string& Name() const { return mName; } //!< Return variable name.
    const std::string& GetValue() const { return mStrValue; } //!< get value

    /*!
      Templated function so that a derived class can conveniently cast base class to the desired derived class type.
      For example, ValueVariable* val = var->CastInstance<ValueVariable>();
     */
    template<typename T>
      T* CastInstance()
      {
        T* cast_instance = dynamic_cast<T* >(this);
        return cast_instance;
      }

  protected:
    Variable(const Variable& rOther); //!< Copy constructor
  protected:
    std::string mName; //!< Variable name.
    std::string mStrValue;  //!< The value for variable.
  };

  /*!
    \class ValueVariable
    \brief A derived class for value variable
  */
  class ValueVariable : public Variable {
  public:
    Object* Clone() const override;  //!< Return a cloned object of the same type and same contents of the ValueVariable object.
    const char* Type() const override { return "ValueVariable"; } //!< Return a string describing the actual type of the Variable Object
    ValueVariable() : Variable(), mIntValue(0) { }  //!< constructor
    ~ValueVariable() { } //!< destructor

    void SetUp(const std::string& value) override; //!< build variable for value type
    uint64 Value() const override { return mIntValue; } //!< get value
  protected:
    ValueVariable(const ValueVariable& rOther);
  protected:
    uint64 mIntValue; //!< value
  };

  /*!
    \class ChoiceVariable
    \brief A derived class for choice variable
  */
  class ChoiceVariable : public Variable {
  public:
    Object* Clone() const override;  //!< Return a cloned object of the same type and same contents of the ChoiceVariable object.
    const char* Type() const override { return "ChoiceVariable"; } //!< Return a string describing the actual type of the ChoiceVariable Object
    ChoiceVariable() : Variable(), mpChoiceTree(nullptr), mRangeLimit(MAX_UINT32) { } //!< constructor
    ~ChoiceVariable();  //!< destructor

    ASSIGNMENT_OPERATOR_ABSENT(ChoiceVariable);
    void SetUp(const std::string& value) override; //!< build variable for value type
    void CleanUp() override; //!< clean up objects
    const ChoiceTree* GetChoiceTree() const { return mpChoiceTree; } //!< Return a const pointer to the choice tree object.
    ChoiceTree* GetChoiceTreeMutable() const { return mpChoiceTree; } //!< Return a modifiable pointer to the choice tree object.
    void LimitRange(uint32 limit) const; //!< Set maximum value to ranges.
  protected:
    ChoiceVariable(const ChoiceVariable& rOther); //!< Copy constructor
  protected:
    ChoiceTree *mpChoiceTree; //!< Pointer to choice tree.
    mutable uint32 mRangeLimit; //!< Maximum value allowed for choices.
  };

   /*!
    \class VariableSet
    \brief container for the variables with the same type
  */
  class VariableSet {
  public:
    explicit VariableSet(EVariableType type) : mVariables(), mType(type) {} //!< constructor
    ~VariableSet() { //!< destructor
      for (auto& var : mVariables)
      delete var.second;
    }

    void AddVariable(const std::string& name, const std::string& value); //!< add one varaible
    const Variable* FindVariable(const std::string& name) const; //!< Find varaible by its name, const version.
    Variable* FindVariable(const std::string& rName); //!< Find varaible by its name, modifiable version.
    const Variable* GetVariable(const std::string& rName) const; //!< Get a variable by its name.
    void ModifyVariable(const std::string& name, const std::string& value); //!< modify variable value
    EVariableType VariableType() const { return mType; } //!< get variable type
  protected:
    std::map<std::string, Variable*> mVariables; //!< the map for variable
    Variable* InstantiateVariable(); //!< instantiate variable object
    EVariableType mType; //!< variable type for the set
  };

  /*!
    \ class VariableModerator
    \ brief moderator for variable set
  */
  class VariableModerator : public Object {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(VariableModerator);
    Object* Clone() const override;  //!< Return a cloned object of the same type and same context
    const std::string ToString() const override; //!< Return a string describing the current state of the VariableModerator object.
    const char* Type() const override { return "VariableModerator"; } //!< Return a string describing the actual type of the VariableModerator Object
    explicit VariableModerator(VariableSet* variable_set) : mpVariableSet(variable_set) {} //!< constructor
    ~VariableModerator() { mpVariableSet = nullptr; } //!< destructor
    VariableSet* GetVariableSet() const { return mpVariableSet; } //!< get variable set
  protected:
    VariableModerator(const VariableModerator& rOther); //!< copy constructor
    VariableSet *mpVariableSet; //!< the pointer to variable set
  };

}

#endif
