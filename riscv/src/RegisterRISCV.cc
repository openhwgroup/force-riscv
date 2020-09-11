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
#include <RegisterRISCV.h>
#include <RegisterReserverRISCV.h>
#include <Constraint.h>
#include <Config.h>
#include <UtilityFunctions.h>
#include <RegisterInitPolicyRISCV.h>
//#include <GenConditionRISCV.h>
#include <Log.h>

using namespace std;

/*
  \file RegisterRISCV.cc
  \brief Code for RegisterRISCV class.
*/

namespace Force
{

  RegisterFileRISCV::RegisterFileRISCV()
    : RegisterFile()
  {
#ifndef UNIT_TEST
    AddInitPolicy(new PpnInitPolicy());
    AddInitPolicy(new VlInitPolicy());
    AddInitPolicy(new VstartInitPolicy());
    AddInitPolicy(new VtypeInitPolicy());
#endif
  }

  RegisterFileRISCV::RegisterFileRISCV(const RegisterFileRISCV& rOther)
    : RegisterFile(rOther)
  {

  }

  RegisterFileRISCV::~RegisterFileRISCV()
  {

  }

  Object * RegisterFileRISCV::Clone() const
  {
    return new RegisterFileRISCV(*this);
  }

  void RegisterFileRISCV::SetupRegisterReserver()
  {
    mpRegisterReserver = new RegisterReserverRISCV();
  }

  /*!
    Primary use case is called when about to write register initial value to ISS, and the underlying physical register is partially initialized since it is mapped to be multiple logical registers, some of which might be of smaller size.
    This method return the logical register that covers whole physical register, or could be multiple physical registers.
  */
  Register* RegisterFileRISCV::GetContainingRegister(const Register* pReg) const
  {
    Register* containing_reg = nullptr;

    // TODO(Noah): Specify Q as the containing register prefix when the Q extension is supported.
    const string& reg_name = pReg->Name();
    if (reg_name[0] == 'S') {
      string reg_name_copy = reg_name;
      reg_name_copy[0] = 'D';
      containing_reg = RegisterLookup(reg_name_copy);
    } else if (reg_name[0] == 'H') {
      // half-precision...
      string reg_name_copy = reg_name;
      reg_name_copy[0] = 'D';
      containing_reg = RegisterLookup(reg_name_copy);
    }

    return containing_reg;
  }

  /*!
   *  If a register contains another register AND is otherwise uninitialized then it may be
   *  initialized (depending upon the registers in question) from the contained register.
   *
   *  The only case thusfar where this could be applicable is in the case of a RISCV
   *  half-precision register...
   */

  bool RegisterFileRISCV::InitContainingRegister(Register* rContainingReg, const Register* pReg) const 
  {
    if ( (rContainingReg->Name()[0] == 'D') && (pReg->Name()[0] == 'H') ) {
        LOG(debug) << "{RegisterFileRISCV::InitContainingRegister} initializing containing reg " << rContainingReg->Name()
	           << std::endl;
	InitializeRegister(rContainingReg->Name(), -1ull, NULL); // NaN boxing half-precision value
	return true;
    }
    return false;
  }

  bool RegisterFileRISCV::AllowReExecutionInit(const std::string& rRegName) const
  {
    return false;
  }

  Register* RegisterFileRISCV::RegisterLookupByIndex(uint32 index, const ERegisterType reg_type, uint32 size) const
  {
    //TODO not complete for all cases
    char reg_prefix = 'x';
    char print_buffer[16];
    snprintf(print_buffer, 16, "%c%d", reg_prefix, index);

    return RegisterLookup(print_buffer);
  }

}
