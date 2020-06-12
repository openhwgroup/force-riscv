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
#ifndef Force_VmMappingStrategy_H
#define Force_VmMappingStrategy_H

#include <Defines.h>
#include <vector>
#include <string>

namespace Force {

  class VmasControlBlock;
  class VmVaRange;
  class ConstraintSet;
  class PageSizeInfo;
  class GenPageRequest;

  /*!
    \class VmMappingStrategy
    \brief VM mapping strategy.
  */
  class VmMappingStrategy {
  public:
    VmMappingStrategy() { } //!< Constructor.
    virtual ~VmMappingStrategy() { } //!< Destructor.

    virtual bool GetVmVaRangesForPa(const VmasControlBlock* pCtrlBlock, uint64 PA, std::vector<VmVaRange* >& rVmVaRanges, std::string& rErrMsg) const { return false; } //!< Get VmVaRanges.
    virtual bool GenerateVaForPa(const ConstraintSet* pVaConstr, uint64 PA, uint64 size, const PageSizeInfo& rSizeInfo, uint64& VA, std::string& rErrMsg) const { return false; } //!< Generate VA for PA in the provided constraint and page size.
    virtual bool AllocatePhysicalPage(uint64 VA, const ConstraintSet* pUsablePageAligned, const ConstraintSet* pBoundary, const GenPageRequest* pPageReq, PageSizeInfo& rSizeInfo) const { return false; } //!< Try to allocate physical page with the specified page size.

    ASSIGNMENT_OPERATOR_ABSENT(VmMappingStrategy);
    COPY_CONSTRUCTOR_ABSENT(VmMappingStrategy);
  protected:
  };

  /*!
    \class VmFlatMappingStrategy
    \brief The flat mapping strategy.
  */
  class VmFlatMappingStrategy : public VmMappingStrategy {
  public:
    VmFlatMappingStrategy() : VmMappingStrategy() { } //!< Constructor.
    ~VmFlatMappingStrategy() { } //!< Destructor.

    bool GetVmVaRangesForPa(const VmasControlBlock* pCtrlBlock, uint64 PA, std::vector<VmVaRange* >& rVmVaRanges, std::string& rErrMsg) const override; //!< Get flat map VmVaRange.
    bool GenerateVaForPa(const ConstraintSet* pVaConstr, uint64 PA, uint64 size, const PageSizeInfo& rSizeInfo, uint64& VA, std::string& rErrMsg) const override; //!< Generate flat mapped VA for PA in the provided constraint and page size.
    bool AllocatePhysicalPage(uint64 VA, const ConstraintSet* pUsablePageAligned, const ConstraintSet* pBoundary, const GenPageRequest* pPageReq, PageSizeInfo& rSizeInfo) const override; //!< Try to allocate flat mapped physical page with the specified page size.

    ASSIGNMENT_OPERATOR_ABSENT(VmFlatMappingStrategy);
    COPY_CONSTRUCTOR_ABSENT(VmFlatMappingStrategy);
  };

  /*!
    \class VmRandomMappingStrategy
    \brief The PA mapper dealing with random map situation.
  */
  class VmRandomMappingStrategy : public VmMappingStrategy {
  public:
    VmRandomMappingStrategy() : VmMappingStrategy() { } //!< Constructor.
    ~VmRandomMappingStrategy() { } //!< Destructor.

    bool GetVmVaRangesForPa(const VmasControlBlock* pCtrlBlock, uint64 PA, std::vector<VmVaRange* >& rVmVaRanges, std::string& rErrMsg) const override; //!< Get random map VmVaRange.
    bool GenerateVaForPa(const ConstraintSet* pVaConstr, uint64 PA, uint64 size, const PageSizeInfo& rSizeInfo, uint64& VA, std::string& rErrMsg) const override; //!< Generate random mapped VA for PA in the provided constraint and page size.
    bool AllocatePhysicalPage(uint64 VA, const ConstraintSet* pUsablePageAligned, const ConstraintSet* pBoundary, const GenPageRequest* pPageReq, PageSizeInfo& rSizeInfo) const override; //!< Try to allocate random mapped physical page with the specified page size.

    ASSIGNMENT_OPERATOR_ABSENT(VmRandomMappingStrategy);
    COPY_CONSTRUCTOR_ABSENT(VmRandomMappingStrategy);
  };

}

#endif
