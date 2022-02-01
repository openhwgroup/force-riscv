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
#ifndef Force_Architectures_H
#define Force_Architectures_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <string>
#include <vector>
#include <list>
#include <map>

namespace Force {

  class Generator;
  class InstructionSet;
  class ChoicesSet;
  class VmManager;
  class GenAgent;
  class PagingInfo;
  class RegisterFile;
  class BootOrder;
  class VariableSet;
  class ExceptionRecordManager;
  class GenConditionSet;
  class PhysicalPageManager;
  class MemoryTraitsManager;
  class MemoryTraitsRegistry;
  class PageRequestRegulator;
  class AddressFilteringRegulator;
  class AddressSolutionFilter;
  class AddressTableManager;
  class SimAPI;

  /*!
    \class ArchInfo
    \brief Starting point to create Architectural specific generator threads.  This is a virtual base class.
   */

  class ArchInfo {
  public:
    virtual ~ArchInfo(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(ArchInfo);
    COPY_CONSTRUCTOR_ABSENT(ArchInfo);
    const std::string& Name() const { return mName; } //!< Return architecture name.
    virtual const char* DefaultConfigFile() const { return "config/force.config"; } //!< Return the default config file name
    virtual Generator* CreateGenerator(uint32 threadId) const { return nullptr; } //!< Create a generator object based on the ArchInfo specification.
    virtual void Setup() {} //!< Setup necessary details for the ArchInfo object.
    virtual std::list<EMemBankType> MemoryBankTypes() const { return std::list<EMemBankType>(); } //!< Return all memory bank types.
    virtual std::list<EVmRegimeType> VmRegimeTypes() const { return std::list<EVmRegimeType>(); } //!< Return all virtual memory regime types.
    void AddInstructionFile(const std::string& iFile); //!< Add an instruction file name.
    void AddRegisterFile(const std::string& iFile); //!< Add a register file name.
    void AddChoicesFile(const std::string& iFile); //!< Add a choices file name.
    void AddPagingFile(const std::string& iFile); //!< Add a paging file name.
    void AddVariableFile(const std::string& iFile); //!< Add a variable file name.
    void SetSimulatorApiModule(const std::string& iFile) { mSimulatorApiModule = iFile; } //!< Set path the simulator API module.
    void SetSimulatorDLL(const std::string& iFile) { mSimulatorDLL = iFile; }; //!< Set path to simulator shared object.
    void SetSimulatorStandalone(const std::string& iFile) { mSimulatorStandalone = iFile; }; //!< set path to simulator standalone.
    void SetSimulatorConfigString(const std::string& iCfgStr) { mSimulatorConfigString = iCfgStr; }; //!< set simulator ISA config string
    const std::list<std::string>& InstructionFiles() const { return mInstructionFiles; } //!< Obtain a read-only reference to the instruction file names.
    const std::list<std::string>& RegisterFiles() const { return mRegisterFiles; } //!< Obtain a read-only reference to the register file names.
    const std::list<std::string>& ChoicesFiles() const { return mChoicesFiles; } //!< Obtain a read-only reference to the choices file names.
    const std::list<std::string>& PagingFiles() const { return mPagingFiles; } //!< Obtain a read-only reference to the paging file names.
    const std::list<std::string>& VariableFiles() const { return mVariableFiles; } //!< Obtain a read-only reference to the variable file names
    const std::string& SimulatorApiModule() const { return mSimulatorApiModule; } //!< Obtain a read-only reference to the simulator Api module name.
    const std::string& SimulatorDLL() const { return mSimulatorDLL; } //!< Obtain a read-only reference to the simulator shared object file.
    const std::string& SimulatorStandalone() const { return mSimulatorStandalone; } //!< Obtain a read-only reference to the simulator standalone
    const std::string& SimulatorConfigString() const { return mSimulatorConfigString; } //!< Obtain a read-only reference to the simulator config string
    const std::string& DefaultInstructionClass() const { return mDefaultIClass; } //!< Return default instruction class.
    const std::string& DefaultPteClass() const { return mDefaultPteClass; } //!< Return default PTE class.
    const std::string& DefaultPteAttributeClass() const { return mDefaultPteAttributeClass; } //!< Return default PTE attribute class.
    const std::string DefaultOperandClass(const std::string& operandType) const; //!< Return default operand class, lookup by type.
    const std::vector<EMemBankType>& GetMemoryBanks() const { return mMemoryBanks; } //!< Return supported memory banks.
    virtual uint32 ElfMachineType() const { return 0; } //!< Return ELF machine type.
    virtual const std::string FindOperandShortName(const std::string& longName) const { return ""; } //!< return operand short name
    virtual PhysicalPageManager* InstantiatePhysicalPageManager(EMemBankType bankType, MemoryTraitsManager* pMemTraitsManager) const { return nullptr; } //!< Instantiate a PhysicalPageManager object based on the ArchInfo type.
    virtual MemoryTraitsRegistry* InstantiateMemoryTraitsRegistry() const { return nullptr; } //!< Instantiate a MemoryTraitsRegistry object based on the ArchInfo type.
    virtual void SetupSimAPIs() { } //!< Setup simulator APIs.
  protected:
    explicit ArchInfo(const std::string& name);
    virtual Generator* InstantiateGenerator() const { return nullptr; } //!< Instantiate a generator object based on the ArchInfo type.
    virtual VmManager* InstantiateVmManager() const { return nullptr; } //!< Instantiate a VmManager object based on the ArchInfo type.
    virtual GenAgent* InstantiateGenAgent(EGenAgentType agentType) const { return nullptr; } //!< Instantiate a GenAgent object based on the ArchInfo type and the passed in agentType parameter.
    virtual BootOrder* InstantiateBootOrder() const { return nullptr; } //<! Instantiate a BootOrder object based on the ArchInfo type.
    virtual RegisterFile* InstantiateRegisterFile() const { return nullptr; } //!< Instantiate a RegisterFile object based on the ArchInfo type.
    virtual ExceptionRecordManager* InstantiateExceptionRecordManager() const { return nullptr; } //!< Instantiate a ExceptionRecordManager based on the ArchInfo type for this genthread.
    virtual GenConditionSet* InstantiateGenConditionSet() const { return nullptr; } //!< Instantiate a GenConditionSet object based on the ArchInfo type.
    virtual PageRequestRegulator* InstantiatePageRequestRegulator() const { return nullptr; } //!< Instantiate a PageRequestRegulator object based on the ArchInfo type.
    virtual AddressFilteringRegulator* InstantiateAddressFilteringRegulator() const { return nullptr; } //!< Instantiate a AddressFilteringRegulator object based on the ArchInfo type.
    virtual AddressSolutionFilter* InstantiateAddressSolutionFilter(EAddressSolutionFilterType filterType) const { return nullptr; } //!< Instantiate a AddressSolutionFilter object based on the ArchInfo type and the passed in filterType parameter.
    virtual AddressTableManager* InstantiateAddressTableManager() const { return nullptr; } //!< Instantiate a AddressTableManager object based on the ArchInfo type.
    virtual void AssignGenAgents(Generator* gen) const { } //!< Assign GenAgents to the Generator.
    virtual void AssignAddressSolutionFilters(Generator* pGen) const { } //!< Assign AddressSolutionFilters to the Generator.
  protected:
    std::string mName; //!< Architecture name
    mutable Generator* mpGeneratorTemplate; //!< Pointer to the first Generator of this architecture created, used as template to create subsequent generators;
    mutable InstructionSet* mpInstructionSet; //!< Pointer to the instruction-set container shared by all Generators of the same type.
    mutable PagingInfo* mpPagingInfo; //!< Pointer to the paging info container shared by all Generators of the same type.
    SimAPI* mpSimAPI; //!< Pointer to the SimAPI object shared by all Generators of the same type.
    void* mpSimApiModule; //!< Pointer to the Simulator API module shared libary.
    mutable std::vector<ChoicesSet* > mChoicesSets; //!< Vector of pointers to ChoicesSet containers shared by all Generators of the same type.
    mutable std::vector<VariableSet* > mVariableSets; //!< Vector of pointers to VariableSet containers shared by all Generators of the same type.
    std::vector<EMemBankType> mMemoryBanks; //!< Supported memory banks.
    std::list<std::string> mInstructionFiles; //!< List of instruction files.
    std::list<std::string> mRegisterFiles; //!< List of register files.
    std::list<std::string> mChoicesFiles; //!< List of choices files.
    std::list<std::string> mPagingFiles; //!< List of pagings files.
    std::list<std::string> mVariableFiles; //!< List of variable files.
    std::string mDefaultIClass; //!< Default instruction class name.
    std::string mDefaultPteClass; //!< Default PTE class name.
    std::string mDefaultPteAttributeClass; //!< Default PTE attribute class name.
    std::map<std::string, std::string> mDefaultOperandClasses; //!< Default operand class names, lookup by type.
    std::map<std::string, std::string> mRegisterClasses; //!< Register class names, lookup by type.
    std::string mSimulatorApiModule; //!< Simulator API module.
    std::string mSimulatorDLL; //!< Simulator dynamically loaded library.
    std::string mSimulatorStandalone; //!< Simulator standalone executable.
    std::string mSimulatorConfigString; //!< Simulator ISA config string
    friend class ConfigParser;
  };

  /*!
    \class Architectures
    \brief Act as architectural look up entry, potentially support multiple architectures in one test template
  */
  class Architectures {
  public:
    static void Initialize();  //!< Initialization interface.
    static void Destroy();     //!< Destruction clean up interface.
    inline static Architectures* Instance() { return mspArchitectures; } //!< Access Architectures instance.

    void AssignArchInfoObjects(std::vector<ArchInfo*>& archInfoObjs);
    const ArchInfo* DefaultArchInfo() const { return mpDefaultArchInfo; } //!< Return default ArchInfo object.
    ArchInfo* DefaultArchInfo() { return mpDefaultArchInfo; } //!< Return mutable default ArchInfo object.
    void SetupSimAPIs(); //!< Setup various APIs for simulators.
  private:
    Architectures();  //!< Constructor, private.
    ~Architectures(); //!< Destructor, private.
    ASSIGNMENT_OPERATOR_ABSENT(Architectures);
    COPY_CONSTRUCTOR_ABSENT(Architectures);
  private:
    static Architectures* mspArchitectures; //!< Pointer to singleton Architectures object
    std::map<std::string, ArchInfo*> mArchInfoObjects; //!< Container for all ArchInfo objects, use map to facilitate lookup.
    ArchInfo* mpDefaultArchInfo; //!< Pointer to default ArchInfo object.
  };

}

#endif
