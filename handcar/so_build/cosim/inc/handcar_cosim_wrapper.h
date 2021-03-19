//
// Copyright (C) [2020] Futurewei Technologies, Inc.
//
// FORCE-RISCV is licensed under the Apache License, Version 2.0 (the License);
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

#ifndef __WRAPPER_H__
#define __WRAPPER_H__
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
#include <string>
extern "C" {
#endif

// get_simulator_version function: write the simulator version into the supplied string buffer
//
// inputs:
//      char* version -- pointer to the string buffer for writing the version information, should be managed by the caller
// outputs:
//      char* version -- version information gets written to this buffer
// return:
//      0 -- success
//      1 -- something went wrong like the buffer was not large enough
int get_simulator_version(char* version);

// set_simulator_parameter function: set simulator value or path options before initialization
// inputs:
//     const char* name: string identifier of parameter to be modified
//     const uint64_t* value: pointer to the numeric value up to 64 bits, if any, that the parameter should be set to
//     const char* path: some parameters are filepaths, if this is the case that information is specified with this argument
//
// return values:
//     SUCCESS = 0,
//     NAME_NOT_FOUND = 1, (name was spelled incorrectly or option is not available)
//     VAL_IS_NULL = 2, (even though the option requires a value argument)
//     PATH_IS_NULL = 3, (even though the option requires a path / string argument)
//     NAME_IS_NULL = 4 (all options must be specified by name)
//     10 (Indicates that initialize_simulator was called before set_simulator_parameter, this isn't allowed)
//
// notes:
//     See handcar API test program source to understand how to sequence calls to set_simulator_parameter.
//
int set_simulator_parameter(const char* name, const uint64_t* value, const char* path);

// initialize_simulator function: Initial resource allocation and construction of functional simulator related objects.
//
// warning: must be called in order to use API
//
// inputs:
//     const char* options, space separated string where options are specified, follows the usage as in Spike
//
void initialize_simulator(const char* options);

// terminate_simulator function: free simulator resources.
//
// warning:
//     must be called manually when through with functional simulation
//
void terminate_simulator();

// simulator_load_elf function: load the contents of an elf file into the memory model, target_id is ignored for now
//
// warning:
//     MUST be called after simulator is initialized and before any calls step the simulator.
//
//     target_id is ignored. PMA / PMP per hart and interaction with memory model needs to be looked into especially before MP use.
//
//     Underlying Spike code will throw a runtime exception if elf_path cannot be resolved and opened.
//
// inputs:
//     int target_id, inert, placeholder for future functionality
//     const char* elf_path, host filepath where ELF file to load may be found
//
// returns:
//     0 means success,
//     1 means failure
//
int simulator_load_elf(int target_id, const char* elf_path);

// For development purposes only, will only dump sparse memory in the future when the dense model is no longer required for reference.
//
//
void dump_memory(const char* file_to_create);

// step_simulator function: step the simulation forward num_steps instructions on the target_id hart
//
// inputs:
//     int target_id, which hardware tread to step
//     int num_steps, how many instructions for the chosen hardware thread to perform
//     int stx_failed, unspecified requested api parameter, does nothing currently
//
// returns:
//     0 indicates the requested steps completed sucessfully,
//     1 indicates failure
//
int step_simulator(int target_id, int num_steps, int stx_failed);

// get_disassembly function: for the given pc address, populates the information in the preallocated opcode and disassemby string buffers.
//
// warning:
//     the caller has responsibility over allocating and managing the memory for the strings.
//
// inputs:
//     const uint64_t* pc, given for some reason as a constant pointer (as requested for the API), dereferenced to obtain the pc address
//     char** opcode, pointer to an allocated string buffer
//     char** disassembly, pointer to an allocated string buffer
//
//  outputs:
//      opcode, copys the opcode information as a hex string to opcode
//      disassembly, copys the disassembly text to disassembly
//
//  returns:
//      0 success,
//      2 could not complete because either no processors were instantiated or no allocated opcode or disassembly string buffers were provided
//
int get_disassembly(const uint64_t* pPc, char** pOpcode, char** pDisassembly);
int get_disassembly_for_target(int target_id, const uint64_t* pPc, char** pOpcode, char** pDisassembly);

// read_simulator_memory function: for the given target_id and physical address addr and length, writes the contents of the relevant memory address into the provided data buffer
//
// inputs:
//      int target_id -- refers to the processor id
//      const uint64_t* addr -- refers to the physical address
//      const int* length -- the number of bytes to read from memory and copy into data
//      uint8_t* -- buffer for memory data to be copied into, is expected to be managed by the caller
//
//  outputs:
//      uint8_t* data -- buffer where the memory data requested is copied into
//
//  returns:
//      0 -- success
//      1 -- one or more of the pointer arguments to the function are null
//      2 -- the data buffer is the wrong size for the requested read
//      3 -- other failure
int read_simulator_memory(int target_id, const uint64_t* addr, int length, uint8_t* data);

// write_simulator_memory function: for the given target_id and physical address addr and length, copy the provided data buffer into the memory model
//
// inputs:
//     int target_id -- refers to the processor id
//     const uint64_t* addr -- refers to the physical address
//     int length -- refers to the number of bytes to be copied from the data buffer into the memory model
//     const uint8_t* data -- the data buffer with contents to be copied into the memory model
//
//  side effects:
//     alters the state of the memory model by modifying or initializing its contents
//
//  returns:
//     0 -- success
//     1 -- one or more pointer arguments to the function call are null
//     2 -- the advertised length of the data buffer is negative or greater than 8
//     3 -- other failure
int write_simulator_memory(int target_id, const uint64_t* addr, int length, const uint8_t* data);

// This API was added to facilitate tests of the other APIs listed here.
int initialize_simulator_memory(int target_id, const uint64_t* addr, int length, uint64_t data);

// read_simulator_register function: for the given target_id, register category and register index supply the associated register_name, value and length in bytes
//
//  inputs:
//      int target_id -- the processor id
//      const char* pRegName -- should be an allocated pointer to a character buffer sufficient to hold any relevant register name
//      uint8_t* value -- the buffer for byte string representing the numeric value of the requested register. Management is the responsibility of the caller
//      int length -- the length of the buffer the user provides to the API
//
//  outputs:
//      uint8_t* value -- value is written with the numeric data stored in the requested register
//
//  returns:
//      0 -- success
//      1 -- one or more of the pointer arguments is null
//      2 -- target_id was outside the the expected range given the number of processors being simulated
//      3 -- the register category and index combination could not be mapped to a register
//      4 -- the advertised length of the value buffer was too short
//
int read_simulator_register(int target_id, const char* pRegName, uint8_t* value, int length);

//  inputs:
//
//  outputs:
//
//  returns:
//      0
int partial_read_large_register(int target_id, const char* pRegName, uint8_t* pValue, uint32_t length, uint32_t offset);

//  inputs:
//
//  outputs:
//
//  returns:
//      0
int partial_write_large_register(int target_id, const char* pRegName, const uint8_t* pValue, uint32_t length, uint32_t offset);

// read_simulator_register_fpix function: for the given target_id, register name supply the value and bit mask
//
//  inputs:
//      int target_id -- the processor id
//      const char* registerName -- name of the register for lookup in the simulator
//      uint64_t* value -- the buffer for byte string representing the numeric value of the requested register.
//      uint64_t* mask -- high bits are relevant parts of the returned value
//
//  outputs:
//      uint64_t* value -- the data contents fo the register
//      uint64_t* mask -- indicates which bits to pay attention to in the returned value result
//
//  returns:
//      0 -- success
//      1 -- one or more of the pointer arguments is null
//      2 -- the provided register name could not be mapped to a register in the simulator
//      3 -- other failure
//
int read_simulator_register_fpix(uint32_t target_id, const char* registerName, uint64_t* value, uint64_t* mask);

// write_simulator_register function: for the given target_id, pRegName write the supplied reg_data of length num bytes into the corresponding simulator register
//
//  inputs:
//      int target_id -- refers to the processor id
//      const char* pRegName -- refers to the registers architectural name (not the programmers name) and ordinal
//      const uint8_t* data -- the data buffer to be copied into the specified register
//      int length -- the number of bytes to be copied into the specified register
//
//  side effects:
//      modifies the value of the speficied register in the simulator
//
//  returns:
//      0 -- success
//      1 -- one or more of the pointer arguments to this function are null
//      2 -- the input data buffer is too small for the specified length
//      3 -- the provided register name could not be mapped to a register in the simulator
//      4 -- the advertised length of the data buffer did not match the length of the target register
//      5 -- readback with 'getter' method after write with 'setter' method shows a mismatch
//
int write_simulator_register(int target_id, const char* pRegName, const uint8_t* data, int length);

// write_simulator_register_fpix function: for the given target_id, pRegName write the supplied reg_data of length num bytes into the corresponding simulator register
//
//  inputs:
//      int target_id -- refers to the processor id
//      const char* registerName -- refers to the register's name as in Spike code, usually the programmer name
//      uint64_t value -- the buffer for byte string representing the numeric value of the requested register.
//      uint64_t mask -- high bits are relevant parts of the supplied value
//
//  side effects:
//      modifies the value of the speficied register in the simulator
//
//  returns:
//      0 -- success
//      1 -- one or more of the pointer arguments to this function are null
//      2 -- the provided register name could not be mapped to a register in the simulator
//      3 -- other failure
//
int write_simulator_register_fpix( uint32_t target_id, const char* registerName, uint64_t value, uint64_t mask);

bool inject_simulator_events(uint32_t cpuid, uint32_t events){return false;} //!< inject events into simulator

// translate_virtual_address function: given a target_id(procid), virtual address, intent attempt to translate that address into a physical address and load the relevant PMP information into memattrs
//
// meaning of 'intent':
//   0 - indicates a 'LOAD' access
//   1 - indicates a 'STORE' access
//   2 - indicates a 'FETCH' access
//
// returns:
//   0 - success
//   1 - some pointer arguments were null
//   2 - invalid procid
//   3 - PMP problem with PA after address translation somehow
//   4 - access exception while trying to check pmp status of page table entry PA
//   5 - walk was unsuccessful and access type was FETCH
//   6 - walk was unsuccessful and access type was LOAD
//   7 - walk was unsuccessful and access type was STORE
//   8 - walk was unsuccessful and access type was not any of the above
//
int translate_virtual_address(int target_id, const uint64_t* vaddr, int intent, uint64_t* paddr, uint64_t* memattrs);

//The following "update" method declarations are here to remind the user what callback methods are expected to be present in the code that uses the cosim API.
//In order to disable the requirement, just append "{}" before the ";" symbols.
//
// inputs:
//  uint32_t cpuid : the id of the core from which the register update is being received.
//  const char *pRegName : the name of the logical register in which the element belongs.
//  uint32_t vecRegIndex : the index of the vector register, somewhat redundant with name.
//  uint32_t eltByteWidth : the byte width of the element, important because this can change during runtime.
//  const uint8_t* pValue : whenever access type is read, this points to the entire data content of the whole vector register not just an element.
//  uint32_t byteLength : the number of bytes that pValue points to.
//  const char* pAccessType : should be either "read" or "write".
void update_vector_element(uint32_t cpuid, const char *pRegName, uint32_t vecRegIndex, uint32_t eltIndex, uint32_t eltByteWidth, const uint8_t* pValue, uint32_t  byteLength, const char* pAccessType);

#ifdef __cplusplus
};
#endif

#endif

