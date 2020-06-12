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
#ifndef Force_FreePageRangeClaimer_H
#define Force_FreePageRangeClaimer_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

#include <vector>

namespace Force {

  class Generator;
  class ConstraintSet;
  class FreePageRangeResolver;
  class ChoiceTree;
  class VmPagingMapper;
  class GranulePageSizeTree;

  /*!
    \class FreePageRangeClaimer
    \brief a module to claim free page ranges.
  */
  class FreePageRangeClaimer {
  public:
    FreePageRangeClaimer(); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(FreePageRangeClaimer);
    ~FreePageRangeClaimer(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(FreePageRangeClaimer);

    void Setup(Generator* pGen); //!< Set up some work
    bool ClaimFreePages(const ConstraintSet& requestRanges, const std::vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, std::vector<uint64>& resolvedPageSizes); //!< Claim free pages
  protected:
    void FilterClaimChoices(const std::vector<GranulePageSizeTree* >& granulePageTrees); //!< filter claim choices
    FreePageRangeResolver* FreePageRangeResolverFactory(uint64 type); //!< the factory to create free page range resolver
    ChoiceTree* GetGranulePageTree(EPageGranuleType granuleType) const; // get current page size true supported by page granule
  protected:
    ChoiceTree* mpClaimChoices; //!< Choice tree to claim ranges
    VmPagingMapper* mpVmMapper; //!< The pointer to VmMaper
    std::vector<GranulePageSizeTree* > mGranulePageSizeTrees; //!< The container for granule page size.
    bool mPagingEnabled; //!< whether paging is enabled or not

  };
}

#endif
