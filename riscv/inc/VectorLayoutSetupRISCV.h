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
#ifndef Force_VectorLayoutSetupRISCV_H
#define Force_VectorLayoutSetupRISCV_H

#include <VectorLayout.h>

namespace Force {

  class RegisterFile;
  class VectorLayoutOperandStructure;

  /*!
    \class VectorLayoutSetupRISCV
    \brief Class for configuring VectorLayout objects.
  */
  class VectorLayoutSetupRISCV {
  public:
    explicit VectorLayoutSetupRISCV(const RegisterFile* pRegFile);
    COPY_CONSTRUCTOR_ABSENT(VectorLayoutSetupRISCV);
    DESTRUCTOR_DEFAULT(VectorLayoutSetupRISCV);
    ASSIGNMENT_OPERATOR_ABSENT(VectorLayoutSetupRISCV);

    void SetUpVectorLayoutVtype(VectorLayout& rVecLayout); //!< Configure the VectorLayout object using current vtype register values.
    void SetUpVectorLayoutFixedElementSize(const VectorLayoutOperandStructure& rVecLayoutOprStruct, VectorLayout& rVecLayout); //!< Configure the VectorLayout object using vtype register values, but with the element width specified by the layout operand structure.
    void SetUpVectorLayoutWholeRegister(const VectorLayoutOperandStructure& rVecLayoutOprStruct, VectorLayout& rVecLayout); //!< Configure the VectorLayout object for a whole register instruction.
  private:
    uint32 GetVl() const; //!< Get the current VL value.
    uint32 GetSew() const; //!< Get the current SEW value.
    float GetLmul() const; //!< Get the current LMUL value.
  private:
    const RegisterFile* mpRegFile; //!< Register file
  };

}

#endif  // Force_VectorLayoutSetupRISCV_H
