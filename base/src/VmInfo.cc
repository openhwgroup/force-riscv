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
#include <VmInfo.h>
#include <Register.h>
#include <Generator.h>
#include <Log.h>
#include <sstream>

using namespace std;

/*!
  \file VmInfo.cc
  \breif VmInfo code for VmMapper lookup etc.
*/

namespace Force {

  void VmInfo::SetBoolAttribute(EVmInfoBoolType attrType, bool set)
  {
    EVmInfoBoolTypeBaseType attr_val = EVmInfoBoolTypeBaseType(attrType);
    if (set) {
      mBoolAttributes |= attr_val;
    }
    else {
      mBoolAttributes &= ~attr_val;
    }
    mBoolMask |= attr_val;
  }

  uint32 VmInfo::GetRegisterFieldState(const Register* pReg, EVmInfoBoolType attrType)
  {
    auto field_name = EVmInfoBoolType_to_string(attrType);
    auto reg_field = pReg->RegisterFieldLookup(field_name);
    uint32 field_value = reg_field->FieldValue();
    SetBoolAttribute(attrType, field_value);
    return field_value;
  }

  void VmInfo::GetOtherStates(const Generator& rGen)
  {
    EVmInfoBoolTypeBaseType test_bit = EVmInfoBoolTypeBaseType(1);
    auto reg_file = rGen.GetRegisterFile();

    for (EVmInfoBoolTypeBaseType i = 0; i < EVmInfoBoolTypeSize; ++ i) {
      if ((mBoolMask & test_bit) != test_bit) {
        // the bit has NOT been set.
        EVmInfoBoolType attr_type = EVmInfoBoolType(test_bit);
        auto reg_name = GetRegisterNameForField(attr_type, rGen);
        // << "getting other state for " << EVmInfoBoolType_to_string(attr_type) << " reg name " << reg_name << endl;
        auto reg_ptr = reg_file->RegisterLookup(reg_name);
        GetRegisterFieldState(reg_ptr, attr_type);
      }
      test_bit <<= 1;
    }
  }
}
