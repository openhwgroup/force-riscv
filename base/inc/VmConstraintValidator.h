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
#ifndef Force_VmConstraintValidator_H
#define Force_VmConstraintValidator_H

#include <Defines.h>
#include <vector>

namespace Force
{
  class VmConstraint;
  class GenPageRequest;
  class AddressFilteringRegulator;
  class VmMapper;
  class ConstraintSet;

  /*!
    \class VmConstraintValidator
    \brief A small class used to validating address against VmConstraint.
  */
  class VmConstraintValidator {
  public:
    VmConstraintValidator(const GenPageRequest* pPageReq, const AddressFilteringRegulator* pAddrRegulator, const VmMapper* pVmMapper); //!< Constructor with necessary parameters.
    ~VmConstraintValidator(); //!< Desctructor.

    bool IsValid(uint64 value) const; //!< Return whether value is valid according to the VmConstraint requirements.
    void ApplyOn(ConstraintSet& rConstr) const; //!< Apply VmConstraint requirements on the passed in rConstr.

    ASSIGNMENT_OPERATOR_ABSENT(VmConstraintValidator);
    DEFAULT_CONSTRUCTOR_ABSENT(VmConstraintValidator);
    COPY_CONSTRUCTOR_ABSENT(VmConstraintValidator);
  private:
    const GenPageRequest* mpPageRequest; //!< Pointer to page request for the virtual address to be validated.
    const AddressFilteringRegulator* mpAddressRegulator; //!< Pointer to address regulator used to validate address.
    const VmMapper* mpVmMapper; //!< Pointer to VmMapper used to validate address.
    std::vector<VmConstraint *> mVmConstraints; //!< Container for the VmConstraint objects.
  };

}

#endif
