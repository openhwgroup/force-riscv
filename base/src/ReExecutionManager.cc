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
#include <ReExecutionManager.h>
#include <ReExecutionUnit.h>
#include <Log.h>

using namespace std;

namespace Force {

  ReExecutionManager::ReExecutionManager()
    : Object(), mpCurrentGenUnit(nullptr), mpCurrentGenBlock(nullptr), mActiveGenUnits(), mUnits(), mBlocks()
  {

  }

  ReExecutionManager::ReExecutionManager(const ReExecutionManager& rOther)
    : Object(rOther), mpCurrentGenUnit(nullptr), mpCurrentGenBlock(nullptr), mActiveGenUnits(), mUnits(), mBlocks()
  {

  }

  ReExecutionManager::~ReExecutionManager()
  {
    if (mpCurrentGenUnit != nullptr) {
      LOG(fail) << "{ReExecutionManager::~ReExecutionManager} dangling mpCurrentGenUnit pointer." << endl;
      mpCurrentGenUnit = nullptr;
      FAIL("dangling-gen-unit-pointer");
    }

    if (mpCurrentGenBlock != nullptr) {
      LOG(fail) << "{ReExecutionManager::~ReExecutionManager} dangling mpCurrentGenBlock pointer." << endl;
      mpCurrentGenBlock = nullptr;
      FAIL("dangling-gen-block-pointer");
    }

    if (mActiveGenUnits.size()) {
      LOG(fail) << "{ReExecutionManager::~ReExecutionManager} dangling active gen units." << endl;
      FAIL("dangling-active-gen-units");
    }

    for (auto unit_item : mUnits) {
      delete unit_item;
    }

    for (auto block_item : mBlocks) {
      delete block_item;
    }
  }

  void ReExecutionManager::PushActiveGenUnit(ReExecutionUnit* pNewUnit)
  {
    if (nullptr != mpCurrentGenUnit) {
      mActiveGenUnits.push_back(mpCurrentGenUnit);
    }
    mpCurrentGenUnit = pNewUnit;
  }

  void ReExecutionManager::PopActiveGenUnit(uint64 endAddr)
  {
    if (nullptr == mpCurrentGenUnit) {
      LOG(fail) << "{ReExecutionManager::PopActiveGenUnit} no active gen unit present." << endl;
      FAIL("no-active-gen-unit");
    }
    else {
      mpCurrentGenUnit->SetEndAddress(endAddr);
    }

    if (mActiveGenUnits.size()) {
      mpCurrentGenUnit = mActiveGenUnits.back();
      mActiveGenUnits.pop_back();
    }
    else {
      mpCurrentGenUnit = nullptr;
    }
  }

  uint32 ReExecutionManager::StartLoop(uint64 loopBackAddr)
  {
    auto new_loop = new Loop();
    auto id = mUnits.size();
    new_loop->SetId(id);
    new_loop->SetBeginAddress(loopBackAddr);
    mUnits.push_back(new_loop);
    PushActiveGenUnit(new_loop);
    return id;
  }

  void ReExecutionManager::EndLoop(uint32 loopId, uint64 loopEndAddr)
  {
    if (nullptr == mpCurrentGenUnit) {
      LOG(fail) << "{ReExecutionManager::EndLoop} no current unit." << endl;
      FAIL("no-current-unit");
    }

    if (mpCurrentGenUnit->Id() != loopId) {
      LOG(fail) << "{ReExecutionManager::EndLoop} mismtach between passed in ID: " << dec << loopId << " and active ID: " << mpCurrentGenUnit->Id() << endl;
      FAIL("mismatch-active-loop-id");
    }

    PopActiveGenUnit(loopEndAddr);
  }

  void ReExecutionManager::ReportPostLoopAddress(uint32 loopId, uint64 postLoopAddr)
  {
    if (nullptr == mpCurrentGenUnit) {
      LOG(fail) << "{ReExecutionManager::ReportPostLoopAddress} no current unit." << endl;
      FAIL("no-current-unit");
    }

    if (mpCurrentGenUnit->Id() != loopId) {
      LOG(fail) << "{ReExecutionManager::ReportPostLoopAddress} mismtach between passed in ID: " << dec << loopId << " and active ID: " << mpCurrentGenUnit->Id() << endl;
      FAIL("mismatch-active-loop-id");
    }

    auto current_loop = dynamic_cast<Loop* > (mpCurrentGenUnit);
    if (nullptr == current_loop) {
      LOG(fail) << "{ReExecutionManager::ReportPostLoopAddress} current unit is not a loop." << endl;
      FAIL("active-unit-not-loop");
    }
    current_loop->SetPostLoopAddress(postLoopAddr);
  }

  uint64 ReExecutionManager::PostLoopAddress() const
  {
    if (nullptr == mpCurrentGenUnit) {
      LOG(fail) << "{ReExecutionManager::PostLoopAddress} no current unit." << endl;
      FAIL("no-current-unit");
    }

    auto current_loop = dynamic_cast<Loop* > (mpCurrentGenUnit);
    if (nullptr == current_loop) {
      LOG(fail) << "{ReExecutionManager::PostLoopAddress} current unit is not a loop." << endl;
      FAIL("active-unit-not-loop");
    }

    return current_loop->PostLoopAddress();
  }

  void ReExecutionManager::ReportLoopReconvergeAddress(uint32 loopId, uint64 loopReconvergeAddr)
  {
    if (nullptr == mpCurrentGenUnit) {
      LOG(fail) << "{ReExecutionManager::ReportLoopReconvergeAddress} no current unit." << endl;
      FAIL("no-current-unit");
    }

    if (mpCurrentGenUnit->Id() != loopId) {
      LOG(fail) << "{ReExecutionManager::ReportLoopReconvergeAddress} mismtach between passed in ID: " << dec << loopId << " and active ID: " << mpCurrentGenUnit->Id() << endl;
      FAIL("mismatch-active-loop-id");
    }

    auto current_loop = dynamic_cast<Loop* > (mpCurrentGenUnit);
    if (nullptr == current_loop) {
      LOG(fail) << "{ReExecutionManager::ReportLoopReconvergeAddress} current unit is not a loop." << endl;
      FAIL("active-unit-not-loop");
    }
    current_loop->SetReconvergeAddress(loopReconvergeAddr);
  }

  uint64 ReExecutionManager::LoopReconvergeAddress() const
  {
    if (nullptr == mpCurrentGenUnit) {
      LOG(fail) << "{ReExecutionManager::LoopReconvergeAddress} no current unit." << endl;
      FAIL("no-current-unit");
    }

    auto current_loop = dynamic_cast<Loop* > (mpCurrentGenUnit);
    if (nullptr == current_loop) {
      LOG(fail) << "{ReExecutionManager::LoopReconvergeAddress} current unit is not a loop." << endl;
      FAIL("active-unit-not-loop");
    }

    return current_loop->ReconvergeAddress();
  }

  uint32 ReExecutionManager::StartLinearBlock(uint64 startAddr)
  {
    if (nullptr != mpCurrentGenBlock) {
      LOG(fail) << "{ReExecutionManager::StartLinearBlock} cannot have overlapping linear blocks." << endl;
      FAIL("overlapping-linear-block");
    }

    auto new_block = new LinearBlock();
    auto id = mBlocks.size();
    new_block->SetId(id);
    new_block->SetBeginAddress(startAddr);
    mBlocks.push_back(new_block);
    mpCurrentGenBlock = new_block;
    return id;
  }

  uint64 ReExecutionManager::EndLinearBlock(uint32 blockId, uint64 blockEndAddr)
  {
    if (nullptr == mpCurrentGenBlock) {
      LOG(fail) << "{ReExecutionManager::EndLinearBlock} no current linear block." << endl;
      FAIL("no-current-linear-block");
    }

    if (mpCurrentGenBlock->Id() != blockId) {
      LOG(fail) << "{ReExecutionManager::EndLinearBlock} mismatch between passed in ID:" << dec << blockId << " and active ID: " << mpCurrentGenBlock->Id() << endl;
      FAIL("mismatch-active-block-id");
    }

    uint64 ret_addr = mpCurrentGenBlock->BeginAddress();
    mpCurrentGenBlock->SetEndAddress(blockEndAddr);
    mpCurrentGenBlock = nullptr;
    return ret_addr;
  }

}
