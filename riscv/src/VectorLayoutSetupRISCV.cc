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

/*!
  \file VectorLayoutSetupRISCV.cc
  \brief Code supporting configuring VectorLayout objects.
*/

namespace Force {

  VectorLayoutSetupRISCV::VectorLayoutSetupRISCV(const RegisterFile* pRegFile)
    : mpRegFile(pRegFile)
  {
  }

  void VectorLayoutSetupRISCV::SetUpVectorLayoutVtype(VectorLayout& rVecLayout)
  {
    rVecLayout.mElemCount = GetVl();
    rVecLayout.mElemSize = GetSew();
    rVecLayout.mFieldCount = 1;
    rVecLayout.mRegCount = GetLmul();
    rVecLayout.mRegIndexAlignment = rVecLayout.mRegCount;
  }

  void VectorLayoutSetupRISCV::SetUpVectorLayoutFixedElementSize(const VectorLayoutOperandStructure& rVecLayoutOprStruct, VectorLayout& rVecLayout)
  {
    uint32 sew = GetSew();
    float lmul = GetLmul();

    // EMUL = (EEW / SEW) * LMUL. EEW is the element width for the instruction. Register operands
    // must be aligned to EMUL.
    rVecLayout.mRegIndexAlignment = rVecLayoutOprStruct.GetElementWidth() * lmul / sew;
    if (rVecLayout.mRegIndexAlignment == 0) {
      rVecLayout.mRegIndexAlignment = 1;
    }

    // The total register count is EMUL * NFIELDS. NFIELDS is the register count for the
    // instruction. For instructions other than load/store segment instructions, NFIELDS = 1.
    rVecLayout.mFieldCount = rVecLayoutOprStruct.GetRegisterCount();
    rVecLayout.mRegCount = rVecLayout.mFieldCount * rVecLayout.mRegIndexAlignment;

    rVecLayout.mElemSize = rVecLayoutOprStruct.GetElementWidth();

    // The total element count is NFIELDS * vl.
    rVecLayout.mElemCount = rVecLayout.mFieldCount * GetVl();
  }

  void VectorLayoutSetupRISCV::SetUpVectorLayoutWholeRegister(const VectorLayoutOperandStructure& rVecLayoutOprStruct, VectorLayout& rVecLayout)
  {
    rVecLayout.mElemSize = rVecLayoutOprStruct.GetElementWidth();
    rVecLayout.mRegCount = rVecLayoutOprStruct.GetRegisterCount();

    Config* config = Config::Instance();
    rVecLayout.mElemCount = (config->LimitValue(ELimitType::MaxPhysicalVectorLen) / rVecLayout.mElemSize) * rVecLayout.mRegCount;

    rVecLayout.mFieldCount = 1;
    rVecLayout.mRegIndexAlignment = rVecLayoutOprStruct.GetRegisterIndexAlignment();
  }

  uint32 VectorLayoutSetupRISCV::GetVl() const
  {
    Register* vl_reg = mpRegFile->RegisterLookup("vl");
    return vl_reg->Value();
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

}
