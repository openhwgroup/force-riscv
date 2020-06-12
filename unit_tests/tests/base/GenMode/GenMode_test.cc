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
#include <lest/lest.hpp>
#include <Log.h>
#include <GenMode.h>
#include <Enums.h>
#include <memory>

using text = std::string;
using namespace Force;

// This function tests positive modes such that when the specified generator mode is pushed onto the stack, the
// isModeEnabled() method will return true.
void test_mode_toggling_positive(lest::env& lest_env, const EGenModeType genModeType, bool (GenMode::*isModeEnabled)() const, GenMode* genMode)
{
  EXPECT(not (genMode->*isModeEnabled)());
  genMode->PushGenMode(EGenModeTypeBaseType(genModeType));
  EXPECT((genMode->*isModeEnabled)());
  genMode->PopGenMode(EGenModeTypeBaseType(genModeType));
  EXPECT(not (genMode->*isModeEnabled)());
}

// This function tests negative modes such that when the specified generator mode is pushed onto the stack, the
// isModeEnabled() method will return false.
void test_mode_toggling_negative(lest::env& lest_env, const EGenModeType genModeType, bool (GenMode::*isModeEnabled)() const, GenMode* genMode)
{
  EXPECT((genMode->*isModeEnabled)());
  genMode->PushGenMode(EGenModeTypeBaseType(genModeType));
  EXPECT(not (genMode->*isModeEnabled)());
  genMode->PopGenMode(EGenModeTypeBaseType(genModeType));
  EXPECT((genMode->*isModeEnabled)());
}

const lest::test specification[] = {

CASE( "Test GenMode" ) {

  SETUP( "Setup GenMode" )  {
    GenMode gen_mode(EGenModeTypeBaseType(0));

    SECTION( "Test cloning GenMode" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      std::unique_ptr<GenMode> gen_mode_clone(dynamic_cast<GenMode*>(gen_mode.Clone()));
      EXPECT(gen_mode_clone->NoSkip());
      EXPECT(not gen_mode_clone->InException());
    }

    SECTION( "Test converting GenMode to string" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
      EXPECT(gen_mode.ToString() == "GenMode: ReExe,IssSim");
    }

    SECTION( "Test GenMode type" ) {
      EXPECT(gen_mode.Type() == "GenMode");
    }

    SECTION( "Test escape mode" ) {
      test_mode_toggling_negative(lest_env, EGenModeType::NoEscape, &GenMode::CheckEscape, &gen_mode);
    }

    SECTION( "Test simulation enabled" ) {
      test_mode_toggling_negative(lest_env, EGenModeType::SimOff, &GenMode::SimulationEnabled, &gen_mode);
    }

    SECTION( "Test ISS enabled" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::SimOff));
      test_mode_toggling_negative(lest_env, EGenModeType::NoIss, &GenMode::HasISS, &gen_mode);
    }

    SECTION( "Test re-execution mode" ) {
      test_mode_toggling_positive(lest_env, EGenModeType::ReExe, &GenMode::ReExecution, &gen_mode);
    }

    SECTION( "Test exception mode" ) {
      test_mode_toggling_positive(lest_env, EGenModeType::Exception, &GenMode::InException, &gen_mode);
    }

    SECTION( "Test no-skip mode" ) {
      test_mode_toggling_positive(lest_env, EGenModeType::NoSkip, &GenMode::NoSkip, &gen_mode);
    }

    SECTION( "Test loop mode" ) {
      test_mode_toggling_positive(lest_env, EGenModeType::InLoop, &GenMode::InLoop, &gen_mode);
    }

    SECTION( "Test delay-init mode" ) {
      test_mode_toggling_positive(lest_env, EGenModeType::DelayInit, &GenMode::DelayInit, &gen_mode);
    }

    SECTION( "Test no-jump mode" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::SimOff));
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::NoEscape));
      test_mode_toggling_positive(lest_env, EGenModeType::NoJump, &GenMode::NoJump, &gen_mode);
    }

    SECTION( "Test lower power mode" ) {
      test_mode_toggling_positive(lest_env, EGenModeType::LowPower, &GenMode::LowPower, &gen_mode);
    }

    SECTION( "Test address protection mode" ) {
      test_mode_toggling_positive(lest_env, EGenModeType::InLoop, &GenMode::AddressProtection, &gen_mode);

      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::InLoop));
      test_mode_toggling_negative(lest_env, EGenModeType::RecordingState, &GenMode::AddressProtection, &gen_mode);
    }

    SECTION( "Test recording state mode" ) {
      test_mode_toggling_positive(lest_env, EGenModeType::RecordingState, &GenMode::RecordingState, &gen_mode);
    }

    SECTION( "Test filler mode" ) {
      test_mode_toggling_positive(lest_env, EGenModeType::Filler, &GenMode::IsFiller, &gen_mode);
    }

    SECTION( "Test speculative mode" ) {
      test_mode_toggling_positive(lest_env, EGenModeType::Speculative, &GenMode::IsSpeculative, &gen_mode);
    }

    SECTION( "Test address shortage mode" ) {
      test_mode_toggling_positive(lest_env, EGenModeType::AddressShortage, &GenMode::IsAddressShortage, &gen_mode);
    }

    SECTION( "Test popping mode with intervening push" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::RecordingState));
      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));

      // We expect RecordingState mode to be removed because it was the last mode pushed onto the stack, even though
      // ReExe mode is specified in the call to PopGenMode()
      EXPECT(not gen_mode.RecordingState());
      EXPECT(gen_mode.ReExecution());
    }

    SECTION( "Test changing mode by string" ) {
      EXPECT(not gen_mode.InLoop());
      gen_mode.PushGenMode("InLoop");
      EXPECT(gen_mode.InLoop());
      gen_mode.PopGenMode("InLoop");
      EXPECT(not gen_mode.InLoop());
    }

    SECTION( "Test checking address shortage" ) {
      EXPECT(gen_mode.CheckAddressShortage());

      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::Filler));
      EXPECT(not gen_mode.CheckAddressShortage());
      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::Filler));
      EXPECT(gen_mode.CheckAddressShortage());

      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::NoEscape));
      EXPECT(not gen_mode.CheckAddressShortage());
      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::NoEscape));
      EXPECT(gen_mode.CheckAddressShortage());

      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::Exception));
      EXPECT(not gen_mode.CheckAddressShortage());
      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::Exception));
      EXPECT(gen_mode.CheckAddressShortage());
    }

    SECTION( "Test popping empty stack" ) {
      GenMode no_skip_gen_mode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      EXPECT_FAIL(no_skip_gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip)), "pop-empty-mode-stack");
    }

    SECTION( "Test popping wrong mode" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
      EXPECT_FAIL(gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::Filler)), "pop-gen-mode-check-failed");
    }

    SECTION( "Test pushing empty mode string" ) {
      EGenModeTypeBaseType orig_mode = gen_mode.CurrentMode();
      gen_mode.PushGenMode("");
      EGenModeTypeBaseType new_mode = gen_mode.CurrentMode();
      EXPECT(new_mode == orig_mode);
    }
  }
},

CASE( "Test enabling and disabling modes apart from the mode stack" ) {

  SETUP( "Setup GenMode" )  {
    GenMode gen_mode(0);

    SECTION( "Test enabling and disabling a mode" ) {
      EXPECT(not gen_mode.InLoop());
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::InLoop));
      EXPECT(gen_mode.InLoop());
      gen_mode.DisableGenMode(EGenModeTypeBaseType(EGenModeType::InLoop));
      EXPECT(not gen_mode.InLoop());
    }

    SECTION( "Test enabling and disabling a mode by string" ) {
      EXPECT(not gen_mode.LowPower());
      gen_mode.EnableGenMode("LowPower");
      EXPECT(gen_mode.LowPower());
      gen_mode.DisableGenMode("LowPower");
      EXPECT(not gen_mode.LowPower());
    }

    SECTION( "Test enabling and disabling multiple modes" ) {
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::DelayInit));
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::NoEscape));
      EXPECT(gen_mode.DelayInit());
      EXPECT(not gen_mode.CheckEscape());

      gen_mode.DisableGenMode(EGenModeTypeBaseType(EGenModeType::NoEscape));
      EXPECT(gen_mode.DelayInit());
      EXPECT(gen_mode.CheckEscape());

      gen_mode.DisableGenMode(EGenModeTypeBaseType(EGenModeType::DelayInit));
      EXPECT(not gen_mode.DelayInit());
      EXPECT(gen_mode.CheckEscape());
    }

    SECTION( "Test disabling and re-enabling a mode" ) {
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::Filler));
      EXPECT(gen_mode.IsFiller());
      gen_mode.DisableGenMode(EGenModeTypeBaseType(EGenModeType::Filler));
      EXPECT(not gen_mode.IsFiller());
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::Filler));
      EXPECT(gen_mode.IsFiller());
    }

    SECTION( "Test disabling mode that wasn't enabled" ) {
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::Speculative));
      EXPECT_FAIL(gen_mode.DisableGenMode(EGenModeTypeBaseType(EGenModeType::InLoop)), "disable-gen-mode-check-failed");
    }

    SECTION( "Test popping mode after enabling a different mode" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::RecordingState));
      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
      EXPECT(not gen_mode.ReExecution());
      EXPECT(gen_mode.RecordingState());
    }

    SECTION( "Test mode on stack with multiple enabled modes" ) {
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::SimOff) | EGenModeTypeBaseType(EGenModeType::NoEscape) | EGenModeTypeBaseType(EGenModeType::NoJump));
      test_mode_toggling_positive(lest_env, EGenModeType::LowPower, &GenMode::LowPower, &gen_mode);
    }

    SECTION( "Test enabled mode persistence after popping stack" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::InLoop));
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::SimOff) | EGenModeTypeBaseType(EGenModeType::NoEscape) | EGenModeTypeBaseType(EGenModeType::NoJump));
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      EXPECT(gen_mode.InLoop());
      EXPECT(gen_mode.NoSkip());

      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::InLoop));
      EXPECT(not gen_mode.InLoop());
      EXPECT(not gen_mode.NoSkip());

      // The enabled modes should still be there after popping the stack
      EXPECT(not gen_mode.SimulationEnabled());
      EXPECT(not gen_mode.CheckEscape());
      EXPECT(gen_mode.NoJump());
    }

    SECTION( "Test cloning GenMode with stack modes and non-stack modes" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::RecordingState));

      std::unique_ptr<GenMode> gen_mode_clone(dynamic_cast<GenMode*>(gen_mode.Clone()));
      gen_mode_clone->PopGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
      EXPECT(not gen_mode_clone->ReExecution());
      EXPECT(gen_mode_clone->RecordingState());
    }
  }
},

CASE( "Test setting mode directly" ) {

  SETUP( "Setup GenMode" )  {
    GenMode gen_mode(0);

    SECTION( "Test setting mode directly" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      EGenModeTypeBaseType cur_mode = gen_mode.CurrentMode();
      cur_mode |= EGenModeTypeBaseType(EGenModeType::Speculative);
      gen_mode.SetGenMode(cur_mode);
      EXPECT(gen_mode.NoSkip());
      EXPECT(gen_mode.IsSpeculative());

      // We expect that any mode set directly will be unset the next time the mode stack is popped
      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      EXPECT(not gen_mode.NoSkip());
      EXPECT(not gen_mode.IsSpeculative());
    }

    SECTION( "Test setting mode directly with enabled modes" ) {
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::SimOff) | EGenModeTypeBaseType(EGenModeType::NoEscape) | EGenModeTypeBaseType(EGenModeType::NoJump));
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      EGenModeTypeBaseType cur_mode = gen_mode.CurrentMode();
      cur_mode |= EGenModeTypeBaseType(EGenModeType::Speculative);
      gen_mode.SetGenMode(cur_mode);
      EXPECT(gen_mode.NoSkip());
      EXPECT(gen_mode.IsSpeculative());

      // We expect that any mode set directly will be unset the next time the mode stack is popped
      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      EXPECT(not gen_mode.NoSkip());
      EXPECT(not gen_mode.IsSpeculative());
    }

    SECTION( "Test setting mode directly before pushing" ) {
      EGenModeTypeBaseType cur_mode = gen_mode.CurrentMode();
      cur_mode |= EGenModeTypeBaseType(EGenModeType::Speculative);
      gen_mode.SetGenMode(cur_mode);
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      EXPECT(gen_mode.IsSpeculative());
      EXPECT(gen_mode.NoSkip());

      // We expect that the mode that existed prior to the push should be restored after the pop
      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      EXPECT(gen_mode.IsSpeculative());
      EXPECT(not gen_mode.NoSkip());
    }
  }
},

CASE( "Test duplicate modes" ) {

  SETUP( "Setup GenMode" )  {
    GenMode gen_mode(0);
    gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
    gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));

    SECTION( "Test pushing a mode that has already been pushed" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
      EXPECT(gen_mode.ReExecution());
      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
      EXPECT(gen_mode.ReExecution());
    }

    SECTION( "Test pushing a mode that has already been enabled" ) {
      gen_mode.PushGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      EXPECT(gen_mode.NoSkip());
      gen_mode.PopGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip));
      EXPECT(gen_mode.NoSkip());
    }

    SECTION( "Test enabling a mode that has already been pushed" ) {
      gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
      EXPECT(gen_mode.ReExecution());
      gen_mode.DisableGenMode(EGenModeTypeBaseType(EGenModeType::ReExe));
      EXPECT(gen_mode.ReExecution());
    }

    SECTION( "Test enabling a mode that has already been enabled" ) {
      EXPECT_FAIL(gen_mode.EnableGenMode(EGenModeTypeBaseType(EGenModeType::NoSkip)), "mode-already-enabled");
    }
  }
},

};

int main(int argc, char* argv[])
{
  Force::Logger::Initialize();
  int ret = lest::run(specification, argc, argv);
  Force::Logger::Destroy();
  return ret;
}
