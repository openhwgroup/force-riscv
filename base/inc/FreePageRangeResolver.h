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
#ifndef Force_FreePageRangeResolver_H
#define Force_FreePageRangeResolver_H

#include <map>
#include <vector>

#include "Defines.h"
#include "Enums.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class ConstraintSet;
  class ChoiceTree;
  class Choice;

  /*!
    \class GranulePageSizeTree
    \brief class for granule page sizes
  */
  class GranulePageSizeTree {
  public:
    GranulePageSizeTree(EPageGranuleType granuleType, ChoiceTree* pPageSizeTree); //!< Constructor
    ~GranulePageSizeTree(); //!< Destructor

    ASSIGNMENT_OPERATOR_ABSENT(GranulePageSizeTree);
    COPY_CONSTRUCTOR_ABSENT(GranulePageSizeTree);

    bool Contains(const ConstraintSet& rpageSizes) const; //!< Whether granule type sizes contain the specified ones.
    ChoiceTree* CloneTree(); //!< Clone page size tree
    inline const Choice* MinimalPageSizeChoice() const { return mpMinimalPageSizeChoice; } //!< get minimal page size choice
    inline const ConstraintSet* PageSizeConstraint() const { return mpPageSizeConstraint; } //!< get page size constraint
  protected:
    EPageGranuleType mGranuleType; //!< Page granule
    ChoiceTree* mpPageSizeTree; //!< The choice tree for page size
    ConstraintSet* mpPageSizeConstraint; //!< The pointer to page size constraint
    const Choice* mpMinimalPageSizeChoice; //!< The pointer to the minimal page size choice
  };

  /*!
    \ class PageSizeChoice
    \ brief a module to describe page size choice
   */
  class PageSizeChoice {
  public:
    PageSizeChoice(const std::string& name, uint32 value);
  protected:
    std::string mName; //!< Page size value
    uint64 mSize; //!< Page size
    uint32 mValue; //!< The value for the choice

    friend class FreePageRangeResolver;
    friend class FreePageCrossRangeResolver;
  };

  /*!
    \class FreePageRangeResolver
    \brief a module to resolve free page ranges.
  */
  class FreePageRangeResolver {
  public:
    FreePageRangeResolver(ConstraintSet* virtualUsable, GranulePageSizeTree& granulePageTree); //!< Constructor
    virtual ~FreePageRangeResolver(); //!< Destructor

    ASSIGNMENT_OPERATOR_ABSENT(FreePageRangeResolver);
    COPY_CONSTRUCTOR_ABSENT(FreePageRangeResolver);

    virtual bool ResolveFreePageRanges(const ConstraintSet& requestRanges, const std::vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, std::vector<uint64>& resolvedPageSizes); //!< resolve free pages
  protected:
    virtual bool ResolvePageRange( const std::vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, std::vector<uint64>& resolvedPageSizes); //!< resolve page range
    virtual bool ResolvePageRangeRandomly(const std::vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, std::vector<uint64>& resolvedPageSizes); //!< resolve page range randomly
    virtual bool CheckPageSizeRequest(const std::vector<uint64>& requestPageSizes); //!< check page size request is valid or not
  private:
    void PreHandlePageRange(const std::vector<uint64>& requestPageSizes); //!< Pre-Handle page size to scratch up random page sizes

    void FillRandomPageSize(const ChoiceTree* pPageSizeTree); //!< fill random page size
    void DisableLargePageSize(ChoiceTree* pPageSizeTree); //!< Disable the large page size choice on the tree.
    void GetAlignmentSize(const std::vector<uint64>& requestPageSizes, uint64& align, uint64& size); //!< get alignment and size for the request
  protected:
    ConstraintSet* mpVirtualUsable; //!< Usable virtual address
    std::map<uint32, PageSizeChoice* > mRandomPageSizes; //!< Page sizes generated randomly, key is the index on request page sizes.
    std::vector<uint32> mRandomPageSizeIndices; //!< unspecified page size indice
    GranulePageSizeTree& mrGranulePageTree; //!< The reference to granule page size
  };

  /*!
    \class FreePageCrossRangeResolver
    \brief sub-class to resolve free page cross range
  */
  class FreePageCrossRangeResolver : public FreePageRangeResolver {
  public:
    FreePageCrossRangeResolver(ConstraintSet* virtualUsable, GranulePageSizeTree& granulePageTree, GranulePageSizeTree& extraGranulePageTree); //!< Constructor
    ~FreePageCrossRangeResolver() { } //!< Destructor

  protected:
    bool ResolvePageRange( const std::vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, std::vector<uint64>& resolvedPageSizes) override; //!< resolve page range
    bool ResolvePageRangeRandomly(const std::vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, std::vector<uint64>& resolvedPageSizes) override; //!< resolve page range randomly
    bool CheckPageSizeRequest(const std::vector<uint64>& requestPageSizes) override; //!< check page size request is valid or not
    void FillMinimalPageSize(); //!< fill minimal page size to the page size indices
    void GetPageHighRange(const std::vector<uint64>& requestPageSizes, uint32& rangeSize, ConstraintSet& rangeConstraint); //!< get page high range
    void GetPageLowRange(const std::vector<uint64>& requestPageSizes, uint32& rangeSize, ConstraintSet& rangeConstraint); //!< get page low range
  protected:
    GranulePageSizeTree& mrExtraGranulePageTree; //!< The reference to extra granule page size
    uint32 mPageIndice; //!< page indice to start low range
  };

}
#endif
