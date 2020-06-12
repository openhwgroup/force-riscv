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
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <chrono>

#include <Log.h>
#include <elfio/elfio.h>
#include <Random.h>

#include <ConfigFPIX.h>
#include <SimUtils.h>
#include <SimAPI.h>

using namespace std;
using namespace ELFIO;

/*!
  \file SimUtils.cc
  \brief Code for ISS Scripting Interface simulator utilities class
*/

namespace Force {

  SimAPI* SimUtils::mspSimAPI = nullptr;

  //***************************************************************************************************************
  //!< load/initialize simulator shared object
  //***************************************************************************************************************

  SimUtils::~SimUtils()
  {
  }

  SimAPI * SimUtils::LoadSimulator(SimAPI* pSimAPI, ConfigFPIX *sim_cfg)
  {
    LOG(debug) << "   Loading simulator library...";

    mspSimAPI = pSimAPI;

    ApiSimConfig sim_dll_cfg(sim_cfg->ClusterCount(), /* # of clusters */
			     sim_cfg->NumberOfCores(), /* # cores */
			     sim_cfg->ThreadsPerCpu(), /* # threads */
			     sim_cfg->PhysicalAddressSize(), /* physical address size */
			     "./fpix_sim.log", /* simulator debug trace file */
            false
			     );

    mspSimAPI->InitializeIss(sim_dll_cfg, sim_cfg->SimulatorSharedObjectFile(),"" /* no api trace file */);

    LOG(debug) << "done." << endl;
    LOG(debug) << "      Simulator DLL: '" << sim_cfg->SimulatorSharedObjectFile() << "'." << endl;

    return mspSimAPI;
  }

//***************************************************************************************************************
//!< shutdown/close simulator shared object
//***************************************************************************************************************

void SimUtils::UnloadSimulator() {
  LOG(debug) << "   Shutting down simulator...";
  mspSimAPI->Terminate();
  delete mspSimAPI;
  mspSimAPI = nullptr;
  LOG(debug) << "done." << endl;
}

//***************************************************************************************************************
//!< load test image
//***************************************************************************************************************

bool SimUtils::LoadTest(uint64_t &entry_point, SimAPI *sim_ptr, vector<string> *test_files) {
  bool success = true;

  entry_point = (uint64_t) -1;

  for (vector<string>::iterator i = test_files->begin(); i != test_files->end() && success; i++) {
    string next_test_file = *i;
    uint64_t next_entry_point = 0;

    if (next_test_file.find(".Secondary.ELF") != string::npos) {
      success = LoadTestFromELF(next_entry_point, sim_ptr, next_test_file, true);
    } 
    else if (next_test_file.find(".Default.ELF") != string::npos) {
      success = LoadTestFromELF(next_entry_point,sim_ptr, next_test_file);
    } 
    else {
      success = LoadTestFromELF(next_entry_point,sim_ptr, next_test_file); //!< will ASSUME test is default...
    } 

    if (success) {
      if (entry_point == (uint64_t) -1)
        entry_point = next_entry_point;
      else if (entry_point != next_entry_point) { //!< entry points recorded in each elf-file should be the same, nes pa?
        cerr << "ERROR: test ELF-file: " << next_test_file << " does not have a defined entry-point" << endl;
        success = false;
      }
    }
  } 

  if (success)
    LOG(info) << "   Test entry point: 0x" << hex << entry_point << dec << endl;

  return success;
}

//***************************************************************************************************************
//!< load from a single ELF file, return entry-point
//***************************************************************************************************************

bool SimUtils::LoadTestFromELF(uint64_t &entry_point, SimAPI *sim_ptr, string &elf_file, bool is_secondary) {
  LOG(debug) << "   Loading test from ELF file '" << elf_file << ", sim_ptr? " << hex << sim_ptr << dec << "'..." << endl;

  elfio reader;

  if (!reader.load(elf_file)) {
    LOG(fail) << "Problems loading elf-file: " << elf_file << endl;
    return false;
  }

  entry_point = reader.get_entry();

  LOG(debug) << "   Entry point: 0x" << hex << entry_point << dec << endl;

  // don't care just yet about endianness...
  //bool mBigEndian = (reader.get_encoding() == ELFDATA2MSB) ? true : false;

  Elf_Half sectNum = reader.sections.size();

  uint32_t mem_bank = is_secondary ? 1 : 0;

  for (auto i = 0; i < sectNum; i++) {
     const section *pSection = reader.sections[i];

     Elf_Word type     = pSection->get_type();
     Elf_Xword flags   = pSection->get_flags();
     const char *pData = pSection->get_data();
     uint32 dsize      = pSection->get_size();
     uint64 daddress   = pSection->get_address();
     string pName      = pSection->get_name();

     bool is_instr     = flags & SHF_EXECINSTR;
     bool is_data      = flags & SHF_WRITE;

     if (type != SHT_PROGBITS)  //!< anything else is symbol table, or debug related...
       continue;

     sim_ptr->WritePhysicalMemory(mem_bank, daddress, dsize, (unsigned char *) pData);
     
     /*
     printf("0x%016llx,size: %d, data:",daddress, dsize);
     for (unsigned int i = 0; i < dsize; i++) {
       printf(" 0x%02x", (unsigned char) pData[i]);
     }
     printf("\n");
     */

     if (!is_instr && !is_data) {
       //LOG(fail) << "Unknown section type encountered in ELF file." << endl;
       printf("WARNING: LoadTestFromELF, Unknown section type encountered in ELF file, named: %s with type: %d and flags: %lu \n", pSection->get_name().c_str(), type, flags); 
       //return false;
     }
  }


  LOG(debug) << "done." << endl; 

  return true;
}

//***************************************************************************************************************
//!< load Force-generated test 'markers'
//***************************************************************************************************************

bool SimUtils::LoadTestMarkers() {
  return true;
}

//***************************************************************************************************************
//!< initialize vector of sim-thread IDs
//***************************************************************************************************************
                              
void SimUtils::AssignCpuIDs(vector<uint32_t> &cpuIDs, uint32_t num_chips,uint32_t num_cores,uint32_t num_threads) {
  LOG(debug) << "# chips: " << num_chips << ", # cores: " << num_cores << ", # threads: " << num_threads << endl;

  for (uint32_t iChip = 0; iChip < num_chips; iChip++) {
    for (uint32_t iCore = 0; iCore < num_cores; iCore++) {
      for (uint32_t iThread = 0; iThread < num_threads; iThread++) {
         uint32_t thread_id = (iChip * num_cores * num_threads + iCore * num_threads + iThread); 
         cpuIDs.push_back(thread_id);
         LOG(debug) << "for cluster/core/thread " << iChip << "/" << iCore << "/" << iThread 
                    << " next cpuID is " << dec << thread_id << endl;
      }
    }
  }
}                    

//***************************************************************************************************************
//!< from a set of SimAPI returned register updates, look for a register update by name and access-type
//!< Use 'read' or 'write' for the accessType argument...
//***************************************************************************************************************

bool SimUtils::GetRegisterUpdate(uint64_t &rval, std::vector<RegUpdate> &reg_updates, const string &rname, const string &accessType) {
  bool haveit = false;

  for (std::vector<RegUpdate>::iterator i = reg_updates.begin(); i != reg_updates.end(); i++) {
    if ( ((*i).regname == rname) && ((*i).access_type == accessType) ) {
      rval = (*i).rval;
      haveit = true;
    }
  }

  return haveit;
}

//***************************************************************************************************************
// some debug methods...
//***************************************************************************************************************

//!< Dump contents of register update struct to stdout

void SimUtils::DumpRegisterUpdate(RegUpdate src) const {
  LOG(debug) << "  [register-update] cpuID: " << src.CpuID << ", name: " << src.regname << ",value: 0x" << hex << src.rval 
             << ", mask: 0x" << src.mask << dec << ", access-type: " << src.access_type << endl;
}  

//!< Dump set of register updates to stdout
                    
void SimUtils::DumpRegisterUpdates(std::vector<RegUpdate> reg_updates) const {
  for (vector<RegUpdate>::iterator i = reg_updates.begin(); i != reg_updates.end(); i++) {
    DumpRegisterUpdate(*i);
  }
}
 
//!< Dump contents of memory update struct to stdout
                                  
void SimUtils::DumpMemoryUpdate(MemUpdate src) const {
  LOG(debug) << "  [memory-update] bank: " << src.mem_bank << " VA: 0x" << hex << src.virtual_address 
       << ", PA: 0x" << src.physical_address << dec << ", size: " << src.size 
       << ", access-type: " << src.access_type << " bytes:";
  for (auto i_iter = src.bytes.begin(); i_iter != src.bytes.end(); i_iter++) {
     LOG(debug) << " " << *i_iter;
  }
  LOG(debug) << endl;
}  

//!< Dump set of memory updates to stdout
                                                                
void SimUtils::DumpMemoryUpdates(std::vector<MemUpdate> mem_updates) const {
  for (vector<MemUpdate>::iterator i = mem_updates.begin(); i != mem_updates.end(); i++) {
    DumpMemoryUpdate(*i);
  }
}

//!< Dump trace-info of instruction just stepped

void SimUtils::DumpInstructionTrace(uint32_t cpuID, uint64_t PC, int instr_num) {
  //  printf("c%02u %d %016llx: deadbeef ?\n",cpuID,instr_num,(unsigned long long) PC); // always display, same as iss
  printf("c%02u %d %016llx\n",cpuID,instr_num,(unsigned long long) PC); // opcode, disassembly TBD
}

}
