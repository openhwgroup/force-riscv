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
#include <VectorLayoutSetupRISCV.h>

#include <Config.h>
#include <InstructionStructure.h>
#include <Register.h>

#include <cmath>

/*!
  \file VectorLayoutSetupRISCV.cc
  \brief Code supporting configuring VectorLayout objects.
*/

using namespace std;

namespace Force {

  VectorLayoutSetupRISCV::VectorLayoutSetupRISCV(const RegisterFile* pRegFile)
    : mpRegFile(pRegFile)
  {
  }

  void VectorLayoutSetupRISCV::SetUpVectorLayoutVtype(const VectorRegisterOperandStructure& rVecRegOprStruct, VectorLayout& rVecLayout) const
  {
    SetUpVectorLayoutWithLayoutMultiple(rVecRegOprStruct, rVecRegOprStruct.GetLayoutMultiple(), rVecLayout);
  }

  void VectorLayoutSetupRISCV::SetUpVectorLayoutFixedElementSize(const VectorRegisterOperandStructure& rVecRegOprStruct, VectorLayout& rVecLayout) const
  {
    float layout_multiple = static_cast<float>(rVecRegOprStruct.GetElementWidth()) / GetSew();
    SetUpVectorLayoutWithLayoutMultiple(rVecRegOprStruct, layout_multiple, rVecLayout);
  }

  void VectorLayoutSetupRISCV::SetUpVectorLayoutWholeRegister(const VectorRegisterOperandStructure& rVecRegOprStruct, VectorLayout& rVecLayout) const
  {
    rVecLayout.mElemSize = rVecRegOprStruct.GetElementWidth();
    rVecLayout.mFieldCount = 1;
    rVecLayout.mRegCount = rVecRegOprStruct.GetRegisterCount();

    Config* config = Config::Instance();
    rVecLayout.mElemCount = (config->LimitValue(ELimitType::MaxPhysicalVectorLen) / rVecLayout.mElemSize) * rVecLayout.mRegCount;

    rVecLayout.mRegIndexAlignment = rVecLayout.mRegCount;

    AdjustForLimits(rVecLayout);
  }

  void VectorLayoutSetupRISCV::SetUpVectorLayoutWithLayoutMultiple(const VectorRegisterOperandStructure& rVecRegOprStruct, const float layoutMultiple, VectorLayout& rVecLayout) const
  {
    rVecLayout.mElemSize = lround(GetSew() * layoutMultiple);
    rVecLayout.mElemCount = GetVl();

    // NFIELDS is the register count for the operand. For instructions other than load/store segment
    // instructions, NFIELDS = 1.
    rVecLayout.mFieldCount = rVecRegOprStruct.GetRegisterCount();

    // Register operands must be aligned to EMUL.
    rVecLayout.mRegIndexAlignment = lround(GetLmul() * layoutMultiple);
    if (rVecLayout.mRegIndexAlignment == 0) {
      rVecLayout.mRegIndexAlignment = 1;
    }

    // The total register count is NFIELDS * EMUL.
    rVecLayout.mRegCount = rVecLayout.mFieldCount * rVecLayout.mRegIndexAlignment;

    AdjustForLimits(rVecLayout);
  }

  uint32 VectorLayoutSetupRISCV::GetSew() const
  {
    Register* vtype_reg = mpRegFile->RegisterLookup("vtype");
    RegisterField* vsew_field = vtype_reg->RegisterFieldLookup("VSEW");
    uint32 sew = (1 << vsew_field->FieldValue()) * 8;
    return sew;
  }

  float VectorLayoutSetupRISCV::GetLmul() const
  {
    Register* vtype_reg = mpRegFile->RegisterLookup("vtype");
    RegisterField* vlmul_field = vtype_reg->RegisterFieldLookup("VLMUL");
    float lmul = 0;
    switch (vlmul_field->FieldValue()) {
      case 0:
        lmul = 1;
        break;
      case 1:
        lmul = 2;
        break;
      case 2:
        lmul = 4;
        break;
      case 3:
        lmul = 8;
        break;
      case 4:
        // VLMUL = 4 is reserved (Section 3.3.2)
        LOG(fail) << "{VectorLayoutSetupRISCV::GetLmul} VLMUL = 4 is reserved" << std::endl;
        FAIL("reserved-vlmul");
        break;
      case 5:
        lmul = 0.125;
        break;
      case 6:
        lmul = 0.25;
        break;
      case 7:
        lmul = 0.5;
        break;
      default:
        // VLMUL > 7 is undefined (Section 3.3.2)
        LOG(fail) << "{VectorLayoutSetupRISCV::GetLmul} VLMUL = " << vlmul_field->FieldValue() << " is not defined" << std::endl;
        FAIL("undefined-vlmul");
    }
    return lmul;
  }

  uint32 VectorLayoutSetupRISCV::GetVl() const
  {
    Register* vl_reg = mpRegFile->RegisterLookup("vl");
    return vl_reg->Value();
  }

  void VectorLayoutSetupRISCV::AdjustForLimits(VectorLayout& rVecLayout) const
  {
    rVecLayout.mIsIllegal = false;

    if (rVecLayout.mRegCount > 8) {
      LOG(notice) << "{VectorLayoutSetupRISCV::AdjustForLimits} EMUL * NFIELDS = " << rVecLayout.mRegCount << " > 8" << endl;

      rVecLayout.mRegCount = 8;
      rVecLayout.mIsIllegal = true;
    }

    if (rVecLayout.mRegIndexAlignment > 8) {
      rVecLayout.mRegIndexAlignment = 8;
      rVecLayout.mIsIllegal = true;
    }
  }

}
