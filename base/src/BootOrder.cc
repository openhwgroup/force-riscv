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
#include <BootOrder.h>
#include <sstream>
#include <algorithm>
#include <Register.h>
#include <GenRequest.h>
#include <State.h>
#include <Log.h>

using namespace std;

/*!tab
  \file BootOrder.cc
  \
*/

namespace Force {

  using namespace std;

  // helper function to provide custom sort registers according to their boot value
  bool boot_comparator(const Register* lRegister, const Register* rRegister)
  {
    return lRegister->Boot() > rRegister->Boot();
  }

  RegisterElement::RegisterElement(Register* pRegister, EBootElementActionType actType)
      : BootElement(actType), mpRegister(pRegister), mValue(pRegister->InitialValue())
  {
  }

  const std::string RegisterElement::RegisterName() const
  {
    return mpRegister->Name();
  }

  const std::string RegisterElement::ToString() const
  {
    stringstream ss;
    ss << mpRegister->Boot() << " : " << mpRegister->Name() << " : " << mValue << endl;
    return ss.str();
  }

  const std::string LargeRegisterElement::ToString() const
  {
    stringstream ss;
    ss << mpRegister->Boot() << " : " << mpRegister->Name() << " : ";
    std::for_each(mValues.begin(), mValues.end(), [&](uint64 value){ ss << hex << value; });
    ss << endl;
    return ss.str();
  }

  LargeRegisterElement::LargeRegisterElement(Register* pRegister)
      : RegisterElement(pRegister, EBootElementActionType::LoadLargeRegister), mValues(), mIndex(0), mIndexForced(false)
  {
    auto large_reg_ptr = dynamic_cast<LargeRegister* >(pRegister);
    if (large_reg_ptr != nullptr) {
      mValues = large_reg_ptr->InitialValues();
    }
    else {
      LOG(info) << "{LargeRegisterElement} register: " << pRegister->Name() << ", initial value:0x" << hex << pRegister->InitialValue()  << endl;
      mValues.push_back(pRegister->InitialValue());
    }
  }

  uint32 LargeRegisterElement::ImmOffset() const
  {
    return mIndexForced ? mIndex : -1;
  }

  BootOrder::BootOrder(const BootOrder& rOther)
    : Object(rOther), mRegisterList(), mpLastGPR(nullptr), mLoadingBaseAddr(0), mLoadingSize(0)
  {

  }

  Object * BootOrder::Clone() const
  {
    return new BootOrder(*this);
  }

  BootOrder::~BootOrder()
  {
    for (auto element_ptr : mRegisterList) {
      delete element_ptr;
    }
    delete mpLastGPR;
  }

  const std::string BootOrder::ToString() const
  {
    stringstream ss;
    ss << "BootOrder is " << endl;
    for (auto element_ptr : mRegisterList)
    {
      ss << element_ptr->ToString() << endl;
    }

    return ss.str();
  }

  void BootOrder::AssignLastGPR(Register* pReg)
  {
    if (mpLastGPR != nullptr)
    {
      LOG(fail) << "trying to assign last GPR, but it already existing " << mpLastGPR->RegisterName() << endl;
      FAIL("boot-order-last-gpr-alreadyexisting-fail");
    }
    mpLastGPR = new RegisterElement(pReg);
  }

  void BootOrder::RegisterLoadingRequests(vector<GenRequest*>& rRequests)
  {
    for (auto element_ptr : mRegisterList)
    {
      switch (element_ptr->ActionType()) {
      case EBootElementActionType::LoadRegister:
        rRequests.push_back(new GenLoadRegister(element_ptr->RegisterName(), element_ptr->Value()));
        if (element_ptr->RegisterName() == mpLastGPR->RegisterName()) {
          rRequests.push_back(new GenSetRegister(mpLastGPR->GetRegister(), element_ptr->Value()));
        }
        break;
      case EBootElementActionType::LoadLargeRegister: {
        auto large_element_ptr = element_ptr->CastInstance<LargeRegisterElement>();
        rRequests.push_back(new GenLoadLargeRegister(large_element_ptr->RegisterName(), large_element_ptr->Values(), mpLastGPR->RegisterName(), large_element_ptr->ImmOffset()));
        break;
        }
      case EBootElementActionType::InstructionBarrier:
        AppendInstructionBarrier(rRequests);
        break;
      //case EBootElementActionType::DataBarrier:
      //break;
      default:
        LOG(fail) << "{BootOrder::RegisterLoadingRequests} unsupported boot element action : " << EBootElementActionType_to_string(element_ptr->ActionType()) << endl;
        FAIL("unsupported-boot-element-action");
      }
    }
  }

  void BootOrder::CreateStateElements(State* pState)
  {
    for (auto element_ptr : mRegisterList) {
      switch (element_ptr->ActionType()) {
      case EBootElementActionType::LoadRegister:
        pState->AddRegisterStateElement(element_ptr->RegisterName(), {element_ptr->Value()}, 1);
        break;
      case EBootElementActionType::LoadLargeRegister:
        {
          auto large_element_ptr = element_ptr->CastInstance<LargeRegisterElement>();
          pState->AddRegisterStateElement(large_element_ptr->RegisterName(), large_element_ptr->Values(), 1);
        }

        break;
      case EBootElementActionType::InstructionBarrier:
        // Instruction barriers should be generated automatically by the appropriate
        // StateTransitionHandler when needed.
        break;
      default:
        LOG(fail) << "{BootOrder::BootStateTransitionRequest} unsupported boot element action : " << EBootElementActionType_to_string(element_ptr->ActionType()) << endl;
        FAIL("unsupported-boot-element-action");
      }
    }
  }

  void BootOrder::JumpToTestStart(vector<GenRequest*>& rRequests)
  {
    auto jump_req = new GenSequenceRequest("JumpToStart");
    rRequests.push_back(jump_req);
  }

}
