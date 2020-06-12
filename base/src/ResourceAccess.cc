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
#include <ResourceAccess.h>
#include <Constraint.h>
#include <Generator.h>
#include <StringUtils.h>
#include <Log.h>

#include <sstream>

using namespace std;

/*!
  \file ResourceAccess.cc
  \brief Resource access dependency related code.
*/

namespace Force {

//#define DEBUG_OBJECT_DELETE 1
#ifdef DEBUG_OBJECT_DELETE
#define DELETE_OBJECT(obj) \
  if (nullptr == obj) { \
    LOG(notice) << "Deleting : nullptr." << endl; \
  } \
  else { \
    LOG(notice) << "Deleting : " << obj->ToString() << endl; \
    delete obj; \
  }
#else
#define DELETE_OBJECT(obj) delete obj
#endif

//#define DEBUG_ENTROPY 1

  ResourceAccessStage::ResourceAccessStage() : mSourceAccesses(), mDestAccesses()
  {
    mSourceAccesses.assign(EResourceTypeSize, nullptr);
    mDestAccesses.assign(EResourceTypeSize, nullptr);
  }

  ResourceAccessStage::ResourceAccessStage(const ResourceAccessStage& rOther) : mSourceAccesses(), mDestAccesses()
  {
    for (auto other_access : rOther.mSourceAccesses) {
      if (other_access) {
        auto access = dynamic_cast<ConstraintSet* >(other_access->Clone());
        mSourceAccesses.push_back(access);
      }
      else
        mSourceAccesses.push_back(nullptr);
    }

    for (auto other_access : rOther.mDestAccesses) {
      if (other_access) {
        auto access = dynamic_cast<ConstraintSet* >(other_access->Clone());
        mDestAccesses.push_back(access);
      }
      else
        mDestAccesses.push_back(nullptr);
    }
  }

  Object* ResourceAccessStage::Clone() const
  {
    return new ResourceAccessStage(*this);
  }

  ResourceAccessStage::~ResourceAccessStage()
  {
    for (EResourceTypeBaseType i = 0; i < EResourceTypeSize; ++ i) {
      DELETE_OBJECT(mSourceAccesses[i]);
      mSourceAccesses[i] = nullptr;
      DELETE_OBJECT(mDestAccesses[i]);
      mDestAccesses[i] = nullptr;
    }
  }

  void ResourceAccessStage::RecordAccess(ERegAttrType access, EResourceType resType, ConstraintSet* pConstr)
  {
    vector<ConstraintSet* > * container_ptr = nullptr;
    switch (access) {
    case ERegAttrType::Read:
      container_ptr = &mSourceAccesses;
      break;
    case ERegAttrType::ReadWrite:
    case ERegAttrType::Write:
      container_ptr = &mDestAccesses;
      break;
    default:
      LOG(fail) << "{ResourceAccessStage::RecordAccess} unimplemented dependence access type:\"" << ERegAttrType_to_string(access) << "\"" << endl;
      FAIL("unimplemented-dependence-access-attribute");
    }

    EResourceTypeBaseType res_index = EResourceTypeBaseType(resType);
    auto existing_ptr = (*container_ptr)[res_index];
    if (nullptr != existing_ptr) {
      existing_ptr->MergeConstraintSet(*pConstr);
      delete pConstr; // TODO add a MergeConsumeConstraintSet method.
    }
    else {
      (*container_ptr)[res_index] = pConstr;
    }
  }

  bool ResourceAccessStage::HasSourceAccess(uint32 index, EResourceType resType) const
  {
    auto constr_ptr = mSourceAccesses[EResourceTypeBaseType(resType)];
    if (constr_ptr != nullptr && constr_ptr->ContainsValue(index))
      return true;

    return false;
  }

  bool ResourceAccessStage::HasDestAccess(uint32 index, EResourceType resType) const
  {
    auto constr_ptr = mDestAccesses[EResourceTypeBaseType(resType)];
    if (constr_ptr != nullptr && constr_ptr->ContainsValue(index))
      return true;

    return false;
  }

  void ResourceAccessStage::RemoveSourceAccess(uint32 index, EResourceType resType)
  {
    auto constr_ptr = mSourceAccesses[EResourceTypeBaseType(resType)];
    if (nullptr != constr_ptr) {
      constr_ptr->SubValue(index);
      if (constr_ptr->IsEmpty()) {
        delete constr_ptr;
        mSourceAccesses[EResourceTypeBaseType(resType)] = nullptr;
      }
    }
  }

  void ResourceAccessStage::RemoveDestAccess(uint32 index, EResourceType resType)
  {
    auto constr_ptr = mDestAccesses[EResourceTypeBaseType(resType)];
    if (nullptr != constr_ptr) {
      constr_ptr->SubValue(index);
      if (constr_ptr->IsEmpty()) {
        delete constr_ptr;
        mDestAccesses[EResourceTypeBaseType(resType)] = nullptr;
      }
    }
  }

  const ConstraintSet* ResourceAccessStage::GetDependenceConstraint(EResourceType resType, EDependencyType depType) const
  {
    const ConstraintSet* ret_constr = nullptr;
    switch (depType) {
    case EDependencyType::OnSource:
      ret_constr = mSourceAccesses[EResourceTypeBaseType(resType)];
      break;
    case EDependencyType::OnTarget:
      ret_constr = mDestAccesses[EResourceTypeBaseType(resType)];
      break;
    case EDependencyType::NoDependency:
      break;
    default:
      LOG(fail) << "{ResourceAccessStage::GetDependenceConstraint} unexpected depenency type: " << EDependencyType_to_string(depType);
      FAIL("unexpected-dependency-type");
    }
    return ret_constr;
  }

  void ResourceAccessStage::Retire(uint32 index, std::vector<ResourceTypeEntropy* >& rTypeEntropy)
  {
    for (EResourceTypeBaseType i = 0; i < EResourceTypeSize; ++ i) {
      auto dest_constr = mDestAccesses[i];
      if (nullptr != dest_constr) {
        LOG(notice)<< "retire dest stage: " << hex << index << ", access: " << dest_constr->ToSimpleString() << ", size: " << dest_constr->Size() << ", type: "  << int(i) << endl;
        rTypeEntropy[i]->DestEntropy().Decrease(dest_constr->Size());
      }
      auto src_constr = mSourceAccesses[i];
      if (nullptr != src_constr) {
        LOG(notice)<< "retire source stage: " << hex << index << ", access: " << src_constr->ToSimpleString() << ", size: " << src_constr->Size() << ", type: "  << int(i) << endl;
        rTypeEntropy[i]->SourceEntropy().Decrease(src_constr->Size());
      }
    }
  }

  const string ResourceAccessStage::ToString() const
  {
    stringstream out_str;

    out_str << "ResourceAccessStage:";
    for (EResourceTypeBaseType i = 0; i < EResourceTypeSize; ++ i) {
      EResourceType res_type = EResourceType(i);
      auto src_acc = mSourceAccesses[i];
      if (nullptr != src_acc) {
        out_str << " " << EResourceType_to_string(res_type) << " source (" << src_acc->ToSimpleString() << ")";
      }
      auto dest_acc = mDestAccesses[i];
      if (nullptr != dest_acc) {
        out_str << " " << EResourceType_to_string(res_type) << " dest (" << dest_acc->ToSimpleString() << ")";
      }
    }

    return out_str.str();
  }

  const string ResourceAccessStage::ToSimpleString() const
  {
    stringstream out_str;

    for (EResourceTypeBaseType i = 0; i < EResourceTypeSize; ++ i) {
      EResourceType res_type = EResourceType(i);
      auto src_acc = mSourceAccesses[i];
      if (nullptr != src_acc) {
        out_str << "+" << EResourceType_to_string(res_type) << "_SRC(" << src_acc->ToSimpleString() << ")";
      }
      auto dest_acc = mDestAccesses[i];
      if (nullptr != dest_acc) {
        out_str << "+" << EResourceType_to_string(res_type) << "_DST(" << dest_acc->ToSimpleString() << ")";
      }
    }

    return out_str.str();
  }

  AccessAge::AccessAge()
    : mIndex(0), mAge(-1), mResourceType(EResourceType(0)), mAccessType(EAccessAgeType(0))
  {
  }

  AccessAge::AccessAge(const AccessAge& rOther): mIndex(rOther.mIndex), mAge(rOther.mAge), mResourceType(rOther.mResourceType), mAccessType(rOther.mAccessType)
  {

  }

  void AccessAge::Setup(uint32 index, EResourceType resType)
  {
    mIndex = index;
    mResourceType = resType;
  }

  void AccessAge::UpdateAge(uint32 age, EAccessAgeType ageType, ResourceAccessQueue* pAccessQueue)
  {
    switch (mAccessType) {
    case EAccessAgeType::Invalid:
      break;
    case EAccessAgeType::Read:
      // Clear existing read record
      pAccessQueue->RemoveSourceAccess(mIndex, mAge, mResourceType);
      break;
    case EAccessAgeType::Write:
      // Clear existing write record
      pAccessQueue->RemoveDestAccess(mIndex, mAge, mResourceType);
      break;
    default:
      LOG(fail) << "{AccessAge::UpdateAge} unexpected resource access time stamp type: " << EAccessAgeType_to_string(mAccessType) << endl;
      FAIL("unexpected-res-acc-ts-type");
    }
    mAge = age;
    mAccessType = ageType;
  }

  static const uint32 DEP_MAX_ENTRY_COUNT = 256; //!< set a practical maximum dependency entry count.
  Object* ResourceTypeAges::Clone() const
  {
    return new ResourceTypeAges(*this);
  }

  ResourceTypeAges::ResourceTypeAges(const ResourceTypeAges& rOther) : mType(rOther.mType), mAges(rOther.mAges)
  {

  }

  void ResourceTypeAges::Setup(uint32 numEntries)
  {
    if (numEntries > DEP_MAX_ENTRY_COUNT) {
      LOG(fail) << "{ResourceTypeAges::Setup} specified entry count: " << dec << numEntries << " exceeded max entry count: " << DEP_MAX_ENTRY_COUNT << endl;
      FAIL("exceeded-max-entry-count");
    }

    mAges.assign(numEntries, AccessAge());
    uint32 item_index = 0;
    for (auto & age_item : mAges) {
      age_item.Setup(item_index, mType);
      ++ item_index;
    }
  }

  void ResourceTypeAges::UpdateAge(uint32 index, uint32 age, EAccessAgeType ageType, ResourceAccessQueue* pAccessQueue)
  {
    AccessAge& age_entry = mAges[index];
    age_entry.UpdateAge(age, ageType, pAccessQueue);
  }

  const AccessAge& ResourceTypeAges::GetAccessAge(uint32 index) const
  {
    return mAges[index];
  }

  ResourceTypeEntropy::ResourceTypeEntropy(EResourceType type)
    : mType(type), mSourceEntropy(), mDestEntropy()  {
    mSourceEntropy.SetName("Source");
    mDestEntropy.SetName("Dest");
  }

  ResourceTypeEntropy::ResourceTypeEntropy(const ResourceTypeEntropy& rOther) : mType(rOther.mType), mSourceEntropy(rOther.mSourceEntropy), mDestEntropy(rOther.mDestEntropy)
  {

  }

  Object* ResourceTypeEntropy::Clone() const
  {
    return new ResourceTypeEntropy(*this);
  }

  void WindowLookUpFar::SlideWindow(uint32& value) const
  {
    if (value == mHigh)
      value = mLow;
    else
      value ++;

    LOG(info) << "Slide dependency window from the nearest to farthest, slot #" << value << endl;
  }

  void WindowLookUpNear::SlideWindow(uint32& value) const
  {
    if (value == mLow)
      value = mHigh;
    else
      value --;

    LOG(info) << "Slide dependency window from the farthest to nearest, slot #" << value << endl;
  }

  AccessEntropy::AccessEntropy()
    : mName(), mState(EEntropyStateType(0)), mEntropy(0), mOnThreshold(0), mOffThreshold(0)
  {
  }

  AccessEntropy::AccessEntropy(const AccessEntropy& rOther) : mName(rOther.mName), mState(rOther.mState), mEntropy(rOther.mEntropy), mOnThreshold(rOther.mOnThreshold), mOffThreshold(rOther.mOffThreshold)
  {

  }

  void AccessEntropy::SetThresholds(uint32 on, uint32 off)
  {
    mOnThreshold = on;
    mOffThreshold = off;
  }

  void AccessEntropy::SetThresholds(const std::string& rStr)
  {
    StringSplitter splitter(rStr, '-');
    bool error_status = false;

    mOnThreshold = parse_uint32(splitter.NextSubString(), &error_status);
    if (error_status) {
      LOG(fail) << "{AccessEntropy::SetThreshold} invalid entropy window format: " << rStr << endl;
      FAIL("Invalid-entropy-window-format");
    }

    mOffThreshold = parse_uint32(splitter.NextSubString(), &error_status);
    if (error_status) {
      LOG(fail) << "{AccessEntropy::SetThreshold} invalid entropy window format: " << rStr << endl;
      FAIL("Invalid-entropy-window-format");
    }
  }

  void AccessEntropy::Decrease(uint32 num)
  {
    uint32 entropy = mEntropy;
    mEntropy -= num;
    if (mEntropy > entropy) {
      LOG(fail) << "{AccessEntropy::Decrease} resource entropy is underflow, try to descrease " << entropy << "  by " << hex << num << endl;
      FAIL("resource-entropy-underflow");
    }
  }

  void AccessEntropy::UpdateState()
  {
    switch (mState) {
    case EEntropyStateType::WarmUp:
    case EEntropyStateType::CoolDown:
      if (mEntropy >= mOnThreshold) {
        LOG(info) << "{AccessEntropy::UpdateState} " << Name() << " dependence turned ON from " << EEntropyStateType_to_string(mState) << endl;
        mState = EEntropyStateType::Stable;
      }
      break;
    case EEntropyStateType::Stable:
      if (mEntropy < mOffThreshold) {
        LOG(info) << "{AccessEntropy::UpdateState} " << Name() << " dependence turned OFF from " << EEntropyStateType_to_string(mState) << endl;
        mState = EEntropyStateType::CoolDown;
      }
      break;
    default:
      LOG(fail) << "{AccessEntropy::UpdateState} Unsupported entropy state type: "<<  EEntropyStateType_to_string(mState) << endl;
      FAIL("unsupported-entropy-state-type");
    }
  }

  const string AccessEntropy::ToString() const
  {
    stringstream out_str;

    out_str << Name() << " entropy state: " << EEntropyStateType_to_string(mState) << ", on-threshold: " << dec << mOnThreshold << ", off-threshold: " << mOffThreshold;

    return out_str.str();
  }

  ResourceAccessQueue::ResourceAccessQueue()
    : Object(), mAge(0), mHistoryLimit(0), mIndex(0), mLookUpFar(), mLookUpNear(), mpReturnConstraint(nullptr), mQueue(), mTypeAges(), mTypeEntropies()
  {

  }

  ResourceAccessQueue::ResourceAccessQueue(const ResourceAccessQueue& rOther)
    : Object(rOther), mAge(0), mHistoryLimit(0), mIndex(0), mLookUpFar(), mLookUpNear(), mpReturnConstraint(nullptr), mQueue(), mTypeAges(), mTypeEntropies()
  {
    // << "copy constructor const version" << endl;
  }

  ResourceAccessQueue::ResourceAccessQueue(ResourceAccessQueue& rOther)
    : Object(rOther), mAge(rOther.mAge), mHistoryLimit(rOther.mHistoryLimit), mIndex(rOther.mIndex), mLookUpFar(rOther.mLookUpFar), mLookUpNear(rOther.mLookUpNear), mpReturnConstraint(nullptr), mQueue(), mTypeAges(), mTypeEntropies()
  {
    // << "copy constructor non-const version" << endl;
    mpReturnConstraint = dynamic_cast<ConstraintSet* >(rOther.mpReturnConstraint->Clone());
    for (auto other_stage : rOther.mQueue) {
      auto stage = dynamic_cast<ResourceAccessStage* >(other_stage->Clone());
      mQueue.push_back(stage);
    }

    for (auto other_age : rOther.mTypeAges) {
      auto age = dynamic_cast<ResourceTypeAges* >(other_age->Clone());
      mTypeAges.push_back(age);
    }

    for (auto other_entropy : rOther.mTypeEntropies) {
      auto entropy = dynamic_cast<ResourceTypeEntropy* >(other_entropy->Clone());
      mTypeEntropies.push_back(entropy);
    }

  }

  ResourceAccessQueue::~ResourceAccessQueue()
  {
    delete mpReturnConstraint;

    for (auto acc_entry : mQueue) {
      delete acc_entry;
    }

    for (auto type_item : mTypeAges) {
      delete type_item;
    }

    for (auto entropy_item : mTypeEntropies) {
      delete entropy_item;
    }
  }

  Object* ResourceAccessQueue::Clone() const
  {
    return new ResourceAccessQueue(*this);
  }

  const string ResourceAccessQueue::ToString() const
  {
    stringstream out_stream;
    out_stream << "ResourceAccessQueue: mAge=" << dec << mAge << " mHistoryLimit=" << mHistoryLimit << " mIndex=" << mIndex << endl;
    for (uint32 i = 0; i < mHistoryLimit; i ++) {
      out_stream << " slot " << i << " : " << mQueue[i]->ToString() << endl;
    }
    return out_stream.str();
  }

  void ResourceAccessQueue::Setup(uint32 historyLimit)
  {
    mpReturnConstraint = new ConstraintSet();
    mHistoryLimit = historyLimit;

    for (uint64 i = 0; i < mHistoryLimit ; i ++) {
      mQueue.push_back(new ResourceAccessStage());
    }
  }

  ResourceAccessStage* ResourceAccessQueue::CreateHotResource()
  {
    return new ResourceAccessStage();
  }

  void ResourceAccessQueue::RetireReuseStage()
  {
    auto retired_stage = mQueue[mIndex];
    retired_stage->Retire(mIndex, mTypeEntropies);
    mQueue[mIndex] = nullptr;
    delete retired_stage;
  }

  void ResourceAccessQueue::Commit(ResourceAccessStage* pHotResource)
  {
    RetireReuseStage();
    mQueue[mIndex] = pHotResource;

    UpdateAccessEntropy(pHotResource);

#ifdef DEBUG_ENTROPY
    // Do redundant calculation to verify entropy values.
    VerifyAccessEntropy();
#endif

    UpdateAccessAge(pHotResource);

    UpdateEntropyState();

    if (++ mIndex == mHistoryLimit) {
      mIndex = 0;
    }
  }

  void ResourceAccessQueue::UpdateAccessEntropy(const ResourceAccessStage* pHotResource)
  {
    auto src_accesses = pHotResource->GetSourceAccesses();
    auto dest_accesses = pHotResource->GetDestAccesses();

    for (EResourceTypeBaseType i = 0; i < EResourceTypeSize; ++ i) {
      auto entropy = mTypeEntropies[i];
      auto src_constr = src_accesses[i];
      if (nullptr != src_constr) {
        entropy->SourceEntropy().Increase(src_constr->Size());
        LOG(notice) << "current source entropy:"<< entropy->SourceEntropy().Entropy() << ", resource type: " << int(i) << endl;
      }
      auto dest_constr = dest_accesses[i];
      if (nullptr != dest_constr) {
        entropy->DestEntropy().Increase(dest_constr->Size());
        LOG(notice) << "current dest entropy:"<< entropy->DestEntropy().Entropy() << ", resource type: " << int(i) << endl;
      }
    }
  }

  void ResourceAccessQueue::UpdateEntropyState()
  {
    for (EResourceTypeBaseType i = 0; i < EResourceTypeSize; ++ i) {
      auto entropy = mTypeEntropies[i];
      entropy->SourceEntropy().UpdateState(); // update source on/off state.
      entropy->DestEntropy().UpdateState(); // update dest on/off state.
    }
  }

  void ResourceAccessQueue::VerifyAccessEntropy() const
  {
    // local check lambda to simply code.
    auto verify_access_entropy_lambda = [&] (EResourceType res_type, EDependencyType dep_type, const AccessEntropy& acc_entropy) {
      uint32 entropy_sum = CalculateAccessEntropy(res_type, dep_type);
      uint32 entropy_saved = acc_entropy.Entropy();
      bool check_okay = (entropy_sum == entropy_saved);
      LOG(info) << "{ResourceAccessQueue::VerifyAccessEntropy} saved entropy: " << dec << entropy_saved << (check_okay ? "==" : " != ") << " " << entropy_sum << " for " << EResourceType_to_string(res_type) << " " << EDependencyType_to_string(dep_type) << endl;
      if (not check_okay) {
        LOG(fail) << "{ResourceAccessQueue::VerifyAccessEntropy} " << dec << entropy_saved << " != " << entropy_sum << endl;
        FAIL("entropy-check-fail");
      }
    };

    // Do redundant calculation to verify entroy value.
    for (EResourceTypeBaseType i = 0; i < EResourceTypeSize; ++ i) {
      EResourceType res_type = EResourceType(i);
      auto res_entropy = mTypeEntropies[i];
      const AccessEntropy& acc_src = res_entropy->SourceEntropy();
      verify_access_entropy_lambda(res_type, EDependencyType::OnSource, acc_src);
      const AccessEntropy& acc_dest = res_entropy->DestEntropy();
      verify_access_entropy_lambda(res_type, EDependencyType::OnTarget, acc_dest);
    }
  }

  uint32 ResourceAccessQueue::CalculateAccessEntropy(EResourceType resType, EDependencyType depType) const
  {
    uint32 entropy_sum = 0;
    for (auto acc_entry : mQueue) {
      const ConstraintSet* entry_constr = acc_entry->GetDependenceConstraint(resType, depType);
      if (nullptr != entry_constr) {
        entropy_sum += entry_constr->Size();
      }
    }
    return entropy_sum;
  }

  void ResourceAccessQueue::UpdateAccessAge(const ResourceAccessStage* pHotResource)
  {
    auto stage_age = mAge;
    const vector<ConstraintSet* >& src_accesses = pHotResource->GetSourceAccesses();
    const vector<ConstraintSet* >& dest_accesses = pHotResource->GetDestAccesses();

    for (EResourceTypeBaseType i = 0; i < EResourceTypeSize; ++ i) {
      auto age_container = mTypeAges[i];

      // Update source register ages.
      auto src_constr = src_accesses[i];
      if (nullptr != src_constr) {
        vector<uint64> regs;
        src_constr->GetValues(regs);
        for (auto reg : regs) {
          age_container->UpdateAge(reg, stage_age, EAccessAgeType::Read, this);
        }
      }

      // Update dest register ages, this need to go after source registers.
      auto dest_constr = dest_accesses[i];
      if (nullptr != dest_constr) {
        vector<uint64> regs;
        dest_constr->GetValues(regs);
        for (auto reg : regs) {
          age_container->UpdateAge(reg, stage_age, EAccessAgeType::Write, this);
        }
      }
    }

    ++ mAge;
  }

  ResourceAccessStage* ResourceAccessQueue::GetAccessStage(uint32 dist)
  {
    uint32 queue_index = GetQueueIndex(dist);
    return mQueue[queue_index];
  }

  const ResourceAccessStage* ResourceAccessQueue::ChosenAccessStage(uint32 dist) const
  {
    if (dist > mHistoryLimit)  {
      LOG(fail) << "Too large dependence looking up value: "<< dist << endl;
      FAIL("too-large-looking-up-value");
    }
    uint32 queue_index = GetQueueIndex(dist);
    LOG(info) << "Chosen dependence resource #" << dec << dist << " slot: " << queue_index << " resource index: " << mIndex << " history limit: " << mHistoryLimit << endl;
    return mQueue[queue_index];
  }

  void ResourceAccessQueue::RemoveSourceAccess(uint32 index, uint32 age, EResourceType resType)
  {
    //<< "{ResourceAccessQueue::RemoveSourceAccess} current age: " << dec << mAge << " removing source access, index=" << index << " age=" << age << " resource-type=" << EResourceType_to_string(resType) << endl;
    auto stage_dist = mAge - age;
    if (stage_dist >= mHistoryLimit) // check if it has already fall out of bound of history limit.
      return;

    auto access_stage = GetAccessStage(stage_dist);
    // << "access stage: " << access_stage->ToSimpleString() << endl;
    if (access_stage->HasSourceAccess(index, resType)) {
      access_stage->RemoveSourceAccess(index, resType);
      auto type_entropy = mTypeEntropies[EResourceTypeBaseType(resType)];
      type_entropy->SourceEntropy().Decrease();
    }
    else {
      LOG(fail) << "{ResourceAccessQueue::RemoveSourceAccess} access stage :" << access_stage->ToSimpleString()
                << " not have source with regno: " << index << " regiter type: " << EResourceType_to_string(resType) << endl;
      FAIL("remove an invalid-resource-index");
    }

  }

  void ResourceAccessQueue::RemoveDestAccess(uint32 index, uint32 age, EResourceType resType)
  {
    //<< "{ResourceAccessQueue::RemoveDestAccess} current age: " << dec << mAge << " removing dest access, index=" << index << " age=" << age << " resource-type=" << EResourceType_to_string(resType) << endl;
    auto stage_dist = mAge - age;
    if (stage_dist >= mHistoryLimit) // check if it has already fallen out of bounds of history limit.
      return;

    auto access_stage = GetAccessStage(stage_dist);
    // << "access stage: " << access_stage->ToSimpleString() << endl;
    if (access_stage->HasDestAccess(index, resType)) {
      access_stage->RemoveDestAccess(index, resType);
      auto type_entropy = mTypeEntropies[EResourceTypeBaseType(resType)];
      type_entropy->DestEntropy().Decrease();
    }
    else {
      LOG(fail) << "{ResourceAccessQueue::RemoveDestAccess} access stage :" << access_stage->ToSimpleString()
                << " not have source with regno: " << index << " register type: " << EResourceType_to_string(resType) << endl;
      FAIL("remove an invalid-resource-index");
    }
  }

  const ConstraintSet* ResourceAccessQueue::GetOptimalResourceConstraint(uint32 chosenValue,const WindowLookUp& rLookUp, EResourceType resType, EDependencyType depType) const
  {
    LOG(info) << "Chosen initial value for dependency window constraint : " << dec << chosenValue << " window: " << rLookUp.Low() << "-" << rLookUp.High() << " direction: " << rLookUp.Direction() << " resource index: " << mIndex
              << " history limit: " << mHistoryLimit << " resource type: " << EResourceType_to_string(resType) << " dep type: " << EDependencyType_to_string(depType) << endl;

    uint32 window_size = rLookUp.Size();
    for (uint32 looks = 0; looks < window_size; ++ looks) {
      uint32 queue_index = GetQueueIndex(chosenValue);
      auto dep_constr = mQueue[queue_index]->GetDependenceConstraint(resType, depType);
      if (nullptr != dep_constr) {
        LOG(info) << "Chosen optimal resource slot #" << chosenValue << endl;
        return dep_constr;
      }
      rLookUp.SlideWindow(chosenValue);
    }
    LOG(info) << "Not found optimal dependence resource, returning nullptr" << endl;
    return nullptr;
  }

  const ConstraintSet* ResourceAccessQueue::GetRandomResourceConstraint(uint32 low, uint32 high, EResourceType resType, EDependencyType depType) const
  {
    LOG(info) << "Chosen random dependency constraint window: " << dec << low << "-" << high << " resource index: " << mIndex << " history limit: " << mHistoryLimit << " resource type: " << EResourceType_to_string(resType)
              << " dep type: " << EDependencyType_to_string(depType) << endl;

    mpReturnConstraint->Clear();
    mLookUpFar.SetRange(low, high); // just use lookup far since we are going through whole range.
    uint32 window_entry = low;
    uint32 window_size = mLookUpFar.Size();
    bool has_match = false;
    uint32 looks = 0;

    while (1) {
      uint32 queue_index = GetQueueIndex(window_entry);
      auto dep_constr = mQueue[queue_index]->GetDependenceConstraint(resType, depType);
      if (nullptr != dep_constr) {
        LOG(info) << "Gathered random resource slot #" << window_entry << endl;
        mpReturnConstraint->MergeConstraintSet(*dep_constr);
        has_match = true;
      }

      ++ looks;
      if (looks >= window_size) break;

      mLookUpFar.SlideWindow(window_entry);
    }

    if (has_match) {
      LOG(info) << "Chosen random dependence constraint: " << mpReturnConstraint->ToSimpleString() << endl;
      return mpReturnConstraint;
    }
    else {
      LOG(info) << "Not found random dependence resource, returning nullptr" << endl;
      return nullptr;
    }
  }

  bool ResourceAccessQueue::EntropyStable(EResourceType resType, EDependencyType depType) const
  {
    bool stable = false;
    auto type_entropy = mTypeEntropies[EResourceTypeBaseType(resType)];
    switch (depType) {
    case EDependencyType::OnSource:
      stable = type_entropy->SourceEntropy().Stable();
      break;
    case EDependencyType::OnTarget:
      stable = type_entropy->DestEntropy().Stable();
      break;
    case EDependencyType::NoDependency:
      stable = true;
      break;
     default:
       LOG(fail) << "{ResourceDependence::EntropyStable} Unimplemented register dependence value: "<<  EDependencyType_to_string(depType) << endl;
       FAIL("invalid register dependence choice value");
    }
    return stable;
  }

}
