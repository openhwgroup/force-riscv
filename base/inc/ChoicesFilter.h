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
#ifndef Force_ChoicesFilter_H
#define Force_ChoicesFilter_H

#include "Defines.h"

namespace Force {

  class Choice;

  /*!
    \class ChoicesFilter
    \brief Base class of all Choice filtering class.
  */

  class ChoicesFilter {
  public:
    ChoicesFilter() { } //!< Default constructor.
    virtual ~ChoicesFilter() { } //!< Destructor.
    virtual bool Usable(const Choice* choice) const = 0; //!< Main interface function, check if a Choice is usable.
  };

  class ConstraintSet;

  /*!
    \class ConstraintChoicesFilter
    \brief Filtering choices using constraints.
  */

  class ConstraintChoicesFilter : public ChoicesFilter {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(ConstraintChoicesFilter);
    COPY_CONSTRUCTOR_DEFAULT(ConstraintChoicesFilter);
    explicit ConstraintChoicesFilter(const ConstraintSet* constraint) //!< Constructor with ConstraintSet parameter.
      : ChoicesFilter(), mpConstraint(constraint)
    {
    }
    ~ConstraintChoicesFilter() { mpConstraint = nullptr; }

    bool Usable(const Choice* choice) const override; //!< Check if a Choice object is usable.
  protected:
    ConstraintChoicesFilter() : ChoicesFilter(), mpConstraint(nullptr) { } //!< Default constructor.

  protected:
    const ConstraintSet* mpConstraint; //!< Pointer to to-be-applied ConstraintSet object.
  };

  class MultiRegisterOperand;

  /*!
    \class MultiRegisterChoicesFilter
    \brief Filtering choices using constraints considering multiple registers case.
  */

  class MultiRegisterChoicesFilter : public ConstraintChoicesFilter {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(MultiRegisterChoicesFilter);
    COPY_CONSTRUCTOR_DEFAULT(MultiRegisterChoicesFilter);
    MultiRegisterChoicesFilter(const ConstraintSet* pConstr, const MultiRegisterOperand* pOpr) //!< Constructor with ConstraintSet and MultiRegisterOperand parameter.
      : ConstraintChoicesFilter(pConstr), mpOperand(pOpr)
    {
    }
    ~MultiRegisterChoicesFilter() { mpOperand = nullptr; }

    bool Usable(const Choice* choice) const override; //!< Check if a Choice object is usable.
  protected:
    MultiRegisterChoicesFilter() : ConstraintChoicesFilter(), mpOperand(nullptr) { } //!< Default constructor.

  protected:
    const MultiRegisterOperand* mpOperand; //!< Pointer to MultiRegisterOperand instance.
  };

}

#endif
