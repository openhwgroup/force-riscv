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
#ifndef Force_AddressFilteringRegulatorRISCV_H
#define Force_AddressFilteringRegulatorRISCV_H

#include "AddressFilteringRegulator.h"

namespace Force {

  /*!
    \class AddressFilteringRegulatorRISCV
    \brief RISCV layer class regulating GenPageRequest object before passing it to Paging system.
  */
  class AddressFilteringRegulatorRISCV : public AddressFilteringRegulator {
  public:
    Object* Clone() const override;  //!< Return a cloned object of the same type and same contents of the AddressFilteringRegulatorRISCV object.
    const char* Type() const override { return "AddressFilteringRegulatorRISCV"; } //!< Return a string describing the actual type of the AddressFilteringRegulatorRISCV object

    AddressFilteringRegulatorRISCV(); //!< Default constructor.
    ~AddressFilteringRegulatorRISCV(); //!< Destructor.

    void Setup(const Generator* pGen) override; //!< Setup the AddressFilteringRegulatorRISCV object.
  protected:
    AddressFilteringRegulatorRISCV(const AddressFilteringRegulatorRISCV& rOther); //!< Copy constructor.
    ASSIGNMENT_OPERATOR_ABSENT(AddressFilteringRegulatorRISCV);
    void GetInstrVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, std::vector<VmConstraint* >& rVmConstraints) const override; //!< Return necessary VmConstraint objects for instruction access in RISCV layer.
    void GetDataVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, std::vector<VmConstraint* >& rVmConstraints) const override; //!< Return necessary VmConstraint objects for data access in RISCV layer.
    void GetInstrAccessVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, std::vector<VmConstraint* >& rVmConstraints, EExceptionConstraintType permConstrType ) const; //!< Handle instruction permission related VM constraints.
    void GetInstrPageFaultVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, std::vector<VmConstraint* >& rVmConstraints, EExceptionConstraintType permConstrType ) const; //!< Handle instruction page fault related VM constraints.
    void GetDataAccessVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, std::vector<VmConstraint* >& rVmConstraints, EExceptionConstraintType permConstrType) const; //!< Handle data permission related VM constraints.
    void GetDataPageFaultVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, std::vector<VmConstraint* >& rVmConstraints, EExceptionConstraintType permConstrType) const; //!< Handle data page fault related VM constraints.
  protected:
   };

}

#endif //Force_AddressFilteringRegulatorRISCV_H
