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
#include <VmPaMapper.h>
#include <VmAddressSpace.h>
#include <GenRequest.h>
#include <Page.h>
#include <VmasControlBlock.h>
#include <RandomUtils.h>
#include <Constraint.h>
#include <Choices.h>
#include <PagingChoicesAdapter.h>
#include <GenException.h>
#include <VmMappingStrategy.h>
#include <Log.h>

#include <memory>

/*!
  \file VmPaMapper.cc
  \brief Code managing generating new page mapping for a given PA.  Has a strong coupling with VmAddressSpace class.
*/

using namespace std;

namespace Force {

  VmPaMapper::VmPaMapper(VmAddressSpace* pAddressSpace)
    : mPA(0), mMemoryBank(EMemBankType(0)), mRemainingSize(0), mIsInstr(false), mSizeInfo(), mVmVaRanges(), mpAddressSpace(pAddressSpace), mpControlBlock(pAddressSpace->GetControlBlock()), mpPageRequest(nullptr), mpMappingStrategy(nullptr), mpErrMsg(nullptr)
  {
    mSizeInfo.UpdateMaxPhysical(mpControlBlock->MaxPhysicalAddress());
  }

  VmPaMapper::~VmPaMapper()
  {
    mpAddressSpace = nullptr;
    mpControlBlock = nullptr;
    mpPageRequest = nullptr;
    mpErrMsg = nullptr;

    delete mpMappingStrategy;

    for (auto vmva_range : mVmVaRanges) {
      delete vmva_range;
    }
  }

  void VmPaMapper::SetMappingParameters(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, GenPageRequest* pPageReq, string& rErrMsg)
  {
    mPA = PA;
    mMemoryBank = bank;
    mRemainingSize = size;
    mIsInstr = isInstr;
    mpPageRequest = pPageReq;
    mpPageRequest->SetBankType(mMemoryBank);
    mpErrMsg = &rErrMsg;

    if (mpPageRequest->GenBoolAttributeDefaultFalse(EPageGenBoolAttrType::FlatMap))
      mpMappingStrategy = new VmFlatMappingStrategy();
    else
      mpMappingStrategy = new VmRandomMappingStrategy();
  }

  const Page* VmPaMapper::SetupPageMapping(uint64 PA, EMemBankType bank, uint64 size, bool isInstr, GenPageRequest* pPageReq, string& rErrMsg)
  {
    SetMappingParameters(PA, bank, size, isInstr, pPageReq, rErrMsg);

    if (not mpControlBlock->IsPaValid(mPA, mMemoryBank, *mpErrMsg)) return nullptr;
    if (not mpMappingStrategy->GetVmVaRangesForPa(mpControlBlock, mPA, mVmVaRanges, *mpErrMsg)) return nullptr;

    std::vector<VmVaRange *>copy_vec = mVmVaRanges;
    while (copy_vec.size() > 0) {
      uint32 index = choose_item_index(copy_vec, &VmVaRange::Size);
      VmVaRange* vmva_range = copy_vec[index];
      const Page* page_ptr = SetupMappingInRange(vmva_range);
      if (nullptr != page_ptr) {
        return page_ptr;
      }
      copy_vec.erase(copy_vec.begin() + index);
    }

    return nullptr;
  }

  const Page* VmPaMapper::SetupMappingInRange(VmVaRange* pVmVaRange)
  {
    ConstraintSet* vmva_constr = pVmVaRange->GetConstraint();
    if (not ApplyVmConstraints(vmva_constr)) {
      *mpErrMsg += ", VmVaRange constraint become empty.";
      return nullptr;
    }

    ChoiceTree* page_size_tree = mpControlBlock->GetChoicesAdapter()->GetPageSizeChoiceTree(pVmVaRange->GranuleSuffix());
    std::unique_ptr<ChoiceTree> page_size_tree_storage(page_size_tree);
    // << "Now: " << pVmVaRange->ToString() << " page size tree: " << page_size_tree->Name() << endl;

    const Page* ret_page = nullptr;
    try {
      uint64 smallest_size = MAX_UINT64;

      while (true) {
        auto chosen_ptr = page_size_tree->ChooseMutable();
        const string& size_str = chosen_ptr->Name();
        PageSizeInfo::StringToPageSizeInfo(size_str, mSizeInfo);

        if (mSizeInfo.Size() >= smallest_size) {
          // larger page size won't work if smaller size already don't work.
          LOG(info) << "{VmPaMapper::SetupMappingInRange} Chosen page type: " << EPteType_to_string(mSizeInfo.mType) << " size=0x" << hex << mSizeInfo.Size() << " larger than smallest size tried: 0x" << smallest_size << " skipping it." << endl;
          chosen_ptr->SetWeight(0);
          continue;
        }
        smallest_size = mSizeInfo.Size(); // update smallest size.

        uint64 target_va = 0;
        string err_msg;
        if (not mpMappingStrategy->GenerateVaForPa(vmva_constr, mPA, mRemainingSize, mSizeInfo, target_va, err_msg)) {
          LOG(info) << "{VmPaMapper::SetupMappingInRange} failed to select VA for PA=0x" << hex << mPA << " with page size: " << EPteType_to_string(mSizeInfo.mType) << ", due to: " << err_msg << endl;
          *mpErrMsg += err_msg;
          chosen_ptr->SetWeight(0);
          continue;
        }

        ret_page = mpAddressSpace->CreatePage(target_va, mRemainingSize, mpPageRequest, mSizeInfo, err_msg);
        if (nullptr == ret_page) {
          LOG(info) << "{VmPaMapper::SetupMappingInRange} failed to create page with VA=0x" << hex << target_va << "=>0x" << mPA << ", due to: " << err_msg << endl;
          *mpErrMsg += err_msg;
          chosen_ptr->SetWeight(0);
          continue;
        }

        LOG(info) << "{VmPaMapper::SetupMappingInRange} mapping created for VA=0x" << hex << target_va << "=>[" << EMemBankType_to_string(mMemoryBank) << "]0x" << mPA << endl;
        break;
      }
    }
    catch (const ChoicesError& choices_error) {
      stringstream err_stream;
      err_stream << "Paging selection - failed to pick choice: " << choices_error.what();
      (*mpErrMsg) += err_stream.str();
    }

    return ret_page;
  }

  bool VmPaMapper::ApplyVmConstraints(ConstraintSet* pConstr) const
  {
    pConstr->SubConstraintSet(*(mpAddressSpace->GetVmConstraint(EVmConstraintType::Existing))); // Sub tract already mapped regions.
    if (pConstr->IsEmpty()) {
      *mpErrMsg += "after subtracting (Existing) mapped regions";
      return false;
    }
    ApplyArchVmConstraints(pConstr);
    if (pConstr->IsEmpty()) return false;

    pConstr->ApplyConstraintSet(*(mpAddressSpace->VirtualUsableConstraintSet(mIsInstr)));
    if (pConstr->IsEmpty()) {
      *mpErrMsg += "after applying virtual-usable constraint.";
      return false;
    }
    return true;
  }

}
