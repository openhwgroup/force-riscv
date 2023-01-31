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

#ifndef CosimTest_SimLoader_H
#define CosimTest_SimLoader_H

#include <stddef.h>
#include <stdint.h>

// The Handcar simulator is to be loaded dynamically. All function addresses can then be set...

struct SimDllApi {
  void *sim_lib;  //!<   pointer to simulator DLL

  //!< simulator function addresses to get once simulator is loaded:
  int (*set_simulator_parameter)(const char*, const uint64_t*, const char*);
  void (*initialize_simulator)(const char*);
  void (*terminate_simulator)();
  int (*simulator_load_elf)(int, const char*);
  int (*step_simulator)(int, int, int);
  int (*get_disassembly)(const uint64_t*, char**, char**);
  int (*get_simulator_version)(char*);
  int (*read_simulator_register)(int, const char*, uint8_t*, int);
  int (*partial_read_large_register)(int, const char*, uint8_t*, uint32_t, uint32_t);
  int (*partial_write_large_register)(int, const char*, const uint8_t*, uint32_t, uint32_t);
  int (*write_simulator_register)(int, const char*, const uint8_t*, int);
  int (*read_simulator_memory)(int, const uint64_t*, int, uint8_t*);
  int (*write_simulator_memory)(int, const uint64_t*, int, const uint8_t*);
  int (*translate_virtual_address)(int, const uint64_t*, int, uint64_t*, uint64_t*);
  int (*initialize_simulator_memory)(int, const uint64_t*, int, uint64_t);

  SimDllApi() : 
    sim_lib(NULL),
    initialize_simulator(NULL),
    terminate_simulator(NULL),
    set_simulator_parameter(NULL),
    simulator_load_elf(NULL),
    step_simulator(NULL),
    get_disassembly(NULL),
    get_simulator_version(NULL),
    read_simulator_register(NULL),
    partial_read_large_register(NULL),
    partial_write_large_register(NULL),
    write_simulator_register(NULL),
    read_simulator_memory(NULL),
    write_simulator_memory(NULL),
    translate_virtual_address(NULL),
    initialize_simulator_memory(NULL)
    {};
  
  // other simulator functions as they become available...
};

//!< load the simulator shared object, get api function addresses:
int open_sim_dll(const char *sim_dll_path,struct SimDllApi *api_ptrs);
void close_sim_dll(struct SimDllApi *api_ptrs);

#endif
