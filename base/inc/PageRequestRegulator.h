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
#ifndef Force_PageRequestRegulator_H
#define Force_PageRequestRegulator_H

#include <Defines.h>
#include <Object.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>

namespace Force {

  class Generator;
  class GenPageRequest;
  class LoadStoreOperandStructure;
  class BranchOperandStructure;
  class VmMapper;
  class ValueVariable;

  /*!
    \class PageRequestRegulator
    \brief Class regulating GenPageRequest object before passing it to Paging system.
  */
  class PageRequestRegulator : public Object {
  public:
    Object* Clone() const override = 0;  //!< Return a cloned object of the same type and same contents of the PageRequestRegulator object.
    const std::string ToString() const override; //!< Return a string describing the current state of the PageRequestRegulator object.
    const char* Type() const override { return "PageRequestRegulator"; } //!< Return a string describing the actual type of the PageRequestRegulator Object

    PageRequestRegulator(); //!< Default constructor.
    ~PageRequestRegulator(); //!< Destructor.

    virtual void Setup(const Generator* pGen); //!< Setup the PageRequestRegulator object.
    virtual void RegulateLoadStorePageRequest(const VmMapper* pVmMapper, const LoadStoreOperandStructure* pLsStruct, GenPageRequest* pPageReq) const; //!< Regulate load-store type page request object.
    virtual void RegulateBranchPageRequest(const VmMapper* pVmMapper, const BranchOperandStructure* pBrStruct, GenPageRequest* pPageReq) const; //!< Regulate branch type page request object.
    void RegulatePageRequest(const VmMapper* pVmMapper, GenPageRequest* pPageReq) const; //!< Regulate page request object.
  protected:
    PageRequestRegulator(const PageRequestRegulator& rOther); //!< Copy constructor.
    ASSIGNMENT_OPERATOR_ABSENT(PageRequestRegulator);
    virtual const char* GetExceptionString(EPagingExceptionType exceptType) const { return nullptr; } //!< Return exception string for the specified attribute type.
    virtual void PreventDataAbort(GenPageRequest* pPageReq) const; //!< Setup the page request object to prevent instruction aborts.
    virtual void PreventInstrAbort(GenPageRequest* pPageReq) const; //!< Setup the page request object to prevent instruction aborts.
  protected:
    const Generator* mpGenerator; //!< Pointer to Generator instance.
    const ValueVariable* mpNoDataAbortVariable; //!< Pointer to no data abort value variable.
    const ValueVariable* mpNoInstrAbortVariable; //!< Pointer to no instruction abort value variable.
    std::vector<EPagingExceptionType> mDataExceptionTypes; //!< Data paging exception types to process.
    std::vector<EPagingExceptionType> mInstrExceptionTypes; //!< Instruction paging exception types to process.
  };

}

#endif
