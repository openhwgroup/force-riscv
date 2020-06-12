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
#include <Log.h>
#include <Random.h>
//------------------------------------------------
// include necessary header files here
//------------------------------------------------
#include <GenException.h>
#include <Defines.h>
#include <Memory.h>
#include <Enums.h>
#include <UtilityFunctions.h>

using text = std::string;
using namespace Force;

const lest::test specification[] = {

CASE( "Memory module testing" ) {

    SETUP( "setup and test Memory class" )  {
        //-----------------------------------------
        // include necessary setup code here
        //-----------------------------------------
        Memory mem(EMemBankType::Default);
        uint8 buffer[8] = {0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00};
        uint8 mem_attrs_buffer[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        //-----------------------------------------
        // do some initial checking here
        //-----------------------------------------

        SECTION( "Test Aligned Access." ) {

          mem.Initialize(0x1000, 0x0ull, 8, EMemDataType::Data);

          mem.Write(0x1000, 0x0102030405060708ull, 8);

          uint64 value = mem.Read(0x1000, 4);
          EXPECT(value == 0x01020304ull);
          value = mem.Read(0x1004, 4);
          EXPECT(value == 0x05060708ull);
          value = mem.Read(0x1000, 8);
          EXPECT(value == 0x0102030405060708ull);
          for (auto i = 0ull; i < 8; i += 2) {
            value = mem.Read(0x1000 + i, 2);
            EXPECT(value == (((i+1) << 8) | (i+2)));
          }
          for (auto i = 0ull; i < 8; i ++) {
            value = mem.Read(0x1000 + i, 1);
            EXPECT(value == i + 1);
          }

          mem.Initialize(0x1008, buffer, mem_attrs_buffer, 8, EMemDataType::Data);
          mem.Write(0x1008, buffer, 8);
          //EXPECT_FAIL(mem.Write(0x100e, buffer, 3), "write uninitialized memory");

          for (auto i = 0ull; i < 8; i ++) {
             value = mem.Read(0x1008 + i, 1);
             EXPECT(value == buffer[i]);
          }
          mem.Dump(std::cout);
        }

        SECTION( "Test Misaligned Access." ) {
          mem.Initialize(0x2003, 0x0ull, 7, EMemDataType::Both);
          mem.Write(0x2003, 0x01020304050607, 7);
          uint64 value = mem.Read(0x2003, 4);
          EXPECT(value == 0x01020304ull);
          value = mem.Read(0x2007, 3);
          EXPECT(value == 0x050607ull);

          mem.Initialize(0x3003, buffer, mem_attrs_buffer, 8, EMemDataType::Instruction);
          mem.Write(0x3003, buffer, 8);
          for (auto i = 0ull; i < 8; i ++) {
            value = mem.Read(0x3003 + i, 1);
            EXPECT(value == buffer[i]);
          }

          mem.Dump(std::cout);
        }

        SECTION ("Test no-init, re-init") {
            mem.Dump(std::cout);
            //EXPECT_FAIL(mem.Write(0x4004, 0x0102030405060708, 8), "write uninitialized memory");
            //EXPECT_FAIL(mem.Read(0x4004, 8), "read uninitialized memory");
            mem.Initialize(0x3004, buffer, mem_attrs_buffer, 4, EMemDataType::Data);
            //EXPECT_FAIL(mem.Initialize(0x3007, buffer, 8, EMemDataType::Data), "re-initialize memory");
            EXPECT(mem.IsInitialized(0x3007, 1) == true);
            //EXPECT_FAIL(mem.Initialize(0x3004, 0x01020304, 4, EMemDataType::Data), "re-initialize memory");
        }
    }
},

CASE( "Memory::IsInitialized issue 03/07/2017" ) {
   SETUP( "setup and test Memory class" )  {
     Memory mem(EMemBankType::Default);

     mem.Initialize(0xff0000, 0x0ull, 8, EMemDataType::Data);
     mem.Initialize(0xff0010, 0x0ull, 8, EMemDataType::Data);
     mem.Initialize(0xff0023, 0x0ull, 8, EMemDataType::Instruction);
     mem.Initialize(0xff002b, 0x0ull, 8, EMemDataType::Both);
     mem.Initialize(0xff0033, 0x0ull, 5, EMemDataType::Data);
     mem.Dump(std::cout);

     SECTION ("Test no-init, re-init") {
       EXPECT(mem.IsInitialized(0xff0000, 20) == false);
       EXPECT(mem.IsInitialized(0xff0025, 19) == true);
     }
   }
},

CASE( "Test FAIL path" ) {
   SETUP( "setup and test Memory class" )  {
     Memory mem(EMemBankType::Default);

     mem.Initialize(0xfff0000, 0x0ull, 8, EMemDataType::Data);
     mem.Initialize(0xfff0010, 0x0ull, 8, EMemDataType::Data);
     mem.Initialize(0xfff0023, 0x0ull, 8, EMemDataType::Instruction);
     mem.Initialize(0xfff0033, 0x0ull, 5, EMemDataType::Data);
     mem.Dump(std::cout);

     SECTION ("Test no-init, re-init") {
      EXPECT_FAIL(mem.Read(0xfff0007, 8), "read-un-initialized-memory");
      EXPECT_FAIL(mem.Write(0xfff0025, 0x1020304050607080ULL, 8), "write-un-initialized-memory");
     }
   }
},

CASE ("Test IsEmpty() with empty Memory") {
   SETUP( "setup and test Memory class" ) {
     Memory mem(EMemBankType::Default);

     SECTION ("Test Memory::IsEmpty() true for empty Memory") {
       EXPECT(mem.IsEmpty() == true);
     }
   }
},

CASE ("Test IsEmpty() with nonempty Memory") {
   SETUP( "setup and test Memory class" ) {
     Memory mem(EMemBankType::Default);

     mem.Initialize(0xfff0000, 0x0ull, 8, EMemDataType::Data);

     SECTION ("Test Memory::IsEmpty() false for nonempty Memory") {
       EXPECT(mem.IsEmpty() == false);
     }
   }
},

CASE ("Test MemoryBankType() of Memory object specified default") {
   SETUP( "setup and test Memory class" ) {
     Memory mem(EMemBankType::Default); 

     SECTION ("Test Memory::mBankType is initialized to EMemBankType::Default if specified") {
       EXPECT(mem.MemoryBankType() == EMemBankType::Default);
     }
   }
},

CASE ("Test MemoryBankType() of Memory object specified default") {
   SETUP( "setup and test Memory class" ) {
     Memory mem(EMemBankType::Default); 

     SECTION ("Test Memory::mBankType is initialized to EMemBankType::Default if specified") {
       EXPECT(mem.MemoryBankType() == EMemBankType::Default);
     }
   }
},

CASE ("Test GetSections(...)") {
   SETUP( "setup and test Memory class" ) {
     std::vector<Force::Section> original_sections{ 
	Force::Section(0xff0000, 8, EMemDataType::Data)  
	,Force::Section(0xff0010, 8, EMemDataType::Data) 
	,Force::Section(0xff0020, 8, EMemDataType::Instruction) 
     };
     
     Memory mem(EMemBankType::Default); 

     mem.Initialize(0xff0000, 0x0ull, 8, EMemDataType::Data);
     mem.Initialize(0xff0010, 0x0ull, 8, EMemDataType::Data);
     mem.Initialize(0xff0020, 0x0ull, 8, EMemDataType::Instruction);

     SECTION ("Test GetSections(...) output matches initialization results") {
       std::vector<Force::Section> acquired_sections;
       mem.GetSections(acquired_sections);

       bool sections_same_number = (acquired_sections.size() == original_sections.size());

       bool sections_all_same_content = true;

       if( sections_same_number )
       for(std::size_t index = 0; index < acquired_sections.size(); ++index){
         sections_all_same_content &= ( (acquired_sections[index].mAddress == original_sections[index].mAddress )
				      &&(acquired_sections[index].mSize == original_sections[index].mSize )
     				      &&(acquired_sections[index].mType == original_sections[index].mType ) );
       }
	
       EXPECT( sections_same_number );
       EXPECT( sections_all_same_content );
     }
   }
},

CASE ("Test initial Value") {
   SETUP( "setup and test Memory class" )  {
     Memory mem(EMemBankType::Default);

     mem.Initialize(0xfff0003, 0x0102030405060708ull, 8, EMemDataType::Data);

     SECTION ("Test initial value") {
       EXPECT(mem.ReadInitialValue(0xfff0003, 4) == 0x01020304ull);
       EXPECT_FAIL(mem.ReadInitialValue(0xfff000a, 4), "read-un-initialized-memory");
     }

     SECTION("Test value after initial") {
       EXPECT(mem.Read(0xfff0004, 4) == 0x02030405ull);
     }

     SECTION("Test ReadInitialWithPattern() method") {
       uint8 mem_data_buffer[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
       mem.ReadInitialWithPattern(0xfff0000, 16, mem_data_buffer);
       EXPECT(data_array_to_element_value_big_endian(mem_data_buffer + 3, 8) == 0x0102030405060708ull);
     }
   }
},

CASE ("Test partial initialization") {
  SETUP( "Setup Memory object" )  {
    Memory mem(EMemBankType::Default);
    mem.Initialize(0x0, 0xf23459ac, 4, EMemDataType::Data);

    SECTION ("Test initialize zero bytes") {
      uint8 mem_data_buffer[1];
      uint8 mem_attrs_buffer[1];
      EXPECT_FAIL(mem.Initialize(0x10, mem_data_buffer, mem_attrs_buffer, 0, EMemDataType::Data), "unsupported-number-of-bytes");
    }

    SECTION ("Test small, aligned subsequent initialization") {
      mem.Initialize(0xc, 0x6798f23c, 4, EMemDataType::Data);
      cuint32 byte_count = 2;
      uint8 mem_data_buffer[byte_count] = {0x29, 0xb5};
      uint8 mem_attrs_buffer[byte_count] = {0x00, 0x00};
      mem.Initialize(0x8, mem_data_buffer, mem_attrs_buffer, byte_count, EMemDataType::Data);
      EXPECT(mem.ReadInitialValue(0xc, 4) == 0x6798f23cull);
      EXPECT(mem.Read(0xc, 4) == 0x6798f23cull);
      EXPECT(mem.ReadInitialValue(0x8, byte_count) == 0x29b5ull);
      EXPECT(mem.Read(0x8, byte_count) == 0x29b5ull);
    }

    SECTION ("Test small, un-aligned subsequent initialization") {
      cuint32 byte_count = 2;
      uint8 mem_data_buffer[byte_count] = {0xac, 0xf6};
      uint8 mem_attrs_buffer[byte_count] = {0x05, 0x00};
      mem.Initialize(0x3, mem_data_buffer, mem_attrs_buffer, byte_count, EMemDataType::Data);
      EXPECT(mem.ReadInitialValue(0x0, 5) == 0xf23459acf6ull);
      EXPECT(mem.Read(0x0, 5) == 0xf23459acf6ull);
    }

    SECTION ("Test large, aligned subsequent initialization") {
      cuint32 byte_count = 16;
      uint8 mem_data_buffer[byte_count] = {0xf2, 0x34, 0x59, 0xac, 0x43, 0x51, 0xf2, 0x39, 0x64, 0x43, 0x55, 0xab, 0xcd, 0xef, 0x43, 0xbb};
      uint8 mem_attrs_buffer[byte_count] = {0x05, 0x05, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
      mem.Initialize(0x0, mem_data_buffer, mem_attrs_buffer, byte_count, EMemDataType::Data);
      EXPECT(mem.ReadInitialValue(0x0, 8) == 0xf23459ac4351f239ull);
      EXPECT(mem.Read(0x0, 8) == 0xf23459ac4351f239ull);
      EXPECT(mem.ReadInitialValue(0x8, 8) == 0x644355abcdef43bbull);
      EXPECT(mem.Read(0x8, 8) == 0x644355abcdef43bbull);
    }

    SECTION ("Test large, un-aligned subsequent initialization") {
      cuint32 byte_count = 13;
      uint8 mem_data_buffer[byte_count] = {0x34, 0x59, 0xac, 0x43, 0x51, 0xf2, 0x39, 0x64, 0x43, 0x55, 0xab, 0xcd, 0xef};
      uint8 mem_attrs_buffer[byte_count] = {0x05, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
      mem.Initialize(0x1, mem_data_buffer, mem_attrs_buffer, byte_count, EMemDataType::Data);
      EXPECT(mem.ReadInitialValue(0x0, 8) == 0xf23459ac4351f239ull);
      EXPECT(mem.Read(0x0, 8) == 0xf23459ac4351f239ull);
      EXPECT(mem.ReadInitialValue(0x8, 6) == 0x644355abcdefull);
      EXPECT(mem.Read(0x8, 6) == 0x644355abcdefull);
    }

    SECTION ("Test initialization across MemoryBytes boundaries") {
      cuint32 byte_count = 8;
      uint8 mem_data_buffer[byte_count] = {0x00, 0x23, 0x34, 0x45, 0x36, 0xd2, 0x53, 0xbb};
      uint8 mem_attrs_buffer[byte_count] = {0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
      mem.Initialize(0x3, mem_data_buffer, mem_attrs_buffer, byte_count, EMemDataType::Data);
      EXPECT(mem.ReadInitialValue(0x3, byte_count) == 0xac23344536d253bbull);
      EXPECT(mem.Read(0x3, byte_count) == 0xac23344536d253bbull);
      EXPECT(mem.ReadInitialValue(0x0, 4) == 0xf23459acull);
      EXPECT(mem.Read(0x0, 4) == 0xf23459acull);
    }

    SECTION ("Test initialization with gap") {
      mem.Initialize(0x5, 0xd5, 1, EMemDataType::Data);
      cuint32 byte_count = 4;
      uint8 mem_data_buffer[byte_count] = {0xac, 0x23, 0xd5, 0x45};
      uint8 mem_attrs_buffer[byte_count] = {0x05, 0x00, 0x05, 0x00};
      mem.Initialize(0x3, mem_data_buffer, mem_attrs_buffer, byte_count, EMemDataType::Data);
      EXPECT(mem.ReadInitialValue(0x3, byte_count) == 0xac23d545ull);
      EXPECT(mem.Read(0x3, byte_count) == 0xac23d545ull);
      EXPECT(mem.ReadInitialValue(0x0, 4) == 0xf23459acull);
      EXPECT(mem.Read(0x0, 4) == 0xf23459acull);
    }

    SECTION ("Test initialization mismatch") {
      cuint32 byte_count = 4;
      uint8 mem_data_buffer[byte_count] = {0x59, 0xac, 0x36, 0x42};
      uint8 mem_attrs_buffer[byte_count] = {0x05, 0x00, 0x00, 0x00};
      EXPECT_FAIL(mem.Initialize(0x2, mem_data_buffer, mem_attrs_buffer, byte_count, EMemDataType::Data), "reinitilize-memory");
    }
  }
},

CASE ("Test reading partially initalized memory") {
  SETUP( "Setup Memory object" )  {
    Memory mem(EMemBankType::Default);
    mem.Initialize(0x0, 0xf23459ac, 4, EMemDataType::Data);

    SECTION ("Test reading partially initialized data") {
      cuint32 byte_count = 8;
      uint8 mem_data_buffer[byte_count];
      mem.ReadPartiallyInitialized(0x0, byte_count, mem_data_buffer);
      EXPECT(data_array_to_element_value_big_endian(mem_data_buffer, byte_count) == 0xf23459ac00000000ull);
    }

    SECTION ("Test read zero bytes") {
      uint8 mem_data_buffer[1] = {0x00};
      mem.ReadPartiallyInitialized(0x0, 0, mem_data_buffer);
      EXPECT(data_array_to_element_value_big_endian(mem_data_buffer, 1) == 0x0ull);
    }

    SECTION ("Test read short block") {
      cuint32 byte_count = 2;
      uint8 mem_data_buffer[byte_count];
      mem.ReadPartiallyInitialized(0x0, byte_count, mem_data_buffer);
      EXPECT(data_array_to_element_value_big_endian(mem_data_buffer, byte_count) == 0xf234ull);
    }

    SECTION ("Test read long, un-aligned block") {
      mem.Initialize(0x0a, 0xf1b69255, 4, EMemDataType::Instruction);
      cuint32 byte_count = 16;
      uint8 mem_data_buffer[byte_count];
      mem.ReadPartiallyInitialized(0x02, byte_count, mem_data_buffer);
      EXPECT(data_array_to_element_value_big_endian(mem_data_buffer, 8) == 0x59ac000000000000ull);
      EXPECT(data_array_to_element_value_big_endian(mem_data_buffer + 8, 8) == 0xf1b6925500000000ull);
    }

    SECTION ("Test read fully initialized") {
      cuint32 byte_count = 4;
      uint8 mem_data_buffer[byte_count];
      mem.ReadPartiallyInitialized(0x0, byte_count, mem_data_buffer);
      EXPECT(data_array_to_element_value_big_endian(mem_data_buffer, byte_count) == 0xf23459acull);
    }

    SECTION ("Test read fully un-initialized") {
      cuint32 byte_count = 4;
      uint8 mem_data_buffer[byte_count];
      mem.ReadPartiallyInitialized(0x30, byte_count, mem_data_buffer);
      EXPECT(data_array_to_element_value_big_endian(mem_data_buffer, byte_count) == 0x0ull);
    }
  }
},

CASE ("Test memory attributes") {
  SETUP( "Setup Memory object" )  {
    Memory mem(EMemBankType::Default);
    mem.Initialize(0x20, 0x92357605, 4, EMemDataType::Data);

    SECTION ("Test GetMemoryAttributes() method") {
      cuint32 byte_count = 8;
      uint8 mem_attrs_buffer[byte_count];
      mem.GetMemoryAttributes(0x20, byte_count, mem_attrs_buffer);
      EXPECT(data_array_to_element_value_big_endian(mem_attrs_buffer, byte_count) == 0x0505050500000000ull);
    }

    SECTION ("Test get zero attributes") {
      uint8 mem_attrs_buffer[1] = {0x00};
      mem.GetMemoryAttributes(0x20, 0, mem_attrs_buffer);
      EXPECT(data_array_to_element_value_big_endian(mem_attrs_buffer, 1) == 0x0ull);
    }

    SECTION ("Test get attributes for short block") {
      cuint32 byte_count = 2;
      uint8 mem_attrs_buffer[byte_count];
      mem.GetMemoryAttributes(0x20, byte_count, mem_attrs_buffer);
      EXPECT(data_array_to_element_value_big_endian(mem_attrs_buffer, byte_count) == 0x0505ull);
    }

    SECTION ("Test get attributes for long, un-aligned block") {
      mem.Initialize(0x2a, 0xf1b69255, 4, EMemDataType::Instruction);
      cuint32 byte_count = 16;
      uint8 mem_attrs_buffer[byte_count];
      mem.GetMemoryAttributes(0x22, byte_count, mem_attrs_buffer);
      EXPECT(data_array_to_element_value_big_endian(mem_attrs_buffer, 8) == 0x0505000000000000ull);
      EXPECT(data_array_to_element_value_big_endian(mem_attrs_buffer + 8, 8) == 0x0303030300000000ull);
    }

    SECTION ("Test get attributes for un-initialized memory") {
      cuint32 byte_count = 4;
      uint8 mem_attrs_buffer[byte_count];
      mem.GetMemoryAttributes(0x30, byte_count, mem_attrs_buffer);
      EXPECT(data_array_to_element_value_big_endian(mem_attrs_buffer, byte_count) == 0x0ull);
    }

    SECTION ("Test get memory attributes for initialized byte") {
      EXPECT(mem.GetByteMemoryAttributes(0x21) == 0x5u);
    }

    SECTION ("Test get memory attributes for un-initialized byte") {
      EXPECT(mem.GetByteMemoryAttributes(0x24) == 0x0u);
    }
  }
},

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
