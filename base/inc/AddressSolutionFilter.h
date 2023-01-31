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
#ifndef Force_AddressSolutionFilter_H
#define Force_AddressSolutionFilter_H

#include <vector>

#include "Defines.h"
#include "Object.h"

namespace Force {

  class Generator;
  class AddressSolver;
  class Instruction;

  /*!
    \class AddressSolutionFilter
    \brief Base class for various sub class of AddressSolutionFilter
  */
  class AddressSolutionFilter : public Object {
  public:
    AddressSolutionFilter() //!< Default constructor.
      : Object(), mpGenerator(nullptr)
      {
      }

    const std::string ToString() const override; //!< Return a string describing the current state of the AddressSolutionFilter object.
    const char* Type() const override { return "AddressSolutionFilter"; } //!< Return AddressSolutionFilter object type in string format.

    ASSIGNMENT_OPERATOR_ABSENT(AddressSolutionFilter);
    virtual bool FilterSolutions(AddressSolver& rAddrSolver, const Instruction& rInstr, uint32& rRemainder) const = 0; //!< Solution filtering method interface.
    virtual void Setup(const Generator* pGen); //!< Setup the address solution filter.
  protected:
    AddressSolutionFilter(const AddressSolutionFilter& rOther) //!< Copy constructor.
      : Object(rOther), mpGenerator(nullptr)
    {
    }

  protected:
    const Generator* mpGenerator;
  };

  /*!
    \class BaseDependencyFilter
    \brief Used for filtering base register dependency.
  */
  class BaseDependencyFilter : public AddressSolutionFilter {
  public:
    BaseDependencyFilter() //!< Default constructor.
      : AddressSolutionFilter()
      {
      }

    Object* Clone() const override { return new BaseDependencyFilter(*this); } //!< Return a cloned object of BaseDependencyFilter type.
    const char* Type() const override { return "BaseDependencyFilter"; } //!< Return BaseDependencyFilter object type in string format.

    bool FilterSolutions(AddressSolver& rAddrSolver, const Instruction& rInstr, uint32& rRemainder) const override; //!< Filtering solutions based on register dependency.
    void Setup(const Generator* pGen) override; //!< Setup the address solution filter.
  protected:
    BaseDependencyFilter(const BaseDependencyFilter& rOther) //!< Copy constructor.
      : AddressSolutionFilter(rOther)
    {
    }

  protected:

  };

  class ConstraintSet;

  /*!
    \class IndexDependencyFilter
    \brief Used for filtering index register dependency.
  */
  class IndexDependencyFilter : public AddressSolutionFilter {
  public:
    IndexDependencyFilter() //!< Default constructor.
      : AddressSolutionFilter()
      {
      }

    Object* Clone() const override { return new IndexDependencyFilter(*this); } //!< Return a cloned object of IndexDependencyFilter type.
    const char* Type() const override { return "IndexDependencyFilter"; } //!< Return IndexDependencyFilter object type in string format.

    bool FilterSolutions(AddressSolver& rAddrSolver, const Instruction& rInstr, uint32& rRemainder) const override; //!< Filtering solutions indexd on register dependency.
    void Setup(const Generator* pGen) override; //!< Setup the address solution filter.
  protected:
    IndexDependencyFilter(const IndexDependencyFilter& rOther) //!< Copy constructor.
      : AddressSolutionFilter(rOther)
    {
    }

  protected:
  private:
    const ConstraintSet* GetIndexDependenceConstraint(const AddressSolver& rAddrSolver, const Instruction& rInstr) const;

  };

  /*!
    \class SpAlignmentFilter
    \brief Used for filtering SP register.
  */
  class SpAlignmentFilter : public AddressSolutionFilter {
  public:
    SpAlignmentFilter() : AddressSolutionFilter(), mPreventHard(false), mUnalign(false) { } //!< Default constructor.

    Object* Clone() const override { return new SpAlignmentFilter(*this); } //!< Return a cloned object of SpAlignmentFilter type.
    const char* Type() const override { return "SpAlignmentFilter"; } //!< Return SpAlignmentFilter object type in string format.

    bool FilterSolutions(AddressSolver& rAddrSolver, const Instruction& rInstr, uint32& rRemainder) const override; //!< Filtering solutions.
    void Setup(const Generator* pGen) override; //!< Setup the address solution filter.
  protected:
    SpAlignmentFilter(const SpAlignmentFilter& rOther) : AddressSolutionFilter(rOther), mPreventHard(false), mUnalign(false) { } //!< Copy constructor.
    bool FilterIndexSolutions(AddressSolver& rAddrSolver, const Instruction& rInstr, uint32& rRemainder) const; //!< Filtering solutions.
    bool FilterOffsetSolutions(AddressSolver& rAddrSolver, const Instruction& rInstr, uint32& rRemainder) const; //!< Filtering solutions.
  private:
    mutable bool mPreventHard; //!< Whether the filter is hard.
    mutable bool mUnalign; //!< Sp shoulde be align or unalign.
  };
}

#endif
