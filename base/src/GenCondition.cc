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
#include <GenCondition.h>
#include <ObjectRegistry.h>
#include <Log.h>

using namespace std;

/*!
  \file GenCondition.cc
  \brief GenCodition and its sub classes.
*/

namespace Force {

  GenCondition::GenCondition()
    : Object(), Receiver(), mCondition(false)
  {
  }

  GenCondition::GenCondition(const GenCondition& rOther)
    : Object(rOther), Receiver(rOther), mCondition(false)
  {

  }

  const string GenCondition::ToString() const
  {
    return Type();
  }

  void GenCondition::HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload)
  {
    mCondition = UpdateCondition();
  }

  GenConditionActor::GenConditionActor()
    : GenCondition(), mpSender(nullptr)
  {

  }

  GenConditionActor::GenConditionActor(const GenConditionActor& rOther)
    : GenCondition(rOther), mpSender(nullptr)
  {

  }

  GenConditionActor::~GenConditionActor()
  {
    delete mpSender;
  }

  NotificationSender* GenConditionActor::SenderInstance() const
  {
    return new NotificationSender();
  }

  void GenConditionActor::Setup(const Generator& rGen)
  {
    GenCondition::Setup(rGen);

    delete mpSender;
    mpSender = SenderInstance();
  }

  const string GenConditionActor::ToString() const
  {
    string ret_str = Type();

    if (nullptr != mpSender) {
      ret_str += mpSender->Description();
    }

    return ret_str;
  }

  void GenConditionActor::HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload)
  {
    auto old_cond = mCondition;
    GenCondition::HandleNotification(sender, eventType, pPayload);

    if (old_cond != mCondition) {
      // condition changed.
      mpSender->SendNotification(ENotificationType::ConditionUpdate, this);
    }
  }

  GenConditionSet::GenConditionSet()
    : Object(), mpGenerator(nullptr), mConditions()
  {

  }

  GenConditionSet::GenConditionSet(const GenConditionSet& rOther)
    : Object(rOther), mpGenerator(nullptr), mConditions()
  {
    for (auto other_item : rOther.mConditions) {
      mConditions[other_item.first] = dynamic_cast<GenCondition* >(other_item.second->Clone());
    }
  }

  GenConditionSet::~GenConditionSet()
  {
    for (auto cond_item : mConditions) {
      delete cond_item.second;
    }
  }

  Object* GenConditionSet::Clone() const
  {
    return new GenConditionSet(*this);
  }

  const string GenConditionSet::ToString() const
  {
    return Type();
  }

  void GenConditionSet::Setup(const Generator* pGen)
  {
    mpGenerator = pGen;

    for (auto cond_item : mConditions) {
      cond_item.second->Setup(*pGen);
    }
  }

  void GenConditionSet::SignUp() const
  {
    for (auto cond_item : mConditions) {
      cond_item.second->SignUp(*mpGenerator);
    }
  }

  GenCondition* GenConditionSet::GetCondition(const std::string& rCondName) const
  {
    auto cond_finder = mConditions.find(rCondName);
    if (cond_finder != mConditions.end())
      return cond_finder->second;

    // not found existing GenCondition object, try to obtain an instance from ObjectRegistry.

    auto cond_inst = dynamic_cast<GenCondition* >(ObjectRegistry::Instance()->ObjectInstance(rCondName));
    AddCondition(cond_inst);
    return cond_inst;
  }

  void GenConditionSet::AddCondition(GenCondition* pCond) const
  {
    auto cond_name = pCond->Type();
    auto cond_finder = mConditions.find(cond_name);
    if (cond_finder != mConditions.end()) {
      LOG(fail) << "{GenConditionSet::AddCondition} adding duplicated GenCondition: \"" << cond_name << "\"." << endl;
      FAIL("duplicated-cond-type");
    }
    mConditions[cond_name] = pCond;
  }

}
