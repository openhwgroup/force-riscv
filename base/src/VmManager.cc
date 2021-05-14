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
#include <VmManager.h>
#include <VmMapper.h>
#include <VmInfo.h>
#include <VmFactory.h>
#include <VmContextParameter.h>
#include <Log.h>

#include <fstream>
#include <memory>

using namespace std;

/*!
  \file VmManager.cc
  \brief Code for VmManager base class.
*/

namespace Force {

  /*!
    \class VmContextInfo
    \brief VmContextInfo to lookup VmContext.
  */

  class VmContextInfo {
  public:
    VmContextInfo(uint32 contextId, EVmRegimeType regimeType, VmMapper* pMapper) //!< Constructor.
      : mGenContextId(contextId), mRegimeType(regimeType), mpMapper(pMapper)
    {
    }

    uint32 GenContextId() const { return mGenContextId; } //!< Return generator context ID.
    EVmRegimeType RegimeType() const { return mRegimeType; } //!< Return regime type.
    VmMapper* GetVmMapper() const { return mpMapper; } //!< Return VmMapper.

    DEFAULT_CONSTRUCTOR_ABSENT(VmContextInfo);
    COPY_CONSTRUCTOR_ABSENT(VmContextInfo);
    ASSIGNMENT_OPERATOR_ABSENT(VmContextInfo);
  private:
    uint32 mGenContextId;  //!< Generator context ID for the VmMapper.
    EVmRegimeType mRegimeType; //!< Associated VmRegime type.
    VmMapper* mpMapper; //!< Pointer to VmMapper that holds the context ID.
  };


  VmManager::VmManager(const VmManager& rOther)
    : Object(rOther), mpCurrentRegime(nullptr), mpCurrentMapper(nullptr), mpGenerator(nullptr), mpVmInfo(nullptr), mVmRegimes(), mVmContextInfoCache(), mVmFactories()
  {
    for (auto const mapper_ptr : rOther.mVmRegimes) {
      if (nullptr != mapper_ptr) {
        mVmRegimes.push_back(dynamic_cast<VmRegime* >(mapper_ptr->Clone()));
      }
      else {
        mVmRegimes.push_back(nullptr);
      }
    }
  }

  VmManager::~VmManager()
  {
    delete mpVmInfo;
    mpVmInfo = nullptr;

    for (auto mapper_ptr : mVmRegimes) {
      delete mapper_ptr;
    }

    for (auto cinfo_item: mVmContextInfoCache) {
      delete (cinfo_item.second);
    }

    for (auto factory_iter : mVmFactories) {
      delete (factory_iter.second);
    }
  }

  VmRegime* VmManager::GetVmRegime(EVmRegimeType regimeType) const
  {
    auto regime_ptr = mVmRegimes.at(EVmRegimeTypeBaseType(regimeType));
    if (nullptr == regime_ptr) {
      LOG(fail) << "{VmManager::GetVmRegime} VmRegime: " << EVmRegimeType_to_string(regimeType) << " is not initialized." << endl;
      FAIL("vm-regime-not-initialized");
    }
    return regime_ptr;
  }

  const VmFactory* VmManager::GetVmFactory(EVmRegimeType vmRegimeType) const
  {
    auto factory_finder = mVmFactories.find(vmRegimeType);
    if (factory_finder != mVmFactories.end()) {
      return factory_finder->second;
    }

    auto new_factory = VmFactoryInstance(vmRegimeType);
    mVmFactories[vmRegimeType] = new_factory;
    return new_factory;
  }

  void VmManager::Initialize(const list<EVmRegimeType>& regimeTypes)
  {
    if (regimeTypes.size() == 0) {
      LOG(fail) << "{VmManager::Initialize} empty regime types list." << endl;
      FAIL("empty-regime-types-list");
    }

    mVmRegimes.assign(EVmRegimeTypeSize, nullptr);

    for (auto regime_type : regimeTypes) {
      const VmFactory* vm_factory = GetVmFactory(regime_type);
      auto vm_regime = vm_factory->VmRegimeInstance();
      mVmRegimes[int(regime_type)] = vm_regime;
    }

  }

  void VmManager::Setup(Generator* gen)
  {
    for (auto regime_ptr : mVmRegimes) {
      if (nullptr != regime_ptr) {
        regime_ptr->Setup(gen);
      }
    }

    mpCurrentRegime = mVmRegimes.front();
    mpCurrentMapper = mpCurrentRegime->CurrentVmMapper();
    mpGenerator = gen;
    mpVmInfo = VmInfoInstance();
  }

  void VmManager::Update()
  {
    mpVmInfo->Clear();
    mpVmInfo->GetCurrentStates(*mpGenerator);
    EVmRegimeType curr_regime_type = mpVmInfo->RegimeType();

    VmRegime* new_regime = GetVmRegime(curr_regime_type);

    if (new_regime != mpCurrentRegime)
    {
      if (nullptr != mpCurrentRegime)
      {
        LOG(notice) << "{VmManager::Update} deactivating current regime type=" << mpCurrentRegime->ToString() << endl;
        mpCurrentRegime->Deactivate();
      }
      mpCurrentRegime = new_regime;
    }

    if (nullptr == mpCurrentRegime) {
      LOG(fail) << "{VmManager::Update} current translation regime not set, current regime type is: " << EVmRegimeType_to_string(curr_regime_type) << endl;
      FAIL("null-current-translation-regime");
    }

    LOG(notice) << "VM states: " << mpVmInfo->ToString() << " Current Regime: " << mpCurrentRegime->ToString() << " Regime Type:" << EVmRegimeType_to_string(curr_regime_type) << endl;
    mpCurrentRegime->Activate();
    mpCurrentMapper = mpCurrentRegime->CurrentVmMapper();
  }

  VmMapper* VmManager::GetVmMapper(const VmInfo& rVmInfo) const
  {
    EVmRegimeType regime_type = rVmInfo.RegimeType();
    VmRegime* target_regime = GetVmRegime(regime_type);
    if (nullptr == target_regime) {
      LOG(fail) << "{VmManager::GetVmMapper} VM regime not found for: " << EVmRegimeType_to_string(regime_type) << endl;
      FAIL("regime-type-not-found");
    }
    return target_regime->GetVmMapper(rVmInfo);
  }

  const VmContextInfo* VmManager::GetVmContextInfo(uint32 contextId) const
  {
    auto find_iter = mVmContextInfoCache.find(contextId);
    if (find_iter != mVmContextInfoCache.end()) {
      return find_iter->second;
    }
    return nullptr;
  }

  void VmManager::DumpPage(const EDumpFormat dumpFormat) const
  {
    ofstream os;

    string file_ext;
    switch (dumpFormat) {
    case EDumpFormat::Text:
      file_ext = ".txt";
      break;
    case EDumpFormat::JSON:
      file_ext = ".json";
      break;
    default:
      LOG(fail) << "{VmManager::DumpPage} Unknown dump format " << EDumpFormat_to_string(dumpFormat) << endl;
      FAIL("unknown-dump-format");
    }

    for (auto regime : mVmRegimes) {
      auto regime_type = regime->VmRegimeType();
      string file_name = "Pages" + EVmRegimeType_to_string(regime_type) + file_ext;
      os.open(file_name);
      if (os.bad()) {
        LOG(fail) << "Can't open file " << file_name << std::endl;
        FAIL("Can't open file");
      }
      regime->DumpPage(dumpFormat, os);
      os.close();
    }
  }

  // generate a new VmMapper based on users rawContextParams input
  uint32 VmManager::GenContextIdRequest(EVmRegimeType regimeType, const std::map<std::string, uint64> & rRawContextParams)
  {
    //1. use factory to create tmpVmContextParameters based on regime
    //2. compare and find if VmcontextParams exist
    //3. if not exist, create new address space, add to cache, delete tmpVmContextParameters and return id

    if (!mpVmInfo->PagingEnabled()) {
      FAIL("Paging-not-enabled");
    }
    const VmFactory* factory = GetVmFactory(regimeType);
    std::unique_ptr<VmContext> context_ptr_storage(factory->CreateVmContext());
    VmContext* tmp_context = context_ptr_storage.get();

    tmp_context->InitializeContext(mpGenerator);
    tmp_context->UpdateContextParams(rRawContextParams);

    // find if contextparams exist
    auto vm_mapper = FindVmMapper(*tmp_context, regimeType);

    if (nullptr == vm_mapper) {
      // Couldn't find existing VmContext that matches the requirement.
      // Get the target regime, create VmMapper, get the ControlBlock, change its Context, add to the cache

      VmRegime* target_regime = GetVmRegime(regimeType);
      vm_mapper = target_regime->CreateVmMapper(tmp_context);
    }

    uint32 context_id = UpdateVmMapperCache(regimeType, vm_mapper);
    return context_id;
  }

  VmMapper* VmManager::FindVmMapper(const VmContext& rVmContext, EVmRegimeType regimeType) const
  {
    auto regime_obj = GetVmRegime(regimeType);
    return regime_obj->FindVmMapper(rVmContext);
  }

  // input: contextId
  // output: a deltaMap of context parameters contextId vs current default
  bool VmManager::GetVmContextDelta(std::map<std::string, uint64> & rDeltaMap,  uint32 contextId) const
  {
    if (!mpVmInfo->PagingEnabled()) {
      FAIL("Paging-not-enabled");
    }

    auto context_info = GetVmContextInfo(contextId);
    if (nullptr == context_info) {
      LOG(fail) << "{VmManager::GetVmContextDelta} Can not find VmContextInfo." << endl;
      FAIL("can-not-find-vm-context-info");
    }

    context_info->GetVmMapper()->GetVmContextDelta(rDeltaMap);
    return true;
  }

  uint32 VmManager::UpdateVmMapperCache(EVmRegimeType regimeType, VmMapper* pVmMapper)
  {
    uint32 context_id = pVmMapper->GenContextId();
    auto existing_context_info = GetVmContextInfo(context_id);
    if (nullptr != existing_context_info) {
      // Cache entry exist, check if things match up correctly.
      if (existing_context_info->RegimeType() != regimeType) {
        LOG(fail) << "{VmManager::UpdateVmMapperCache} regimeType does not match" << std::endl;
        FAIL("regime-type-does-not-match");
      }
      if (existing_context_info->GetVmMapper() != pVmMapper) {
        LOG(fail) << "{VmManager::UpdateVmMapperCache} VmMapper instance does not match" << std::endl;
        FAIL("vm-mapper-instance-does-not-match");
      }
    }
    else {
      // Add new cache entry.
      mVmContextInfoCache[context_id] = new VmContextInfo(context_id, regimeType, pVmMapper);
    }
    return context_id;
  }

  // input: None
  // output: Current context.
  uint32 VmManager::GetVmCurrentContext() const
  {
    return CurrentVmMapper()->GenContextId();
  }

}
