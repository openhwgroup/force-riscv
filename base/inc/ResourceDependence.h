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
#ifndef Force_ResourceDependence_H
#define Force_ResourceDependence_H

#include <ResourceAccess.h>
#include <Notify.h>
#include <NotifyDefines.h>

namespace Force {

  class Generator;
  class ChoiceTree;
  class Choice;
  class ChoiceVariable;
  class Variable;

  /*!
    \class ResourceDependence
    \brief resource dependence module.
  */
  class ResourceDependence : public ResourceAccessQueue, public NotificationReceiver {
  public:
    ResourceDependence();  //!< Constructor.
    ~ResourceDependence(); //!< destructor

    ASSIGNMENT_OPERATOR_ABSENT(ResourceDependence);
    Object* Clone() const override; //!< return cloned object
    const std::string ToString() const override;  //!< Return a string describing the current state of the Resource object.
    const char* Type() const override { return "ResourceDependence"; } //!< Return the type of the Resource in C string

    void Setup(const Generator* pGen); //!< set up choice and trees and so on
    const ConstraintSet* GetDependenceConstraint(ERegAttrType access, EResourceType resType, const ResourceAccessStage* pHotResource) const; //!< Get dependence constraint.
    void HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload) override; //!< Receive Notification override
    const ResourceDependence* Snapshot() const; //!< take a snapshot
  protected:
    ResourceDependence(const ResourceDependence& rOther); //!< Copy constructor.
    ResourceDependence(ResourceDependence& rOther); //!< Copy constructor.
    const ConstraintSet* GetInterDependenceConstraint(ERegAttrType access, EResourceType resType) const; //!< get resource inter-dependence on the register operand.
    const ConstraintSet* GetIntraDependenceConstraint(ERegAttrType access, EResourceType resType, const ResourceAccessStage* pHotResource) const; //!< get resource inter-dependence on the register operand.
    void UpdateChoiceTrees( ); //!< Clone choice trees.
    void UpdateVariable(const Variable* pVar); //!< Update variables.
    const ConstraintSet* ChooseResourceConstraint(EResourceType resType, EDependencyType depType) const; //!< choose resource by choices tree
    uint32 ChooseDependenceType(ERegAttrType access) const; //!< Choose dependence type.
  protected:
    const ChoiceTree* mpDependenceTree; //!< Pointer to the dependence choices tree.
    const ChoiceTree* mpSourceTree;  //!< source choice tree
    const ChoiceTree* mpTargetTree;  //!< target choice tree
    const ChoiceTree* mpPriorityTree; //!< priority choice tree
    const ChoiceTree* mpOptimalDirectionTree; //!< the direction choice tree for optimal dependency
    const ChoiceVariable* mpWindowVariable; //!< looking up Variable
    const Variable* mpSourceEntropyWindow; //!< Entropy window for source.
    const Variable* mpTargetEntropyWindow; //!< Entropy window for target.
    const Generator* mpGenerator; //!< the pointer to the generator
  };

}

#endif
