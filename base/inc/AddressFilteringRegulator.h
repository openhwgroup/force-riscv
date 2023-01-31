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
#ifndef Force_AddressFilteringRegulator_H
#define Force_AddressFilteringRegulator_H

#include <vector>

#include "Defines.h"
#include "Enums.h"
#include "Object.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class Generator;
  class AddressSolutionFilter;
  class AddressingMode;
  class VmConstraint;
  class GenPageRequest;
  class VmMapper;

  /*!
    \class AddressFilteringRegulator
    \brief Class regulating AddressSolutionFilter objects to be used in AddressSolver module.
  */
  class AddressFilteringRegulator : public Object {
  public:
    Object* Clone() const override;  //!< Return a cloned object of the same type and same contents of the AddressFilteringRegulator object.
    const std::string ToString() const override; //!< Return a string describing the current state of the AddressFilteringRegulator object.
    const char* Type() const override { return "AddressFilteringRegulator"; } //!< Return a string describing the actual type of the AddressFilteringRegulator Object

    AddressFilteringRegulator(); //!< Default constructor.
    ~AddressFilteringRegulator(); //!< Destructor.

    virtual void Setup(const Generator* pGen); //!< Setup the AddressFilteringRegulator object.
    void GetVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, std::vector<VmConstraint* >& rVmConstraints) const; //!< Return necessary VmConstraint objects.
    void GetAddressSolutionFilters(const AddressingMode& rAddrMode, std::vector<AddressSolutionFilter* >& rSolutionFilters) const; //!< Get suitable address solution filters.
  protected:
    AddressFilteringRegulator(const AddressFilteringRegulator& rOther); //!< Copy constructor.
    ASSIGNMENT_OPERATOR_ABSENT(AddressFilteringRegulator);
    virtual void GetInstrVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, std::vector<VmConstraint* >& rVmConstraints) const { } //!< Return necessary VmConstraint objects for instruction access.
    virtual void GetDataVmConstraints(const GenPageRequest& rPageReq, const VmMapper& rVmMapper, std::vector<VmConstraint* >& rVmConstraints) const { } //!< Return necessary VmConstraint objects for data access.
  protected:
    std::vector<AddressSolutionFilter* > mAddressSolutionFilters; //!< Pointers to address solution filters.

    friend class ArchInfoBase;
  };

}

#endif
