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
#ifndef Force_PteAttribute_H
#define Force_PteAttribute_H

#include <Defines.h>
#include <Object.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

namespace Force {

  class GenPageRequest;
  class PteAttributeStructure;
  class PageTableEntry;
  class VmAddressSpace;
  class ConstraintSet;

  /*!
    \class PteAttribute
    \brief Class to support page table entry attributes.
  */
  class PteAttribute : public Object {
  public:
    Object* Clone() const override; //!< Return a cloned PteAttribute object of the same type and same contents of the object.
    const std::string ToString() const override; //!< Return a string describing the current state of the PteAttribute object.
    const char* Type() const override { return "PteAttribute"; } //!< Return the type of the PteAttribute object.

    PteAttribute(); //!< Constructor.
    ~PteAttribute(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(PteAttribute);
    EPteAttributeType PteAttributeType() const; //!< Return PTE attribute type.
    uint32 Size() const; //!< Return PTE attribute field size in number of bits.
    uint64 Value() const { return mValue; } //!< Return value of the PTE attribute.
    uint64 Encoding() const; //!< Return PteAttribute encoding.
    virtual void Initialize(const PteAttributeStructure* pPteAttrStruct); //!< Initialize PteAttribute object.
    virtual void Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte); //!< Generate PTE attribute details.

  protected:
    PteAttribute(const PteAttribute& rOther); //!< Copy constructor.
    const ConstraintSet* ValueForced(const GenPageRequest& rPagingReq, bool &rValForced); //!< Check if PTE value is forced.
    virtual bool GetValueConstraint(const GenPageRequest& rPagingReq, ConstraintSet& rPermissionConstr) const { return false; } //!< Get value constraint.
  protected:
    const PteAttributeStructure* mpStructure; //!< Pointer to PteAttributeStructure object that describes the structure of the PteAttribute object.
    uint64 mValue; //!< Value of the PTE attribute field.
  };

  /*!
    \class RandomPteAttribute
    \brief PTE attribute field that can assume random value.
  */
  class RandomPteAttribute : public PteAttribute {
  public:
    Object* Clone() const override; //!< Return a cloned RandomPteAttribute object of the same type and same contents of the object.
    const char* Type() const override { return "RandomPteAttribute"; } //!< Return the type of the RandomPteAttribute object.

    RandomPteAttribute(); //!< Constructor.
    ~RandomPteAttribute(); //!< Destructor.

    void Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte) override; //!< Generate RandomPteAttribute details.
  protected:
    RandomPteAttribute(const RandomPteAttribute& rOther); //!< Copy constructor.
  };

  /*!
    \class ValuePteAttribute
    \brief PTE attribute field that has a constant value.
  */
  class ValuePteAttribute : public PteAttribute {
  public:
    Object* Clone() const override; //!< Return a cloned ValuePteAttribute object of the same type and same contents of the object.
    const char* Type() const override { return "ValuePteAttribute"; } //!< Return the type of the ValuePteAttribute object.

    ValuePteAttribute(); //!< Constructor.
    ~ValuePteAttribute(); //!< Destructor.

    void Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte) override; //!< Generate ValuePteAttribute details.
  protected:
    ValuePteAttribute(const ValuePteAttribute& rOther); //!< Copy constructor.
  };

  /*!
    \class AddressPteAttribute
    \brief PTE attribute field that has a constant value.
  */
  class AddressPteAttribute : public PteAttribute {
  public:
    Object* Clone() const override; //!< Return a cloned AddressPteAttribute object of the same type and same contents of the object.
    const char* Type() const override { return "AddressPteAttribute"; } //!< Return the type of the AddressPteAttribute object.

    AddressPteAttribute(); //!< Constructor.
    ~AddressPteAttribute(); //!< Destructor.

    void Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte) override; //!< Generate AddressPteAttribute details.
  protected:
    AddressPteAttribute(const AddressPteAttribute& rOther); //!< Copy constructor.
  };

  /*!
    \class ConstraintPteAttribute
    \brief PTE attribute field that can assume constrainted random value.
  */
  class ConstraintPteAttribute : public PteAttribute {
  public:
    Object* Clone() const override = 0; //!< Return a cloned ConstraintPteAttribute object of the same type and same contents of the object.  Need to be implemented by sub class.
    const char* Type() const override { return "ConstraintPteAttribute"; } //!< Return the type of the ConstraintPteAttribute object.

    ConstraintPteAttribute(); //!< Constructor.
    ~ConstraintPteAttribute(); //!< Destructor.

    void Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte) override; //!< Generate ConstraintPteAttribute details.
  protected:
    ConstraintPteAttribute(const ConstraintPteAttribute& rOther); //!< Copy constructor.
    bool GetValueConstraint(const GenPageRequest& rPagingReq, ConstraintSet& rValueConstr) const override = 0; //!< Get value constraint.  Need to be implemented by sub class.
    void GetDefaultConstraint(ConstraintSet& rValueConstr) const; //!< Get default constraint when no additional constraint specified.
    virtual void SetPteGenAttribute(const GenPageRequest& rPagingReq, PageTableEntry& rPte) const { } //!< Interface to set PageGenAttribute onto the PageTableEntry object.
  };

  /*!
    \class ExceptionConstraintPteAttribute
    \brief PTE attribute field that can assume constrainted random value.
  */
  class ExceptionConstraintPteAttribute : public ConstraintPteAttribute {
  public:
    Object* Clone() const override = 0; //!< Return a cloned ExceptionConstraintPteAttribute object of the same type and same contents of the object.  Need to be implemented by sub class.
    const char* Type() const override { return "ExceptionConstraintPteAttribute"; } //!< Return the type of the ExceptionConstraintPteAttribute object.

    ExceptionConstraintPteAttribute(); //!< Constructor.
    ~ExceptionConstraintPteAttribute(); //!< Destructor.
  protected:
    ExceptionConstraintPteAttribute(const ExceptionConstraintPteAttribute& rOther); //!< Copy constructor.
    bool GetValueConstraint(const GenPageRequest& rPagingReq, ConstraintSet& rExceptConstr) const override; //!< Get value constraint.  Need to be implemented by sub class.
    virtual EPagingExceptionType GetExceptionType(const GenPageRequest& rPagingReq) const = 0; //!< Get exception type.
    virtual void ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, ConstraintSet& rTriggerConstr) const = 0; //!< Return constraint that will trigger the exception.
    virtual void ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, ConstraintSet& rPreventConstr) const = 0; //!< Return constraint that will prevent the exception.
  };

}

#endif
