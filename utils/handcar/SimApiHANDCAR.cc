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
#include <SimApiHANDCAR.h>
#include <GenException.h>

PICKY_IGNORE_BLOCK_START
#include <SimLoader.h>
PICKY_IGNORE_BLOCK_END

#include <cstring>
#include <iostream>

/*!
  \file SimApiHANDCAR.cc
  \brief Code supporting SimApiHANDCAR class that connects HANDCAR APIs with the FORCE C++ code.
*/

using namespace std;

namespace Force {

  static SimApiHANDCAR *spSimApiHandle = nullptr; //!< pointer to singletion SimApiHANDCAR object

}


//!< simulator calls these C functions:
extern "C" {

  Force::SimAPI * instantiate_api()
  {
    return new Force::SimApiHANDCAR();
  }

  void update_generator_register(uint32_t cpuid, const char *pRegName, uint64_t rval, uint64_t mask, const char *pAccessType)
  {
    //Force demands that fp register names contain implementation specific physical register suffixes. So, just tack on '_0' to fp register names
    //if we can assume that the current design is limited to a max precision of double precision floating point instructions.
    std::string reg_name_copy(pRegName);
    if(pRegName[0] == 'f' && pRegName[1] < (char)(58))
    {
      reg_name_copy += std::string("_0");
    }
    
    Force::spSimApiHandle->RecordRegisterUpdate(cpuid, reg_name_copy.c_str(), rval, mask, pAccessType);
  }

  void update_generator_memory(uint64_t virtualAddress, uint32_t memBank, uint64_t physicalAddress, uint32_t size, const char *pBytes, const char *pAccessType)
  {
    Force::spSimApiHandle->RecordMemoryUpdate(virtualAddress, memBank, physicalAddress, size, pBytes, pAccessType);
  }

  void update_mmu_event(Force::MmuEvent *event)
  {
    Force::spSimApiHandle->RecordMmuEvent(event);
  }

  struct SimException {
    SimException() : mExceptionID(0), mExceptionAttributes(0), mpComments(""), mEPC(0) {}
    SimException(uint32_t exceptionID, uint32_t exceptionAttributes, const char* comments, uint64_t epc) :
      mExceptionID(exceptionID), mExceptionAttributes(exceptionAttributes), mpComments(comments), mEPC(epc) {}
    uint32_t mExceptionID; //!< 0x4E: eret. Others are scause or mcause values.  
    uint32_t  mExceptionAttributes;  //!< copied from tval.
    const char* mpComments; //!<  exception comments, indicate exit / enter and m versus s mode.
    uint64_t mEPC; //!< exception program counter
  };

  void update_exception_event(const SimException *exception)
  {
    Force::spSimApiHandle->RecordExceptionUpdate(exception);
  }

  // inputs:
  //  uint32_t cpuid : the id of the core from which the register update is being received.
  //  const char *pRegName : the name of the logical register in which the element belongs.
  //  uint32_t vecRegIndex : the index of the vector register, somewhat redundant with name.
  //  uint32_t eltByteWidth : the byte width of the element, important because this can change during runtime.
  //  const uint8_t* pValue : whenever access type is read, this points to the entire data content of the whole vector register not just an element.
  //  uint32_t byteLength : the number of bytes that pValue points to.
  //  const char* pAccessType : should be either "read" or "write".
  void update_vector_element(uint32_t cpuid, const char *pRegName, uint32_t vecRegIndex, uint32_t eltIndex, uint32_t eltByteWidth, const uint8_t* pValue, uint32_t  byteLength, const char* pAccessType)
  {
    Force::spSimApiHandle->RecordVectorRegisterUpdate(cpuid, pRegName, vecRegIndex, eltIndex, eltByteWidth, pValue, byteLength, pAccessType);
  }
  
};

using namespace std;

namespace Force {

  SimApiHANDCAR::SimApiHANDCAR()
    : SimAPI(), mpSimDllAPI(nullptr)
  {
    mpSimDllAPI = new SimDllApi();
  }

  SimApiHANDCAR::~SimApiHANDCAR()
  {
    delete mpSimDllAPI;
  }

    //!< Initialize simulator shared object using standard Force/HANDCAR parameters...

  void SimApiHANDCAR::InitializeIss(const ApiSimConfig& rConfig, const string &rSimSoFile, const string& rApiTraceFile)
  {
    spSimApiHandle = this;
 
    OpenApiTrace(rApiTraceFile);

    if (mOfsApiTrace.is_open()) {
      mOfsApiTrace << "#include <stdio.h>\n"
		   << "#include <stdlib.h>\n\n"
		   << "#include <dlfcn.h>\n\n"
		   << "#include \"handcar_cosim_wrapper.h\"\n\n"
		   << "#include \"SimLoader.h\"\n\n"
		   << "int main(int argc,char **argv) {\n\n"
		   << "  SimDllApi sim_api;\n\n"
		   << "  if (open_sim_dll(\"" << rSimSoFile << "\", &sim_api)) {\n"
		   << "    printf(\"Program aborted due to sim errors.\n\");\n"
		   << "    return -1;\n"
		   << "  }\n\n"
		   << std::endl;
    }

    if(rConfig.mUseTraceFile) {
      OpenOfs(rConfig.mpTraceFile, mOfsSimTrace);
    }

    if (open_sim_dll(rSimSoFile.c_str(), mpSimDllAPI)) {
      stringstream err_stream;
      err_stream << "Simulator shared library failed to load\n";
      throw SimulationError(err_stream.str());
    }

    if (mOfsApiTrace.is_open()) {
      mOfsApiTrace << "sim_api.initialize_simulator(\"-l\");\n"
		   << std::endl;
    }

    SetVectorRegisterWidth(rConfig.mVectorRegLen);

    stringstream varch;
    varch << "--varch=vlen:" << rConfig.mVectorRegLen << ",slen:" << rConfig.mVectorRegLen << ",elen:" << rConfig.mVectorRegLen;
    string varch_str = varch.str();

    mpSimDllAPI->initialize_simulator(varch_str.c_str());
  }

  //!< write out simulation trace file (if any), send 'terminate' directive to simulator, close sim dll...

  void SimApiHANDCAR::Terminate()
  {
    PrintSummary();

    if (mOfsApiTrace.is_open()) {
      mOfsApiTrace << "  sim_api.terminate_simulator();\n"
		   << "  close_sim_dll(&sim_api);\n"
		   << "  return(0);\n}\n" << std::endl;
    }
    CloseApiTrace();

    mpSimDllAPI->terminate_simulator();
    close_sim_dll(mpSimDllAPI);
    spSimApiHandle = nullptr;
  }

  //!< return simulator version info as String...

  void SimApiHANDCAR::GetSimulatorVersion(std::string &sim_version) {
    if (mOfsApiTrace.is_open()) {
      mOfsApiTrace << "{\n"
		   << "char tbuf[1024];\n"
		   << "strcpy(tbuf,\"?\");\n"
		   << "sim_api.get_simulator_version(tbuf);\n"
		   << "printf(\"Simulator version: '%s'\n\",tbuf);\n"
		   << "}" << std::endl;
    }
    char tbuf[1024];
    strcpy(tbuf,"?");

    mpSimDllAPI->get_simulator_version(tbuf);
    sim_version.assign(tbuf);
  }

  //!< obtain the opcode and dissassembly that corresponds to a given PC address
  
  void SimApiHANDCAR::GetDisassembly(const uint64_t* pPc, std::string& rOpcode, std::string& rDisassembly)
  {
    char* op = new char[128];
    char* dis = new char[128];
    memset(op, '0', 128);
    memset(dis,'0', 128);
    op[127]='\0';
    dis[127]='\0';
    char** opp = &op;
    char** disp = &dis;

    int rcode = mpSimDllAPI->get_disassembly(pPc, opp, disp);

    if (rcode == 1) {
      // A page fault on a branch could result in an exception. The disassembly call in the simulator
      // will fail since there is no mapping for the fault address. The expected return code in
      // this case is 1...
      strcpy(op,"00000000");
      strcpy(dis,"?");
    } else if (rcode != 0) {
      // any other non-zero return code is NOT expected...
      stringstream err_stream;
      err_stream << "Problems getting disassembly" << "\n";
      throw SimulationError(err_stream.str());
    }

    rOpcode = op;
    rDisassembly = dis;

    delete[] op;
    delete[] dis;
  }

  //!< read simulator physical memory. Return 0 if no errors...

  void SimApiHANDCAR::ReadPhysicalMemory(uint32 memBank, uint64 address, uint32 size, unsigned char *pBytes)
  {
    if (mOfsApiTrace.is_open()) {
      char tbuf[1024];
      sprintf(tbuf,"  int rcode = sim_api.read_simulator_memory(%d,0x%llx,%d,tbuf);\n",
              memBank,(unsigned long long) address,size);
      mOfsApiTrace << "{\n"
		   << "  char tbuf[size + 1];\n"
		   << tbuf << "\n";
      ApiTraceCheckRcode("read_simulator_memory");
      mOfsApiTrace << "  printf(\"read_simulator_memory, bytes read: \");\n"
		   << "  tbuf[0] = '\0';\n"
		   << "  unsigned char tb[3];"
		   << "  for (int i = 0; i < size; i++) {"
		   << "     sprintf(tb,\" 0x%02x\",tbuf[i]);\n"
		   << "     strcat(tbuf,tb);\n"
		   << "  }\n"
		   << "  tbuf\n"
		   << "}" << std::endl;
    }

    if (0 != mpSimDllAPI->read_simulator_memory(memBank, (const uint64_t*)&address, size, (uint8_t *) pBytes)) {
      stringstream err_stream;
      err_stream << "Problems reading from simulator memory. PA: 0x" << hex << address
                 << dec << ", # of bytes: " << size << "\n";
      throw SimulationError(err_stream.str());
    }
  }

  //!< write simulator physical memory. Return 0 if no errors...

  void SimApiHANDCAR::WritePhysicalMemory(uint32 memBank, uint64 address, uint32 size, const unsigned char *pBytes) {
    if (mOfsApiTrace.is_open()) {
      char tbuf[1024];
      for (unsigned int i = 0; i < size; i++) {
         sprintf(tbuf," 0x%02x",pBytes[i]);
         if ( (size > 1) && (i < (size - 1)) )
           strcat(tbuf,",");
      }
      mOfsApiTrace << "{\n"
		   << "  char tbuf = { " 
		   << tbuf 
		   << "  };\n";
      sprintf(tbuf,"sim_api.write_simulator_memory(%d,0x%llx,%d,tbuf);",memBank,(unsigned long long) address,size);
      mOfsApiTrace << tbuf
		   << "}" << std::endl;
    }

    int error_code = mpSimDllAPI->write_simulator_memory(memBank, (const uint64_t*)&address, size, (const uint8_t *) pBytes);
    //if (mpSimDllAPI->write_simulator_memory(memBank, (const uint64_t*)&address, size, (const uint8_t *) pBytes)) {
    if(error_code != 0){
      stringstream err_stream;
      err_stream << dec << "error code: " << error_code << " Problems writing to simulator memory. PA: 0x" << hex << address
                 << dec << ", # of bytes: " << size << "\n";
      throw SimulationError(err_stream.str());
    }

  }

  //!< read register value...

  void SimApiHANDCAR::ReadRegister(uint32 CpuID, const char *regname, uint64 *rval, uint64 *pRegMask)
  {
    if (mOfsApiTrace.is_open()) {
      char tbuf[1024];
      sprintf(tbuf,"  sim_api.read_simulator_register(0x%x,\"%s\",&rval,&rmask);\n",CpuID,regname);
      mOfsApiTrace << "{\n"
		   << " uint64_t rval,rmask;\n" 
		   << tbuf;
      sprintf(tbuf,"  printf(\"  %s",regname);
      mOfsApiTrace << tbuf
		   << " value/mask: 0x\%llx/0x\%llx\n\",(unsigned long long) rval,(unsigned long long) rmask);\n"
		   << "};" << std::endl;
    }

    //Remove any suffix the generator has attached to the register name, the simulator doesn't use the suffix.
    int errorcode = 0;
    char* p_edited_regname = nullptr;
    const char* p_fp_reg_f = strstr(regname, "f");
    const char* p_phys_reg_suffix = strstr(regname, "_"); 

    if(p_fp_reg_f != nullptr && p_phys_reg_suffix != nullptr)
    {

      ptrdiff_t arch_regname_length = p_phys_reg_suffix - p_fp_reg_f;
      p_edited_regname = new char[arch_regname_length+1];
      p_edited_regname[arch_regname_length] = '\0';

      memcpy(p_edited_regname, regname, arch_regname_length);

      errorcode = mpSimDllAPI->read_simulator_register(CpuID, p_edited_regname, (uint64_t *)rval, (uint64_t *)pRegMask);
      delete[] p_edited_regname;
    }
    else
    {
      errorcode = mpSimDllAPI->read_simulator_register(CpuID, regname, (uint64_t *)rval, (uint64_t *)pRegMask);
    }

    //std::cout << "SimApiHANDCAR::ReadRegister(...) : " << regname << " val: " << std::hex << rval << std::endl;

    if (0 != errorcode) {
      stringstream err_stream;
      err_stream << "Problems reading simulator register. CPU ID: " << hex << CpuID << dec
                 << ", register: '" << regname << "', mask: 0x" << hex << pRegMask << dec << "\n";
      throw SimulationError(err_stream.str());
    }
  }

  //!< read the bytes corresponding to a physical register from a large logical register without forcing the simulator to use Force's naming convention.
  
  void SimApiHANDCAR::PartialReadLargeRegister(uint32 CpuID, const char* pRegname, uint8_t* pBytes, uint32 length, uint32 offset)
  {
    if (mOfsApiTrace.is_open()) {
      char tbuf[1024];
      sprintf(tbuf,"  sim_api.partial_read_large_register(0x%x,\"%s\",&bytes,length,offset);\n",CpuID,pRegname);
      mOfsApiTrace << "{\n"
           << " uint8_t* pBytes;\n"
           << tbuf;
      sprintf(tbuf,"  printf(\"  %s",pRegname);
    }
 
    if (0 != mpSimDllAPI->partial_read_large_register(CpuID, pRegname, pBytes, (uint32_t)length, (uint32_t)offset)) {
      stringstream err_stream;
      err_stream << "Problems reading simulator register. CPU ID: " << hex << CpuID  << dec
                 << ", register: '" << pRegname << "\n";
      throw SimulationError(err_stream.str());
    }
  }

  //!< write the bytes corresponding to a physical register from a large logical register without forcing the simulator to use Force's naming convention.
  
  void SimApiHANDCAR::PartialWriteLargeRegister(uint32 CpuID, const char* pRegname, const uint8_t* pBytes, uint32 length, uint32 offset)
  {
    if (mOfsApiTrace.is_open()) {
      char tbuf[1024];
      sprintf(tbuf,"  sim_api.partial_write_large_register(0x%x,\"%s\",&bytes,length,offset);\n",CpuID,pRegname);
      mOfsApiTrace << "{\n"
           << " uint8_t* pBytes;\n"
           << tbuf;
      sprintf(tbuf,"  printf(\"  %s",pRegname);
    }
 
    if (0 != mpSimDllAPI->partial_write_large_register(CpuID, pRegname, pBytes, (uint32_t)length, (uint32_t)offset)) {
      stringstream err_stream;
      err_stream << "Problems writing simulator register. CPU ID: " << hex << CpuID << dec
                 << ", register: '" << pRegname << "\n";
      throw SimulationError(err_stream.str());
    }
  }

  //!< write simulator register. mask indicates which bits to write...

  void SimApiHANDCAR::WriteRegister(uint32 CpuID, const char *regname, uint64 rval, uint64 rmask)
  {
    if (mOfsApiTrace.is_open()) {
      char tbuf[1024];
      sprintf(tbuf,"  sim_api.write_simulator_register(0x%x,\"%s\", 0x%llx, 0x%llx);\n",
              CpuID,regname,(unsigned long long) rval,(unsigned long long) rmask);
      mOfsApiTrace << tbuf << std::endl;
    }

    //Remove any suffix the generator has attached to the register name, the simulator doesn't use the suffix.
    // TODO make this available for Quad FP
    int errorcode = 0;
    string reg_name = regname;
    uint64 underscore_pos = reg_name.find("_");
    string reg_name_no_suffix = reg_name.substr(0, underscore_pos);
    if (underscore_pos == string::npos) {
      errorcode = mpSimDllAPI->write_simulator_register(CpuID, reg_name_no_suffix.c_str(), rval, rmask);
    }
    else if (reg_name[0] == 'f') {
      errorcode = mpSimDllAPI->write_simulator_register(CpuID, reg_name_no_suffix.c_str(), rval, rmask);
    }
    else if (reg_name[0] == 'v') {
      uint32 phys_reg_sub_index = stoul(reg_name.substr(underscore_pos + 1));
      errorcode = mpSimDllAPI->partial_write_large_register(CpuID, reg_name_no_suffix.c_str(), reinterpret_cast<const uint8_t*>(&rval), sizeof(rval), phys_reg_sub_index * sizeof(rval));
    }
    else {
      errorcode = -1;
    }

    //std::cout << "SimApiHANDCAR::WriteRegister(...) : " << regname << " val: " << std::hex << rval << std::endl;

    if (0 != errorcode) {
      stringstream err_stream;
      err_stream << "Error code: " << errorcode << ", Problems writing simulator register. CPU ID: " << hex << CpuID
                 << ", register: '" << regname << "', value: 0x" << rval
                 << ", mask: 0x" << rmask << dec << "\n";
      throw SimulationError(err_stream.str());
    }
  }

  //!< request simulator to 'inject event(s)...

  void SimApiHANDCAR::InjectEvents(uint32 CpuID, uint32 events)
  {
    if (mOfsApiTrace.is_open()) {
      char tbuf[1024];
      sprintf(tbuf,"inject_events(0x%x,0x%x);\n",CpuID, events);
      mOfsApiTrace << tbuf << std::endl;
    }

    if (!mpSimDllAPI->inject_simulator_events(CpuID, events)) {
      stringstream err_stream;
      err_stream << "Problems injecting simulator events. CPU ID: " << hex << CpuID 
                 << ", events: 0x" << events << "\n"; 
            throw SimulationError(err_stream.str());
    }
  }

  //!< standard step method...

  void SimApiHANDCAR::Step(uint32 cpuid, vector<RegUpdate> &rRegUpdates, vector<MemUpdate> &rMemUpdates, vector<MmuEvent> &rMmuEvents, vector<ExceptionUpdate> &rExceptUpdates)
  {
    // clear updates from previous step...
    mRegisterUpdates.erase(mRegisterUpdates.begin(),mRegisterUpdates.end());
    mMemoryUpdates.erase(mMemoryUpdates.begin(),mMemoryUpdates.end());
    mMmuEvents.erase(mMmuEvents.begin(),mMmuEvents.end());
    mExceptionUpdates.erase(mExceptionUpdates.begin(), mExceptionUpdates.end());
    mVectorElementUpdates.erase(mVectorElementUpdates.begin(), mVectorElementUpdates.end());

    // get the disassembly
    uint64 rval = 0ull;
    uint64 rmask = 0ull;
    ReadRegister(cpuid, "PC", &rval, &rmask);
    uint64_t orval = rval;
    std::string opcode;
    std::string disassembly;
    GetDisassembly(&orval, opcode, disassembly);

    ReadRegister(cpuid, "PC", &rval, &rmask);

    // step the simulator; simulator returns after all updates recorded...
    if (0 != mpSimDllAPI->step_simulator((int) cpuid, 1, 0)) {
      stringstream err_stream;
      err_stream << "Simulator 'step_simulator' failed.\n";
      throw SimulationError(err_stream.str());
    }

    //Bundle the vector register updates in with the regular register updates
    GetVectorRegisterUpdates(mRegisterUpdates);

    // print the step and updates information
    uint32 icount = UpdateCurrentInstructionCount(cpuid);
    PrintInstructionStep(rval, cpuid, icount, opcode, disassembly);
    PrintRegisterUpdates();
    PrintMemoryUpdates();
    PrintMmuEvents();
    PrintExceptionUpdates();

    // retreive simulator step updates...
    GetRegisterUpdates(rRegUpdates);
    GetMemoryUpdates(rMemUpdates);
    GetMmuEvents(rMmuEvents);
    GetExceptionUpdates(rExceptUpdates);
  }
  
  void SimApiHANDCAR::RecordExceptionUpdate(const SimException *pException)
  {
    mExceptionUpdates.push_back(ExceptionUpdate(pException->mExceptionID, pException->mExceptionAttributes, pException->mpComments));
  }
  
  void SimApiHANDCAR::WakeUp(uint32 cpuId)
  {
    // TODO
  }

  void SimApiHANDCAR::TurnOn(uint32 cpuId)
  {
    // TODO
  }

  void SimApiHANDCAR::EnterSpeculativeMode(uint32 cpuId)
  {
    mInSpeculativeMode[cpuId] = true;  
  }

  void SimApiHANDCAR::LeaveSpeculativeMode(uint32 cpuId)
  {
    mInSpeculativeMode[cpuId] = false;
  }
  
}
