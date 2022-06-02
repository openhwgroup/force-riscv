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
#include "SimAPI.h"

#include <iomanip>

/*!
  \file SimAPI.cc
  \brief Code supporting SimAPI class.
*/

using namespace std;

namespace Force {

  SimAPI::SimAPI()
  : mRegisterUpdates(), mMemoryUpdates(), mMmuEvents(), mExceptionUpdates(), 
    mVectorElementUpdates(), mThreadSummaries(),
    mOfsApiTrace(),
    mOfsSimTrace(),
    mVecRegWidth(0),
    mVecPhysRegNames(),
    mNumPhysRegs(0),
    mPhysRegSize(8u),
    mInSpeculativeMode(),
    mSpecModeStrings{" ", " (spcltv) "}
  {
  }

  SimAPI::~SimAPI()
  {
    CloseApiTrace();
    CloseOfs(mOfsSimTrace);
  }

  //!< for logging a 'trace session':
  void SimAPI::OpenApiTrace(const string& rApiTraceFile)
  {
    if (rApiTraceFile.size() > 0) {
      mOfsApiTrace.open(rApiTraceFile);
    }
  }

  void SimAPI::CloseApiTrace()
  {
    if (mOfsApiTrace.is_open()) {
      mOfsApiTrace << "}\n" << std::endl;
      mOfsApiTrace.flush();
      mOfsApiTrace.close();
    }
  }

  void SimAPI::ApiTraceCheckRcode(const std::string& function)
  {
      mOfsApiTrace << "  if (rcode) { printf(\"!!! non-zero return code from call to '"
                   << function
                   << "', rcode: %d !!!\n\",rcode); return -1; }";
  }

  void SimAPI::OpenOfs(const std::string& rTraceFileName, std::ofstream& rStreamToOpen)
  {
    if (rTraceFileName.size() > 0) {
      rStreamToOpen.open(rTraceFileName);

    }
  }

  void SimAPI::CloseOfs(std::ofstream& rStreamToClose)
  {
    if(rStreamToClose.is_open()){
      rStreamToClose.flush();
      rStreamToClose.close();
    }
  }

  void SimAPI::PrintRegisterUpdates()
  {
    if(not mOfsSimTrace.is_open()) return;

    std::string _speculative_mode;

    for(const RegUpdate& rRegUp : mRegisterUpdates)
    {
      _speculative_mode = mSpecModeStrings[0];
      auto _spec_mode_lookup = mInSpeculativeMode.find(rRegUp.CpuID);
      if(_spec_mode_lookup != mInSpeculativeMode.end())
      {
	_speculative_mode = mSpecModeStrings[_spec_mode_lookup->second];
      } 

      mOfsSimTrace << "Cpu " << dec << rRegUp.CpuID << _speculative_mode;

      if(rRegUp.access_type == std::string("write"))
      {
        mOfsSimTrace << "Reg W ";
      } 
      else 
      {
        mOfsSimTrace << "Reg R ";
      }

      mOfsSimTrace << rRegUp.regname << " val 0x" << hex << setfill('0') << setw(16) << rRegUp.rval << " mask 0x" << setfill('0') << setw(16) << rRegUp.mask << "\n";
    }
  }

  void SimAPI::PrintMemoryUpdates()
  {
    if(not mOfsSimTrace.is_open()) return;

    std::string _speculative_mode;

    for(const MemUpdate& rMemUp : mMemoryUpdates)
    {
      if(rMemUp.CpuID != uint32(-1)) // For negative number cpu ID's dont print status for speculative BNT. Negative CPUid indicates simulator non-isa backend is using MMU to configure test.
      {
	_speculative_mode = mSpecModeStrings[0];
	auto _spec_mode_lookup = mInSpeculativeMode.find(rMemUp.CpuID);
	if(_spec_mode_lookup != mInSpeculativeMode.end())
	{
	  _speculative_mode = mSpecModeStrings[_spec_mode_lookup->second];
	}
	mOfsSimTrace << "Cpu " << dec << rMemUp.CpuID << _speculative_mode;
      }

      if(rMemUp.access_type == std::string("write"))
      {
        mOfsSimTrace << "Mem W";
      } 
      else 
      {
        mOfsSimTrace << "Mem R";
      }

      mOfsSimTrace << " bank " << dec << rMemUp.mem_bank << " VA 0x" << hex << setfill('0') << setw(16) << rMemUp.virtual_address << " PA 0x" << setfill('0') << setw(16) << rMemUp.physical_address << " size " << rMemUp.size << " bytes 0x" << hex;

      for(unsigned char byte : rMemUp.bytes)
      {
        mOfsSimTrace << setfill('0') << setw(2) << uint64_t(byte);
      }

      mOfsSimTrace << "\n";
    }
  }

  void SimAPI::PrintMmuEvents()
  {
    if(not mOfsSimTrace.is_open()) return;

    for(const MmuEvent& rMmuEvent : mMmuEvents)
    {
      mOfsSimTrace << "MMU evt VA 0x" << hex << setfill('0') << setw(16) << rMmuEvent.va << " PA 0x" << hex << setfill('0') << setw(16) << rMmuEvent.pa << " type ";

      switch(rMmuEvent.type)
      {
          case Strong: mOfsSimTrace << " Strong "; break;
          case Device: mOfsSimTrace << " Device "; break;
          case Normal: mOfsSimTrace << " Normal "; break;
          default: mOfsSimTrace << " Unknown "; break;
      }

      mOfsSimTrace << "has-stage 2? " << (rMmuEvent.has_stage_two ? "yes" : "no") << "outer cache type/attributes: 0x" << hex << rMmuEvent.outer_type << "/" << rMmuEvent.outer_attrs;
      mOfsSimTrace << " inner cache type/attributes: 0x" << rMmuEvent.inner_type << "/" << rMmuEvent.inner_attrs << "\n";
    } 
  }

  void SimAPI::PrintExceptionUpdates()
  {
    if(not mOfsSimTrace.is_open()) return;

    for(const ExceptionUpdate& rExpUp : mExceptionUpdates)
    {
      mOfsSimTrace  << "Excpt ID 0x" << hex << rExpUp.mExceptionID << " attrs 0x" << rExpUp.mExceptionAttributes << " cmnts " << rExpUp.mComments << endl;
    }
  }

  void SimAPI::PrintInstructionStep(uint64 pc, uint32 CpuID, uint32 currentICount, const std::string& rOpcode, const std::string& rDisassembly)
  {
    if(not mOfsSimTrace.is_open()) return;

    size_t idx = 0;
    std::string _speculative_mode = mSpecModeStrings[0];
    auto _spec_mode_lookup = mInSpeculativeMode.find(CpuID);
    if(_spec_mode_lookup != mInSpeculativeMode.end())
    {
      _speculative_mode = mSpecModeStrings[_spec_mode_lookup->second];
    } 

    mOfsSimTrace << dec << "Cpu " << CpuID  << _speculative_mode << currentICount << " ----" << endl;
    mOfsSimTrace << dec << "Cpu " << CpuID << _speculative_mode << "PC(VA) 0x" << hex << setfill('0') << setw(16) << pc << " op: 0x" << hex << setfill('0') << setw(16) << stoull(rOpcode, &idx, 16) << " : " << rDisassembly << endl;
  }

  void SimAPI::PrintSummary() const
  {
    if(not mOfsSimTrace.is_open()) return;

    uint32 total_count = 0;
    string exit_msg = "SUCCESS";
    for (auto map_item : mThreadSummaries) {
      const ThreadSummary& thread_sum = map_item.second;
      mOfsSimTrace << dec << "Cpu " << map_item.first << " executed " << thread_sum.mInstructionCount << " instructions." << endl;
      total_count += thread_sum.mInstructionCount;
      if (thread_sum.mExitCode != 0) {
	exit_msg = thread_sum.mSummary;
	break;
      }
    }

    mOfsSimTrace << dec << "Executed " << total_count << " instructions, exit status (" << exit_msg << ")" << endl;
  }
  
  uint32 SimAPI::UpdateCurrentInstructionCount(uint32 CpuId)
  {
    ThreadSummary& thread_sum = mThreadSummaries[CpuId];
    uint32 current_icount = ++ (thread_sum.mInstructionCount);
    return current_icount;
  }

  //!< form 'cpuID' from cluster,core,thread...

  uint32 SimAPI::CpuID(uint32 socket, uint32 cluster, uint32 core, uint32 thread)
  {
    return (socket<<24) | (cluster<<16) | (core<<8) | thread;
  }

  //!< 'record' methods used by extern C 'update' methods...

  void SimAPI::RecordRegisterUpdate(uint32 CpuID,const char *pRegName,uint64 rval, uint64 mask, const char *pAccessType)
  {
    mRegisterUpdates.push_back(RegUpdate(CpuID, pRegName, rval, mask, pAccessType));
  }

  void SimAPI::RecordMemoryUpdate(uint32 CpuID, uint64 virtualAddress, uint32 memBank, uint64 physicalAddress, uint32 size, const char *pBytes, const char *pAccessType)
  {
    mMemoryUpdates.push_back(MemUpdate(CpuID, virtualAddress, memBank, physicalAddress, size, pBytes, pAccessType));
  }

  void SimAPI::RecordMmuEvent(MmuEvent *pEvent)
  {
    mMmuEvents.push_back(*pEvent);
  }

  void SimAPI::RecordVectorRegisterUpdate(uint32 CpuID, const char* pRegname, uint32 vecRegIndex, uint32 eltIndex, uint32 eltByteWidth, const uint8_t* pValue, uint32 byteLength, const char* pAccessType)
  {
    std::map<uint32, VectorElementUpdates>::iterator it = mVectorElementUpdates.find(vecRegIndex);
    if(it != mVectorElementUpdates.end())
    {
      it->second.insert(CpuID, pRegname, eltIndex, eltByteWidth, pValue, byteLength, pAccessType);
    }
    else{
      VectorElementUpdates temp(CpuID, mVecRegWidth, mVecPhysRegNames, mPhysRegSize, mNumPhysRegs); 
      temp.insert(CpuID, pRegname, eltIndex, eltByteWidth, pValue, byteLength, pAccessType);
      mVectorElementUpdates.insert(std::pair<uint32, VectorElementUpdates>(vecRegIndex, temp)); 
    }
  }

  void SimAPI::RecordTermination(uint32 CpuId, uint32 exitCode, const char* pMsg)
  {
     ThreadSummary& thread_sum = mThreadSummaries[CpuId];
     thread_sum.mExitCode = exitCode;
     thread_sum.mSummary = pMsg;
  }

  void SimAPI::SetVectorRegisterWidth(cuint64 vecRegWidthBits)
  {
    mVecRegWidth = vecRegWidthBits / 8;
    mNumPhysRegs = mVecRegWidth / mPhysRegSize;
    for (uint64 sub_index = 0; sub_index < mNumPhysRegs; sub_index++) {
      mVecPhysRegNames.push_back("_" + to_string(sub_index));
    }
  }

  //!< copy simulator step updates...

  bool SimAPI::GetRegisterUpdates(std::vector<RegUpdate> &rRegUpdates)
  {
    rRegUpdates = mRegisterUpdates;
    return mRegisterUpdates.size() > 0;
  }

  bool SimAPI::GetMemoryUpdates(std::vector<MemUpdate> &rMemUpdates)
  {
    rMemUpdates = mMemoryUpdates;
    return mMemoryUpdates.size() > 0;
  }

  bool SimAPI::GetMmuEvents(std::vector<MmuEvent> &rMmuEvents) {
    rMmuEvents = mMmuEvents;
    return mMmuEvents.size() > 0;
  }

  bool SimAPI::GetExceptionUpdates(std::vector<ExceptionUpdate> &rExceptUpdates)
  {
    rExceptUpdates = mExceptionUpdates;
    return mExceptionUpdates.size() > 0;
  }

  bool SimAPI::GetVectorRegisterUpdates(std::vector<RegUpdate> &rRegUpdates)
  {
    for(std::map<uint32, VectorElementUpdates>::iterator it = mVectorElementUpdates.begin(); it != mVectorElementUpdates.end(); ++it)
    {
      it->second.translateElementToRegisterUpdates(*this, rRegUpdates);
    }
    return mVectorElementUpdates.size() > 0;
  }

}


