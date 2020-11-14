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
#ifndef Force_PyInterface_H
#define Force_PyInterface_H

#include <Defines.h>
#include <string>
#include <Python.h>
#include <pybind11/pybind11.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <map>

namespace py = pybind11;

namespace Force {

  class Scheduler;

  /*!
    \class PyInterface
    \brief Class handling interfacing between back end and front end.
   */

  class PyInterface {
  public:
    explicit PyInterface(Scheduler* scheduler) : mpScheduler(scheduler), mLibPath(), mEnvObject(), mTemplateObject() {} //!< Constructor
    ~PyInterface() {} //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(PyInterface);
    COPY_CONSTRUCTOR_DEFAULT(PyInterface);
    void RunTest(); //!< Run test template to generate test.
    uint32 CallBackTemplate(uint32 threadId, ECallBackTemplateType callBackType, const std::string& primaryValue, const std::map<std::string, uint64>& callBackValues); //!< call back some fuction on test template
    // Interface functions that will be exposed to front end
    uint32 NumberOfChips() const; //!< API that returns number of chips in the system.
    uint32 NumberOfCores() const; //!< API that returns number of cores in each chip.
    uint32 NumberOfThreads() const; //!< API that returns number of threads in each core.
    uint32 CreateGeneratorThread(uint32 iThread, uint32 iCore, uint32 iChip); //!< Called to create back end generator thread.
    py::object GenInstruction(uint32 threadId, const std::string& instrName, const py::dict& parms); //!< API that generate an instruction requested by front-end.
    py::object GenMetaInstruction(uint32 threadId, const std::string& instrName, const py::dict& metaParms); //!< API that generate a meta instruction requested by front-end.
    void InitializeMemory(uint32 threadId, uint64 addr, uint32 bank, uint32 size, uint64 data, bool isInstr, bool isVirtual); //!< Initialize a memory location.
    py::object AddChoicesModification(uint32 threadId, const py::object& choicesType, const std::string& treeName, const py::dict& params, bool globalModification = false); //!< Add choices modification
    void CommitModificationSet(uint32 threadId, const py::object& choicesType, const py::object& setId); //!< commit a modification set
    void RevertModificationSet(uint32 threadId, const py::object& choicesType, const py::object& setId);  //!< revert a modification set with the given ID
    void AddMemoryRange(uint32 bank, uint64 start, uint64 end); //!< Add usable physical memory range.
    void SubMemoryRange(uint32 bank, uint64 start, uint64 end); //!< Subtract usable physical memory range.

    // Page related APIs
//    py::object GenPA(uint32 threadId, uint64 size, bool isInstr, uint64 align) const;
    py::object GenPA(uint32 threadId, const py::dict& parms) const;
    py::object GenVA(uint32 threadId, const py::dict& parms) const; //!< Generrate a valid VA based on the constraints provided by the front end.
    py::object GenVMVA(uint32 threadId, const py::dict& parms) const; //!< Generrate a valid VA based on the constraints provided by the front end.
    py::object GenVAforPA(uint32 threadId, const py::dict& parms) const; //!< Generrate a valid VA based on the constraints provided by the front end targeting specified physical address.
    py::object GenFreePagesRange(uint32 threadId, const py::dict& parms) const; //!< Generate free page ranges
    py::object GenVmContext(uint32 threadId, const py::dict& parms) const; //!< Generate a VmContext but not switch to
    py::object GetVmContextDelta(uint32 threadId, const py::dict& parms) const; //!< returns a map of field that are different from target ContextId to current default values.

    // Random module APIs
    py::object Sample(uint64 totals, uint64 samples) const; //!< Return a vector of randomly sampled numbers between 0 and totals-1

    // Register module APIs
    py::object GetRandomRegisters(cuint32 threadId, cuint32 number, const std::string& rRegType, const std::string& rExcludes) const; //!< Get random registers that are not reserved.
    py::object GetRandomRegistersForAccess(cuint32 threadId, cuint32 number, const std::string& rRegType, const std::string& rAccess, const std::string& rExcludes) const; //!< Get random registers that are not reserved for the specified access.
    py::object IsRegisterReserved(uint32 threadId, const std::string& name, const std::string& access, const std::string& type) const; //!< API to check if register is reserved

    void ReserveRegisterByIndex(uint32 threadId, uint32 size, uint32 index, const std::string& regType, const std::string& access) const;  //!< API that reserve a register requested by front-end.
    void ReserveRegister(uint32 threadId, const std::string& name,  const std::string& access) const;  //!< API that reserve a register requested by front-end.
    void UnreserveRegisterByIndex(uint32 threadId, uint32 size, uint32 index, const std::string& regType, const std::string& access) const; //!< API that unreserve a register requested by front-end.
    void UnreserveRegister(uint32 threadId, const std::string& name, const std::string& access, const std::string& reserveType) const; //!< API that unreserve a register requested by front-end.
    py::object ReadRegister(uint32 threadId, const std::string& name, const std::string& field) const; //!< Getting register values.
    void WriteRegister(uint32 threadId, const std::string& name, const std::string& field, const py::object& value, bool update); //!< API that writes value to a register requested by front-end.
    void InitializeRegister(uint32 threadId, const std::string& name, const std::string& field, const py::object& value); //!< API that unreserve a register requested by front-end.
    void InitializeRegisterFields(uint32 threadId, const std::string& registerName, const py::dict& field_value_map); //!< API that reserve a list register fields of specified field value requested by front-end.
    void RandomInitializeRegister(uint32 threadId, const std::string& name, const std::string& field); //!< API that unreserve a register requested by front-end.
    void RandomInitializeRegisterFields(uint32 threadId, const std::string& registerName, const py::list& fieldList); //!< API that randomly intialize a list of fields of a specified register as requested from front-end.
    py::object GetRegisterFieldMask(uint32 threadId, const std::string& regName, const py::list& fieldList) const;  //!< Get the regiser field mask according to given registr name and field name list

    void GenSequence(uint32 threadId, const std::string& sequenceType, const py::dict& rParams) const; //<! API that calls different sequence routines supported in the backend.

    // Memory related APIs
    void ReserveMemory(uint32 threadId, const std::string& name, const std::string& range, uint32 bank, bool isVirtual); //!< Reserving memory with provided parameters.
    void UnreserveMemory(uint32 threadId, const std::string& name, const std::string& range, uint32 bank, bool isVirtual);  //!< Unreserving memory with provided parameters.

    void ModifyVariable(uint32 threadId, const std::string& name, const std::string& value, const std::string& var_type); //!< modify variable

    std::string GetVariable(uint32 threadId, const std::string& name, const std::string& var_type) const; //!< get variable

    void PartitionThreadGroup(const std::string& policy, const py::dict& params); //!< partition thread group
    void SetThreadGroup(uint64 groupId, const std::string& job, const std::string& threads); //!< set thread group
    py::object QueryThreadGroup(const py::object& groupId) const; //!< query thread group info
    py::object GetThreadGroupId(uint32 threadId) const; //!< get thread group id the thread belongs to
    py::object GetFreeThreads() const; //!< get free threads which does not belongs to any group

    void LockThreadScheduler(uint32 threadId); //!< Lock thread scheduler so the current thread can keep progressing.
    void UnlockThreadScheduler(uint32 threadId); //!< Unlock thread scheduler.
    void SynchronizeWithBarrier(uint32 threadId, const py::dict& params); //!< Let the current PE participate in the synchronize barrier specified by params.

    py::object GenSemaphore(uint32 threadId, const std::string& name, uint64 counter, uint32 bank, uint32 size); //!< Generate a semaphore

    // Misc APIs
    py::object GetOption(const std::string& optName) const; //!< API that return an options value and if it is valid.
    py::object Query(uint32 threadId, const std::string& queryName, const std::string& primaryString, const py::dict& rParams) const;  //!< API for query of various types.
    py::object VirtualMemoryRequest(uint32 threadId, const std::string& reqName, const py::dict& rParams) const; //!< API servicing virtual memory request of various types.
    py::object StateRequest(uint32 threadId, const std::string& actionName, const std::string& stateName, const py::object& stateValue, const py::dict& rParams) const; //!< API servicing state request of various types.
    py::object ExceptionRequest(uint32 threadId, const std::string& rReqName, const py::dict& rParams) const; //!< API servicing exception related requests.
    void BeginStateRestoreLoop(cuint32 threadId, cuint32 loopRegIndex, cuint32 simCount, cuint32 restoreCount, const py::list& restoreExclusions) const; //!< Begin generating a loop that automically restores state with each iteration.
    void EndStateRestoreLoop(cuint32 threadId, cuint32 loopId) const; //!< Mark the end of a state restore loop.
    void GenerateLoopRestoreInstructions(cuint32 threadId, cuint32 loopId) const; //!< Mark the beginning of the loop restore instruction sequence.
    void RegisterModificationSet(uint32 threadId, const py::object& choicesType, uint32 set_id); //!< API to register choices modifications
    bool VerifyVirtualAddress(uint32 threadId, uint64 va, uint64 size, bool isInstr) const; //!< verify virtual address is usable or not
  private:
    void SetupModulePaths(const std::string& templatePath); //!< Setup necessary paths for loading Python modules.
    void GenerateTemplate(py::object& template_obj); //!< Generate test using test template.
    std::string GetLibModuleName(const std::string& inFilePath); //!< Convert file path to library module name.
    py::object LoadTestTemplate(const std::string& templatePath, py::object& globals); //!< Load the test template as a Python module.
  private:
    Scheduler* mpScheduler; //!< Pointer to the back-end scheduler.
    std::string mLibPath; //!< Path to generator Python library.
    py::object mEnvObject; //!< front-end enviroment object
    py::object mTemplateObject; //!< front-end template object
  };

}

#endif
