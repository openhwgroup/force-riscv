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
#ifndef Force_Scheduler_H
#define Force_Scheduler_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include "Defines.h"
#include "Enums.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class PyInterface;
  class Generator;
  class GenRequest;
  class GenInstructionRequest;
  class GenQuery;
  class InstructionStructure;
  class SchedulingStrategy;
  class ThreadGroupModerator;
  class ThreadGroup;
  class PartitionArgument;
  class SemaphoreManager;
  class ConstraintSet;
  class SynchronizeBarrierManager;

  /*!
    \class Scheduler
    \brief Scheduling class and interface with Python front end.
   */
  class Scheduler {
  public:
    static void Initialize(); //!< Create Scheduler instance.
    static void Destroy(); //!< Destroy Scheduler instance.
    static Scheduler* Instance() { return mspScheduler; } //!< Access Scheduler instance.

    void Run(); //!< Start up scheduler, run the generator threads.
    void OutputTest(); //!< Output test.

    uint32 NumberOfChips() const { return mNumChips; } //!< Return number of chips in the system.
    uint32 NumberOfCores() const { return mNumCores; } //!< Return number of cores in each chip.
    uint32 NumberOfThreads() const { return mNumThreads; } //!< Return number of threads in each core.
    uint32 CreateGeneratorThread(uint32 iThread, uint32 iCore, uint32 iChip); //!< Called to create back end generator thread.
    void GenInstruction(uint32 threadId, GenInstructionRequest * instrReq, std::string& rec_id); //!< Called to generate an instruction.
    uint32 ThreadId(uint32 iThread, uint32 iCore, uint32 iChip); //!< Return a thread identifier encoding in an integer.
    void InitializeMemory(uint32 threadId, uint64 addr, uint32 bank, uint32 size, uint64 data, bool isInstr, bool isVirtual); //!< Initialize a memory location.
    uint32 AddChoicesModification(uint32 threadId, EChoicesType choicesType, const std::string& treeName,
                                const std::map<std::string, uint32>& modifications); //!< add choice modification
    void CommitModificationSet(uint32 threadId,  EChoicesType choicesType, uint32 setId); //!< commit modification set
    void RevertModificationSet(uint32 threadId,  EChoicesType choicesType, uint32 setId); //!< revert modification set
    uint32 DoChoicesModification(uint32 threadId, EChoicesType choicesType, const std::string& treeName,
                               const std::map<std::string, uint32>& modifications); //!< do choices modification
    void AddMemoryRange(uint32 bank, uint64 start, uint64 end); //!< Add usable physical memory range.
    void SubMemoryRange(uint32 bank, uint64 start, uint64 end); //!< Subtract usable physical memory range.
    void AddArchitectureMemoryAttributes(cuint32 threadId, cuint32 bank, cuint64 start, cuint64 end, const std::vector<EMemoryAttributeType>& rMemAttributes); //!< Assign architecture memory attributes to the specified physical memory range.
    void AddImplementationMemoryAttributes(cuint32 threadId, cuint32 bank, cuint64 start, cuint64 end, const std::vector<std::string>& rMemAttributes); //!< Assign implementation memory attributes to the specified physical memory range.

    // Virtual memory related API
    void GenVmRequest(uint32 threadId, GenRequest* pVmReq); //!< Pass virtual memory request to the back end.

    // Random module API
    void Sample (uint64 totals, uint64 samples, std::vector<uint64>& sampleList) const; //!< obtain a sample list between 0 and totals-1

    // Register module API
    bool GetRandomRegisters(cuint32 threadId, cuint32 number, const std::string& rRegType, const std::string& rExcludes, std::vector<uint64>& rRegIndices) const; //!< Get random registers that are not reserved.
    bool GetRandomRegistersForAccess(cuint32 threadId, cuint32 number, const std::string& rRegType, const std::string& rAccess, const std::string& rExcludes, std::vector<uint64>& rRegIndices) const; //!< Get random registers that are not reserved for the specified access.
    bool IsRegisterReserved(uint32 threadId, const std::string& name, const std::string& access, const std::string& type) const; //!< API that checks if given register is reserved

    void ReserveRegisterByIndex(uint32 threadId, uint32 size, uint32 index, const std::string& regType, const std::string& access); //!< API that reserve a register requested by front-end.
    void ReserveRegister(uint32 threadId, const std::string& name, const std::string& access); //!< API that reserve a register requested by front-end.
    void UnreserveRegisterByIndex(uint32 threadId, uint32 size, uint32 index, const std::string& regType, const std::string& access); //!< API that unreserve a register requested by front-end.
    void UnreserveRegister(uint32 threadId, const std::string& name, const std::string& access, const std::string& reserveType); //!< API that unreserve a register requested by front-end.
    bool ReadRegister(uint32 threadId, const std::string& name, const std::string& field, uint64& reg_value) const; //!< Getting register values.
    void WriteRegister(uint32 threadId, const std::string& name, const std::string& field, uint64 value, bool update); //!< API that writes value to a register requested by front-end.
    void InitializeRegister(uint32 threadId, const std::string& name, const std::string& field, uint64 value); //!< API that initializes a register requested by front-end.
    void InitializeRegister(uint32 threadId, const std::string& name, std::vector<uint64> values); //!< API that initializes a large register with a vector of values
    void InitializeRegisterFields(uint32 threadId, const std::string& registerName, const std::map<std::string, uint64>& field_value_map); //!< API that initialize a list of register fields of specified field value from front end API.
    void RandomInitializeRegister(uint32 threadId, const std::string& name, const std::string& field); //!< API that randomize a register requested by front-end.
    void RandomInitializeRegisterFields(uint32 threadId, const std::string& registerName, const std::vector<std::string>& fieldList); //!< API that initialize a list of fields from a specified register requested by front-end.
    uint64 GetRegisterFieldMask(uint32 threadId, const std::string& regName, const std::vector<std::string>& fieldList);  //!< Get register Mask according to given register name and field list
    void GenSequence(uint32 threadId, GenRequest* pGenRequest); //!< API calling back-end to generate functional sequence.
    void ConfigureMemoryBanks(); //!< Called to configure generator base line memory constraints.
    void Query(uint32 threadId, const GenQuery& rGenQuery); //!< Called to obtain results looking for by the various queries.
    void StateRequest(uint32 threadId, GenRequest* pStateReq); //!< Pass state request to the back end.
    void ReserveMemory(uint32 threadId, const std::string& name, const std::string& range, uint32 bank, bool isVirtual); //!< Reserving memory with provided parameters.
    void UnreserveMemory(uint32 threadId, const std::string& name, const std::string& range, uint32 bank, bool isVirtual);  //!< Unreserving memory with provided parameters.
    void ExceptionRequest(uint32 threadId, GenRequest* pExceptReq); //!< Pass exception related request to the back end.
    Generator* LookUpGenerator(uint32 threadId); //!< Look up Generator instance by thread ID.
    const Generator* LookUpGenerator(uint32 threadId) const; //!< Constant version, look up Generator instance by thread ID.
    const std::map<uint32, Generator* >& GetGenerators() const {return mGenerators; } //!< return all generators.
    void ModifyVariable(uint32 threadId, const std::string& name, const std::string& value, const std::string& var_type); //!< modify variable
    const std::string& GetVariable(uint32 threadId, const std::string& name, const std::string& var_type) const; //!< get variable
    const InstructionStructure* GetInstructionStructure(uint32 threadId, const std::string& instrName) const; // get instruction structure
    uint32 RegisterModificationID(uint32 threadId) const; //!< register a modification ID
    void RegisterModificationSet(uint32 threadId,  EChoicesType choicesType, uint32 set_id); //!< register choices modification
    bool VerifyVirtualAddress(uint32 threadId, uint64 va, uint64 size, bool isInstr) const; //!< verify virtual address is usable or not

    // Following methods will be redirected to scheduling-strategy object.
    void AddThreadId(uint32 threadId); //!< Add front end thread ID for scheduling.
    void RemoveThreadId(uint32 threadId); //!< Remove front end thread ID for scheduling.
    void NextThread(); //!< Advance to next thread.
    void RefreshSchedule(); //!< Refresh scheduling for next round.
    uint32 ActiveThreadCount() const; //!< Return number of active threads that are being scheduled.
    uint32 CurrentThreadId() const; //!< Return current thread ID.
    void LockThreadScheduler(uint32 threadId); //!< Lock thread scheduler so the current thread can keep progressing.
    void UnlockThreadScheduler(uint32 threadId); //!< Unlock thread scheduler.
    void PartitionThreadGroup(EPartitionThreadPolicy policy, const PartitionArgument* pPartitionArg = nullptr); //!< parition thread group
    void SetThreadGroup(uint32 groupId, const std::string& job, const std::string& threads); //!< set thread group
    void QueryThreadGroup(uint32 groupId, std::vector<ThreadGroup* >& threadGroups) const; //!< query thread group
    void QueryThreadGroup(const std::string& groupJob, std::vector<ThreadGroup* >& threadGroups) const; //!< query thread group
    uint32 GetThreadGroupId(uint32 threadId) const; //!< get group id the thread belongs to
    void GetFreeThreads(std::vector<uint32>& freeThreads) const; //!< get free threads
    bool GenSemaphore(uint32 threadId, const std::string& name, uint64 counter, uint32 bank, uint32 size, uint64& address, bool& reverseEndian); //!< generate a semaphore
    void SynchronizeWithBarrier(uint32 threadId, const ConstraintSet& rSynchronizedThreadIds); //!< Let threadId participate in the synchronize barrier specified by threadIds.
  private:
    Scheduler(); //!< Constructor.
    COPY_CONSTRUCTOR_ABSENT(Scheduler);
    ~Scheduler(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(Scheduler);
  private:
    static Scheduler* mspScheduler; //!< Static pointer to Scheduler.
    uint32 mNumChips; //!< Number of chips in the system.
    uint32 mNumCores; //!< Number of cores in each chip.
    uint32 mNumThreads; //!< Number of threads in each core.
    uint32 mChipsLimit; //!< Maximum number of chips.
    uint32 mCoresLimit; //!< Maximum number of cores.
    uint32 mThreadsLimit; //!< Maximum number of threads.
    PyInterface* mpPyInterface; //!< Pointer to the Python interface instance.
    SchedulingStrategy* mpSchedulingStrategy; //!< Pointer to the thread scheduling strategy instance.
    ThreadGroupModerator *mpGroupModerator; //!< thread group moderator
    SemaphoreManager *mpSemaManager; //!< semaphore manager
    SynchronizeBarrierManager* mpSyncBarrierManager; //!< Synchronize barriers manager.

    std::map<uint32, Generator *> mGenerators; //!< Holder of all Generators.
  };

}

#endif
