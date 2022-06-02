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
#ifndef Force_ChoicesModerator_H
#define Force_ChoicesModerator_H

#include <list>
#include <map>

#include "Defines.h"
#include "Enums.h"
#include "Notify.h"
#include "NotifyDefines.h"
#include "Object.h"

namespace Force {

  class ChoiceTree;
  class ChoicesSet;

  /*!
    \class ChoicesTreeModifications
    \brief class of ChoicesTree modifications
  */

  class ChoiceTreeModifications {
  public:
    void ApplyModifications(ChoiceTree * choiceTree) const; //!< Apply modifications to a ChoiceTree
    void Merge(const ChoiceTreeModifications& rOther); //!< Merge modifications from "other", the modifcations of other take precedence.

    explicit ChoiceTreeModifications(const std::map<std::string, uint32>& modifications) : mModifications(modifications) { } //!< Constructor with modification map provided.
    ChoiceTreeModifications() : mModifications() {} //!< Default constructor.
    const std::map<std::string, uint32>& GetModifications(void) const { return mModifications; } //!<get modifications
  private:
    void ApplyOneModification(ChoiceTree* choiceTree, const std::string& name, uint32 weight) const; //!< apply one modification
  private:
    std::map<std::string, uint32> mModifications; //!< Modifications container.
  };

  /*!
    \class ChoicesModificationSet
    \brief class of Choices modification set
  */

  class ChoiceModificationSet {
  public:
    ChoiceModificationSet(): mId(0), mModificationSet() {} //!< default constructor
    explicit ChoiceModificationSet(uint32 id) : mId (id), mModificationSet() { } //!< constructor
    uint32 Id() const { return mId; } //!< Return modification set ID
    void Clear(); //!< Clear all modifications
    void ApplyModifications(ChoiceTree* choiceTree) const; //!< Apply modifications to a ChoiceTree, if applicable
    void Merge(const ChoiceModificationSet& rOther); //!< Merge modifications from the other set, the other set's modification take precedence
    uint32 AddChoicesModification(const std::string& choiceTreeName, const std::map<std::string, uint32>& modifications); //!< add modifications for a ChoiceTree to the set
    const std::string ToString() const; //!< return a string description of the contents of the modification set
    const std::map<std::string, ChoiceTreeModifications>& GetModificationSet(void) const { return mModificationSet;}
  private:
    uint32 mId; //!< modification set ID
    std::map<std::string, ChoiceTreeModifications> mModificationSet; //!< container of all the modifications in the set
   };


  /*!
    \class ChoicesModerator
    \brief Moderator class of a certain ChoicesSet
  */

  class ChoicesModerator : public Object, public NotificationSender {
  public:
    Object* Clone() const override;  //!< Return a cloned object of the same type and same contents of the ChoicesModerator object.
    const std::string ToString() const override; //!< Return a string describing the current state of the ChoicesModerator object.
    const char* Type() const override { return "ChoicesModerator"; } //!< Return a string describing the actual type of the ChoicesModerator Object

    explicit ChoicesModerator(const ChoicesSet* choicesSet); //!< Constructor with ChoicesSet pointer provided.
    ChoicesModerator() : Object(), mpChoicesSet(nullptr), mCurrentModificationSet(), mNewModificationSets(), mModificationSetStack() { } //!< Default constructor.
    ~ChoicesModerator(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(ChoicesModerator);

    ChoiceTree* CloneChoiceTree(const std::string& treeName) const; //!< clone a ChoiceTree from the base line choice set and apply ChoiceModification if there is any
    ChoiceTree* TryCloneChoiceTree(const std::string& treeName) const; //!< clone a ChoiceTree from the base line choice set and apply ChoiceModification if there is any, called when not sure if the choice tree exists or not
    void AddChoicesModification(const std::string& treeName, const std::map<std::string, uint32>& modifications, uint32 setId); //!< Add modifcations for a ChoiceTree to mNewModificationSet.
    void CommitModificationSet(uint32 setId); //!< Commit the mNewModifciationSet to the stack, clear it and increment the ID
    void RevertModificationSet(uint32 setId);  //!< Found and remove the modification set from the stack
    const ChoicesSet* GetChoicesSet() const { return mpChoicesSet; } //!< return ChoicesSet to be used directly.
    uint32 DoChoicesModification(const std::string& treeName,  const std::map<std::string, uint32>& modifications); //!< do choices modification for a choice tree
  private:
    ChoicesModerator(const ChoicesModerator& rOther); //!< Copy constructor.
    void ValidateModifications(const std::string& rTreeName, const std::map<std::string, uint32>& rModifications); //!< Fail if the specified choice modifications are not valid, e.g. the specified choice tree or choices do not exist.
  private:
    const ChoicesSet* mpChoicesSet; //!< Constant pointer to shared baseline choices set.
    ChoiceModificationSet mCurrentModificationSet; //!< a merge of all the modification sets in the stack
    std::list<ChoiceModificationSet* > mNewModificationSets; //!< New modification sets being constructed
    std::list<ChoiceModificationSet* > mModificationSetStack; //!< list used as stack to hold the ChoiceModificationSet objects affecting the current scope.
  };

  /*!
    \class ChoicesModerators
    \brief Moderators class for all kinds of choices moderator
  */
  class ChoicesModerators : public Object{
  public:
    Object* Clone() const override;  //!< Return a cloned object of the same type and same contents of the ChoicesModerator object.
    const std::string ToString() const override; //!< Return a string describing the current state of the ChoicesModerator object.
    const char* Type() const override { return "ChoicesModerators"; } //!< Return a string describing the actual type of the ChoicesModerator Object

    ChoicesModerators(): mChoicesModerators(), mId(1) {} //!< constructor
    ~ChoicesModerators();  //!< destructor

    void Setup(const std::vector<ChoicesSet*>& mChoicesSets); //!< set up moderators
    uint32 AddChoicesModification(EChoicesType choicesType, const std::string& treeName,
                  const std::map<std::string, uint32>& modifications); //!< add choice modification
    void CommitModificationSet(EChoicesType choicesType, uint32 set_id); //!< commit modification set
    void RevertModificationSet(EChoicesType choicesType, uint32 set_id); //!< revert modification set
    ChoicesModerator* GetChoicesModerator(EChoicesType choiceType) const { return mChoicesModerators[int(choiceType)]; } //!< Return a const pointer to a ChoicesModerator of the specified type.
  private:
    ChoicesModerators(const ChoicesModerators& rOther); //!< Copy constructor.
    uint32 GetId() { return mId ++ ;} //!< get id
  private:
    std::vector<ChoicesModerator* > mChoicesModerators; //!< Various ChoicesModerator classes
    uint32 mId; //!< unique id, starting from one, not zero
  };

}

#endif
