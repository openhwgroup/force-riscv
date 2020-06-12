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

#include <Record.h>
#include <UtilityFunctions.h>

using text = std::string;

using namespace std;
using namespace Force;

bool is_byte_stream_zero(cuint8* byte_stream, cuint32 size)
{
  bool all_zero = true;
  for (uint32 i = 0; i < size; i++) {
    if (byte_stream[i] != 0) {
      all_zero = false;
      break;
    }
  }

  return all_zero;
}

const lest::test specification[] = {

CASE( "Test cases set 1 size == element size for Record module" ) {

    SETUP( "setup code Record module" )  {
      RecordArchive record_archive;
      cuint32 thread_id = 0;

        SECTION( "Instruction init test" ) {
          uint32 instr_size = 4;
          uint64 pa = 0xffff0000;
          uint32 opcode = 0xa1b2c3d4;

          MemoryInitRecord* mem_init_data = record_archive.GetMemoryInitRecord(thread_id, instr_size, instr_size, EMemDataType::Instruction);
          mem_init_data->SetData(pa, 0, opcode, instr_size, false);
          EXPECT(mem_init_data->DataString() == "d4c3b2a1");
          EXPECT(is_byte_stream_zero(mem_init_data->InitAttributes(), mem_init_data->Size()));
          EXPECT_FAIL(mem_init_data->SetData(pa, 0, opcode, 5, false), "set-data-mismatch-size");
        }

        SECTION( "Data init test" ) {
          uint64 pa = 0x10004000;
          uint64 value = 0x1a2b3c4d5e6f7e8dull;

          MemoryInitRecord* mem_init_data = record_archive.GetMemoryInitRecord(thread_id, 8, 8, EMemDataType::Data);
          mem_init_data->SetData(pa, 1, value, 8, false);
          EXPECT(mem_init_data->DataString() == "8d7e6f5e4d3c2b1a");
          EXPECT(is_byte_stream_zero(mem_init_data->InitAttributes(), mem_init_data->Size()));

          mem_init_data->SetData(pa, 1, value, 8, true);
          EXPECT(mem_init_data->DataString() == "1a2b3c4d5e6f7e8d");
          EXPECT(is_byte_stream_zero(mem_init_data->InitAttributes(), mem_init_data->Size()));
        }

        SECTION( "Element size test" ) {
          uint64 pa = 0x10004000;
          uint64 value = 0x1a2b3c4d5e6f7e8dull;
          uint32 data_size = 7;

          MemoryInitRecord* mem_init_data = record_archive.GetMemoryInitRecord(thread_id, data_size, data_size, EMemDataType::Data);
          EXPECT_FAIL(mem_init_data->SetData(pa, 0, value, data_size, false), "unexpected-element-size");
          EXPECT(is_byte_stream_zero(mem_init_data->InitAttributes(), mem_init_data->Size()));
        }

      SECTION( "Element ID test" ) {
          MemoryInitRecord* mem_init_data1 = record_archive.GetMemoryInitRecord(thread_id, 8, 8, EMemDataType::Data);
          MemoryInitRecord* mem_init_data2 = record_archive.GetMemoryInitRecord(thread_id, 8, 8, EMemDataType::Data);
          EXPECT(mem_init_data1->Id() == 0u);
          EXPECT(mem_init_data2->Id() == 1u);
      }
    }
},

CASE( "Test cases set 2 size > element size for Record module" ) {

  SETUP( "setup code Record module" )  {
    RecordArchive record_archive;
    cuint32 thread_id = 1;

    SECTION( "Element-size == 1 init test" ) {
      uint32 size = 4;
      uint64 pa = 0x20000000;
      uint32 value = 0xa1b2c3d4;

      MemoryInitRecord* mem_init_data = record_archive.GetMemoryInitRecord(thread_id, size, 1, EMemDataType::Data);
      mem_init_data->SetData(pa, 0, value, size, false);
      EXPECT(mem_init_data->DataString()  == "a1b2c3d4");
      EXPECT(is_byte_stream_zero(mem_init_data->InitAttributes(), mem_init_data->Size()));
    }

    SECTION( "Thumb2 style element-size == 2 init test" ) {
      uint32 size = 4;
      uint64 pa = 0x30000000;
      uint32 value = 0xa1b2c3d4;

      MemoryInitRecord* mem_init_data = record_archive.GetMemoryInitRecord(thread_id, size, 2, EMemDataType::Instruction);
      mem_init_data->SetData(pa, 1, value, size, false);
      EXPECT(mem_init_data->DataString()  == "b2a1d4c3");
      EXPECT(is_byte_stream_zero(mem_init_data->InitAttributes(), mem_init_data->Size()));
    }

    SECTION( "Data total size 8, element size 4 init test" ) {
      uint64 pa = 0x40000000;
      uint64 value = 0xa1b2c3d45a6b7c8dull;

      MemoryInitRecord* mem_init_data = record_archive.GetMemoryInitRecord(thread_id, 8, 4, EMemDataType::Data);
      mem_init_data->SetData(pa, 0, value, 8, false);
      EXPECT(mem_init_data->DataString()  == "d4c3b2a18d7c6b5a");
      EXPECT(is_byte_stream_zero(mem_init_data->InitAttributes(), mem_init_data->Size()));
    }

  }
},

CASE( "Record with memory attributes" ) {

  SETUP( "Setup code for Record module" )  {
    cuint32 thread_id = 2;
    uint64 pa = 0xf235;
    uint32 mem_id = 2;
    cuint32 size = 8;
    MemoryInitRecord mem_init_data(thread_id, size, 4, EMemDataType::Data);

    SECTION( "Test SetDataWithAttributes() method" ) {
      uint8* mem_data = new uint8[size] {0x12, 0xc5, 0x89, 0xf2, 0x35, 0x44, 0x32, 0x98};
      uint8* mem_attrs = new uint8[size] {0x05, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00};
      mem_init_data.SetDataWithAttributes(pa, mem_id, mem_data, mem_attrs, size);

      EXPECT(mem_init_data.Address() == pa);
      EXPECT(mem_init_data.MemoryId() == mem_id);

      uint8* init_data = mem_init_data.InitData();
      EXPECT(data_array_to_element_value_big_endian(init_data, size) == 0x12c589f235443298ull);
      uint8* init_attrs = mem_init_data.InitAttributes();
      EXPECT(data_array_to_element_value_big_endian(init_attrs, size) == 0x0505050000000000ull);
    }

    SECTION( "Verify SetData() after SetDataWithAttributes()" ) {
      uint8* mem_data = new uint8[size] {0x12, 0xc5, 0x89, 0xf2, 0x35, 0x44, 0x32, 0x98};
      uint8* mem_attrs = new uint8[size] {0x05, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00};
      mem_init_data.SetDataWithAttributes(pa, mem_id, mem_data, mem_attrs, size);

      uint64 updated_pa = 0x1235;
      uint32 updated_mem_id = 5;
      uint8* updated_mem_data = new uint8[size] {0x44, 0x39, 0x63, 0xf2, 0x42, 0xab, 0xcd, 0xef};
      mem_init_data.SetData(updated_pa, updated_mem_id, updated_mem_data, size);

      EXPECT(mem_init_data.Address() == updated_pa);
      EXPECT(mem_init_data.MemoryId() == updated_mem_id);

      uint8* init_data = mem_init_data.InitData();
      EXPECT(data_array_to_element_value_big_endian(init_data, size) == 0x443963f242abcdefull);
      EXPECT(is_byte_stream_zero(mem_init_data.InitAttributes(), mem_init_data.Size()));
    }

    // Specifying a null attributes pointer is not an expected use case, but we test it here to make sure it doesn't
    // crash
    SECTION( "Test setting attributes to null" ) {
      uint8* mem_data = new uint8[size] {0x12, 0xc5, 0x89, 0xf2, 0x35, 0x44, 0x32, 0x98};
      mem_init_data.SetDataWithAttributes(pa, mem_id, mem_data, nullptr, size);
      uint8* init_data = mem_init_data.InitData();
      EXPECT(data_array_to_element_value_big_endian(init_data, size) == 0x12c589f235443298ull);

      uint64 updated_data_value = 0xffffee65eeeea5b9;
      mem_init_data.SetData(pa, mem_id, updated_data_value, size, true);
      init_data = mem_init_data.InitData();
      EXPECT(data_array_to_element_value_big_endian(init_data, size) == updated_data_value);
      EXPECT(mem_init_data.InitAttributes() == nullptr);

      uint8* updated_mem_data = new uint8[size] {0x44, 0x39, 0x63, 0xf2, 0x42, 0xab, 0xcd, 0xef};
      mem_init_data.SetData(pa, mem_id, updated_mem_data, size);
      init_data = mem_init_data.InitData();
      EXPECT(data_array_to_element_value_big_endian(init_data, size) == 0x443963f242abcdefull);
      EXPECT(mem_init_data.InitAttributes() == nullptr);

      uint8* updated_mem_data_2 = new uint8[size] {0x12, 0x21, 0x34, 0x43, 0xa9, 0x9a, 0xb6, 0x6b};
      uint8* mem_attrs = new uint8[size] {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00};
      mem_init_data.SetDataWithAttributes(pa, mem_id, updated_mem_data_2, mem_attrs, size);

      init_data = mem_init_data.InitData();
      EXPECT(data_array_to_element_value_big_endian(init_data, size) == 0x12213443a99ab66bull);
      uint8* init_attrs = mem_init_data.InitAttributes();
      EXPECT(data_array_to_element_value_big_endian(init_attrs, size) == 0x0101010101010100ull);
    }
  }
},

CASE( "Record with memory access type" ) {

  SETUP( "Setup code for Record module" )  {
    cuint32 thread_id = 3;
    MemoryInitRecord mem_init_data_unknown(thread_id, 8, 4, EMemDataType::Data);
    MemoryInitRecord mem_init_data_write(thread_id, 8, 4, EMemDataType::Data, EMemAccessType::Write);

    SECTION( "Test AccessType() method" ) {
      EXPECT(mem_init_data_unknown.AccessType() == EMemAccessType::Unknown);
      EXPECT(mem_init_data_write.AccessType() == EMemAccessType::Write);
    }

    SECTION( "Test GetMemoryInitRecord() with access type" ) {
      RecordArchive record_archive;
      MemoryInitRecord* mem_init_data = record_archive.GetMemoryInitRecord(thread_id, 4, 2, EMemDataType::Instruction, EMemAccessType::Read);
      EXPECT(mem_init_data->AccessType() == EMemAccessType::Read);
    }

    SECTION( "Test getting thread ID" ) {
      EXPECT(mem_init_data_write.ThreadId() == thread_id);
    }
  }
},

};

int main( int argc, char * argv[] )
{
    Force::Logger::Initialize();
    int ret = lest::run( specification, argc, argv );
    Force::Logger::Destroy();
    return ret;
}
