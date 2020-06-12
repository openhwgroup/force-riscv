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
#ifndef Force_GenMode_H
#define Force_GenMode_H

#include <Object.h>
#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>

namespace Force {

  /*!
    \class GenMode
    \brief A simple class encapsulate Generator mode state transition.
  */
  class GenMode : public Object {
  public:
    explicit GenMode(EGenModeTypeBaseType modeType); //!< Constructor with mode type parameter given.
    Object* Clone() const override; //!< Clone a GenMode object of the same type.
    const std::string ToString() const override; //!< Return a string describing the current state of the GenMode object.
    const char* Type() const override { return "GenMode"; } //!< Return object type in a C string.

    inline bool CheckEscape() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::NoEscape)) == 0); } //!< Return whether escape check is on.
    inline bool SimulationEnabled() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::SimOff)) == 0); } //!< Return whether simulation is enabled.
    inline bool HasISS() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::NoIss)) == 0); } //!< Return whether ISS is available.
    inline bool ReExecution() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::ReExe)) != 0); } //!< Return whether the Generator is in re-execution mode.
    inline bool InException() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::Exception)) != 0); } //!< Return whether the Generator is in re-execution mode.
    inline bool NoSkip() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::NoSkip)) != 0); } //!< Return whether the Generator is in no-skip mode.
    inline bool InLoop() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::InLoop)) != 0); } //!< Return whether the Generator is in loop mode.
    inline bool DelayInit() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::DelayInit)) != 0); } //!< Return whether the Generator is in delay-init mode.
    inline bool NoJump() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::NoJump)) != 0); } //!< Return whether the Generator is in no-jump mode.
    inline bool LowPower() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::LowPower)) != 0); } //!< Return whether the Generator is in low power mode
    inline bool AddressProtection() const { return (((mMode & EGenModeTypeBaseType(EGenModeType::InLoop)) != 0) and not RecordingState()); } //!< Return whether the Generator is in a mode that needs address protection.
    inline bool RecordingState() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::RecordingState)) != 0); } //!< Return whether the Generator is in recording state mode.
    inline bool RestoreStateLoop() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::RestoreStateLoop)) != 0); } //!< Return whether the Generator is in RestoreStateLoop mode.
    inline bool IsFiller() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::Filler)) != 0); } //!< Return whether the Generator is in filler mode
    inline bool IsSpeculative() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::Speculative)) != 0); } //!< Return whether the Generator is in filler mode
    inline bool IsAddressShortage() const { return ((mMode & EGenModeTypeBaseType(EGenModeType::AddressShortage)) != 0); } //!< Return whether the Generator is in address shortage mode

    void PushGenMode(EGenModeTypeBaseType modeVar); //!< Push generator mode with value parameter.
    void PushGenMode(const std::string& inChange); //!< Push generator mode with string parameter.
    void PopGenMode(EGenModeTypeBaseType modeVar); //!< Pop generator mode with value parameter.
    void PopGenMode(const std::string& inChange); //!< Pop generator mode with string parameter.
    void SetGenMode(EGenModeTypeBaseType genMode); //!< Set generator mode.
    EGenModeTypeBaseType CurrentMode() const { return mMode; } //!< Return current generator mode.
    void EnableGenMode(EGenModeTypeBaseType modeVar); //!< Enable generator mode with value parameter; this mode persists until disabled, even when the mode stack is popped. If the mode being enabled was previously pushed onto the mode stack, this method will have no effect. If the mode being enabled was previously enabled via this method, but not disabled, the call will fail.
    void EnableGenMode(const std::string& inChange); //!< Enable generator mode with string parameter. See the other version of EnableGenMode() for additional details.
    void DisableGenMode(EGenModeTypeBaseType modeVar); //!< Disable generator mode with value parameter; use this method to disable modes that were enabled by EnableGenMode(). If the mode being disabled was previously pushed onto the mode stack, this method will have no effect. If the mode being disabled is not currently enabled, this method will fail.
    void DisableGenMode(const std::string& inChange); //!< Disable generator mode with string parameter. See the other version of DisableGenMode() for additional details.
    bool CheckAddressShortage() const { return !(IsFiller() || InException() || !CheckEscape()); } //!< Return whether address shortage check is on.
  protected:
    GenMode(); //!< Default constructor.
    GenMode(const GenMode& rOther); //!< Copy constructor.
    EGenModeTypeBaseType ParseModeChanges(const std::string& inChange) const; //!< Parse incoming mode changes.
  protected:
    EGenModeTypeBaseType mMode; //!< Generator mode, including overlay.
    EGenModeTypeBaseType mModeOverlay; //!< Generator mode overlay.
    std::vector<EGenModeTypeBaseType> mModeStack; //!< A stack of previous generator modes without overlays.
  private:
    void SetGenModeValue(EGenModeTypeBaseType genMode); //!< Set generator mode to the specified value.
  };

}

#endif
