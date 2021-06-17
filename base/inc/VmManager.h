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
#ifndef Force_VmManager_H
#define Force_VmManager_H

#include <Defines.h>
#include <Object.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>
#include <list>
#include <map>

namespace Force {

  class VmMapper;
  class VmRegime;
  class Generator;
  class VmInfo;
  class VmFactory;
  class VmContext;
  class VmContextInfo;

  /*!
    \class VmManager
    \brief Virtual memory manager class.
  */

  class VmManager : public Object {
  public:
    Object * Clone() const override { return new VmManager(*this); } //!< Clone VmManager object.
    const std::string ToString() const override { return Type(); } //!< Return a string describing the VmManager object.
    const char* Type() const override { return "VmManager"; }

    VmManager() : Object(), mpCurrentRegime(nullptr), mpCurrentMapper(nullptr), mpGenerator(nullptr), mpVmInfo(nullptr), mVmRegimes(), mVmContextInfoCache(), mVmFactories() { } //!< Default constructor.
    ~VmManager(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(VmManager);

    void Initialize(const std::list<EVmRegimeType>& regimeTypes); //!< Initialize the VmManager object.
    void Setup(Generator* gen); //!< Setup VmManager interinals.
    VmMapper* CurrentVmMapper() const { return mpCurrentMapper; } //!< Return pointer to the current virtual memory mapper.
    VmRegime* CurrentVmRegime() const { return mpCurrentRegime; } //!< Return pointer to the current virtual memory translation regime.
    uint32 GenContextIdRequest(EVmRegimeType regimeType, const std::map<std::string, uint64> & rRawContextParams);//!< Request generator Context ID, new VM context could be created.
    VmMapper* FindVmMapper(const VmContext& rVmContext, EVmRegimeType regimeType) const; //!< Find if there is a VmMapper match the VmContext object passed in.
    bool GetVmContextDelta(std::map<std::string, uint64> & rDeltaMap, uint32 contextId) const;//!< Find the delta map between contextId and currect default context.
    uint32 GetVmCurrentContext() const;//!< Get Current VM Context.
    virtual void Update(); //!< Called to update VmManager.
    void UpdateVmContext(EVmRegimeType regimeType, uint32 contextId); //!< Switch the address space for the specified regime.

    VmMapper* GetVmMapper(const VmInfo& rVmInfo) const; //!< Get VmMapper by using VmInfo for lookup.
    virtual VmInfo* VmInfoInstance() const { return nullptr; } //!< Return a VmInfo instance.
    void DumpPage() const; //!< dump pages on Vm regimes
    void DumpPageAndMemoryAttributesJson() const; //!< dump pages and memory attributes on Vm regimes in JSON format
  protected:
    VmManager(const VmManager& rOther); //!< Copy constructor.
    virtual VmFactory* VmFactoryInstance(EVmRegimeType vmRegimeType) const { return nullptr; } //!< Return an VmFactory object based on the VM architecture type.
    const VmFactory* GetVmFactory(EVmRegimeType vmRegimeType) const; //!< Return a VmFactory object.
    VmRegime* GetVmRegime(EVmRegimeType regimeType) const; //!< Return a VmRegime object.
    const VmContextInfo* GetVmContextInfo(uint32 contextId) const; //!< Retrieve VmContextInfo with the specified context ID.
    uint32 UpdateVmMapperCache(EVmRegimeType regimeType, VmMapper* pVmMapper); //!< add VmMapper to the cache.
  protected:
    VmRegime* mpCurrentRegime; //!< Pointer to the current virtual memory regime.
    VmMapper* mpCurrentMapper; //!< Pointer to the current virtual memory mapper.
    Generator* mpGenerator; //!< Pointer to generator object.
    VmInfo* mpVmInfo; //!< VmInfo object used for querying.
    std::vector<VmRegime* > mVmRegimes; //!< Container for all VmRegime objects in the virtual memory system.
    std::map<uint32, const VmContextInfo* > mVmContextInfoCache; //!< Stores information to help lookup all VmContext objects.
  private:
    mutable std::map<EVmRegimeType, VmFactory*> mVmFactories; //!< Container of all VmFactories.  Not to be copied.
  };
};

#endif
