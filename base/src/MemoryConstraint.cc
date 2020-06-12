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
#include <AddressReuseMode.h>
#include <Constraint.h>
#include <ConstraintUtils.h>
#include <Log.h>
#include <MemoryConstraint.h>

#include <memory>

using namespace std;

namespace Force {

  MemoryConstraint::MemoryConstraint()
    : mpUsable(nullptr), mpShared(nullptr), mInitialized(false)
  {
    mpUsable = new LargeConstraintSet();
    mpShared = new LargeConstraintSet();
  }

  MemoryConstraint::MemoryConstraint(const MemoryConstraint& rOther)
    : mpUsable(nullptr), mpShared(nullptr), mInitialized(false)
  {
    mpUsable = rOther.mpUsable->Clone();
    mpShared = rOther.mpShared->Clone();
    mInitialized = rOther.mInitialized;
  }

  MemoryConstraint::~MemoryConstraint()
  {
    delete mpUsable;
    delete mpShared;
  }

  void MemoryConstraint::Initialize(const ConstraintSet& constrSet)
  {
    Uninitialize();
    mpUsable->Initialize(constrSet);
    mInitialized = true;
  }

  void MemoryConstraint::Uninitialize()
  {
    mpUsable->Clear();
    mpShared->Clear();
    mInitialized = false;
  }

  void MemoryConstraint::MarkUsed(cuint64 startAddress, cuint64 endAddress)
  {
    // << "mem_constr markused start=0x" << hex << startAddress << " end=0x" << endAddress << endl;
    mpUsable->SubRange(startAddress, endAddress);
  }

  void MemoryConstraint::MarkUsed(const ConstraintSet& constrSet)
  {
    //auto cset_s_constr = ConstraintSetSerializer(constrSet, FORCE_CSET_DEFAULT_PERLINE);
    // << "mem_constr markused constr=" << cset_s_constr.ToDebugString() << endl;
    mpUsable->SubConstraintSet(constrSet);
  }

  void MemoryConstraint::MarkUsedForType(cuint64 startAddress, cuint64 endAddress, const EMemDataType memDataType, const EMemAccessType memAccessType, cuint32 threadId)
  {
    mpUsable->SubRange(startAddress, endAddress);

    if (memDataType == EMemDataType::Data) {
      MarkDataUsedForType(startAddress, endAddress, memAccessType, threadId);
    }
  }

  void MemoryConstraint::MarkShared(cuint64 startAddress, cuint64 endAddress)
  {
    // << "mem_constr markshared start=0x" << hex << startAddress << " end=0x" << endAddress << endl;
    mpShared->AddRange(startAddress, endAddress);
    mpUsable->SubRange(startAddress, endAddress);
  }

  void MemoryConstraint::MarkShared(const ConstraintSet& constrSet)
  {
    // auto cset_s_constr = ConstraintSetSerializer(constrSet, FORCE_CSET_DEFAULT_PERLINE);
    // << "mem_constr markshared constr=" << cset_s_constr.ToDebugString() << endl;
    mpShared->MergeConstraintSet(constrSet);
    mpUsable->SubConstraintSet(constrSet);
  }

  void MemoryConstraint::UnmarkUsed(cuint64 startAddress, cuint64 endAddress)
  {
    // << "mem_constr unmarkused start=0x" << hex << startAddress << " end=0x" << endAddress << endl;
    mpUsable->AddRange(startAddress, endAddress);
  }

  void MemoryConstraint::UnmarkUsed(const ConstraintSet& constrSet)
  {
    //auto cset_s_constr = ConstraintSetSerializer(constrSet, FORCE_CSET_DEFAULT_PERLINE);
    // << "mem_constr unmarkused constr=" << cset_s_constr.ToDebugString() << endl;
    mpUsable->MergeConstraintSet(constrSet);
  }

  void MemoryConstraint::ReplaceUsableInRange(uint64 lower, uint64 upper, ConstraintSet& rReplaceConstr)
  {
    mpUsable->GetConstraintSet()->ReplaceInRange(lower, upper, rReplaceConstr);
  }

  void MemoryConstraint::ApplyToConstraintSet(const EMemDataType memDataType, const EMemAccessType memAccessType, cuint32 threadId, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const
  {
    switch (memDataType) {
    case EMemDataType::Data:
      ApplyToDataConstraintSet(memAccessType, threadId, rAddrReuseMode, constrSet);
      break;
    default:
      ApplyToNonDataConstraintSet(constrSet);
    }
  }

  // This method finds the intersection of the specified constraint with the union of the usable,
  // applicable used and shared constraints.
  void MemoryConstraint::ApplyToDataConstraintSet(const EMemAccessType memAccessType, cuint32 threadId, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const
  {
    auto usable_constr = mpUsable->GetConstraintSet();
    auto shared_constr = mpShared->GetConstraintSet();
    if ((usable_constr->ContainsConstraintSet(*constrSet)) or (shared_constr->ContainsConstraintSet(*constrSet))) {
      // If constrSet is fully usable or fully shared, there's nothing to do.
      return;
    }

    const ConstraintSet* read_used_constr = nullptr;
    const ConstraintSet* write_used_constr = nullptr;
    switch (memAccessType) {
    case EMemAccessType::Read:
      if (rAddrReuseMode.IsReuseTypeEnabled(EAddressReuseType::ReadAfterRead)) {
        read_used_constr = DataReadUsed(threadId);
      }

      if (rAddrReuseMode.IsReuseTypeEnabled(EAddressReuseType::ReadAfterWrite)) {
        write_used_constr = DataWriteUsed(threadId);
      }

      break;
    case EMemAccessType::Write:
    case EMemAccessType::ReadWrite:
      if (rAddrReuseMode.IsReuseTypeEnabled(EAddressReuseType::WriteAfterRead)) {
        read_used_constr = DataReadUsed(threadId);
      }

      if (rAddrReuseMode.IsReuseTypeEnabled(EAddressReuseType::WriteAfterWrite)) {
        write_used_constr = DataWriteUsed(threadId);
      }

      break;
    case EMemAccessType::Unknown:
      break;
    default:
      LOG(fail) << "{MemoryConstraint::ApplyToDataConstraintSet} memory access type " << EMemAccessType_to_string(memAccessType) << " is not supported." << endl;
      FAIL("unsupported-mem-access-type");
    }

    unique_ptr<ConstraintSet> read_used_clone;
    if (read_used_constr != nullptr) {
      // Reuse of read addresses is enabled.
      if (read_used_constr->ContainsConstraintSet(*constrSet)) {
        // If constrSet is fully contained in the previously used read address set, it's fully
        // usable, so there's nothing to do.
        return;
      }
      else if (not read_used_constr->IsEmpty()) {
        // Otherwise, find the parts of constrSet that intersect with the previously used addresses.
        read_used_clone.reset(constrSet->Clone());
        read_used_clone->ApplyLargeConstraintSet(*read_used_constr);
      }
    }

    unique_ptr<ConstraintSet> write_used_clone;
    if (write_used_constr != nullptr) {
      // Reuse of write addresses is enabled.
      if (write_used_constr->ContainsConstraintSet(*constrSet)) {
        // If constrSet is fully contained in the previously used write address set, it's fully
        // usable, so there's nothing to do.
        return;
      }
      else if (not write_used_constr->IsEmpty()) {
        // Otherwise, find the parts of constrSet that intersect with the applicable previously used
        // addresses.
        write_used_clone.reset(constrSet->Clone());
        write_used_clone->ApplyLargeConstraintSet(*write_used_constr);
      }
    }

    // Find the parts of constrSet that intersect with the shared addresses.
    unique_ptr<ConstraintSet> shared_clone;
    if (not shared_constr->IsEmpty()) {
      shared_clone.reset(constrSet->Clone());
      shared_clone->ApplyLargeConstraintSet(*shared_constr);
    }

    // Find the parts of constrSet that intersect with the usable addresses.
    if (not usable_constr->IsEmpty()) {
      constrSet->ApplyLargeConstraintSet(*usable_constr);
    }
    else {
      constrSet->Clear();
    }

    // Merge all of the intersection results together to yield the final constraint.
    if (read_used_clone != nullptr) {
      constrSet->MergeConstraintSet(*read_used_clone);
    }

    if (write_used_clone != nullptr) {
      constrSet->MergeConstraintSet(*write_used_clone);
    }

    if (shared_clone != nullptr) {
      constrSet->MergeConstraintSet(*shared_clone);
    }
  }

  void MemoryConstraint::ApplyToNonDataConstraintSet(ConstraintSet* constrSet) const
  {
    auto usable_constr = mpUsable->GetConstraintSet();
    if (usable_constr->ContainsConstraintSet(*constrSet)) {
      // If constrSet is fully usable, there's nothing to do.
      return;
    }
    else if (not usable_constr->IsEmpty()) {
      // Find the parts of constrSet that intersect with the usable addresses.
      constrSet->ApplyLargeConstraintSet(*usable_constr);
    }
    else {
      constrSet->Clear();
    }
  }

  SingleThreadMemoryConstraint::SingleThreadMemoryConstraint(cuint32 threadId)
    : MemoryConstraint(), mThreadId(threadId), mpDataReadUsed(nullptr), mpDataWriteUsed(nullptr)
  {
    mpDataReadUsed = new LargeConstraintSet();
    mpDataWriteUsed = new LargeConstraintSet();
  }

  SingleThreadMemoryConstraint::SingleThreadMemoryConstraint(const SingleThreadMemoryConstraint& rOther)
    : MemoryConstraint(rOther), mThreadId(rOther.mThreadId), mpDataReadUsed(nullptr), mpDataWriteUsed(nullptr)
  {
    mpDataReadUsed = rOther.mpDataReadUsed->Clone();
    mpDataWriteUsed = rOther.mpDataWriteUsed->Clone();
  }

  SingleThreadMemoryConstraint::~SingleThreadMemoryConstraint()
  {
    delete mpDataReadUsed;
    delete mpDataWriteUsed;
  }

  void SingleThreadMemoryConstraint::Uninitialize()
  {
    MemoryConstraint::Uninitialize();

    mpDataReadUsed->Clear();
    mpDataWriteUsed->Clear();
  }

  void SingleThreadMemoryConstraint::MarkDataUsedForType(cuint64 startAddress, cuint64 endAddress, const EMemAccessType memAccessType, cuint32 threadId)
  {
    if (threadId == mThreadId) {
      switch (memAccessType) {
      case EMemAccessType::Read:
        mpDataReadUsed->AddRange(startAddress, endAddress);
        mpDataWriteUsed->SubRange(startAddress, endAddress);
        break;
      case EMemAccessType::Write:
        mpDataWriteUsed->AddRange(startAddress, endAddress);
        mpDataReadUsed->SubRange(startAddress, endAddress);
        break;
      default:
        // Don't allow address reuse for other cases
        break;
      }
    }
    // else ignore the update if the thread ID doesn't match
  }

  const ConstraintSet* SingleThreadMemoryConstraint::DataReadUsed(cuint32 threadId) const
  {
    if (threadId != mThreadId) {
      LOG(fail) << "{SingleThreadMemoryConstraint::DataReadUsed} read used memory constraint not found for Thread " << dec << threadId << endl;
      FAIL("mem-constraint-not-found");
    }

    return mpDataReadUsed->GetConstraintSet();
  }

  const ConstraintSet* SingleThreadMemoryConstraint::DataWriteUsed(cuint32 threadId) const
  {
    if (threadId != mThreadId) {
      LOG(fail) << "{SingleThreadMemoryConstraint::DataWriteUsed} write used memory constraint not found for Thread " << dec << threadId << endl;
      FAIL("mem-constraint-not-found");
    }

    return mpDataWriteUsed->GetConstraintSet();
  }

  MultiThreadMemoryConstraint::MultiThreadMemoryConstraint(cuint32 threadCount)
    : MemoryConstraint(), mDataReadUsedByThread(), mDataWriteUsedByThread()
  {
    for (uint32 i = 0; i < threadCount; i++) {
      mDataReadUsedByThread.emplace(i, new LargeConstraintSet());
      mDataWriteUsedByThread.emplace(i, new LargeConstraintSet());
    }
  }

  MultiThreadMemoryConstraint::MultiThreadMemoryConstraint(const MultiThreadMemoryConstraint& rOther)
    : MemoryConstraint(rOther), mDataReadUsedByThread(), mDataWriteUsedByThread()
  {
    for (const auto& data_read_used_entry : rOther.mDataReadUsedByThread) {
      auto data_read_used_clone = dynamic_cast<LargeConstraintSet*>(data_read_used_entry.second->Clone());
      mDataReadUsedByThread.emplace(data_read_used_entry.first, data_read_used_clone);
    }

    for (const auto& data_write_used_entry : rOther.mDataWriteUsedByThread) {
      auto data_write_used_clone = dynamic_cast<LargeConstraintSet*>(data_write_used_entry.second->Clone());
      mDataWriteUsedByThread.emplace(data_write_used_entry.first, data_write_used_clone);
    }
  }

  MultiThreadMemoryConstraint::~MultiThreadMemoryConstraint()
  {
    for (const auto& data_read_used_entry : mDataReadUsedByThread) {
      delete data_read_used_entry.second;
    }

    for (const auto& data_write_used_entry : mDataWriteUsedByThread) {
      delete data_write_used_entry.second;
    }
  }

  void MultiThreadMemoryConstraint::Uninitialize()
  {
    MemoryConstraint::Uninitialize();

    for (const auto& data_read_used_entry : mDataReadUsedByThread) {
      data_read_used_entry.second->Clear();
    }

    for (const auto& data_write_used_entry : mDataWriteUsedByThread) {
      data_write_used_entry.second->Clear();
    }
  }

  void MultiThreadMemoryConstraint::MarkDataUsedForType(cuint64 startAddress, cuint64 endAddress, const EMemAccessType memAccessType, cuint32 threadId)
  {
    auto read_itr = mDataReadUsedByThread.find(threadId);
    if (read_itr == mDataReadUsedByThread.end()) {
      return;
    }

    LargeConstraintSet* data_read_used = read_itr->second;

    auto write_itr = mDataWriteUsedByThread.find(threadId);
    if (write_itr == mDataWriteUsedByThread.end()) {
      return;
    }

    LargeConstraintSet* data_write_used = write_itr->second;

    switch (memAccessType) {
    case EMemAccessType::Read:
      data_read_used->AddRange(startAddress, endAddress);
      data_write_used->SubRange(startAddress, endAddress);
      break;
    case EMemAccessType::Write:
      data_write_used->AddRange(startAddress, endAddress);
      data_read_used->SubRange(startAddress, endAddress);
      break;
    default:
      // Don't allow address reuse for other cases
      break;
    }
  }

  const ConstraintSet* MultiThreadMemoryConstraint::DataReadUsed(cuint32 threadId) const
  {
    auto itr = mDataReadUsedByThread.find(threadId);
    if (itr == mDataReadUsedByThread.end()) {
      LOG(fail) << "{MultiThreadMemoryConstraint::DataReadUsed} read used memory constraint not found for Thread " << dec << threadId << endl;
      FAIL("mem-constraint-not-found");
    }

    return itr->second->GetConstraintSet();
  }

  const ConstraintSet* MultiThreadMemoryConstraint::DataWriteUsed(cuint32 threadId) const
  {
    auto itr = mDataWriteUsedByThread.find(threadId);
    if (itr == mDataWriteUsedByThread.end()) {
      LOG(fail) << "{MultiThreadMemoryConstraint::DataWriteUsed} write used memory constraint not found for Thread " << dec << threadId << endl;
      FAIL("mem-constraint-not-found");
    }

    return itr->second->GetConstraintSet();
  }

  LargeConstraintSet::LargeConstraintSet()
    : mpConstraintSet(nullptr), mpCachedAdds(nullptr), mpCachedSubs(nullptr), mState(0)
  {
    mpConstraintSet = new ConstraintSet();
    mpCachedAdds = new ConstraintSet();
    mpCachedSubs = new ConstraintSet();
  }

  LargeConstraintSet::LargeConstraintSet(const LargeConstraintSet& rOther)
    : mpConstraintSet(nullptr), mpCachedAdds(nullptr), mpCachedSubs(nullptr), mState(rOther.mState)
  {
    if (nullptr != rOther.mpConstraintSet) {
      mpConstraintSet = rOther.mpConstraintSet->Clone();
    }
    if (nullptr != rOther.mpCachedAdds) {
      mpCachedAdds = rOther.mpCachedAdds->Clone();
    }
    if (nullptr != rOther.mpCachedSubs) {
      mpCachedSubs = rOther.mpCachedSubs->Clone();
    }
  }

  LargeConstraintSet::~LargeConstraintSet()
  {
    delete mpConstraintSet;
    delete mpCachedAdds;
    delete mpCachedSubs;
  }

  void LargeConstraintSet::Initialize(const ConstraintSet& rInitConstrs)
  {
    mpConstraintSet->MergeConstraintSet(rInitConstrs);
  }

  void LargeConstraintSet::Clear()
  {
    mpConstraintSet->Clear();
    mpCachedAdds->Clear();
    mpCachedSubs->Clear();
  }

  ConstraintSet* LargeConstraintSet::GetConstraintSet()
  {
    if (SomethingCached()) {
      if (AddCached()) {
        CommitCachedAdds();
      }
      if (SubCached()) {
        CommitCachedSubs();
      }
    }

    return mpConstraintSet;
  }

  void LargeConstraintSet::CommitCachedAdds()
  {
    mpConstraintSet->MergeConstraintSet(*mpCachedAdds);
    mpCachedAdds->Clear();
    ClearAddCached();
  }

  void LargeConstraintSet::CommitCachedSubs()
  {
    mpConstraintSet->SubConstraintSet(*mpCachedSubs);
    mpCachedSubs->Clear();
    ClearSubCached();
  }

  void LargeConstraintSet::AddRange(uint64 lower, uint64 upper)
  {
    if (SubCached()) {
      CommitCachedSubs();
    }

    mpCachedAdds->AddRange(lower, upper);
    SetAddCached();
  }

  void LargeConstraintSet::MergeConstraintSet(const ConstraintSet& rConstrSet)
  {
    if (SubCached()) {
      CommitCachedSubs();
    }

    mpCachedAdds->MergeConstraintSet(rConstrSet);
    SetAddCached();
  }

  void LargeConstraintSet::SubRange(uint64 lower, uint64 upper)
  {
    if (AddCached()) {
      CommitCachedAdds();
    }

    mpCachedSubs->AddRange(lower, upper);
    SetSubCached();
  }

  void LargeConstraintSet::SubConstraintSet(const ConstraintSet& rConstrSet)
  {
    if (AddCached()) {
      CommitCachedAdds();
    }

    mpCachedSubs->MergeConstraintSet(rConstrSet);
    SetSubCached();
  }

}
