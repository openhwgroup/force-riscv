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
#ifndef Force_VmConstraint_H
#define Force_VmConstraint_H

#include <string>

#include "Defines.h"
#include "Enums.h"
#include ARCH_ENUM_HEADER

namespace Force
{
  class ConstraintSet;

  /*!
    \class VmConstraint
    \brief A relatively simple wrapping class around a ConstraintSet object associated with a VmConstraintType
  */
  class VmConstraint {
  public:
    VmConstraint(EVmConstraintType constrType, const ConstraintSet* pConstr) : mType(constrType), mpConstraint(pConstr) { } //!< Most used constructor.
    virtual ~VmConstraint() //!< Destructor.
      {
        mpConstraint = nullptr;
      }

    VmConstraint() : mType(EVmConstraintType(0)), mpConstraint(nullptr) { } //!< Default constructor.

    virtual void ApplyOn(ConstraintSet& rConstrSet) const = 0; //!< Apply the VmConstraint on the passed in ConstraintSet.
    virtual bool Allows(uint64 value) const = 0; //!< Check if value is allowed by the VmConstraint.

    virtual const char* Requiring() const = 0; //!< Semantic requirement.
    inline EVmConstraintType Type() const { return mType; } //!< Return VmConstraint type.
    const ConstraintSet* GetConstraintSet() const { return mpConstraint; } //!< Return pointer to ConstraintSet object.
    std::string ToString() const; //!< Return a string describing the VmConstraint object.
    COPY_CONSTRUCTOR_DEFAULT(VmConstraint); //!< Use default copy constructor.
    ASSIGNMENT_OPERATOR_DEFAULT(VmConstraint); //!< Use default assignment operator.
  protected:
    EVmConstraintType mType; //!< Type of VmConstraint.
    const ConstraintSet* mpConstraint; //!< Const pointer to associated VmConstraintSet.
  };

  /*!
    \class VmInConstraint
    \brief A relative simple wrapping class around a ConstraintSet object where in the constraint means satisfying the VmConstraint.
  */
  class VmInConstraint : public VmConstraint {
  public:
    VmInConstraint(EVmConstraintType constrType, const ConstraintSet* pConstr) : VmConstraint(constrType, pConstr) { } //!< Most used constructor.
    ~VmInConstraint() { } //!< Destructor.

    void ApplyOn(ConstraintSet& rConstrSet) const override; //!< Apply the VmConstraint on the passed in ConstraintSet.
    bool Allows(uint64 value) const override; //!< Return if the value is allowed by the VmConstraint.
    const char* Requiring() const override { return "In"; } //!< Semantic requirement.
    COPY_CONSTRUCTOR_DEFAULT(VmInConstraint); //!< Use default copy constructor.
    ASSIGNMENT_OPERATOR_DEFAULT(VmInConstraint); //!< Use default assignment operator.
  };

  /*!
    \class VmNotInConstraint
    \brief A relative simple wrapping class around a ConstraintSet object where NOT in the constraint means satisfying the VmConstraint.
  */
  class VmNotInConstraint : public VmConstraint {
  public:
    VmNotInConstraint(EVmConstraintType constrType, const ConstraintSet* pConstr) : VmConstraint(constrType, pConstr) { } //!< Most used constructor.
    ~VmNotInConstraint() { } //!< Destructor.

    void ApplyOn(ConstraintSet& rConstrSet) const override; //!< Apply the VmConstraint on the passed in ConstraintSet.
    bool Allows(uint64 value) const override; //!< Return if the value is allowed by the VmConstraint.
    const char* Requiring() const override { return "Not In"; } //!< Semantic requirement.
    COPY_CONSTRUCTOR_DEFAULT(VmNotInConstraint); //!< Use default copy constructor.
    ASSIGNMENT_OPERATOR_DEFAULT(VmNotInConstraint); //!< Use default assignment operator.
  };

}

#endif //Force_VmConstraint_H
