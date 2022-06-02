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
#ifndef Force_PageRequestRegulatorRISCV_H
#define Force_PageRequestRegulatorRISCV_H

#include "PageRequestRegulator.h"

namespace Force {

  class RegisterFile;
  class RegisterField;
  class GenCondition;

  /*!
    \class PageRequestRegulatorRISCV
    \brief RISC-V layer class regulating GenPageRequest object before passing it to Paging system.
  */
  class PageRequestRegulatorRISCV : public PageRequestRegulator {
  public:
    Object* Clone() const override;  //!< Return a cloned object of the same type and same contents of the PageRequestRegulatorRISCV object.
    const char* Type() const override { return "PageRequestRegulatorRISCV"; } //!< Return a string describing the actual type of the PageRequestRegulatorRISCV object

    PageRequestRegulatorRISCV(); //!< Default constructor.
    ~PageRequestRegulatorRISCV(); //!< Destructor.

    void Setup(const Generator* pGen) override; //!< Setup the PageRequestRegulatorRISCV object.
    void RegulateLoadStorePageRequest(const VmMapper* pVmMapper, const LoadStoreOperandStructure* pLsStruct, GenPageRequest* pPageReq) const override; //!< Regulate load-store type page request object, RISC-V layer.
    void RegulateBranchPageRequest(const VmMapper* pVmMapper, const BranchOperandStructure* pBrStruct, GenPageRequest* pPageReq) const override; //!< Regulate branch type page request object, RISC-V layer.
  protected:
    PageRequestRegulatorRISCV(const PageRequestRegulatorRISCV& rOther); //!< Copy constructor.
    ASSIGNMENT_OPERATOR_ABSENT(PageRequestRegulatorRISCV);
    //void PreventDataPageFault(GenPageRequest* pPageReq) const override; //!< Setup the page request object to prevent RISC-V data aborts.
    //void PreventInstrPageFault(GenPageRequest* pPageReq) const override; //!< Setup the page request object to prevent RISC-V instruction aborts.
  };

}

#endif
