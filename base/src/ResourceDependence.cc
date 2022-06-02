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
#include "ResourceDependence.h"

#include <sstream>

#include "Choices.h"
#include "ChoicesModerator.h"
#include "Config.h"
#include "Constraint.h"
#include "GenException.h"
#include "Generator.h"
#include "Log.h"
#include "UtilityFunctions.h"
#include "Variable.h"

using namespace std;

/*!
  \file ResourceDependence.cc
  \brief Code to generate resource dependence like register inter-dependence and register intra-dependence.
*/

namespace Force {

  ResourceDependence::ResourceDependence()
    : ResourceAccessQueue(), mpDependenceTree(nullptr), mpSourceTree(nullptr), mpTargetTree(nullptr), mpPriorityTree(nullptr),
      mpOptimalDirectionTree(nullptr), mpWindowVariable(nullptr), mpSourceEntropyWindow(nullptr), mpTargetEntropyWindow(nullptr), mpGenerator(nullptr)
  {
  }

  ResourceDependence::ResourceDependence(const ResourceDependence& rOther)
    : ResourceAccessQueue((const ResourceAccessQueue&) rOther), mpDependenceTree(nullptr), mpSourceTree(nullptr), mpTargetTree(nullptr), mpPriorityTree(nullptr),
      mpOptimalDirectionTree(nullptr), mpWindowVariable(nullptr), mpSourceEntropyWindow(nullptr), mpTargetEntropyWindow(nullptr), mpGenerator(nullptr)
  {
    // << "copy constructor const version" << endl;
  }

  ResourceDependence::ResourceDependence(ResourceDependence& rOther)
    : ResourceAccessQueue((ResourceAccessQueue&) rOther), mpDependenceTree(nullptr), mpSourceTree(nullptr), mpTargetTree(nullptr), mpPriorityTree(nullptr),
      mpOptimalDirectionTree(nullptr), mpWindowVariable(nullptr), mpSourceEntropyWindow(nullptr), mpTargetEntropyWindow(nullptr), mpGenerator(rOther.mpGenerator)

  {
    // << "copy constructor non-const version" << endl;
    mpDependenceTree = dynamic_cast<ChoiceTree* >(rOther.mpDependenceTree->Clone());
    mpSourceTree = dynamic_cast<ChoiceTree* >(rOther.mpSourceTree->Clone());
    mpTargetTree = dynamic_cast<ChoiceTree* >(rOther.mpTargetTree->Clone());
    mpPriorityTree = dynamic_cast<ChoiceTree* >(rOther.mpPriorityTree->Clone());
    mpOptimalDirectionTree = dynamic_cast<ChoiceTree* >(rOther.mpOptimalDirectionTree->Clone());
    mpWindowVariable = dynamic_cast<ChoiceVariable* >(rOther.mpWindowVariable->Clone());
    mpSourceEntropyWindow = dynamic_cast<Variable* >(rOther.mpSourceEntropyWindow->Clone());
    mpTargetEntropyWindow = dynamic_cast<Variable* >(rOther.mpTargetEntropyWindow->Clone());
  }

  ResourceDependence::~ResourceDependence()
  {
    delete mpDependenceTree;
    delete mpSourceTree;
    delete mpTargetTree;
    delete mpPriorityTree;
    delete mpOptimalDirectionTree;

    mpWindowVariable = nullptr;
    mpSourceEntropyWindow = nullptr;
    mpTargetEntropyWindow = nullptr;
  }

  Object* ResourceDependence::Clone() const
  {
    return new ResourceDependence((const ResourceDependence&) *this);
  }

  const ResourceDependence* ResourceDependence::Snapshot() const
  {
    return new ResourceDependence((ResourceDependence&) *this);
  }

  const std::string ResourceDependence::ToString() const
  {
    return ResourceAccessQueue::ToString();
  }

  void ResourceDependence::Setup(const Generator* pGen)
  {
    mpGenerator = pGen;

    Config * config_ptr = Config::Instance();
    auto limit_value = config_ptr->LimitValue(ELimitType::DependencyHistoryLimit);
    ResourceAccessQueue::Setup(limit_value);

    for (EResourceTypeBaseType i = 0; i < EResourceTypeSize; ++ i) {
      EResourceType res_type = EResourceType(i);

      ResourceTypeAges* res_type_ages = new ResourceTypeAges(res_type);
      res_type_ages->Setup(pGen->GetResourceCount(res_type));
      mTypeAges.push_back(res_type_ages);

      ResourceTypeEntropy* res_type_entropy = new ResourceTypeEntropy(res_type);
      mTypeEntropies.push_back(res_type_entropy);
    }

    mpGenerator->GetChoicesModerator(EChoicesType::DependenceChoices)->SignUp(this);

    UpdateChoiceTrees();

    const VariableModerator* var_mod = mpGenerator->GetVariableModerator(EVariableType::Choice);
    mpWindowVariable = dynamic_cast<const ChoiceVariable*>(var_mod->GetVariableSet()->FindVariable("Inter-Dependency Window"));
    if (mpWindowVariable == nullptr) {
      LOG(fail) << "Failed to find variable: inter-Depdendency Looking up Window" << endl;
      FAIL("Failed-to-find-variable");
    } else {
      // limit the ranges based on history limit, in case use specified ranges out of bound.
      mpWindowVariable->LimitRange(limit_value);
    }

    const VariableModerator* str_var_mod = mpGenerator->GetVariableModerator(EVariableType::String);
    mpSourceEntropyWindow = str_var_mod->GetVariableSet()->GetVariable("Source Entropy Window");
    mpSourceEntropyWindow->SignUp(this);
    mpTargetEntropyWindow = str_var_mod->GetVariableSet()->GetVariable("Target Entropy Window");
    mpTargetEntropyWindow->SignUp(this);

    UpdateVariable(mpSourceEntropyWindow);
    UpdateVariable(mpTargetEntropyWindow);
  }

  void ResourceDependence::UpdateChoiceTrees()
  {
    delete mpDependenceTree;
    mpDependenceTree = nullptr;
    delete mpSourceTree;
    mpSourceTree = nullptr;
    delete mpTargetTree;
    mpTargetTree = nullptr;
    delete mpPriorityTree;
    mpPriorityTree = nullptr;
    delete mpOptimalDirectionTree;
    mpOptimalDirectionTree = nullptr;

    const ChoicesModerator* choices_mod = mpGenerator->GetChoicesModerator(EChoicesType::DependenceChoices);
    try {
      mpDependenceTree = choices_mod->CloneChoiceTree("Register Dependency");
      mpSourceTree = choices_mod->CloneChoiceTree("Source Dependency");
      mpTargetTree = choices_mod->CloneChoiceTree("Target Dependency");
      mpPriorityTree = choices_mod->CloneChoiceTree("Inter-Dependency Priority");
      mpOptimalDirectionTree = choices_mod->CloneChoiceTree("The Optimal Inter-Dependency Direction");
    }
    catch (const ChoicesError& choices_err) {
      LOG(fail) << "{ResourceDependence::Setup} " << choices_err.what() << endl;
      FAIL("resource-dependence-setup-error");
    }
  }

  void ResourceDependence::UpdateVariable(const Variable* pVar)
  {
    if (pVar == mpSourceEntropyWindow) {
      const auto & var_str = mpSourceEntropyWindow->GetValue();
      for (EResourceTypeBaseType i = 0; i < EResourceTypeSize; ++ i) {
        ResourceTypeEntropy* res_type_entropy = mTypeEntropies[i];
        AccessEntropy& acc_entropy = res_type_entropy->SourceEntropy();
        acc_entropy.SetThresholds(var_str);
        LOG(notice) << "{ResourceDependence::UpdateVariable} " << EResourceType_to_string(EResourceType(i)) << ", " << acc_entropy.ToString() << endl;
      }
    }
    else if (pVar == mpTargetEntropyWindow) {
      const auto & var_str = mpTargetEntropyWindow->GetValue();
      for (EResourceTypeBaseType i = 0; i < EResourceTypeSize; ++ i) {
        ResourceTypeEntropy* res_type_entropy = mTypeEntropies[i];
        AccessEntropy& acc_entropy = res_type_entropy->DestEntropy();
        acc_entropy.SetThresholds(var_str);
        LOG(notice) << "{ResourceDependence::UpdateVariable} " << EResourceType_to_string(EResourceType(i)) << ", " << acc_entropy.ToString() << endl;
      }
    }
    else {
      LOG(fail) << "{ResourceDependence::UpdateVariable} unsupported variable: " << pVar->Name() << endl;
      FAIL("unsupported-variable");
    }
  }

  const ConstraintSet* ResourceDependence::GetDependenceConstraint(ERegAttrType access, EResourceType resType, const ResourceAccessStage* pHotResource) const
  {
    const ConstraintSet* dep_constr = nullptr;
    auto dep_val = mpDependenceTree->Choose()->Value();
    switch (dep_val) {
    case 0:  // no dependency
      break;
    case 1: // inter dependency
      dep_constr = GetInterDependenceConstraint(access, resType);
      break;
    case 2: // intra dependency
      {
        dep_constr = GetIntraDependenceConstraint(access, resType, pHotResource);
        break;
      }
    default:
      LOG(fail) << "{ResourceDependence::GetDependenceConstraint} Unimplemented register dependence value: "<<  dep_val << endl;
      FAIL("invalid register dependence choice value");
    }

    return dep_constr;
  }

  uint32 ResourceDependence::ChooseDependenceType(ERegAttrType access) const
  {
    const Choice* choice_ptr = nullptr;
    switch (access) {
    case ERegAttrType::Read:
    case ERegAttrType::ReadWrite:
      choice_ptr = mpSourceTree->Choose();
      break;
    case ERegAttrType::Write:
      choice_ptr = mpTargetTree->Choose();
      break;
    default:
      LOG(fail) << "{ResourceDependence::ChooseDependenceType} Unexpected register attribute:\"" << ERegAttrType_to_string(access) << "\"" << endl;
      FAIL("unexpected-register-attribute");
    }
    return choice_ptr->Value();
  }

  const ConstraintSet* ResourceDependence::GetInterDependenceConstraint(ERegAttrType access, EResourceType resType) const
  {
    EDependencyType dep_type = EDependencyType(ChooseDependenceType(access));

    if (EDependencyType::NoDependency == dep_type) {
      return nullptr;
    }

    bool entropy_stable = EntropyStable(resType, dep_type);
    LOG(info) << "{ResourceDependence::GetInterDependenceConstraint} " <<  EResourceType_to_string(resType) << ", " << EDependencyType_to_string(dep_type) << " entropy stable? " << entropy_stable << endl;
    if (not entropy_stable) {
      return nullptr;
    }

    return ChooseResourceConstraint(resType, dep_type);
  }

  const ConstraintSet* ResourceDependence::GetIntraDependenceConstraint(ERegAttrType access, EResourceType resType, const ResourceAccessStage* pHotResource) const
  {
    EDependencyType dep_type = EDependencyType(ChooseDependenceType(access));

    if (EDependencyType::NoDependency == dep_type) return nullptr;

    return pHotResource->GetDependenceConstraint(resType, dep_type);
  }

  void ResourceDependence::HandleNotification(const NotificationSender* pSender, ENotificationType eventType, Object* pPayload)
  {
    switch (eventType) {
    case ENotificationType::ChoiceUpdate:
      UpdateChoiceTrees();
      break;
    case ENotificationType::VariableUpdate:
      UpdateVariable(dynamic_cast<const Variable* >(pSender));
      break;
    default:
      LOG(fail) << "{ResourceInterDependence::HandleNotification} unexpected event: " << ENotificationType_to_string(eventType) << endl;
      FAIL("unexpected-event-type");
    }
  }

  const ConstraintSet* ResourceDependence::ChooseResourceConstraint(EResourceType resType, EDependencyType depType) const
  {
    auto priority = mpPriorityTree->Choose()->Value();
    auto choice_window = dynamic_cast<const RangeChoice*>(mpWindowVariable->GetChoiceTree()->Choose());
    // << "choice range: " << choice_window->ToString() << endl;
    uint32 low = 0, high = 0;
    choice_window->GetRange(low, high);
    const ConstraintSet* return_constr = nullptr;
    switch (priority) {
    case 0:  // the neareast
      return_constr = ChosenAccessStage(low)->GetDependenceConstraint(resType, depType);
      break;
    case 1: // the farest
      return_constr = ChosenAccessStage(high)->GetDependenceConstraint(resType, depType);
      break;
    case 2:  // the optimal
      {
        auto direction = mpOptimalDirectionTree->Choose()->Value();
        ConstraintSet windowConstraint(low, high);
        uint32 chosen_value =  windowConstraint.ChooseValue();
        WindowLookUp* lookup_ptr = (direction == 0) ? (WindowLookUp*)(&mLookUpFar) : (WindowLookUp*)(&mLookUpNear);
        lookup_ptr->SetRange(low, high);
        return_constr = GetOptimalResourceConstraint(chosen_value, *lookup_ptr, resType, depType);
      }
      break;
    case 3: // random
      {
        return_constr = GetRandomResourceConstraint(low, high, resType, depType);
      }
      break;
    default:
      LOG(fail) << "Unimplemented priority choise value: "<<  priority << endl;
      FAIL("invalid priority choice value");
    }

    return return_constr;
  }

}
