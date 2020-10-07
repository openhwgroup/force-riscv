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
#ifndef Force_SimAPI_H
#define Force_SimAPI_H

#pragma GCC visibility push(default)

#include <vector>
#include <map>
#include <string>
#include <fstream>

#include <Defines.h>
#include <VectorElementUpdates.h>

struct SimException;

namespace Force {

  /*!
    \class SimAPI
    \brief A C++ wrapper class for the Force/Handcar C API.
  */

  typedef enum _INTERRUPT { SError=1, VSError=2, IR4=4, VIRQ=8, FIQ=16, VFIQ=32 } INTERRUPT;

  typedef enum _Memtype { Strong,Device,Normal } Memtype;

  typedef unsigned int CacheType;
  typedef unsigned int CacheAttrs;

  //TODO I recommend adding a struct for instruction updates that includes things like instruction number, pc, text, operands, and adding callback to make use of it.

  //!< MemUpdate - struct used to record memory updates from simulator...

  struct MemUpdate {
    MemUpdate(uint32 _CpuID, uint64 _virtual_address, unsigned int _mem_bank, uint64 _physical_address, unsigned int _size, const char *_bytes, const char *_access_type)
      : mem_bank(_mem_bank), virtual_address(_virtual_address), physical_address(_physical_address), size(_size), bytes(), access_type(), CpuID(_CpuID)
    {
      for (unsigned int i = 0; i < _size; i++) {
        bytes.push_back(_bytes[i]);
      }

      access_type.assign(_access_type);
    }

    uint32 mem_bank;
    uint64 virtual_address;
    uint64 physical_address;
    uint32 size;
    std::vector<unsigned char> bytes;
    std::string access_type;
    uint32 CpuID;
  };

  //!< RegUpdate - struct used to record register updates from simulator...

  struct RegUpdate {
    RegUpdate(uint32 _CpuID, const char *_regname, uint64 _rval, uint64  _mask, const char *_access_type)
      : CpuID(_CpuID), regname(), rval(_rval), mask(_mask), access_type()
    {
      regname.assign(_regname);
      access_type.assign(_access_type);
    }

    uint32 CpuID;
    std::string regname;
    uint64 rval;
    uint64 mask;
    std::string access_type;
  };

  //!< MmuEvent - struct used to record memory events from simulator...

  struct MmuEvent {
    MmuEvent(uint64 _va, uint64 _pa, Memtype _type, bool _has_stage_two, CacheType _outer_type, CacheAttrs _outer_attrs, CacheType _inner_type, CacheAttrs _inner_attrs)
      : va(_va), pa(_pa), type(_type), has_stage_two(_has_stage_two), outer_type(_outer_type), outer_attrs(_outer_attrs), inner_type(_inner_type), inner_attrs(_inner_attrs)
    {
    }

    uint64 va;
    uint64 pa;
    Memtype type;
    bool has_stage_two;
    CacheType outer_type;
    CacheAttrs outer_attrs;
    CacheType inner_type;
    CacheAttrs inner_attrs;
  };

  struct ExceptionUpdate {
    ExceptionUpdate(uint32 exceptionID, uint32 exceptionAttributes, const char* comments)
      : mExceptionID(exceptionID), mExceptionAttributes(exceptionAttributes), mComments()
    {
      mComments.assign(comments);
    }

    uint32 mExceptionID; //!< Exception ID
    uint32 mExceptionAttributes; //!< Exception attributes
    std::string mComments;  //!< exception description
  };

  struct ThreadSummary {
    uint32 mInstructionCount; //!< Instruction count for a thread.
    uint32 mExitCode; //!< Exit code for a thread.
    std::string mSummary; //!< Summary information for the thread.
    ThreadSummary() : mInstructionCount(0), mExitCode(0), mSummary() { } //!< Constructor.
  };

  class ApiSimConfig {
  public:
    ApiSimConfig() : mChipNum(1), mCoreNum(1), mThreadNum(1), mPhysicalAddressSize(48u), mVectorRegLen(128), mMaxVectorElemWidth(32), mpTraceFile(NULL), mUseTraceFile(false), mSimConfigString("") { } //!< default constructor

    ApiSimConfig(uint32 chipNum, uint32 coreNum, uint32 threadNum, uint32 physicalAddressSize, uint32 vectorRegLen, uint32 maxVectorElemWidth, const char* pTraceFile, bool outputTraceFile, std::string simCfgStr) //!< Constructor.
      : mChipNum(chipNum), mCoreNum(coreNum), mThreadNum(threadNum), mPhysicalAddressSize(physicalAddressSize), mVectorRegLen(vectorRegLen), mMaxVectorElemWidth(maxVectorElemWidth), mpTraceFile(pTraceFile), mUseTraceFile(outputTraceFile), mSimConfigString(simCfgStr)
    {
    }

    ~ApiSimConfig() {} //!< Destructor

    ASSIGNMENT_OPERATOR_ABSENT(ApiSimConfig);
    COPY_CONSTRUCTOR_ABSENT(ApiSimConfig);

    uint32 mChipNum;  //!< Chip number.
    uint32 mCoreNum; //!< Core number per chip.
    uint32 mThreadNum; //!< Thread number per core.
    uint32 mPhysicalAddressSize; //!< Supported physical address size.
    uint32 mVectorRegLen; //!< Vector register length in bits.
    uint32 mMaxVectorElemWidth; //!< Maximum vector element width in bits.
    const char* mpTraceFile; //!< trace file. not output trace if it is NULL
    bool mUseTraceFile;
    std::string mSimConfigString; //!< any additions to simulator config 
  };

  /*!
    \class SimAPI
    \brief Access interface class for simulator C APIs.
  */
  class SimAPI {
  public:
    SimAPI(); //!< Constructor.

    virtual ~SimAPI(); //!< Destructor.

    //!< Initialize simulator shared object using standard Force parameters...
    virtual void InitializeIss(const ApiSimConfig& rConfig, const std::string &rSimSoFile, const std::string& rApiTraceFile) = 0;

    //!< write out simulation trace file (if any), send 'terminate' directive to simulator...
    virtual void Terminate() = 0;

    //!< return simulator version info as String...
    virtual void GetSimulatorVersion(std::string &sim_version) = 0;

    //!< read simulator physical memory. Return 0 if no errors...
    virtual void ReadPhysicalMemory(uint32 mem_bank, uint64 address, uint32 size, unsigned char *pBytes) = 0;

    //!< write simulator physical memory. Return 0 if no errors...
    virtual void WritePhysicalMemory(uint32 mem_bank, uint64 address, uint32  size, const unsigned char *pBytes) = 0;

    //!< read register value...
    virtual void ReadRegister(uint32 CpuID, const char *regname, uint64 *rval, uint64 *rmask) = 0;

    //!< read the bytes corresponding to a physical register from a large logical register without forcing the simulator to used Force's naming convention.
    virtual void PartialReadLargeRegister(uint32 CpuID, const char* pRegname, uint8_t* pBytes, uint32 length, uint32 offset) = 0;

    //!< write the bytes corresponding to a physical register from a large logical register without forcing the simulator to used Force's naming convention.
    virtual void PartialWriteLargeRegister(uint32 CpuID, const char* pRegname, const uint8_t* pBytes, uint32 length, uint32 offset) = 0;

    //!< write simulator register. mask indicates which bits to write...
    virtual void WriteRegister(uint32 CpuID,const char *regname,uint64 rval,uint64 rmask) = 0;

    //!< request simulator to 'inject event(s)...
    virtual void InjectEvents(uint32 CpuID, uint32 interrupt_sets) = 0;

    //!< step instruction for specified cpu; returns after simulator step complete, with all updates.
    virtual void Step(uint32 cpuid,std::vector<RegUpdate> &rRegUpdates,std::vector<MemUpdate> &rMemUpdates, std::vector<MmuEvent> &rMmuEvents, std::vector<ExceptionUpdate> &rExceptions) = 0;

    virtual void WakeUp(uint32 cpuId) = 0; //!< Wake up from lower power state.
    virtual void TurnOn(uint32 cpuId) = 0; //!< Turn the Iss thread on.
    virtual void EnterSpeculativeMode(uint32 cpuId) = 0; //!< The CPU thread enters speculative mode.
    virtual void LeaveSpeculativeMode(uint32 cpuId) = 0; //!< The CPU thread leaves speculative mode.
    virtual void RecordExceptionUpdate(const SimException *pException) = 0; //!< Record exceptions.

    //!< form 'cpuID' from cluster,core,thread...
    uint32 CpuID(uint32 socket, uint32 cluster, uint32 core, uint32 thread);

    //!< 'record' methods used by extern C 'update' methods; not protected or private since the extern C functions need
    //!< to be able to access...
    void RecordRegisterUpdate(uint32 CpuID,const char *regname,uint64 rval,uint64 mask, const char *pAccessType);
    void RecordMemoryUpdate(uint32 CpuID, uint64 virtualAddress, unsigned int memBank,
                            uint64 physicalAddress, unsigned int size,
                            const char* pBytes, const char* pAccessType);
    void RecordMmuEvent(MmuEvent *event);
    void RecordVectorRegisterUpdate(uint32 CpuID, const char* pRegname, uint32 vecRegIndex, uint32 eltIndex, uint32 eltByteWidth, const uint8_t* pValue, uint32 byteLength, const char* pAccessType);
    void RecordTermination(uint32 CpuId, uint32 exitCode, const char* pMsg); //!< Return termination state of the CPU with the specified ID.
  protected:
    void SetVectorRegisterWidth(cuint64 vecRegWidthBits); //!< Set the vector register width in bits.

    //!< used by Step:
    bool GetRegisterUpdates(std::vector<RegUpdate> &rRegUpdates);
    bool GetMemoryUpdates(std::vector<MemUpdate> &rMemUpdates);
    bool GetMmuEvents(std::vector<MmuEvent> &rMmuEvents);
    bool GetExceptionUpdates(std::vector<ExceptionUpdate> &rExceptUpdates);
    bool GetVectorRegisterUpdates(std::vector<RegUpdate> &rRegUpdates);

    //!< for logging a 'trace session':
    void OpenApiTrace(const std::string& rApiTraceFile);
    void CloseApiTrace();
    void ApiTraceCheckRcode(const std::string& function);

    void OpenOfs(const std::string& rTraceFileName, std::ofstream& rStreamToOpen);
    void CloseOfs(std::ofstream& rStreamToClose);

    void PrintRegisterUpdates();
    void PrintMemoryUpdates();
    void PrintMmuEvents();
    void PrintExceptionUpdates();
    void PrintInstructionStep(uint64 pc, uint32 CpuID, uint32 iCount, const std::string& rOpcode, const std::string& rDisassembly);
    uint32 UpdateCurrentInstructionCount(uint32 CpuId); //!< Set the current Instruction count for instruction stream printing.
    void PrintSummary() const; //!< Print summary information before terminating the ISS.
  protected:
    //!< at end of step these vectors have recorded any updates from running step on the simulator:
    std::vector<RegUpdate> mRegisterUpdates;
    std::vector<MemUpdate> mMemoryUpdates;
    std::vector<MmuEvent> mMmuEvents;
    std::vector<ExceptionUpdate> mExceptionUpdates;
    std::map<uint32, VectorElementUpdates> mVectorElementUpdates;
    std::map<uint32, ThreadSummary> mThreadSummaries; //!< Summary information for each CPU.

    //!< Use to get simulator API trace file, for debugging the API itself...
    mutable std::ofstream mOfsApiTrace;
    mutable std::ofstream mOfsSimTrace;   

    //!< Implementation specific characteristics of vector registers
    uint32 mVecRegWidth;
    std::vector<std::string> mVecPhysRegNames;
    uint32 mNumPhysRegs;
    cuint32 mPhysRegSize;

    //!< Speculative mode output modification
    std::map<uint32, uint32> mInSpeculativeMode;
    const std::vector<std::string> mSpecModeStrings;
    //const std::string mSpecModeOn;
    //const std::string mSpecModeOff;
  };

}

extern "C" {

  Force::SimAPI * instantiate_api();

}

#pragma GCC visibility pop

#endif
