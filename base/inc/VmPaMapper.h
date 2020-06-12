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
#ifndef Force_VmPaMapper_H
#define Force_VmPaMapper_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <VmUtils.h>
#include <vector>

namespace Force {

  class VmAddressSpace;
  class VmasControlBlock;
  class GenPageRequest;
  class Page;
  class VmMappingStrategy;

  /*!
    \class VmPaMapper
    \brief Used in creating new page mapping for PA.
  */
  class VmPaMapper {
  public:
    explicit VmPaMapper(VmAddressSpace* pAddressSpace); //!< Constructor.
    virtual ~VmPaMapper(); //!< Destructor.

    const Page* SetupPageMapping(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, GenPageRequest* pPageReq, std::string& rErrMsg); //!< Setup page mapping.

    ASSIGNMENT_OPERATOR_ABSENT(VmPaMapper);
    DEFAULT_CONSTRUCTOR_ABSENT(VmPaMapper);
    COPY_CONSTRUCTOR_ABSENT(VmPaMapper);

  protected:
    void SetMappingParameters(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, GenPageRequest* pPageReq, std::string& rErrMsg); //!< Set necessary parameters.
    const Page* SetupMappingInRange(VmVaRange* pVmVaRange); //!< Setup page mapping in the VmVaRange.
    bool ApplyVmConstraints(ConstraintSet* pConstr) const; //!< Apply VmConstraints.
    virtual bool ApplyArchVmConstraints(ConstraintSet* pConstr) const { return true; } //!< Apply architecture specific VmConstraints.
  protected:
    uint64 mPA; //!< PA to map.
    EMemBankType mMemoryBank; //!< Memory bank for PA.
    uint64 mRemainingSize; //!< Remaining size to map.
    bool mIsInstr; //!< Whether a instruction access or not.
    PageSizeInfo mSizeInfo; //!< Page size info.
    std::vector<VmVaRange* > mVmVaRanges; //!< Container for all useable VmVaRanges.
    VmAddressSpace* mpAddressSpace; //!< Pointer to address space that the mapping will be created in.
    const VmasControlBlock* mpControlBlock; //!< Pointer to control block of the address space object.
    GenPageRequest* mpPageRequest; //!< Pointer to page request object.
    VmMappingStrategy* mpMappingStrategy; //!< Pointer to mapping strategy.
    std::string* mpErrMsg; //!< Pointer to error message string.
  };

}

#endif
