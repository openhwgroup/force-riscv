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
#ifndef Force_GenVirtualMemoryAgent_H
#define Force_GenVirtualMemoryAgent_H

#include "Defines.h"
#include "GenAgent.h"

namespace Force {

  class GenPageRequest;
  class GenVirtualMemoryRequest;
  class MemoryTraitsManager;

  /*!
    \class GenVirtualMemoryAgent
    \brief A generator agent class to handle sequence generation.
  */
  class GenVirtualMemoryAgent : public GenAgent {
  public:
    explicit GenVirtualMemoryAgent(Generator* gen) : GenAgent(gen), mpVirtualMemoryRequest(nullptr) { } //!< Constructor with Generator pointer parameter.
    GenVirtualMemoryAgent() : GenAgent(), mpVirtualMemoryRequest(nullptr) { } //!< Constructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenVirtualMemoryAgent);
    virtual Object* Clone() const override;  //!< Return a cloned GenVirtualMemoryAgent object of the same type and content.
    const char* Type() const override { return "GenVirtualMemoryAgent"; } //!< Return type of the GenVirtualMemoryAgent object.

    EGenAgentType GenAgentType() const override { return EGenAgentType::GenVirtualMemoryAgent; } //!< Return type of the generator agent.
    void SetGenRequest(GenRequest* genRequest) override; //!< Set pointer to GenRequest object.
  protected:
    GenVirtualMemoryAgent(const GenVirtualMemoryAgent& rOther) : GenAgent(rOther), mpVirtualMemoryRequest(nullptr) { } //!< Copy constructor, do not copy the request pointer.
    void HandleRequest() override; //!< Handle GenVirtualMemoryRequest transaction.
    void CleanUpRequest() override; //!< Clean up request item.
    void ResetRequest() override; //!< Reset request item.

    void GenVA(); //!< Generate a valid virtual address.
    void GenVMVA();  //!< Generate a valid virtual address for the given VM context.
    void GenPA(); //!< Generate a valid physical address.
    void GenVAforPA(); //!< Generate a valid virtual address that translates to the given physical address.
    void GenFreePageRanges(); //!< Generate free page ranges
    virtual void GenVmContext() { }; //!< Generate a VmContext.
    void UpdateVm(); //!<  Handle Activate VM request.
    void GetPhysicalRegion(); //!< Request a physicl region.
    void HandlerMemory(); //!< Allocate handler memory.
    void ExceptionStack(); //!< Allocate exception stack.
    void ResetRegion(); //!< Allocate reset region.
    void BootRegion(); //!< Allocate boot region.
    void RandomInitializeMemory(uint64 physAddr, uint32 size, uint32 bank, bool isInstr); //!< Randomly initialize a block of memory.
  protected:
    GenVirtualMemoryRequest* mpVirtualMemoryRequest; //!< Pointer to GenVirtualMemoryRequest object.
  private:
    void SetCommonPageRequestAttributes(const GenVirtualMemoryRequest& rGenVmReq, GenPageRequest* pPageReq); //!< Set PageRequest attributes common to different types of GenVirtualMemoryRequests.
    void AddMemoryTraits(const GenVirtualMemoryRequest& rGenVmReq, cuint64 startAddr, cuint64 size, MemoryTraitsManager& rMemTraitsManager); //!< Associate memory traits from the request with the specified address range.
  };

}

#endif
