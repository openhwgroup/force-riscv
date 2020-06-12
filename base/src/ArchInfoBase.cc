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
#include <ArchInfoBase.h>
#include <Generator.h>
#include <InstructionSet.h>
#include <Register.h>
#include <GenInstructionAgent.h>
#include <GenSequenceAgent.h>
#include <GenVirtualMemoryAgent.h>
#include <GenQueryAgent.h>
#include <GenStateAgent.h>
#include <GenCallBackAgent.h>
#include <GenStateTransitionAgent.h>
#include <UtilityFunctions.h>
#include <MemoryManager.h>
#include <VirtualMemoryInitializer.h>
#include <Choices.h>
#include <ChoicesParser.h>
#include <ChoicesModerator.h>
#include <VmManager.h>
#include <PagingInfo.h>
#include <GenMode.h>
#include <GenExceptionAgent.h>
#include <Config.h>
#include <ResourceDependence.h>
#include <Variable.h>
#include <VariableParser.h>
#include <AddressFilteringRegulator.h>
#include <SimAPI.h>
#include <Log.h>

#include <dlfcn.h>

using namespace std;

namespace Force {

  ArchInfoBase::ArchInfoBase(const std::string& name)
    : ArchInfo(name)
  {

  }

  ArchInfoBase::~ArchInfoBase()
  {
    delete mpInstructionSet;
    mpInstructionSet = nullptr;
    delete mpPagingInfo;
    mpPagingInfo = nullptr;

    if (nullptr != mpSimAPI) {
      mpSimAPI->Terminate();
      delete mpSimAPI;
      mpSimAPI = nullptr;
    }

    if (nullptr != mpSimApiModule) {
      dlclose(mpSimApiModule);
      mpSimApiModule = nullptr;
    }

    for (auto choices_set : mChoicesSets) {
      delete choices_set;
    }
    for (auto variable_set : mVariableSets) {
      delete variable_set;
    }
  }

  Generator* ArchInfoBase::CreateGenerator(uint32 threadId) const
  {
    Generator* ret_gen = nullptr;
    if (nullptr == mpGeneratorTemplate) {
      if (nullptr != mpInstructionSet) {
        LOG(fail) << "Expecting instruction-set pointer to be nullptr at this point." << endl;
        FAIL("instruction-set-not-nullptr");
      }

      mpInstructionSet = new InstructionSet();
      mpInstructionSet->Setup(*this);
      mpPagingInfo = new PagingInfo();
      mpPagingInfo->Setup(*this);

      mpGeneratorTemplate = InstantiateGenerator();
      mpGeneratorTemplate->mpArchInfo = this;
      mpGeneratorTemplate->mpInstructionSet = mpInstructionSet;
      mpGeneratorTemplate->mpPagingInfo = mpPagingInfo;
      mpGeneratorTemplate->mpSimAPI = mpSimAPI;
      mpGeneratorTemplate->mpMemoryManager = MemoryManager::Instance();

      RegisterFile* register_file = InstantiateRegisterFile();
      register_file->LoadRegisterFiles(RegisterFiles());
      mpGeneratorTemplate->mpRegisterFile = register_file;

      VmManager* vm_manager = InstantiateVmManager();
      vm_manager->Initialize(VmRegimeTypes());
      mpGeneratorTemplate->mpVmManager = vm_manager;
      mpGeneratorTemplate->mpVirtualMemoryInitializer = new VirtualMemoryInitializer(vm_manager, mpGeneratorTemplate->mpRecordArchive, MemoryManager::Instance());

      mpGeneratorTemplate->mpBootOrder = InstantiateBootOrder();
      mpGeneratorTemplate->mpExceptionRecordManager = InstantiateExceptionRecordManager();
      mpGeneratorTemplate->mpConditionSet = InstantiateGenConditionSet();
      mpGeneratorTemplate->mpPageRequestRegulator = InstantiatePageRequestRegulator();
      mpGeneratorTemplate->mpAddressFilteringRegulator = InstantiateAddressFilteringRegulator();
      mpGeneratorTemplate->mpAddressTableManager = InstantiateAddressTableManager();

      EGenModeTypeBaseType gen_mode = 0;
      auto config_inst = Config::Instance();
      if (not config_inst->DoSimulate()) {
        gen_mode |= EGenModeTypeBaseType(EGenModeType::NoIss) | EGenModeTypeBaseType(EGenModeType::SimOff);
      }
      bool no_skip_valid = false;
      auto no_skip = config_inst->GetOptionValue(ESystemOptionType_to_string(ESystemOptionType::NoSkip), no_skip_valid);
      if (no_skip) {
        LOG(notice) << "System option: " << ESystemOptionType_to_string(ESystemOptionType::NoSkip) << " set." << endl;
        gen_mode |= EGenModeTypeBaseType(EGenModeType::NoSkip);
      }
      mpGeneratorTemplate->mpGenMode = new GenMode(gen_mode);

      mpGeneratorTemplate->mMaxInstructions = config_inst->MaxInstructions();
      mpGeneratorTemplate->mMaxPhysicalVectorLen = config_inst->MaxVectorLen();


      AssignGenAgents(mpGeneratorTemplate);
      ret_gen = mpGeneratorTemplate;

      ChoicesParser choices_parser(mChoicesSets);
      choices_parser.Setup(*this);
      mpGeneratorTemplate->mpChoicesModerators->Setup(mChoicesSets);

      VariableParser variable_parser(mVariableSets);
      variable_parser.Setup(*this);
      for (auto variable_set : mVariableSets) {
        VariableModerator* pModerator;
        pModerator = new VariableModerator(variable_set);
        mpGeneratorTemplate->mVariableModerators.push_back(pModerator);
      }

      AssignAddressSolutionFilters(mpGeneratorTemplate);
    } else {
      ret_gen = dynamic_cast<Generator*>(mpGeneratorTemplate->Clone());
    }
    ret_gen->Setup(threadId);

    return ret_gen;
  }

  list<EMemBankType> ArchInfoBase::MemoryBankTypes() const
  {
    list<EMemBankType> membank_types;
    for (EMemBankTypeBaseType i = 0; i < EMemBankTypeSize; ++ i) {
      membank_types.push_back(EMemBankType(i));
    }

    return membank_types;
  }

  list<EVmRegimeType> ArchInfoBase::VmRegimeTypes() const
  {
    list<EVmRegimeType> regime_types;
    for (EVmRegimeTypeBaseType i = 0; i < EVmRegimeTypeSize; ++ i) {
      regime_types.push_back(EVmRegimeType(i));
    }

    return regime_types;
  }

  GenAgent* ArchInfoBase::InstantiateGenAgent(EGenAgentType agentType) const
  {
    switch (agentType) {
    case EGenAgentType::GenInstructionAgent:
      return new GenInstructionAgent();
    case EGenAgentType::GenSequenceAgent:
      return new GenSequenceAgent();
    case EGenAgentType::GenVirtualMemoryAgent:
      return new GenVirtualMemoryAgent();
    case EGenAgentType::GenQueryAgent:
      return new GenQueryAgent();
    case EGenAgentType::GenStateAgent:
      return new GenStateAgent();
    case EGenAgentType::GenExceptionAgent:
      return new GenExceptionAgent();
    case EGenAgentType::GenCallBackAgent:
      return new GenCallBackAgent();
    case EGenAgentType::GenStateTransitionAgent:
      return new GenStateTransitionAgent();
    default:
      LOG(fail) << "{ArchInfoBase::InstantiateGenAgent} unsupported GenAgent type: " << EGenAgentType_to_string(agentType) << endl;
      FAIL("unsupported-gen-agent-type");
    }

    return nullptr;
  }

  void ArchInfoBase::AssignGenAgents(Generator* pGen) const
  {
    check_enum_size(EGenAgentTypeSize);
    for (EGenAgentTypeBaseType i = 0; i < EGenAgentTypeSize; ++ i) {
      EGenAgentType agent_type = EGenAgentType(i);
      auto gen_agent = InstantiateGenAgent(agent_type);
      gen_agent->SetGenerator(pGen);
      pGen->mAgents.push_back(gen_agent);
    }
  }

  AddressSolutionFilter* ArchInfoBase::InstantiateAddressSolutionFilter(EAddressSolutionFilterType filterType) const
  {
    return nullptr;
  }

  void ArchInfoBase::AssignAddressSolutionFilters(Generator* pGen) const
  {
    check_enum_size(EAddressSolutionFilterTypeSize);
    for (EAddressSolutionFilterTypeBaseType i = 0; i < EAddressSolutionFilterTypeSize; ++ i) {
      EAddressSolutionFilterType filter_type = EAddressSolutionFilterType(i);
      auto addr_filter = InstantiateAddressSolutionFilter(filter_type);
      if (nullptr == addr_filter) continue; // TODO, temporary, shouldn't getting inapplicable options in the first place.
      pGen->mpAddressFilteringRegulator->mAddressSolutionFilters.push_back(addr_filter);
    }
  }

  void ArchInfoBase::SetupSimAPIs()
  {
    auto config_ptr = Config::Instance();
    if (config_ptr->DoSimulate()) {
      string api_module_name =  config_ptr->LookUpFile(SimulatorApiModule());
      string sim_so_file = config_ptr->LookUpFile(SimulatorDLL());

      LOG(notice) << "Simulator API module: '" << api_module_name << "'." << endl;
      LOG(notice) << "Simulator DLL: '" << sim_so_file << "'." << endl;

      mpSimApiModule = dlopen(api_module_name.c_str(), RTLD_NOW | RTLD_GLOBAL);
      char *err_msg = dlerror();
      if (err_msg != NULL) {
	LOG(fail) << "Problems loading simulator API module. Error msg from dlopen: " <<  err_msg << endl;
	FAIL("failed-loading-api-module");
      }

      SimAPI* (*instantiate_api_ptr)(void) = (SimAPI* (*)(void)) dlsym(mpSimApiModule, "instantiate_api");
      err_msg = dlerror();
      if (err_msg != NULL) {
	LOG(fail) << "Problems loading instantiation function. Error msg from dlsym: " <<  err_msg << endl;
	FAIL("failed-loading-instantiation-function");
      }

      mpSimAPI = instantiate_api_ptr();

      auto pa_size =  get_mask64_size(config_ptr->LimitValue(ELimitType::PhysicalAddressLimit));

      ApiSimConfig sim_dll_cfg(config_ptr->NumChips(), /* # of clusters */
                               config_ptr->NumCores(), /* # cores */
                               config_ptr->NumThreads(), /* # threads */
                               pa_size, /* physical address size */
                               "./sim.log", /* ignore for now */
                               true
                               );

      mpSimAPI->InitializeIss(sim_dll_cfg, sim_so_file, config_ptr->IssApiTraceFile());
    }

    LOG(notice) << "Simulator Standalone: '" << config_ptr->LookUpFile(SimulatorStandalone()) << "'." << endl;
  }

}
