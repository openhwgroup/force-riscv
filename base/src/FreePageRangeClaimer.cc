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
#include <FreePageRangeResolver.h>
#include <FreePageRangeClaimer.h>
#include <Constraint.h>
#include <Choices.h>
#include <ChoicesFilter.h>
#include <Generator.h>
#include <ChoicesModerator.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <VmAddressSpace.h>
#include <VmasControlBlock.h>
#include <PagingChoicesAdapter.h>
#include <VmUtils.h>
#include <Log.h>

#include <memory>
#include <sstream>

using namespace std;

/*!
  \file FreePageRangeClaimer.cc
  \brief Code to claim free page ranges
*/

namespace Force {

  FreePageRangeClaimer::FreePageRangeClaimer() : mpClaimChoices(nullptr), mpVmMapper(nullptr), mGranulePageSizeTrees(), mPagingEnabled(false)
  {

  }

  FreePageRangeClaimer::~FreePageRangeClaimer()
  {
    delete mpClaimChoices;

    mpVmMapper = nullptr;

    for (auto& tree : mGranulePageSizeTrees)
      delete tree;
  }

  void FreePageRangeClaimer::Setup(Generator* pGen)
  {
    auto regime = dynamic_cast<VmPagingRegime* >(pGen->GetVmManager()->CurrentVmRegime());
    if (regime == nullptr) {
      LOG(fail) << "{FreePageRangeClaimer::Setup} Invalid paging regime" << endl;
      FAIL("Invalid-paging-regime");
    }
    mPagingEnabled = regime->PagingEnabled();
    if (not mPagingEnabled) {
      LOG(debug) << "{FreePageRangeClaimer::Setup} " << regime->ToString() << " paging is disabled" << endl;
      return;
    }

    vector<EPageGranuleType> granuleTypes;
    mpVmMapper = dynamic_cast<VmPagingMapper* >(pGen->GetVmManager()->CurrentVmMapper());
    mpVmMapper->CurrentAddressSpace()->GetControlBlock()->GetGranuleType(granuleTypes);
    for (auto& granule : granuleTypes) {
      auto granule_page_tree = GetGranulePageTree(granule);
      auto tree = new GranulePageSizeTree(granule, granule_page_tree);
      mGranulePageSizeTrees.push_back(tree);
    }

    mpClaimChoices = pGen->GetChoicesModerator(EChoicesType::GeneralChoices)->CloneChoiceTree("Resolving free page range methods");
  }

  bool FreePageRangeClaimer::ClaimFreePages(const ConstraintSet& requestRanges, const std::vector<uint64>& requestPageSizes, uint64& startAddr, ConstraintSet& resolvedRanges, std::vector<uint64>& resolvedPageSizes)
  {
    if (not mPagingEnabled) {
      return false;
    }

    FilterClaimChoices(mGranulePageSizeTrees);

    while (mpClaimChoices->HasChoice()) {
      auto strategy = mpClaimChoices->Choose()->Value();
      auto resolver = FreePageRangeResolverFactory(strategy);
      std::unique_ptr<FreePageRangeResolver> resolver_storage(resolver);

      if (resolver->ResolveFreePageRanges(requestRanges, requestPageSizes, startAddr, resolvedRanges, resolvedPageSizes)) {
        stringstream ss;
        for (auto page_size : resolvedPageSizes)
          ss << "0x" << hex << page_size << ",";

        LOG(debug) << "{FreePageRangeClaimer::ClaimFreePages}" << " Resolved pages:" << ss.str() << " Resolved ranges:" << resolvedRanges.ToSimpleString() << ", start address: 0x" << hex << startAddr << endl;

        return true;
      }
      else {
        Choice* choice_ptr = mpClaimChoices->FindChoiceByValue(strategy);
        choice_ptr->SetWeight(0);
      }
    }
    LOG(info) << "{FreePageRangeClaimer::ClaimFreePages} Failed to resolve the request, request ranges:" << requestRanges.ToSimpleString() << endl;
    return false;
  }

  void FreePageRangeClaimer::FilterClaimChoices(const std::vector<GranulePageSizeTree* >& granulePageSizeTrees)
  {
    if (granulePageSizeTrees.size() == 1) { // only low range tree
      ConstraintSet constr(0);
      ConstraintChoicesFilter choice_filter(&constr);
      mpClaimChoices->ApplyFilter(choice_filter);
    }
  }

  FreePageRangeResolver* FreePageRangeClaimer::FreePageRangeResolverFactory(uint64 type)
  {
    vector<VmVaRange* > va_ranges;
    FreePageRangeResolver *pResolver = nullptr;

    mpVmMapper->CurrentAddressSpace()->GetControlBlock()->GetVmVaRanges(va_ranges, nullptr);

    switch (type) {
    case 0:
      {
        // low range resolver
        auto range_constr = va_ranges[0]->GetConstraint();
        auto virt_usable = mpVmMapper->VirtualUsableConstraintSetClone(false);
        virt_usable->ApplyConstraintSet(*range_constr);
        pResolver = new FreePageRangeResolver(virt_usable, *mGranulePageSizeTrees[0]);
        break;
      }
    case 1:
      {
        // high range resolver
        auto range_constr = va_ranges[1]->GetConstraint();
        auto virt_usable = mpVmMapper->VirtualUsableConstraintSetClone(false);
        virt_usable->ApplyConstraintSet(*range_constr);
        pResolver = new FreePageRangeResolver(virt_usable, *mGranulePageSizeTrees[1]);
        break;
      }
    case 2:
      {
        // cross range resolver
        auto virt_usable = mpVmMapper->VirtualUsableConstraintSetClone(false);
        pResolver = new FreePageCrossRangeResolver(virt_usable, *mGranulePageSizeTrees[0], *mGranulePageSizeTrees[1]);
        break;
      }
    default:
      LOG(fail) << "{FreePageRangeClaimer::FreePageRangeResolverFactory} Unknown resolver-type:" << type << endl;
      FAIL("Unknown-resolver-type");
    }
    return pResolver;
  }

  ChoiceTree* FreePageRangeClaimer::GetGranulePageTree(EPageGranuleType granuleType) const
  {
    auto granule_str =  EPageGranuleType_to_string(granuleType);
    auto choice_name =  "Page size#" + granule_str.substr(1) + " granule";
    return mpVmMapper->CurrentAddressSpace()->GetControlBlock()->GetChoicesAdapter()->GetPagingChoiceTree(choice_name);
  }
}
