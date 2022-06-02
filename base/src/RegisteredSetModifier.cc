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
#include "RegisteredSetModifier.h"

#include "ChoicesModerator.h"
#include "Generator.h"
#include "Log.h"

using namespace std;

/*!
  \file RegisteredSetModifier.cc
  \brief Code to register set modifications
*/

namespace Force {
  class ModificationSet {
  public:
    ModificationSet(EChoicesType choicesType, uint32 mod_id) :
      mModificationID(mod_id), mChoicesType(choicesType)
    {

    }
    ~ModificationSet() { }
  public:
    uint32 mModificationID; //!< mod id
    EChoicesType mChoicesType; //!< choice type
  };

  Object* RegisteredSetModifier::Clone() const
  {
    return new RegisteredSetModifier(*this);
  }

  const std::string RegisteredSetModifier::ToString() const
  {
    return "Registered set modification";
  }

  RegisteredSetModifier::~RegisteredSetModifier()
  {
    for (auto mod : mRegisteredModificationSets)
      delete mod;

    for (auto mod : mCommittedStack)
      delete mod;

    mpGenerator = nullptr;
  }

  void RegisteredSetModifier::Setup(const Generator* pGen)
  {
    mpGenerator = pGen;
  }

  void RegisteredSetModifier::RegisterModificationSet(EChoicesType choicesType, uint32 mod_id)
  {
    ModificationSet *pModificationSet = new ModificationSet(choicesType, mod_id);
    mRegisteredModificationSets.push_back(pModificationSet);
    LOG(notice) << "{RegisteredSetModifier::RegisterModificationSet} Register modification id: " << mod_id << ", choices type :"
                <<  EChoicesType_to_string(choicesType) << endl;
  }

  void RegisteredSetModifier::ApplyModificationSet(uint32 mod_id)
  {
    // << "Apply mod id:" << mod_id << endl;
    for (auto it = mRegisteredModificationSets.begin(); it != mRegisteredModificationSets.end(); ) {
      if ((*it)->mModificationID >= mod_id) {
        auto choices_moderators = mpGenerator->GetChoicesModerators();
        choices_moderators->CommitModificationSet((*it)->mChoicesType, (*it)->mModificationID);
        LOG(notice) << "{RegisteredSetModifier::ApplyModificationSet} Committed modification id: " << (*it)->mModificationID << ", choices type :"
                    <<  EChoicesType_to_string((*it)->mChoicesType) << endl;
        mCommittedStack.push_back(*it);
        it = mRegisteredModificationSets.erase(it);
      }
      else
        ++ it;
    }
  }

  void RegisteredSetModifier::RevertModificationSet(uint32 mod_id)
  {
    while (not mCommittedStack.empty()) {
      auto pMod = mCommittedStack.back();
      if (pMod->mModificationID < mod_id)
        break;
      auto choices_moderators = mpGenerator->GetChoicesModerators();
      choices_moderators->RevertModificationSet(pMod->mChoicesType, pMod->mModificationID);
      LOG(notice) << "{RegisteredSetModifier::RevertModificationSet} Reverted modification mod_id: " << pMod->mModificationID << ", choices type :"
                  <<  EChoicesType_to_string(pMod->mChoicesType) << endl;
      delete pMod;
      mCommittedStack.pop_back();
    }
  }

}
