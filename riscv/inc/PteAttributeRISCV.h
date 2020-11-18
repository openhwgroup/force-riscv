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
#ifndef Force_PteAttributeRISCV_H
#define Force_PteAttributeRISCV_H

#include <PteAttribute.h>
#include <Object.h>

namespace Force {

  /*!
    \class AddressPteAttributeRISCV
    \brief pte attribute to manage address and corresponding fault behavior
  */
  class AddressPteAttributeRISCV : public AddressPteAttribute
  {
  public:
    Object* Clone() const override { return new AddressPteAttributeRISCV(*this); } //!< Return a cloned ValidPteAttributeRISCV object of the same type and same contents of the object.
    const char* Type() const override { return "AddressPteAttributeRISCV"; } //!< Return the type of the ValidPteAttributeRISCV object.

    AddressPteAttributeRISCV() : AddressPteAttribute() { } //!< Constructor.
    ~AddressPteAttributeRISCV() { } //!< Destructor.

    void Generate(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte) override; //!< Generate AddressPteAttributeRISCV details.
  protected:
    AddressPteAttributeRISCV(const AddressPteAttributeRISCV& rOther) : AddressPteAttribute(rOther) { } //!< Copy constructor.
  };

  /*!
    \class DAPteAttributeRISCV
    \brief pte attribute to manage the DA bits and corresponding fault behavior
  */
  class DAPteAttributeRISCV : public ExceptionConstraintPteAttribute
  {
  public:
    Object* Clone() const override { return new DAPteAttributeRISCV(*this); } //!< overridden method to clone XPteAttribute object
    const char* Type() const override { return "DAPteAttributeRISCV"; } //!< overridden method to return XPteAttribute as object type
    DAPteAttributeRISCV() : ExceptionConstraintPteAttribute() { } //!< Constructor
    ~DAPteAttributeRISCV() { } //!< Destructor

  protected:
    DAPteAttributeRISCV(const DAPteAttributeRISCV& rOther) : ExceptionConstraintPteAttribute(rOther) { } //!< Copy Constructor
    EPagingExceptionType GetExceptionType(const GenPageRequest& rPagingReq) const override; //!< Get exception type.
    void ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rTriggerConstr) const override; //!< Return constraint that will trigger the exception.
    void ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rPreventConstr) const override; //!< Return constraint that will prevent the exception.
    void SetPteGenAttribute(const GenPageRequest& rPagingReq, PageTableEntry& rPte) const override; //!< Hook to set PageGenAttribute onto the PageTableEntry object.
    bool EvaluateArchFaultChoice(const VmAddressSpace& rVmas, PageTableEntry& rPte, bool& rHardFaultChoice) const override; //!< Hook to evaluate whether to apply individual field based faults.
  };

  /*!
    \class GPteAttributeRISCV
    \brief pte attribute to manage the global bit
  */
  class GPteAttributeRISCV : public ConstraintPteAttribute
  {
  public:
    Object* Clone() const override { return new GPteAttributeRISCV(*this); } //!< overridden method to clone WPteAttribute object
    const char* Type() const override { return "GPteAttributeRISCV"; } //!< overridden method to return WPteAttribute as object type
    GPteAttributeRISCV() : ConstraintPteAttribute() { } //!< Constructor
    ~GPteAttributeRISCV() { } //!< Destructor

  protected:
    GPteAttributeRISCV(const GPteAttributeRISCV& rOther) : ConstraintPteAttribute(rOther) { } //!< Copy Constructor
    bool GetValueConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, PageTableEntry& rPte, ConstraintSet& rValueConstr) const override; //!< Get value constraint for G pte attribute
  };

  /*!
    \class UPteAttributeRISCV
    \brief pte attribute to manage the 'U' (User access) bit and corresponding fault behavior
  */
  class UPteAttributeRISCV : public ExceptionConstraintPteAttribute
  {
  public:
    Object* Clone() const override { return new UPteAttributeRISCV(*this); } //!< overridden method to clone UPteAttribute object
    const char* Type() const override { return "UPteAttributeRISCV"; } //!< overridden method to return UPteAttribute as object type
    UPteAttributeRISCV() : ExceptionConstraintPteAttribute() { } //!< Constructor
    ~UPteAttributeRISCV() { } //!< Destructor

  protected:
    UPteAttributeRISCV(const UPteAttributeRISCV& rOther) : ExceptionConstraintPteAttribute(rOther) { } //!< Copy Constructor
    EPagingExceptionType GetExceptionType(const GenPageRequest& rPagingReq) const override; //!< Get exception type.
    void ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rTriggerConstr) const override; //!< Return constraint that will trigger the exception.
    void ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rPreventConstr) const override; //!< Return constraint that will prevent the exception.
    void SetPteGenAttribute(const GenPageRequest& rPagingReq, PageTableEntry& rPte) const override; //!< Hook to set PageGenAttribute onto the PageTableEntry object.
    bool EvaluateArchFaultChoice(const VmAddressSpace& rVmas, PageTableEntry& rPte, bool& rHardFaultChoice) const override; //!< Hook to evaluate whether to apply individual field based faults.
  };

  /*!
    \class XPteAttributeRISCV
    \brief pte attribute to manage the 'X' (Executable) bit and corresponding fault behavior
  */
  class XPteAttributeRISCV : public ExceptionConstraintPteAttribute
  {
  public:
    Object* Clone() const override { return new XPteAttributeRISCV(*this); } //!< overridden method to clone XPteAttribute object
    const char* Type() const override { return "XPteAttributeRISCV"; } //!< overridden method to return XPteAttribute as object type
    XPteAttributeRISCV() : ExceptionConstraintPteAttribute() { } //!< Constructor
    ~XPteAttributeRISCV() { } //!< Destructor

  protected:
    XPteAttributeRISCV(const XPteAttributeRISCV& rOther) : ExceptionConstraintPteAttribute(rOther) { } //!< Copy Constructor
    EPagingExceptionType GetExceptionType(const GenPageRequest& rPagingReq) const override; //!< Get exception type.
    void ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rTriggerConstr) const override; //!< Return constraint that will trigger the exception.
    void ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rPreventConstr) const override; //!< Return constraint that will prevent the exception.
    void SetPteGenAttribute(const GenPageRequest& rPagingReq, PageTableEntry& rPte) const override; //!< Hook to set PageGenAttribute onto the PageTableEntry object.
    bool EvaluateArchFaultChoice(const VmAddressSpace& rVmas, PageTableEntry& rPte, bool& rHardFaultChoice) const override; //!< Hook to evaluate whether to apply individual field based faults.
  };

  /*!
    \class WRPteAttributeRISCV
    \brief pte attribute to manage the 'WR' (Writeable/Readable) bits and corresponding fault behavior
  */
  class WRPteAttributeRISCV : public ExceptionConstraintPteAttribute
  {
  public:
    Object* Clone() const override { return new WRPteAttributeRISCV(*this); } //!< overridden method to clone WPteAttribute object
    const char* Type() const override { return "WRPteAttributeRISCV"; } //!< overridden method to return WPteAttribute as object type
    WRPteAttributeRISCV() : ExceptionConstraintPteAttribute() { } //!< Constructor
    ~WRPteAttributeRISCV() { } //!< Destructor

  protected:
    WRPteAttributeRISCV(const WRPteAttributeRISCV& rOther) : ExceptionConstraintPteAttribute(rOther) { } //!< Copy Constructor
    EPagingExceptionType GetExceptionType(const GenPageRequest& rPagingReq) const override; //!< Get exception type.
    void ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rTriggerConstr) const override; //!< Return constraint that will trigger the exception.
    void ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rPreventConstr) const override; //!< Return constraint that will prevent the exception.
    void SetPteGenAttribute(const GenPageRequest& rPagingReq, PageTableEntry& rPte) const override; //!< Hook to set PageGenAttribute onto the PageTableEntry object.
    bool EvaluateArchFaultChoice(const VmAddressSpace& rVmas, PageTableEntry& rPte, bool& rHardFaultChoice) const override; //!< Hook to evaluate whether to apply individual field based faults.
  };

  /*!
    \class VPteAttributeRISCV
    \brief pte attribute to manage the valid bit and corresponding fault behavior
  */
  class VPteAttributeRISCV : public ExceptionConstraintPteAttribute
  {
  public:
    Object* Clone() const override { return new VPteAttributeRISCV(*this); } //!< overridden method to clone VPteAttribute object
    const char* Type() const override { return "VPteAttributeRISCV"; } //!< overridden method to return VPteAttribute as object type
    VPteAttributeRISCV() : ExceptionConstraintPteAttribute() { } //!< Constructor
    ~VPteAttributeRISCV() { } //!< Destructor

  protected:
    VPteAttributeRISCV(const VPteAttributeRISCV& rOther) : ExceptionConstraintPteAttribute(rOther) { } //!< Copy Constructor
    EPagingExceptionType GetExceptionType(const GenPageRequest& rPagingReq) const override; //!< Get exception type.
    void ExceptionTriggeringConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rTriggerConstr) const override; //!< Return constraint that will trigger the exception.
    void ExceptionPreventingConstraint(const GenPageRequest& rPagingReq, const VmAddressSpace& rVmas, ConstraintSet& rPreventConstr) const override; //!< Return constraint that will prevent the exception.
    void SetPteGenAttribute(const GenPageRequest& rPagingReq, PageTableEntry& rPte) const override; //!< Hook to set PageGenAttribute onto the PageTableEntry object.
    bool EvaluateArchFaultChoice(const VmAddressSpace& rVmas, PageTableEntry& rPte, bool& rHardFaultChoice) const override; //!< Hook to evaluate whether to apply individual field based faults.
  };

}

#endif //Force_PteAttributeRISCV_H
