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
#include "TestIO.h"

#include "lest/lest.hpp"

#include "Defines.h"
#include "Enums.h"
#include "Log.h"
#include "Memory.h"
#include "Random.h"
#include "SymbolManager.h"

using text = std::string;

const lest::test specification[] = {

CASE( "Basic Test TestIO Write ELF" ) {
   SETUP( "setup and test TestIO class" ) {
     using namespace Force;

     Memory mem(EMemBankType::Default);
     SymbolManager sym_manager(EMemBankType::Default);

    // instruction sequence for ending emulation
     mem.Initialize(0xffff0028, 0x090080d2ull, 4, EMemDataType::Instruction);  // mov x9, #0x0
     mem.Initialize(0xffff002c, 0x0960a2f2ull, 4, EMemDataType::Instruction);  // movk x9, #0x1300, LSL #16
     mem.Initialize(0xffff0040, 0x8a0080d2ull, 4, EMemDataType::Instruction);         // mov x10, #0x4
     mem.Initialize(0xffff0044, 0x2a0500f8ull, 4, EMemDataType::Instruction);  // str x10, [x9], #0
     //mem.Initialize(0xffff003a, 0x0102, 2, EMemDataType::Data);  //useless data

     mem.Initialize(0xffff0000, 0x0001020304050607ull, 8, EMemDataType::Data);
     mem.Initialize(0xffff0008, 0x08090a0b0c0d0e0full, 8, EMemDataType::Data);
     mem.Initialize(0xffff0020, 0x000080d2ull, 4, EMemDataType::Instruction);  // mov x0, #0x0
     mem.Initialize(0xffff0024, 0x010080d2ull, 4, EMemDataType::Instruction);         // mov x1, #0x0
     
     mem.Dump(std::cout);

     TestIO testio(0, &mem, &sym_manager);

     SECTION ("Test write image to generate ELF file") {
       testio.WriteTestElf("./test.ELF", false, 0xffff0020, 0xF3);  
     }
     SECTION ("Test Section Number") {
       EXPECT(testio.CountSections() == 3u);
     }
   }
},

CASE( "Test TestIO Read ELF" ) {
   SETUP( "setup and test TestIO read image" ) {
     using namespace Force;

     Memory mem(EMemBankType::Default);
     SymbolManager sym_manager(EMemBankType::Default);
     
     TestIO testio(1, &mem, &sym_manager, false);

     SECTION ("Test read elf ") {
       bool bigEndian;
       uint64 entry;
       uint64 data;

       testio.ReadTestElf("./test.ELF", bigEndian, entry, 0xF3);
       EXPECT(bigEndian == false);
       EXPECT(entry == 0xffff0020u);
       
       data = mem.ReadInitialValue(0xffff0008, 8);
       EXPECT(data == 0x08090a0b0c0d0e0full);
    
       data = mem.ReadInitialValue(0xffff002c, 4);
       EXPECT(data == 0x0960a2f2ull);

       data = mem.ReadInitialValue(0xffff0025, 3);
       EXPECT(data == 0x0080d2ull);

       //data = mem.ReadInitialValue(0xffff003a, 2);
       //EXPECT(data == 0x0102ull);
     }
   }
}

};

int main( int argc, char * argv[] )
{
  Force::Logger::Initialize();
  Force::Random::Initialize();
  int ret = lest::run( specification, argc, argv );
  Force::Random::Destroy();
  Force::Logger::Destroy();
  return ret;

}
