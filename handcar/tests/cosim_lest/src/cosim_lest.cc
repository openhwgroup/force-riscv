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

#include <lest/lest.hpp>
//#include <Log.h>
#include <map>
#include <cstring>

//------------------------------------------------
// include necessary header files here
//------------------------------------------------
#include "SimLoader.h"
#include <iostream>

//using text = std::string;
//using namespace Force;

std::vector<uint8_t> global_buffer(32);


const lest::test specification[] = {

CASE("Test 0, basics") {

  SETUP("Load SimDllApi Object")  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    // load the simulator DLL and pointers to API functions...
    SimDllApi sim_api;
    std::string options = "";

    //-----------------------------------------
    // do some initial checking here
    //-----------------------------------------
    EXPECT(not open_sim_dll("../../../bin/handcar_cosim.so", &sim_api));

    SECTION("Test 0, 0: get_simulator_version") {
      //---------------------------------------------------------------
      // include necessary operations on the object being tested here
      //---------------------------------------------------------------
      char original[] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
      char version[] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
      sim_api.get_simulator_version(version);

      //---------------------------------------------------------------
      // do some more checking after the above operations here
      //---------------------------------------------------------------
      EXPECT(original != version);
      // EXPECT(...);
      //
      close_sim_dll(&sim_api);
    }

    SECTION("Test 0, 1: initialize_simulator() and terminate_simulator(), no options") {
      //---------------------------------------------------------------
      // include necessary operations on the object being tested here
      //---------------------------------------------------------------
      sim_api.initialize_simulator(nullptr);
      sim_api.terminate_simulator();

      //---------------------------------------------------------------
      // do some more checking after the above operations here
      //---------------------------------------------------------------
      // EXPECT(...);
      // EXPECT(...);
      //
      close_sim_dll(&sim_api);
    }

    SECTION("Test 0, 2: initialize_simulator() and terminate_simulator(), blank options") {
      //---------------------------------------------------------------
      // include necessary operations on the object being tested here
      //---------------------------------------------------------------
      options = "";
      sim_api.initialize_simulator(options.c_str());
      sim_api.terminate_simulator();

      //---------------------------------------------------------------
      // do some more checking after the above operations here
      //---------------------------------------------------------------
      // EXPECT(...);
      // EXPECT(...);
      //
      close_sim_dll(&sim_api);
    }

    SECTION("Test 0, 3: initialize_simulator() and terminate_simulator(), populated options") {
      //---------------------------------------------------------------
      // include necessary operations on the object being tested here
      //---------------------------------------------------------------
      options = "-p4 --hartids=0,1,2,3 --log-cache-miss";
      sim_api.initialize_simulator(options.c_str());
      sim_api.terminate_simulator();

      //---------------------------------------------------------------
      // do some more checking after the above operations here
      //---------------------------------------------------------------
      // EXPECT(...);
      // EXPECT(...);
      //
      close_sim_dll(&sim_api);
    }

    SECTION("Test 0, 4: initialize_simulator() and terminate_simulator(), options string with invalid options") {
      options = "-p4 --hartids --log-cache-miss=42";
      sim_api.initialize_simulator(options.c_str());
      sim_api.terminate_simulator();

      close_sim_dll(&sim_api);
    }

  }
},

//CASE("Test 1, options api") {
//
//  SETUP("Load SimDllApi Object")  {
//    //-----------------------------------------
//    // include necessary setup code here
//    //-----------------------------------------
//    // load the simulator DLL and pointers to API functions...
//    SimDllApi sim_api;
//    std::string options = "-p3 --hartids=0,1,2";
//    uint64_t num_procs = 4;
//    std::string hartids = "0,1,2,3";
//
//    //-----------------------------------------
//    // do some initial checking here
//    //-----------------------------------------
//    EXPECT(not open_sim_dll("../../../bin/handcar_cosim.so", &sim_api));
//
//    SECTION("Test 1, 0: set_simulator_parameter(...), called before initialization") {
//      sim_api.set_simulator_parameter("p", &num_procs, nullptr);
//      sim_api.set_simulator_parameter("hartids", nullptr, hartids.c_str());
//      sim_api.set_simulator_parameter("log-cache-miss", nullptr, nullptr);
//      sim_api.initialize_simulator(nullptr);
//      sim_api.terminate_simulator();
//
//      close_sim_dll(&sim_api);
//    }
//
//    SECTION("Testing 1, 1: set_simulator_parameter(...), called after initialization") {
//      sim_api.initialize_simulator(nullptr);
//      sim_api.set_simulator_parameter("p", &num_procs, nullptr);
//      sim_api.set_simulator_parameter("hartids", nullptr, hartids.c_str());
//      sim_api.set_simulator_parameter("log-cache-miss", nullptr, nullptr);
//      sim_api.terminate_simulator();
//
//      close_sim_dll(&sim_api);
//    }
//
//    SECTION("Testing 1, 2: set_simulator_parameter(...), called before initialization with an options string") {
//      sim_api.set_simulator_parameter("p", &num_procs, nullptr);
//      sim_api.set_simulator_parameter("hartids", nullptr, hartids.c_str());
//      sim_api.set_simulator_parameter("log-cache-miss", nullptr, nullptr);
//      sim_api.initialize_simulator(options.c_str());
//      sim_api.terminate_simulator();
//
//      close_sim_dll(&sim_api);
//    } 
//
//    SECTION("Testing 1, 3: set_simulator_parameter(...), called after initialization with an options string") {
//      sim_api.initialize_simulator(options.c_str());
//      sim_api.set_simulator_parameter("p", &num_procs, nullptr);
//      sim_api.set_simulator_parameter("hartids", nullptr, hartids.c_str());
//      sim_api.set_simulator_parameter("log-cache-miss", nullptr, nullptr);
//      sim_api.terminate_simulator();
//
//      close_sim_dll(&sim_api);
//    }   
//
//    SECTION("Testing 1, 4: set_simulator_parameter(...), called before initialization with invalid arguments") {
//      sim_api.set_simulator_parameter("p", nullptr, nullptr);
//      sim_api.set_simulator_parameter("hartids", nullptr, nullptr);
//      sim_api.set_simulator_parameter("log-cache-miss", nullptr, nullptr);
//      sim_api.initialize_simulator(nullptr);
//      sim_api.terminate_simulator();
//
//      close_sim_dll(&sim_api);
//    }
//
//  }
//},

//CASE("Test 2, load elf api") {
//
//  SETUP("Load SimDllApi Object")  {
//    //-----------------------------------------
//    // include necessary setup code here
//    //-----------------------------------------
//    // load the simulator DLL and pointers to API functions...
//    SimDllApi sim_api;
//    std::string options = "-p3 --hartids=0,1,2";
//    std::string hartids = "0,1,2,3";
//    std::string elf_path = "";
//
//    //-----------------------------------------
//    // do some initial checking here
//    //-----------------------------------------
//    //
//    EXPECT(not open_sim_dll("../../../bin/handcar_cosim.so", &sim_api));
//
//    SECTION("Test 2, 0: simulator_load_elf(...), called before initialization") {
//      sim_api.simulator_load_elf(0, elf_path.c_str());
//      sim_api.initialize_simulator(nullptr);
//      sim_api.terminate_simulator();
//
//      close_sim_dll(&sim_api);
//    }
//
//    SECTION("Test 2, 1: simulator_load_elf(...), called after initialization, with valid ELF file") {
//      elf_path = "../../resources/multiply.riscv";
//      sim_api.initialize_simulator(nullptr);
//      sim_api.simulator_load_elf(0, elf_path.c_str());
//      sim_api.terminate_simulator();
//
//      close_sim_dll(&sim_api);
//    }
//
//  }
//},
//
//CASE("Test 3, step simulator api") {
//
//  SETUP("Load SimDllApi Object")  {
//    //-----------------------------------------
//    // include necessary setup code here
//    //-----------------------------------------
//    // load the simulator DLL and pointers to API functions...
//    SimDllApi sim_api;
//    std::string options = "-p4 -l";
//    uint64_t num_procs = 4;
//    std::string elf_path = "../../resources/multiply.riscv";
//    int hart_id = 0;
//    int num_steps = 10;
//    int num_step_groups = 1000;
//    int stx_failed = 0;
//    int rcode = 0;
//
//    //-----------------------------------------
//    // do some initial checking here
//    //-----------------------------------------
//    EXPECT(not open_sim_dll("../../../bin/handcar_cosim.so", &sim_api));
//
//    SECTION("Test 3, 0: step_simulator(...)") {
//      sim_api.initialize_simulator(nullptr);
//      sim_api.simulator_load_elf(0, elf_path.c_str());
//      rcode = sim_api.step_simulator(hart_id, num_steps, stx_failed);
//      sim_api.terminate_simulator();
//
//      EXPECT(rcode == 0);
//
//      close_sim_dll(&sim_api);
//    }
//
//    SECTION("Test 3, 1: step_simulator(...), multiple cores") {
//      sim_api.initialize_simulator(options.c_str());
//      sim_api.simulator_load_elf(0, elf_path.c_str());
//
//      for(size_t step_group = 0; step_group < num_step_groups; ++step_group) 
//      {
//        rcode += sim_api.step_simulator(step_group % num_procs, num_steps, stx_failed);
//        EXPECT(rcode == 0);
//      }
//
//      sim_api.terminate_simulator();
//
//      close_sim_dll(&sim_api);
//    }
//
//  }
//},

CASE("Test 4, get_disassembly(...) api") {

  SETUP("Load SimDllApi Object")  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    // load the simulator DLL and pointers to API functions...
    SimDllApi sim_api;
    std::string options = "-p4 -l";
    uint64_t num_procs = 4;
    std::string elf_path = "../../resources/multiply.riscv";
    int num_steps = 10;
    int num_step_groups = 1;
    int stx_failed = 0;
    int rcode = 0;
    char *opcode = (char* )malloc(100);
    char *disassembly = (char*) malloc(100);
    
    memset(opcode,'-',100);
    memset(disassembly,'-',100);
    opcode[99] = '\0';
    disassembly[99] = '\0';
    uint64_t pc = 0x1000;

    //-----------------------------------------
    // do some initial checking here
    //-----------------------------------------
    EXPECT(not open_sim_dll("../../../bin/handcar_cosim.so", &sim_api));

    SECTION("Test 4, 0: get_disassembly(...)") {
      sim_api.initialize_simulator(options.c_str());
      sim_api.simulator_load_elf(0, elf_path.c_str());

      for(size_t step = 0; step < num_steps; ++step) 
      {
        rcode += sim_api.step_simulator(0, 1, stx_failed);
        EXPECT(rcode == 0);
      }

      rcode += sim_api.get_disassembly(&pc, &opcode, &disassembly);
      
      std::cout << "disassembly: " << disassembly << std::endl;

      free(opcode);
      free(disassembly);

      sim_api.terminate_simulator();

      EXPECT(rcode == 0);   

      close_sim_dll(&sim_api);
    }

  }
},

CASE("Test 5, read_simulator_register(...) and write_simulator_register(...) api") {

  SETUP("Load SimDllApi Object")  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    // load the simulator DLL and pointers to API functions...
    SimDllApi sim_api;
    //std::string elf_path = "../../resources/multiply.riscv";
    std::vector<char> name_goes_here(128);
    char* name_ptr = name_goes_here.data();
    uint64_t value[2] = {0ull};
    int length = 16;
    std::map<std::string, std::pair<int,int>> reg_name_to_nums;
    std::vector<int> reg_categories = {1,2,3};
    std::vector<int> reg_numbers = {4096, 32, 32, 32};
    uint64_t write_pattern[2] = {0x123456789ABCDEF0, 0x123456789ABCDEF};
    uint64_t read_back[2] = {0x0000000000000000, 0x0000000000000000};

    //-----------------------------------------
    // do some initial checking here
    //-----------------------------------------
    EXPECT(not open_sim_dll("../../../bin/handcar_cosim.so", &sim_api));
    
    SECTION("Test 5, 0: read and write pc") {
      sim_api.initialize_simulator(nullptr);

      length = 8;
      int status = sim_api.write_simulator_register(0, "pc", reinterpret_cast<const uint8_t*>(&write_pattern[0]), length);
    
      value[0] = 0;
      status |= sim_api.read_simulator_register(0, "pc", reinterpret_cast<uint8_t*>(&value[0]), length);
    
      bool success = ((status==0) && (value[0] == write_pattern[0]));
      EXPECT(success);

      sim_api.terminate_simulator();

      close_sim_dll(&sim_api);
    }

    SECTION("Test 5, 0.a: partial_read_large_register(...)") {
      sim_api.initialize_simulator(nullptr);

      std::vector<uint32_t> physical_register_offsets = {0, 8};
      uint32_t physical_register_length = 8;
      uint32_t number_of_vector_registers = 32;
      length = 16;


      for(uint32_t vec_reg_index = 0; vec_reg_index < number_of_vector_registers; ++vec_reg_index)
      {
          value[0] = 0;
          value[1] = 0;
          std::string reg_name = std::string("v") + std::to_string(vec_reg_index);

          int status = sim_api.write_simulator_register(0, reg_name.c_str(), reinterpret_cast<const uint8_t*>(&write_pattern[0]), length);
          EXPECT(status == 0);

          for(uint32_t offset : physical_register_offsets)
          {
            status += sim_api.partial_read_large_register(0, reg_name.c_str(), reinterpret_cast<uint8_t*>(&value[0]) + offset, physical_register_length, offset);
          }

          EXPECT(status == 0);
          EXPECT(value[0] == write_pattern[0]);
          EXPECT(value[1] == write_pattern[1]);
          //EXPECT((*(reinterpret_cast<uint64_t*>(global_buffer.data()))) == write_pattern[0]);
          //EXPECT((*(reinterpret_cast<uint64_t*>(global_buffer.data()+8))) == write_pattern[1]);
      }

      sim_api.terminate_simulator();
      close_sim_dll(&sim_api);
    }

    SECTION("Test 5, 0.b: partial_write_large_register(...)") {
      sim_api.initialize_simulator(nullptr);

      std::vector<uint32_t> physical_register_offsets = {0, 8};
      uint32_t physical_register_length = 8;
      uint32_t number_of_vector_registers = 32;
      length = 16;


      for(uint32_t vec_reg_index = 0; vec_reg_index < number_of_vector_registers; ++vec_reg_index)
      {
          value[0] = 0;
          value[1] = 0;
          std::string reg_name = std::string("v") + std::to_string(vec_reg_index);

          int status = sim_api.partial_write_large_register(0, reg_name.c_str(), reinterpret_cast<const uint8_t*>(&write_pattern[0]) , physical_register_length, 0);
          status += sim_api.partial_write_large_register(0, reg_name.c_str(), reinterpret_cast<const uint8_t*>(&write_pattern[1]) , physical_register_length, physical_register_length);
          EXPECT(status == 0);

          for(uint32_t offset : physical_register_offsets)
          {
            status += sim_api.partial_read_large_register(0, reg_name.c_str(), reinterpret_cast<uint8_t*>(&value[0]) + offset, physical_register_length, offset);
          }

          EXPECT(status == 0);
          EXPECT(value[0] == write_pattern[0]);
          EXPECT(value[1] == write_pattern[1]);
          //EXPECT((*(reinterpret_cast<uint64_t*>(global_buffer.data()))) == write_pattern[0]);
          //EXPECT((*(reinterpret_cast<uint64_t*>(global_buffer.data()+8))) == write_pattern[1]);
      }

      sim_api.terminate_simulator();
      close_sim_dll(&sim_api);
    }


    SECTION("Test 5, 1: write_simulator_register(...) on scalar application registers") {
      sim_api.initialize_simulator(nullptr);
      //sim_api.simulator_load_elf(0, elf_path.c_str());
    
      std::vector<std::string> reg_names = {
      //,"x0" this register does not change in response to writes.
      "x1"
      ,"x2"
      ,"x3"
      ,"x4"
      ,"x5"
      ,"x6"
      ,"x7"
      ,"x8"
      ,"x9"
      ,"x10"
      ,"x11"
      ,"x12"
      ,"x13"
      ,"x14"
      ,"x15"
      ,"x16"
      ,"x17"
      ,"x18"
      ,"x19"
      ,"x20"
      ,"x21"
      ,"x22"
      ,"x23"
      ,"x24"
      ,"x25"
      ,"x26"
      ,"x27"
      ,"x28"
      ,"x29"
      ,"x30"
      ,"x31"
      ,"f0"
      ,"f1"
      ,"f2"
      ,"f3"
      ,"f4"
      ,"f5"
      ,"f6"
      ,"f7"
      ,"f8"
      ,"f9"
      ,"f10"
      ,"f11"
      ,"f12"
      ,"f13"
      ,"f14"
      ,"f15"
      ,"f16"
      ,"f17"
      ,"f18"
      ,"f19"
      ,"f20"
      ,"f21"
      ,"f22"
      ,"f23"
      ,"f24"
      ,"f25"
      ,"f26"
      ,"f27"
      ,"f28"
      ,"f29"
      ,"f30"
      ,"f31"
      }; 

      for(std::string& regname : reg_names)
      {
          length = 8;
          read_back[0] = 0ull;
          read_back[1] = 0ull;
          int status = sim_api.write_simulator_register(0, regname.c_str() ,reinterpret_cast<const uint8_t*>(&write_pattern[0]), length);

          if(status != 0)
          {
            length = 16;
            status = sim_api.write_simulator_register(0, regname.c_str(), reinterpret_cast<const uint8_t*>(&write_pattern[0]), length);
          }

          if(status == 0)
          {
            length = 16; // Here length means maximum read size. Since we're reading 8 byte and 16 byte values both here, this needs to be 16.
            //status = sim_api.read_simulator_register(0, map_it.second.first, map_it.second.second, &name_ptr, reinterpret_cast<uint8_t*>(&read_back[0]),&length);
            status = sim_api.read_simulator_register(0, regname.c_str(), reinterpret_cast<uint8_t*>(&read_back[0]), length);
            if(status == 0)
            {
              EXPECT(read_back[0] == write_pattern[0]);
            }
           }   

          EXPECT(status == 0);
      }

      sim_api.terminate_simulator();
      close_sim_dll(&sim_api);
    }

  }
},

CASE("Test 6, read_simulator_memory(...) and write_simulator_memory(...) api") {

  SETUP("Load SimDllApi Object")  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    // load the simulator DLL and pointers to API functions...
    SimDllApi sim_api;
    std::string elf_path = "../../resources/multiply.riscv";
    uint64_t buffer = 0ull;
    uint64_t length = 4;
    uint64_t num_chunks_to_read = 150;
    uint64_t offset = 0x80001000ul;
    uint64_t multiplier = 8;

    //-----------------------------------------
    // do some initial checking here
    //-----------------------------------------
    EXPECT(not open_sim_dll("../../../bin/handcar_cosim.so", &sim_api));

    SECTION("Test 6, 0: read_simulator_memory(...)") {
      sim_api.initialize_simulator(nullptr);
      sim_api.simulator_load_elf(0, elf_path.c_str());

      for(uint64_t chunk = 0; chunk < num_chunks_to_read; ++chunk)
      { 
        buffer = 0ul; 
        const uint64_t paddr = (chunk * multiplier) + offset;
        int status = sim_api.read_simulator_memory(0 /*procid*/, &paddr, length, reinterpret_cast<uint8_t*>(&buffer));
        
        EXPECT(status == 0);
           
      }

      sim_api.terminate_simulator();

      close_sim_dll(&sim_api);
    }

    SECTION("Test 6, 1: read_simulator_memory(...) and write_simulator_memory(...)") {
      sim_api.initialize_simulator(nullptr);
      sim_api.simulator_load_elf(0, elf_path.c_str());

      length = 8;
      num_chunks_to_read = 500;

      for(uint64_t chunk = 0; chunk < num_chunks_to_read; ++chunk)
      {
        const uint64_t paddr = (chunk * multiplier) + offset;
    
        buffer = 0ull;
        int status = sim_api.read_simulator_memory(0 /*procid*/, &paddr, length, reinterpret_cast<uint8_t*>(&buffer));
        EXPECT(status == 0);
    
        const uint64_t write_buffer = paddr;
        status = sim_api.write_simulator_memory(0 /*procid*/, &paddr, length, reinterpret_cast<const uint8_t*>(&write_buffer));
        EXPECT(status == 0);
    
        buffer = 0ull;
        status = sim_api.read_simulator_memory(0 /*procid*/, &paddr, length, reinterpret_cast<uint8_t*>(&buffer));
        EXPECT(status == 0);
        EXPECT(write_buffer == buffer);
      }

      sim_api.terminate_simulator();

      close_sim_dll(&sim_api);
    }

  }
},

CASE("Test 7, translate_virtual_address(...) api") {

  SETUP("Load SimDllApi Object")  {
    //-----------------------------------------
    // include necessary setup code here
    //-----------------------------------------
    // load the simulator DLL and pointers to API functions...
    SimDllApi sim_api;
    std::string elf_path = "../../resources/multiply.riscv";
    uint64_t buffer = 0ull;
    uint64_t length = 8;
    uint64_t num_chunks_to_read = 150;
    uint64_t offset = 0x80001000ul;
    uint64_t multiplier = 8;
    uint64_t pmp_info = 0ull;

    //-----------------------------------------
    // do some initial checking here
    //-----------------------------------------
    EXPECT(not open_sim_dll("../../../bin/handcar_cosim.so", &sim_api));

    SECTION("Test 7, 0: translate_virtual_address(...), vm off") {
      sim_api.initialize_simulator(nullptr);
      sim_api.simulator_load_elf(0, elf_path.c_str());

      for(uint64_t chunk = 0; chunk < num_chunks_to_read; ++chunk)
      { 
        buffer = 0ul; 
        const uint64_t paddr = (chunk * multiplier) + offset;
        int status = sim_api.read_simulator_memory(0 /*procid*/, &paddr, length, reinterpret_cast<uint8_t*>(&buffer));
        
        EXPECT(status == 0);
           
      }

      sim_api.terminate_simulator();

      //DEBUG
      close_sim_dll(&sim_api);
    }

    SECTION("Test 7, 1: translate_virtual_address(...), vm on") {
      sim_api.initialize_simulator(nullptr);
      //sim_api.simulator_load_elf(0, elf_path.c_str());

      // Virtual memory related codes
      const size_t RISCV_PGSHIFT = 12;
      const size_t RISCV_PGSIZE = (1 << RISCV_PGSHIFT);
      const size_t PTE_PPN_SHIFT = 10;
      /* page table entry (PTE) fields */
      const size_t PTE_V     = 0x001; /* Valid */
      const size_t PTE_R     = 0x002; /* Read */
      const size_t PTE_W     = 0x004; /* Write */
      const size_t PTE_X     = 0x008; /* Execute */
      const size_t PTE_U     = 0x010; /* User */
      const size_t PTE_G     = 0x020; /* Global */
      const size_t PTE_A     = 0x040; /* Accessed */
      const size_t PTE_D     = 0x080; /* Dirty */
      const size_t PTE_SOFT  = 0x300; /* Reserved for Software */
      const size_t SATP_MODE_SV39 = 8;
      const size_t SATP64_MODE = 0xF000000000000000;
      const size_t SATP64_ASID = 0x0FFFF00000000000;
      const size_t SATP64_PPN  = 0x00000FFFFFFFFFFF;
      const size_t PMP_R     = 0x01;
      const size_t PMP_W     = 0x02;
      const size_t PMP_X     = 0x04;
      const size_t PMP_NAPOT = 0x18;
      const size_t MSTATUS_MPRV = 0x00020000;
      const size_t MSTATUS_MPP = 0x00001800;
    
      // place the pages somewhere above DRAM base (0x8000_0000 by default) and align them to the page size.
      uint64_t l1_paddr = 0x80000000;
      uint64_t l2_paddr = 0x80001000;
      uint64_t l3_paddr = 0x80002000;
      uint64_t scratch_paddr = 0x80003000;
      uint64_t l1_first_entry = (l2_paddr >> RISCV_PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
      uint64_t l3_first_entry = (scratch_paddr >> RISCV_PGSHIFT << PTE_PPN_SHIFT) | PTE_A | PTE_D | PTE_V | PTE_R | PTE_W;
      uint64_t l2_first_entry = (l3_paddr >> RISCV_PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
    
      // level 1
      int status = sim_api.initialize_simulator_memory(0 /*procid*/, &l1_paddr, 8, l1_first_entry);

      //int status = sim_api.initialize_simulator_memory(0 /*procid*/, &scratch_paddr, 8, l1_first_entry);
      EXPECT(status == 0);

      // initialize the rest of the page with zeros
      for(size_t chunk = 8; chunk < RISCV_PGSIZE; chunk+=8)
      {
         uint64_t muh_paddr = l1_paddr + chunk;
         uint64_t zero = 0;
         status = sim_api.initialize_simulator_memory(0 /*procid*/, &muh_paddr, 8, zero);
         EXPECT(status == 0);
      }
    
      // level 3
      status = sim_api.initialize_simulator_memory(0 /*procid*/, &l3_paddr, 8, l3_first_entry);
      EXPECT(status == 0);

      for(size_t chunk = 8; chunk < RISCV_PGSIZE; chunk+=8)
      {  
         uint64_t muh_paddr = l3_paddr + chunk;
         uint64_t zero = 0;
         status = sim_api.initialize_simulator_memory(0 /*procid*/, &muh_paddr, 8, zero);
         EXPECT(status == 0);
      }
    
      // level 2
      status = sim_api.initialize_simulator_memory(0 /*procid*/, &l2_paddr, 8, l2_first_entry);
      EXPECT(status == 0);

      for(size_t chunk = 8; chunk < RISCV_PGSIZE; chunk+=8)
      {  
         uint64_t muh_paddr = l2_paddr + chunk;
         uint64_t zero = 0;
         status = sim_api.initialize_simulator_memory(0 /*procid*/, &muh_paddr, 8, zero);
         EXPECT(status == 0);
      }
    
      // configure the CSRs
      uint64_t CSR_SATP = 0x180;
      uint64_t CSR_PMPADDR2 = 0x3B2;
      uint64_t CSR_PMPCFG0 = 0x3A0;
      uint64_t CSR_MSTATUS = 0x300;
      uint64_t MSTATUS_MXR = 0x00080000;
      uint64_t MSTATUS_SUM = 0x00040000;
    
      uint64_t data_for_satp = (l1_paddr >> RISCV_PGSHIFT) | (SATP_MODE_SV39 * (SATP64_MODE & ~(SATP64_MODE<<1)));
      uint64_t data_for_pmpaddr2 = (uint64_t(0) -1);
      uint64_t data_for_pmpcfg0 = (PMP_NAPOT | PMP_R | PMP_W | PMP_X) << 16;
      uint64_t data_for_mstatus = (MSTATUS_MPRV & (1 << 17)) | (MSTATUS_MPP & (1 << 11)) | (MSTATUS_SUM & (1 << 18)) | (MSTATUS_MXR & (1 << 19));
    
      std::string pmpcfg0_name = "pmpcfg0";
      std::string satp_name = "satp";
      std::string pmpaddr2_name = "pmpaddr2";
      std::string mstatus_name = "mstatus";
    
      length = 8;
      status = sim_api.write_simulator_register(0, pmpcfg0_name.data() ,reinterpret_cast<const uint8_t*>(&data_for_pmpcfg0), length);
      EXPECT(status == 0);
      status = sim_api.write_simulator_register(0, pmpaddr2_name.data() ,reinterpret_cast<const uint8_t*>(&data_for_pmpaddr2), length);
      EXPECT(status == 0);
      status = sim_api.write_simulator_register(0, satp_name.data() ,reinterpret_cast<const uint8_t*>(&data_for_satp), length);
      EXPECT(status == 0);
      status = sim_api.write_simulator_register(0, mstatus_name.data() ,reinterpret_cast<const uint8_t*>(&data_for_mstatus), length);
      EXPECT(status == 0);
    
      // try to translate some addresses, eventually for too high a VA we need the next entry in the last page table which we have not specified here. 
      offset = 0x00;
      multiplier = 256;
      num_chunks_to_read = 10;
      for(uint64_t chunk = 0; chunk < num_chunks_to_read; ++chunk)
      {
        const uint64_t vaddr = (chunk * multiplier) + offset;
    
        // intent:
        // 0 = LOAD,
        // 1 = STORE,
        // 2 = FETCH, // Doesn't really use VM unless proc priv is changed.
        for(int intent = 0; intent < 3; ++intent)
        {
            buffer = 0ull;
            pmp_info = 0ull;
            status = sim_api.translate_virtual_address(0 /*procid*/, &vaddr, intent, &buffer, &pmp_info);

            EXPECT(status == 0);
        }
      }


      sim_api.terminate_simulator();

      close_sim_dll(&sim_api);
    }
  }

},


};

int main(int argc, char* argv[])
{
//  Force::Logger::Initialize();
  int ret = lest::run(specification, argc, argv);
///  Force::Logger::Destroy();
  return ret;
}
