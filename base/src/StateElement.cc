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
#include "StateElement.h"

#include <sstream>

#include "Log.h"

/*!
  \file StateElement.cc
  \brief Code supporting representing values relevant to the state of the simulation.
*/

using namespace std;

namespace Force {

  StateElement::StateElement(const EStateElementType stateElemType, const vector<uint64>& rValues, const vector<uint64>& rMasks, cuint32 priority)
    : Object(), mStateElemType(stateElemType), mValues(rValues), mMasks(rMasks), mPriority(priority)
  {
  }

  bool StateElement::IsDuplicate(const StateElement& rOther) const
  {
    if ((rOther.mStateElemType == mStateElemType) and (rOther.GetName() == GetName())) {
      return true;
    }

    return false;
  }

  bool StateElement::CanMerge(const StateElement& rOther) const
  {
    bool can_merge = IsDuplicate(rOther);
    if ((rOther.mValues.size() == mValues.size()) and (rOther.mMasks.size() == mMasks.size())) {
      for (size_t i = 0; i < mValues.size(); i++) {
        uint64 thisMask = mMasks[i];
        uint64 otherMask = rOther.mMasks[i];

        if ((thisMask & otherMask) != 0) {
          can_merge = false;
          break;
        }
      }
    }
    else {
      can_merge = false;
    }

    return can_merge;
  }

  void StateElement::Merge(const StateElement& rOther)
  {
    if (CanMerge(rOther)) {
      for (size_t i = 0; i < mValues.size(); i++) {
        // Mask the values before combining them
        uint64 thisMaskedValue = mValues[i] & mMasks[i];
        uint64 otherMaskedValue = rOther.mValues[i] & rOther.mMasks[i];

        mValues[i] = thisMaskedValue | otherMaskedValue;
        mMasks[i] = mMasks[i] | rOther.mMasks[i];
      }

      if (rOther.mPriority < mPriority) {
        mPriority = rOther.mPriority;
      }
    }
    else {
      LOG(fail) << "{StateElement::Merge} unable to merge " << rOther.GetName() << " with " << GetName() << "; the two StateElements must represent the same underlying state and must not have overlapping masks." << endl;
      FAIL("state-element-merge-failure");
    }
  }

  MemoryStateElement::MemoryStateElement(cuint64 startAddr, cuint64 value, cuint64 mask, cuint32 priority)
    : StateElement(EStateElementType::Memory, {value}, {mask}, priority), mStartAddr(startAddr)
  {
  }

  string MemoryStateElement::GetName() const
  {
    stringstream out_stream;
    out_stream << "0x" << hex << mStartAddr;
    return out_stream.str();
  }

  bool MemoryStateElement::IsDuplicate(const StateElement& rOther) const
  {
    const MemoryStateElement* other = dynamic_cast<const MemoryStateElement*>(&rOther);
    if ((other != nullptr) and (other->mStartAddr == mStartAddr)) {
      return true;
    }

    return false;
  }

  RegisterStateElement::RegisterStateElement(const EStateElementType stateElemType, const string& rRegName, cuint32 regIndex, const vector<uint64>& rValues, const vector<uint64>& rMasks, cuint32 priority)
    : StateElement(stateElemType, rValues, rMasks, priority), mRegName(rRegName), mRegIndex(regIndex)
  {
  }

  VmContextStateElement::VmContextStateElement(const string& rRegName, const string& rRegFieldName, cuint64 regFieldVal, cuint32 priority)
    : StateElement(EStateElementType::VmContext, {regFieldVal}, {MAX_UINT64}, priority), mRegName(rRegName), mRegFieldName(rRegFieldName)
  {
  }

  PrivilegeLevelStateElement::PrivilegeLevelStateElement(const EPrivilegeLevelType privLevel, cuint32 priority)
    : StateElement(EStateElementType::PrivilegeLevel, {EPrivilegeLevelTypeBaseType(privLevel)}, {MAX_UINT64}, priority), mPrivLevel(privLevel)
  {
  }

  bool PrivilegeLevelStateElement::IsDuplicate(const StateElement& rOther) const
  {
    const PrivilegeLevelStateElement* other = dynamic_cast<const PrivilegeLevelStateElement*>(&rOther);
    if (other != nullptr) {
      return true;
    }

    return false;
  }

  PcStateElement::PcStateElement(cuint64 pcVal, cuint32 priority)
    : StateElement(EStateElementType::PC, {pcVal}, {MAX_UINT64}, priority)
  {
  }

}
