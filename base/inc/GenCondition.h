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
#ifndef Force_GenCondition_H
#define Force_GenCondition_H

#include <Defines.h>
#include <Object.h>
#include <Notify.h>
#include <NotifyDefines.h>
#include <Enums.h>
#include <map>

namespace Force {

  class Generator;

  /*!
    \class GenCondition
    \brief Base class for various generator condition classes.
  */
  class GenCondition : public Object, public NotificationReceiver {
  public:
    Object* Clone() const override = 0; //!< Clone a object of the same type, need to be implemented by derived classes.
    const std::string ToString() const override; //!< Return a string describing the current state of the GenCondition object.
    const char* Type() const override { return "GenCondition"; } //!< Return object type in a C string.

    GenCondition(); //!< Default constructor.
    virtual ~GenCondition() { } //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenCondition);
    bool ConditionTrue() const { return mCondition; } //!< Return whether the condition is true.
    virtual void Setup(const Generator& rGen) { } //!< Setup the GenCondition object.
    virtual void SignUp(const Generator& rGen) = 0; //!< sign up to sender
  protected:
    GenCondition(const GenCondition& rOther); //!< Copy constructor.
    void HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload) override; //!< Handle a notification.
    virtual bool UpdateCondition() = 0; //!< Update condition.
  protected:
    bool mCondition; //!< Condition state.
  };

  /*!
    \class GenConditionActor
    \brief Inherit from GenCondition class, with a Sender attribute.
  */
   class GenConditionActor : public GenCondition {
  public:
    const std::string ToString() const override; //!< Return a string describing the current state of the GenConditionActor object.
    const char* Type() const override { return "GenConditionActor"; } //!< Return object type in a C string.

    GenConditionActor(); //!< Default constructor.
    virtual ~GenConditionActor(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenConditionActor);
    void Setup(const Generator& rGen) override; //!< Setup the GenConditionActor object.
    const NotificationSender* GetSender() const { return mpSender; } //!< Return const pointer to sender object.
  protected:
    GenConditionActor(const GenConditionActor& rOther); //!< Copy constructor.
    virtual NotificationSender* SenderInstance() const; //!< Return a proper sender instance.
    void HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload) override; //!< Handle a notification.
  protected:
    NotificationSender* mpSender; //!< Sender attribute.
  };

  /*!
    \class GenConditionSet
    \brief Container of all GenCondition objects of the same GenThread.
  */
  class GenConditionSet : public Object {
  public:
    Object* Clone() const override; //!< Clone a GenConditionSet object.
    const std::string ToString() const override; //!< Return a string describing the current state of the GenConditionSet object.
    const char* Type() const override { return "GenCondition"; } //!< Return object type in a C string.

    GenConditionSet(); //!< Defeault constructor.
    ~GenConditionSet(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenConditionSet);

    void Setup(const Generator* pGen); //!< Setup the GenConditionSet and its sub items.
    void SignUp() const; //!< Sign up all conditions in the set
    GenCondition* GetCondition(const std::string& rCondName) const; //!< Return a GenCondition object with the name specified.
  protected:
    GenConditionSet(const GenConditionSet& rOther); //!< Copy constructor.
    void AddCondition(GenCondition* pCond) const; //!< Add a GenCondition object.
  protected:
    const Generator* mpGenerator; //!< Const pointer to Generator object.
    mutable std::map<std::string, GenCondition* > mConditions; //!< Map containing all GenCondition objects for the GenThread object.
  };

}

#endif
