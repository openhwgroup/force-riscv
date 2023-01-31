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
#include "GenMode.h"

#include "Log.h"
#include "StringUtils.h"
#include "UtilityFunctions.h"

using namespace std;

/*!
  \file GenMode.cc
  \brief This module handles Generator mode transitions.
*/

namespace Force {

  GenMode::GenMode(EGenModeTypeBaseType modeType)
    : Object(), mMode(modeType), mModeOverlay(0), mModeStack()
  {
  }

  GenMode::GenMode()
    : Object(), mMode(0), mModeOverlay(0), mModeStack()
  {
  }

  GenMode::GenMode(const GenMode& rOther)
    : Object(rOther), mMode(rOther.mMode), mModeOverlay(rOther.mModeOverlay), mModeStack(rOther.mModeStack)
  {
  }

  Object* GenMode::Clone() const
  {
    return new GenMode(*this);
  }

  const string GenMode::ToString() const
  {
    return string("GenMode: ") + get_gen_mode_name(mMode);
  }

  EGenModeTypeBaseType GenMode::ParseModeChanges(const string& inChange) const
  {
    EGenModeTypeBaseType ret_mode = 0;

    StringSplitter ss(inChange, ',');
    while (!ss.EndOfString()) {
      string sub_str = ss.NextSubString();
      ret_mode |= EGenModeTypeBaseType(string_to_EGenModeType(sub_str));
    }
    return ret_mode;
  }

  void GenMode::PushGenMode(const string& inChange)
  {
    EGenModeTypeBaseType mode_change = ParseModeChanges(inChange);
    PushGenMode(mode_change);
  }

  void GenMode::PushGenMode(EGenModeTypeBaseType modeVar)
  {
    mModeStack.push_back(mMode & ~mModeOverlay);
    SetGenModeValue(mMode | modeVar);
  }

  void GenMode::PopGenMode(const string& inChange)
  {
    EGenModeTypeBaseType mode_change = ParseModeChanges(inChange);
    PopGenMode(mode_change);
  }

  void GenMode::PopGenMode(EGenModeTypeBaseType modeVar)
  {
    if ((mMode & modeVar) != modeVar) {
      LOG(fail) << "{GenMode::PopGenMode} passed in mode change: 0x" << hex << uint32(modeVar) << " check failed, current mode: " <<  get_gen_mode_name(mMode) << "(0x" << hex << uint32(mMode) << ")" << endl;
      FAIL("pop-gen-mode-check-failed");
    }

    if (mModeStack.size() == 0) {
      LOG(fail) << "{GenMode::PopGenMode} called with empty mode stack." << endl;
      FAIL("pop-empty-mode-stack");
    }

    EGenModeTypeBaseType prev_mode_no_overlay = mModeStack.back();
    mModeStack.pop_back();
    SetGenModeValue(prev_mode_no_overlay | mModeOverlay);
  }

  void GenMode::SetGenMode(EGenModeTypeBaseType genMode)
  {
    mModeOverlay = 0;
    SetGenModeValue(genMode);
  }

  void GenMode::EnableGenMode(EGenModeTypeBaseType modeVar)
  {
    if ((mModeOverlay & modeVar) == modeVar) {
      LOG(fail) << "{GenMode::EnableGenMode} mode: 0x" << hex << uint32(modeVar) << " is already enabled, current mode: " <<  get_gen_mode_name(mMode) << "(0x" << hex << uint32(mMode) << ")" << endl;
      FAIL("mode-already-enabled");
    }

    // << " enalbling gen-mode: " << get_gen_mode_name(modeVar) << endl;

    // Only add a mode to the overlay if it isn't already part of the generator mode; this prevents overlay modes from
    // interfering with those already added to the mode stack
    EGenModeTypeBaseType mode_to_add = (mMode ^ modeVar) & modeVar;
    mModeOverlay |= mode_to_add;
    SetGenModeValue(mMode | mode_to_add);
  }

  void GenMode::EnableGenMode(const string& inChange)
  {
    EGenModeTypeBaseType mode_change = ParseModeChanges(inChange);
    EnableGenMode(mode_change);
  }

  void GenMode::DisableGenMode(EGenModeTypeBaseType modeVar)
  {
    if ((mMode & modeVar) != modeVar) {
      LOG(fail) << "{GenMode::DisableGenMode} passed in mode change: 0x" << hex << uint32(modeVar) << " check failed, current mode: " <<  get_gen_mode_name(mMode) << "(0x" << hex << uint32(mMode) << ")" << endl;
      FAIL("disable-gen-mode-check-failed");
    }

    // << " disalbling gen-mode: " << get_gen_mode_name(modeVar) << endl;

    // Only remove a mode if it is part of the overlay; this prevents overlay modes from interfering with those added to
    // the mode stack
    EGenModeTypeBaseType mode_to_remove_complement = ~(mModeOverlay & modeVar);
    mModeOverlay &= mode_to_remove_complement;
    SetGenModeValue(mMode & mode_to_remove_complement);
  }

  void GenMode::DisableGenMode(const string& inChange)
  {
    EGenModeTypeBaseType mode_change = ParseModeChanges(inChange);
    DisableGenMode(mode_change);
  }

  void GenMode::SetGenModeValue(EGenModeTypeBaseType genMode)
  {
    LOG(notice) << "Generator mode changed to: " << get_gen_mode_name(genMode) << " from: " << get_gen_mode_name(mMode) << endl;
    mMode = genMode;
  }

}
