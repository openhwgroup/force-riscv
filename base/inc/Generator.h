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
#ifndef Force_Generator_H
#define Force_Generator_H

#include <Defines.h>
#include <Object.h>
#include <VmUtils.h>

#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>
#include <map>

namespace Force {

  class ArchInfo;
  class InstructionSet;
  class PagingInfo;
  class GenRequestQueue;
  class GenRequest;
  class GenAgent;
  class Instruction;
  class ThreadInstructionResults;
  class RecordArchive;
  class MemoryInitRecord;
  class MemoryManager;
  class VirtualMemoryInitializer;
  class RegisterFile;
  class ChoicesModerator;
  class ChoicesModerators;
  class VmManager;
  class GenInstructionRequest;
  class BootOrder;
  class GenPageRequest;
  class GenQuery;
  class GenMode;
  class MemoryReservation;
  class Register;
  class GenPC;
  class ReExecutionManager;
  class TranslationRange;
  class ResourceDependence;
  class VariableModerator;
  class InstructionStructure;
  class RegisteredSetModifier;
  class ExceptionRecordManager;
  class GenConditionSet;
  class PageRequestRegulator;
  class AddressFilteringRegulator;
  class ImageIO;
  class VmMapper;
  class ConditionFlags;
  class BntHookManager;
  class BntNodeManager;
  class AddressTableManager;
  class SimAPI;

  /*!
    \class Generator
    \brief Generator class, main control hub of a test generator thread.
   */

  class Generator : public Object {
  public:
    Generator(); //!< Default constructor.
    explicit Generator(uint64 alignmentMask); //!< Generator constructor with PC alignment mask argument.
    Generator(const Generator& rOther); //!< Copy constructor.
    ~Generator(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(Generator);

    Object* Clone() const override = 0; //!< Clone a object of the same type, need to be implemented by derived classes.
    const std::string ToString() const override; //!< Return a string describing the current state of the Generator object.
    const char* Type() const override { return "Generator"; } //!< Return object type in a C string.

    virtual void Setup(uint32 threadId); //!< Setup generator.
    uint32 ThreadId() const { return mThreadId; }; //!< Return current thread

    const ArchInfo* GetArchInfo() const { return mpArchInfo; } //!< Return a const pointer to ArchInfo object.
    const InstructionSet* GetInstructionSet() const { return mpInstructionSet; } //!< Return a const pointer to InstructionSet object.
    const PagingInfo* GetPagingInfo() const { return mpPagingInfo; }
    RecordArchive* GetRecordArchive() const { return mpRecordArchive; } //!< Return a pointer to RecordArchive object.
    MemoryManager* GetMemoryManager() const { return mpMemoryManager; } //!< Return a pointer to MemoryManager object.
    VirtualMemoryInitializer* GetVirtualMemoryInitializer() const { return mpVirtualMemoryInitializer; } //!< Return a pointer to VirtualMemoryInitializer object.
    ThreadInstructionResults* GetInstructionResults() const { return mpThreadInstructionResults; } //!< Return a const pointer to InstructionResults object.
    ChoicesModerators* GetChoicesModerators() const { return mpChoicesModerators; } //!< Return a pointer to choices moderators
    ChoicesModerator* GetChoicesModerator(EChoicesType choiceType) const; //!< Return a const pointer to a ChoicesModerator of the specified type.
    VariableModerator* GetVariableModerator(EVariableType variableType) const { return mVariableModerators[int(variableType)]; } //!< Return a pointer to VariableModerator of the specific type
    const RegisterFile* GetRegisterFile() const { return mpRegisterFile; } //!< Return a const pointer to RegisterFile object.
    ExceptionRecordManager* GetExceptionRecordManager() { return mpExceptionRecordManager; } //!< Return a const pointer to the ExceptionRecordManager object.
    VmManager* GetVmManager() const { return mpVmManager; } //!< Return pointer to VmManager object.
    BootOrder* GetBootOrder() const { return mpBootOrder; } //!< Return pointer to BootOrder object.
    GenMode* GetGenMode() const { return mpGenMode; } //!< Return pointer to GenMode object.
    GenPC* GetGenPC() const { return mpGenPC; } //!< Return pointer to GenPC object.
    ReExecutionManager* GetReExecutionManager() const { return mpReExecutionManager; } //!< Return pointer to ReExecutionManager object.
    ResourceDependence* GetDependenceInstance() const { return mpDependence; } //!< Return pointer to ResourceInterDependence object.
    void SetDependenceInstance(ResourceDependence* pDependence); //!< Set dependence instance.
    const PageRequestRegulator* GetPageRequestRegulator() const { return mpPageRequestRegulator; } //!< Return pointer to PageRequestRegulator object.
    const AddressFilteringRegulator* GetAddressFilteringRegulator() const { return mpAddressFilteringRegulator; } //!< Return pointer to AddressFilteringRegulator object.
    bool IsReturningToUser(PaTuple &returnAddress); //!< Returns true if the target of the current ERET is user code.
    void SetHandlerBounds(uint64 bank_number, std::vector<std::pair<uint64, uint64>> &bounds); //!< Inform the generator of the exception boundaries

    bool CommitInstruction(Instruction* instr, GenInstructionRequest* instrReq); //!< Commit generated instruction.
    void CommitInstructionFinal(Instruction* instr, GenInstructionRequest* instrReq); //!< Commit generated instruction final step.
    void CommitQueuedInstruction(Instruction* instr); //!< Commit generated and queued instruction.
    void StepInstruction(Instruction* instr); //!< Simulate generated and committed instruction
    uint64 PC() const; //!< Return the current PC value.
    uint64 LastPC() const; //!< Return the last PC value.
    void SetPC(uint64 newPC); //!< Set PC to new value.
    virtual void AdvancePC(); //!< Advance Generator PC by default value
    void AdvancePC(uint32 bytes); //!< Advance generator PC.
    void InitializeMemory(const MemoryInitRecord* memInitRecord); //!< Initialize memory.
    uint64 ReloadValue(const std::string& name, const std::string& field) const;   //!<  Get the reload value of this register or field.
    void PrependRequest(GenRequest* req); //!< Prepend a single request to the beginning of the request queue.
    void PrependRequests(std::vector<GenRequest* >& reqVec); //!< Prepend a vector of requests to the beginning of the request queue.
    bool VerifyVirtualAddress(uint64 va, uint64 size, bool isInstr) const; //!< Verify if the virtual address range is usable.
    uint64 GetRegisterFieldValue(const std::string& regName, const std::string& fieldName); //!< Obtain register field value, initialize if necessary.
    uint64 RegisterFieldReloadValue(const std::string& regName, const std::string& fieldName) const; //!< reload register field value
    void SetupPageTableRegions(); //!< Setup page table regions.
    void InitializeMemoryWithEndian(uint64 addr, uint32 bank, uint32 size, uint64 data, bool isInstr, bool bigEndian); //!< Initialize memory with endianness specified.
    virtual bool IsInstructionBigEndian() const { return false; } //!< Return whether instruction is big endian.
    virtual bool IsDataBigEndian() const { return false; } //!< Return whether data is big endian.
    virtual uint32 GetExceptionSubCode() const { return 0; } //!< Return sub-code of Exception if any.
    virtual uint64 GetMaxVectorLen() const { return 0; } //!< Return maximum vector register length allowed.
    virtual uint32 InstructionAlignment() const { return 4; } //!< Return instruction alignment.
    virtual uint32 DefaultInstructionSize() const { return 4; } //!< Return default instruction size.
    virtual uint32 InstructionSpace() const = 0; //!< Return necessary space between instruction streams.
    virtual uint32 BntReserveSpace() const = 0; //!< Return the number of memory bytes to reserve for a BNT path.
    virtual uint32 BntMinSpace() const = 0; //!< Return the minimum number of memory bytes to generate a BNT path.
    virtual uint32 SpAlignment() const { return 16; } //!< Return stack pointer alignment.
    virtual GenPageRequest* GenPageRequestInstance(bool isInstr=true, EMemAccessType memAccessType=EMemAccessType::Unknown) const; //!< Return a GenPageRequest instance.
    virtual GenPageRequest* GenPageRequestRegulated(const VmMapper *pVmMapper, bool isInstr=true, EMemAccessType memAccessType=EMemAccessType::Unknown) const; //!M Retuan a regulated GenPageRequest instance.
    virtual void SetPrivilegeLevel(uint32 priv) { } //!< Set exception level.
    virtual uint32 PrivilegeLevel() const { return 0; } //!< Return exception level.
    virtual ConditionFlags GetConditionFlags() const = 0; //!< Return current condition flag values.  TODO need to go arch specific.
    virtual std::string GetGPRName(uint32 index) const = 0; //!< Return gpr name from index value.
    virtual std::string GetGPRExcludes() const = 0; //!< Return gpr exclude indices in string format.
    virtual void SetupInstructionGroup(EInstructionGroupType iGrpType) { } //!< Setup for instruction group.
    virtual bool OperandTypeCompatible(ERegisterType regType, EOperandType oprType) const { return false; } //!< Check if the operand type is compatible with the register type.
    virtual bool OperandTypeToResourceType(EOperandType opType, EResourceType& rResType) const { return false; } //!< Convert operand type to resource type.
    virtual uint32 GetResourceCount(EResourceType rResType) const { return 0; } //!< Return resource count for a certain resource type.
    void AddLoadRegisterAmbleRequests(const std::string& regName, uint64 loadValue); //!< Add pre post amble requests when loading a register.
    void AddLoadSysRegistersAmbleRequests(const std::map<std::string, uint64>& rRegisters); //!< Add pre post amble requests when loading system registers.
    void AddLoadLargeRegisterAmbleRequests(const std::string& regName, std::vector<uint64>& loadValues); //!< Add pre post amble requests when loading a large register.
    void AddPreambleRequest(GenRequest* request); //!< Add request to preamble request list.
    void AddPostInstructionStepRequest(GenRequest* request); //!< Add request to be processed after stepping an instruction.
    bool SimulationEnabled() const; //!< Indicate if simulation is enabled with test generation.
    bool HasISS() const; //!< Indicate if the ISS is available during test generation.
    void MapPC(); //!< Map the PC of the current PE.
    bool InException() const; //!< Indicate if the generator is in exception handling state.
    bool NoSkip() const; //!< Indicate if the generator is in no-skip state.
    bool InLoop() const; //!< Indicate if the generator is in loop mode.
    bool DelayInit() const; //!< Indicate if the generator is in delay-init state.
    bool ReExecution() const; //!< Indicate if the generator is in re-execution state.
    bool NoJump() const; //!< Indicate if the generator is in no-jump state.
    bool InLowPower() const; // Indicate if the generator is in low power state
    bool InFiller() const; //!< Indicate if the generator is in filler mode
    bool InSpeculative() const; //!< Indicate if the generator is in speculative mode
    bool AddressProtection() const; //!< Indicate if the current mode need address protection.
    bool RecordingState() const; //!< Indicate if the generator is in recording state mode.
    bool RestoreStateLoop() const; //!< Indicate if the generator is in RestoreStateLoop mode.
    inline uint64 MaxInstructions() const { return mMaxInstructions; } //!< Return maximum number of instructions allowed.
    inline uint64 MaxPhysicalVectorLen() const { return mMaxPhysicalVectorLen; } //!< Return maximum physical vector register length allowed.
    void GetTranslationRange(uint64 VA, TranslationRange& rTransRange) const; //!< Get translation range for the specified VA.

    // Forwarded API calls
    void GenInstruction(GenInstructionRequest * instrReq, std::string &rec_id); //!< Try to generator the specified instruction in the Generator thread.
    void GenSequence(GenRequest* pGenRequest); //!< Try to generate the specified sequence in the Generator thread.
    void GenSummary(); //<! Generate instruction summary for this Generator.
    void SetStateValue(EGenStateType stateType, uint64 value); //!< Set a certain state for the Generator to the given value.
    bool GetStateValue(EGenStateType stateType, uint64& stateValue) const; //!< Return the state numeric value of this Generator.
    void InitializeMemory(uint64 addr, uint32 bank, uint32 size, uint64 data, bool isInstr, bool isVirtual); //!< Initialize a memory location.
    void Query(const GenQuery& rGenQuery) const; //!< Process queries of various types.
    void GenVmRequest(GenRequest* pVmReq); //!< Process virtual memory related requests.
    void StateRequest(GenRequest* pStateReq); //!< Process state related requests.
    void ExceptionRequest(GenRequest* pExceptReq); //!< Process exception related requests.
    void ReserveMemory(const std::string& name, const std::string& range, uint32 bank, bool isVirtual); //!< Reserving memory with provided parameters.
    void UnreserveMemory(const std::string& name, const std::string& range, uint32 bank, bool isVirtual);  //!< Unreserving memory with provided parameters.
    void ReserveMemory(const std::string& name, uint64 start, uint64 size, uint32 bank, bool isVirtual); //!< Reserve memory range.
    void UnreserveMemory(const std::string& name, uint64 start, uint64 size, uint32 bank, bool isVirtual); //!< Unreserve memory range.
    void UpdateVm(); //!< Update virtual memory system.
    void ExecuteHandler(); //!< Excecute exception handler.
    void SleepOnLowPower(); //!< Sleep on low power status
    void ExceptionReturn(); //!< Handle exception return.
    void ReExecute(uint64 addr); //!< Handle re-execution request.

    // Forwarded Register related APIs
    bool GetRandomRegisters(cuint32 number, const ERegisterType regType, const std::string& rExcludes, std::vector<uint64>& rRegIndices) const; //!< Get random registers that are not reserved.
    bool GetRandomRegistersForAccess(cuint32 number, const ERegisterType regType, const ERegAttrType access, const std::string& rExcludes, std::vector<uint64>& rRegIndices) const; //!< Get random registers that are not reserved for the specified access.
    uint32 GetRandomRegister(const ERegisterType regType, const std::string& rExcludes, bool* status=nullptr) const; //!< Return one random register index that is not reserved.
    uint32 GetRandomRegisterForAccess(const ERegisterType regType, const ERegAttrType access, const std::string& rExcludes, bool* status=nullptr) const; //!< Return one random register index that is not reserved for the specified access.

    bool IsRegisterReserved(const std::string& name, const ERegAttrType access,  const ERegReserveType reserveType = ERegReserveType::User) const; //!< API to check if register is reserved
    void ReserveRegisterByIndex(uint32 size, uint32 index, const ERegisterType regType, const ERegAttrType access, const ERegReserveType reserveType = ERegReserveType::User); //!< API that reserve a register requested by front-end.
    void ReserveRegister(const std::string& name, const ERegAttrType access, const ERegReserveType reserveType = ERegReserveType::User); //!< API that reserve a register from name requested by front-end.
    void UnreserveRegisterByIndex(uint32 size, uint32 index, const ERegisterType regType, const ERegAttrType access, const ERegReserveType reserveType = ERegReserveType::User); //!< API that unreserve a register requested by front-end.
    void UnreserveRegister(const std::string& name, const ERegAttrType access, const ERegReserveType reserveType = ERegReserveType::User); //!< API that unreserve a register requested from name by front-end.

    bool ReadRegister(const std::string& name, const std::string& field, uint64& reg_value) const; //!< Getting register values.
    void WriteRegister(const std::string& name, const std::string& field, uint64 value); //!< API that writes value to a register requested by front-end.
    void WriteRegisterWithUpdate(const std::string& name, const std::string& field, uint64 value); //!< API that writes value to a register requested by front-end and check if needs to update the system
    virtual bool RegisterUpdateSystem(const std::string& name, const std::string& field) { return false; } //!< check if given register and field impact system update
    virtual void UpdateSystemWithRegister(const std::string& name, const std::string& field, uint64 value) {} //!< API that checks and update system according to given register name, field, and value
    uint64 InitializeRegister(const std::string& name, const std::string& field, uint64 value) const; //!< API that initializes a register or register field with a specified value. If the field name is empty, the entire register is initialized; otherwise, only the specified field is initialized.
    void InitializeRegister(const std::string& name, std::vector<uint64> values) const; //!< API that initializes a large register from a vector of values.
    void InitializeRegisterFields(const std::string& registerName, const std::map<std::string, uint64>& field_value_map) const; //!< API that initialize a list of register fields of the specified field values form front-end API.
    void RandomInitializeRegister(const std::string& name, const std::string& field) const; //!< API that initializes a register or register field with a random value. If the field name is empty, the entire register is initialized; otherwise, only the specified field is initialized.
    void RandomInitializeRegisterFields(const std::string& registerName, const std::vector<std::string>& fieldList) const; //!< API that initialize a list of fields of specified register requested by front-end.
    uint64 GetRegisterFieldMask(const std::string& regName, const std::vector<std::string>& fieldList); //!< Get register mask according to given register name and field list
    void RandomInitializeRegister(Register* pReg) const; //!< Randomly initialize a register.
    void ModifyVariable(const std::string& name, const std::string& value, EVariableType var_type); //!< modify variable
    const std::string& GetVariable(const std::string& name, EVariableType var_type) const; //!< get variable
    const InstructionStructure* GetInstructionStructure(const std::string& instrName) const;  //!< get instruction structure
    RegisteredSetModifier* GetRegisteredSetModifier() const { return mpRegisteredSetModifier; } //!< get registered set modifier
    BntHookManager* GetBntHookManager() const { return mpBntHookManager; } //!< Get Bnt hook manager.
    BntNodeManager* GetBntNodeManager() const { return mpBntNodeManager; } //!< Get Bnt node manager.
    AddressTableManager* GetAddressTableManager() const { return mpAddressTableManager; } //!< Return pointer to AddressTableManager object.
    SimAPI* GetSimAPI() const { return mpSimAPI; } //!< Return a pointer to the SimAPI object.

    void OutputImage(ImageIO* imageIO) const; //!< Output image in text format.
    void SolveAddressShortage(); //!< Handle Address shortage.
    const std::vector<GenAgent*>& GetAgents(){return mAgents;}

    template<typename T>
    T* GetAgent(const EGenAgentType genAgentType)
    {
      return dynamic_cast<T*>(mAgents[EGenAgentTypeBaseType(genAgentType)]);
    }

  protected:
    virtual void AddArchImageThreadInfo(std::map<std::string, uint64>& rThreadInfo) const = 0; //!< Add architectural specific image thread info.
    void ProcessGenRequest(GenRequest* genRequest); //!< Process GenRequest transaction.
    void ReserveMemory(MemoryReservation* pMemReserv); //!< Reserve memory using a MemoryReservation object.
    void UnreserveMemory(MemoryReservation* pMemReserv); //!< Unreserve memory using a MemoryReservation object.
    void ProcessPostInstructionStepRequests(); //!< Process post instruction step requests.
  protected:
    uint32 mThreadId; //!< Thread ID of the generator.
    uint64 mMaxInstructions; //!< Maximum instructions allowed.
    uint64 mMaxPhysicalVectorLen; //!< Maximum physical vector register length allowed.
    const ArchInfo* mpArchInfo; //!< Pointer to the shared ArchInfo object for this type of Generator.
    const InstructionSet* mpInstructionSet; //!< Pointer to the shared InstructionSet object for this type of Generator.
    const PagingInfo* mpPagingInfo; //!< Pointer to the shared PagingInfo object for this type of Generator.
    MemoryManager* mpMemoryManager; //!< Pointer to the shared memory manager object.
    SimAPI* mpSimAPI; //!< Pointer to the shared SimAPI object for this type of Generator.
    VirtualMemoryInitializer* mpVirtualMemoryInitializer; //!< Pointer to the virtual memory initializer object.
    RegisterFile* mpRegisterFile; //!< Pointer to the register-file object.
    VmManager* mpVmManager; //!< Pointer to the VmManager object.
    GenRequestQueue* mpRequestQueue; //!< A queue holding GenRequest objects to be processed.
    ThreadInstructionResults* mpThreadInstructionResults; //!< Container holding all instruction generated in the generator thread.
    RecordArchive* mpRecordArchive; //!< Container holding various records regarding data generated in the generator thread.
    BootOrder* mpBootOrder; //!< Pointer to the BootOrder class.
    GenMode* mpGenMode; //!< Pointer to the GenMode object.
    GenPC* mpGenPC; //!< Pointer to the GenPC object.
    ReExecutionManager* mpReExecutionManager; //!< Pointer to the ReExecutionManager object.
    ResourceDependence* mpDependence; //!< Pointer to the ResourceDependence object.
    RegisteredSetModifier* mpRegisteredSetModifier; //!< Pointer to the RegisteredSetModifier
    ExceptionRecordManager* mpExceptionRecordManager; //!< Pointer to the ExceptionRecordManager for this generator.
    ChoicesModerators* mpChoicesModerators; //!< Pointer to the choices moderators
    GenConditionSet* mpConditionSet; //!< Pointer to the GenConditionSet object for this GenThread.
    PageRequestRegulator* mpPageRequestRegulator; //!< Pointer to the PageRequestRegulator object for this GenThread.
    AddressFilteringRegulator* mpAddressFilteringRegulator; //!< Pointer to the AddressFilteringRegulator object for this GenThread.
    BntHookManager* mpBntHookManager; //!< Pointer to the BntHookManager object for this GenThread.
    BntNodeManager* mpBntNodeManager; //!< Pointer to BntNodeManager object
    AddressTableManager* mpAddressTableManager; //!< Pointer to AddressTableManager object.
    std::vector<GenAgent* > mAgents; //!< Various agent class to delegate generator functions.
    std::map<EGenStateType, uint64> mGenStateValues; //!< Value type generator states.
    std::map<EGenStateType, std::string> mGenStateStrings; //!< String type generator states.
    std::vector<GenRequest* > mPreAmbleRequests; //!< Pre amble requests for instruction.
    std::vector<GenRequest* > mPostAmbleRequests; //!< Post amble requests for instruction.
    std::vector<GenRequest* > mPostInstrStepRequests; //!< Post instruction step requests for an instruction.
    std::vector<VariableModerator* > mVariableModerators; //!< Various VariableModerator classes
    friend class ArchInfoBase;
  };

}

#endif
