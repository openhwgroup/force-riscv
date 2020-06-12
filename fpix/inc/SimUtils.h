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
#ifndef Fpix_SimUtils_H
#define Fpix_SimUtils_H

#include <string>
#include <vector>
#include <map>

#include <ConfigFPIX.h>
#include <SimAPI.h>

using namespace std;

namespace Force {

class SimAPI;
  
class SimUtils {
 public:
  SimUtils() {};
  ~SimUtils();

  SimAPI *LoadSimulator(SimAPI* pSimAPI, ConfigFPIX *sim_cfg);                                                                          //!< load/initialize simulator shared object
  void UnloadSimulator();                                                                                              //!< shutdown/close simulator shared object

  static bool LoadTest(uint64_t &entry_point, SimAPI *sim_ptr, std::vector<std::string> *test_files);                            //!< load test image

  static bool LoadTestMarkers();                                                                                       //!< load Force-generated test 'markers'

  static void AssignCpuIDs(std::vector<uint32_t> &cpu_ids, uint32_t num_clusters,uint32_t num_cores,uint32_t num_threads);  //!< initialize vector of sim-thread IDs

  static bool GetRegisterUpdate(uint64_t &rval, std::vector<RegUpdate> &reg_updates, const std::string &rname, const std::string &accessType);        //!< return single reg update from reg updates list

  static void DumpInstructionTrace(uint32_t cpuID, uint64_t PC, int instr_num);                                         //!< Dump trace-info of instruction just stepped

  ASSIGNMENT_OPERATOR_ABSENT(SimUtils);
  COPY_CONSTRUCTOR_ABSENT(SimUtils);
  
 protected:
  void DumpRegisterUpdate(RegUpdate src) const;                                                                        //!< Dump contents of register update struct to stdout
  void DumpRegisterUpdates(std::vector<RegUpdate> reg_updates) const;                                                  //!< Dump set of register updates to stdout
  void DumpMemoryUpdate(MemUpdate src) const;                                                                          //!< Dump contents of memory update struct to stdout
  void DumpMemoryUpdates(std::vector<MemUpdate> mem_updates) const;                                                    //!< Dump set of memory updates to stdout

 private:
  static bool LoadTestFromELF(uint64_t &entry_point, SimAPI *sim_ptr, std::string &elf_file, bool is_secondary = false);   //!< load from a single ELF file, return entry-point
  static SimAPI* mspSimAPI; //!< Pointer to SimAPI object.
};

}

#endif
