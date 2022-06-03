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
#ifndef Force_SimLoader_H
#define Force_SimLoader_H

#include <stddef.h>
#include <stdint.h>

// The Iss simulator is to be loaded dynamically. All function addresses can then be set...

struct SimDllApi {
  void *sim_lib;  //!<   pointer to simulator DLL

  //!< simulator function addresses to get once simulator is loaded:
  void (*initialize_simulator)(const char* options);
  int  (*set_simulator_parameter)(const char* name, const uint64_t* value, const char* path);
  void (*terminate_simulator)(void);
  int  (*get_simulator_version)(char *);
  int  (*get_disassembly)(const uint64_t* pPc, char** pOpcode, char** pDisassembly);
  int  (*get_disassembly_for_target)(int target_id, const uint64_t* pPc, char** pOpcode, char** pDisassembly);
  int  (*read_simulator_memory)(int target_id, const uint64_t* addr, int length, uint8_t* data);
  int  (*write_simulator_memory)(int target_id, const uint64_t* addr, int length, const uint8_t* data);
  int  (*read_simulator_register)(uint32_t target_id, const char* registerName, uint64_t* value, uint64_t* mask);
  int  (*partial_read_large_register)(int, const char*, uint8_t*, uint32_t, uint32_t);
  int  (*partial_write_large_register)(int, const char*, const uint8_t*, uint32_t, uint32_t);
  int  (*write_simulator_register)( uint32_t target_id, const char* registerName, uint64_t value, uint64_t mask);
  int  (*step_simulator)(int target_id, int num_steps, int stx_failed);
  bool (*inject_simulator_events)(uint32_t, uint32_t);

  SimDllApi() : sim_lib(NULL),initialize_simulator(NULL),terminate_simulator(NULL),
       get_simulator_version(NULL),get_disassembly(NULL),get_disassembly_for_target(NULL),read_simulator_memory(NULL),write_simulator_memory(NULL),
       read_simulator_register(NULL),partial_read_large_register(NULL),partial_write_large_register(NULL),write_simulator_register(NULL), step_simulator(NULL), inject_simulator_events(NULL) {};

  
  // other simulator functions as they become available...
};

//!< load the simulator shared object, get api function addresses:
int open_sim_dll(const char *sim_dll_path,struct SimDllApi *api_ptrs);
void close_sim_dll(struct SimDllApi *api_ptrs);

#endif
