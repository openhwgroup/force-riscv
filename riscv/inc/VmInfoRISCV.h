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
#ifndef Force_VmInfoRISCV_H
#define Force_VmInfoRISCV_H

#include <VmInfo.h>

namespace Force {

  /*!
    \class VmInfoRISCV
    \brief A class for VmMapper look up.
  */
  class VmInfoRISCV : public VmInfo {
  public:
    VmInfoRISCV() : VmInfo() { } //!< Default constructor.
    ~VmInfoRISCV() { } //!< Destructor.

    EVmRegimeType RegimeType(bool* pIsValid=nullptr) const override; //!< Return the current VM regime type.
    const std::string ToString() const override; //!< Return the VmInfoRISCV in string format.
    void GetCurrentStates(const Generator& rGen) override; //!< Obtain the current RISC-V VM attributes.
    bool PagingEnabled() const override; //!< Indicate whether paging is enabled in the specified VM.
  protected:
    const std::string GetRegisterNameForField(EVmInfoBoolType attrType, const Generator& rGen) const override; //!< Get the register name associated with the VmInfo boolean attribute.
  };

}

#endif
