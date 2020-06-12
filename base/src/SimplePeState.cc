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
#include <SimplePeState.h>
#include <Register.h>
#include <Log.h>

using namespace std;

/*!
  \file SimplePeState.cc
  \brief Code managing PE state save and restore.
*/

namespace Force {

  SimpleRegisterState::SimpleRegisterState(Register* pReg)
    : mpRegister(pReg), mValue(0)
  {
    mValue = mpRegister->Value();
  }

  void SimplePeState::SaveState(Generator* pGen, const std::vector<Register* >& rRegContext)
  {
    mRegisterStates.clear();
    mRegisterStates.reserve(rRegContext.size());

    transform(rRegContext.cbegin(), rRegContext.cend(), back_inserter(mRegisterStates),
      [](Register* pReg) { return SimpleRegisterState(pReg); });
  }

  bool SimplePeState::RestoreState()
  {
    bool state_changed = false;
    for (auto & reg_state : mRegisterStates) {
      auto reg_ptr = reg_state.mpRegister;
      if (reg_ptr->Value() != reg_state.mValue) {
        reg_ptr->SetValue(reg_state.mValue);
        LOG(notice) << "{PeState::RestoreState} restore register " << reg_ptr->Name() << " to 0x" << hex << reg_state.mValue << endl;
        state_changed = true;
      }
    }
    return state_changed;
  }

}
