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
#ifndef Force_VmManagerRISCV_H
#define Force_VmManagerRISCV_H

#include <VmManager.h>

namespace Force {

  /*!
    \class VmManagerRISCV
    \brief RISCV virtual memory manager class.
  */

  class VmManagerRISCV : public VmManager {
  public:
    Object * Clone() const override { return new VmManagerRISCV(*this); } //!< Clone VmManagerRISCV object.
    const std::string ToString() const override { return Type(); } //!< Return a string describing the VmManagerRISCV object.
    const char* Type() const override { return "VmManagerRISCV"; }

    VmManagerRISCV() : VmManager() { } //!< Default constructor.
    ~VmManagerRISCV(); //!< Destructor.

    VmInfo* VmInfoInstance() const override; //!< Return an RISCV VmInfo instance.
  protected:
    VmManagerRISCV(const VmManagerRISCV& rOther); //!< Copy constructor.
    VmFactory* VmFactoryInstance(EVmRegimeType vmRegimeType) const override; //!< Return an VmFactory object based on the RISC-V VM architecture type.
  };

};

#endif
