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
#include "VmMappingStrategy.h"

#include <memory>

#include "Constraint.h"
#include "GenException.h"
#include "GenRequest.h"
#include "Log.h"
#include "VmUtils.h"
#include "VmasControlBlock.h"

/*!
  \file VmMappingStrategy.cc
  \brief Mapping strategy class, mainly to differentiate code for flat mapping and random mapping in a more organized way.
*/

using namespace std;

namespace Force {

  bool VmFlatMappingStrategy::GetVmVaRangesForPa(const VmasControlBlock* pCtrlBlock, uint64 PA, vector<VmVaRange* >& rVmVaRanges, string& rErrMsg) const
  {
    uint64 VA = PA;
    if (not pCtrlBlock->GetVmVaRanges(rVmVaRanges, &VA)) {
      rErrMsg += string_snprintf(128, "PA %#llx flat map VA out of bound for the current address space", PA);
      return false;
    }
    return true;
  }

  bool VmFlatMappingStrategy::GenerateVaForPa(const ConstraintSet* pVaConstr, uint64 PA, uint64 size, const PageSizeInfo& rSizeInfo, uint64& VA, string& rErrMsg) const
  {
    if (pVaConstr->ContainsRange(PA, PA + size - 1)) {
      VA = PA;
      return true;
    }
    rErrMsg += "{VmFlatMappingStrategy::GenerateVaForPa} VA range not available for flat mapped PA";
    return false;
  }

  bool VmFlatMappingStrategy::AllocatePhysicalPage(uint64 VA, const ConstraintSet* pUsablePageAligned, const ConstraintSet* pBoundary, const GenPageRequest* pPageReq, PageSizeInfo& rSizeInfo) const
  {
    uint64 page_aligned_addr = (VA & ~rSizeInfo.mPageMask) >> rSizeInfo.mPageShift;
    bool alloc_okay = pUsablePageAligned->ContainsValue(page_aligned_addr);
    rSizeInfo.UpdatePhysicalStart(VA);
    LOG(trace) << "{VmFlatMappingStrategy::AllocatePhysicalPage} flat map alloc_okay=" << alloc_okay << endl;
    return alloc_okay;
  }

  bool VmRandomMappingStrategy::GetVmVaRangesForPa(const VmasControlBlock* pCtrlBlock, uint64 PA, vector<VmVaRange* >& rVmVaRanges, string& rErrMsg) const
  {
    return pCtrlBlock->GetVmVaRanges(rVmVaRanges, nullptr);
  }

  bool VmRandomMappingStrategy::GenerateVaForPa(const ConstraintSet* pVaConstr, uint64 PA, uint64 size, const PageSizeInfo& rSizeInfo, uint64& VA, string& rErrMsg) const
  {
    uint64 page_mask = rSizeInfo.mPageMask;
    uint64 page_offset = PA & page_mask;
    ConstraintSet copied_constr(*pVaConstr);

    copied_constr.AlignOffsetWithSize(~page_mask, page_offset, size);

    if (copied_constr.IsEmpty()) {
      rErrMsg += "{VmFlatMappingStrategy::GenerateVaForPa} va constr aligned with phys addr and offset is empty";
      return false;
    }
    // << "page mask 0x" << hex << page_mask << " offset 0x" << page_offset << " page shift " << dec << rSizeInfo.mPageShift << endl;
    copied_constr.ShiftRight(rSizeInfo.mPageShift);
    VA = (copied_constr.ChooseValue() << rSizeInfo.mPageShift) + page_offset;
    return true;
  }

  bool VmRandomMappingStrategy::AllocatePhysicalPage(uint64 VA, const ConstraintSet* pUsablePageAligned, const ConstraintSet* pBoundary, const GenPageRequest* pPageReq, PageSizeInfo& rSizeInfo) const
  {
    uint64 page_shift = rSizeInfo.mPageShift;

    if (pUsablePageAligned->IsEmpty()) {
      LOG(trace) << "{VmRandomMappingStrategy::AllocatePhysicalPage} usable page aligned is empty." << endl;
      return false;
    }

    ConstraintSet physical_masked_usable(*pUsablePageAligned);

    //Need to further constrain the physical addresses based on the max supported physical address for current vmas
    uint64 max_physical_shifted = ( rSizeInfo.MaxPhysical() + 1ull ) >> page_shift;
    uint64 constr_upper = pUsablePageAligned->UpperBound();
    if (max_physical_shifted < constr_upper) {
      physical_masked_usable.SubRange(max_physical_shifted, constr_upper);
    }

    if (physical_masked_usable.IsEmpty()) {
      LOG(trace) << "{VmRandomMappingStrategy::AllocatePhysicalPage} usable page aligned is empty after subtracting parts beyond max physical address." << endl;
      return false;
    }

    uint64 pa_target = 0x0ull;
    if (pPageReq->GetAttributeValue(EPageRequestAttributeType::PA, pa_target)) {
      uint64 page_num = (pa_target >> page_shift);
      if (physical_masked_usable.ContainsValue(page_num)) {
        LOG(trace) << "{VmRandomMappingStrategy::AllocatePhysicalPage} specified PA 0x" << hex << pa_target << " works." << endl;
        rSizeInfo.UpdatePhysicalStart(pa_target);
        return true;
      }
      LOG(warn) << "{PhysicalPageManager::NewAllocation} target pa 0x" << hex << pa_target << " not part of usable physical constraint." << endl;
      return false;
    }

    try {
      while (true) {
        uint64 page_num = physical_masked_usable.ChooseValue();
        uint64 pa_start = (page_num << page_shift);
        rSizeInfo.UpdatePhysicalStart(pa_start);
        uint64 pa_translated = (VA & rSizeInfo.mPageMask) | pa_start;
        if (not pBoundary->ContainsValue(pa_translated)) {
          physical_masked_usable.SubValue(page_num);
          LOG(trace) << "{VmRandomMappingStrategy::AllocatePhysicalPage} translated PA 0x" << hex << pa_translated << " not in proper boundary." << endl;
          continue;
        }
        LOG(trace) << "{VmRandomMappingStrategy::AllocatePhysicalPage} randomly picked start PA 0x" << hex << pa_start << endl;
        return true;
      }
    }
    catch (const ConstraintError& constr_error) {
      LOG(info) << "{VmRandomMappingStrategy::AllocatePhysicalPage} physical page allocation, failed to pick a usable value from constraint set." << endl;
    }

    return false;
  }

}
