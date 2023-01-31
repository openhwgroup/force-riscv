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
#include "ImageIO.h"

#include <cstdio>
#include <map>

#include "lest/lest.hpp"

#include "Memory.h"
#include "Register.h"

#define CASE( name ) lest_CASE( specification(), name )

using namespace Force;
using text = std::string;

extern RegisterFile* register_file_top;
extern lest::tests& specification();

CASE("Test write memory to image file, PrintMemoryImage")
{
  Memory write_mem(EMemBankType::Default);
  // instruction sequence for ending emulation
  write_mem.Initialize(0xfffffffff088, 0x090080d2ull, 4, EMemDataType::Instruction);
  write_mem.Initialize(0x0000ffff0040, 0x8a0080d2ull, 4, EMemDataType::Instruction);
  write_mem.Initialize(0x0000ffff0044, 0x2a0500f8ull, 4, EMemDataType::Instruction);

  write_mem.Initialize(0xfffffffffff0, 0x0001020304050607ull, 8, EMemDataType::Data);
  write_mem.Initialize(0x0000abcd0008, 0x08090a0b0c0d0e0full, 8, EMemDataType::Data);
  write_mem.Initialize(0x0000ffff0020, 0x04030201ull, 4, EMemDataType::Instruction);
  write_mem.Initialize(0x0000ffff0024, 0x08070605ull, 4, EMemDataType::Instruction);
  write_mem.Initialize(0x0000ffff0028, 0x0c0b0a09ull, 4, EMemDataType::Instruction);
  write_mem.Initialize(0xffffffff002c, 0x0960a2f2ull, 4, EMemDataType::Instruction);

  ImageIO* image_printer = new ImageIO;
  const std::string output_file_path = "./image_printer_memory_test.img";
  image_printer->PrintMemoryImage(output_file_path, &write_mem);

  Memory read_mem(EMemBankType::Default);
  image_printer->LoadMemoryImage(output_file_path, &read_mem);

  EXPECT(write_mem.ReadInitialValue(0xfffffffff088, 4) == read_mem.ReadInitialValue(0xfffffffff088, 4));
  EXPECT(write_mem.ReadInitialValue(0x0000ffff0040, 8) == read_mem.ReadInitialValue(0x0000ffff0040, 8));
  EXPECT(write_mem.ReadInitialValue(0x0000ffff0020, 8) == read_mem.ReadInitialValue(0x0000ffff0020, 8));
  EXPECT(write_mem.ReadInitialValue(0xfffffffffff0, 8) == read_mem.ReadInitialValue(0xfffffffffff0, 8));
  //write_mem.Dump(std::cout);
  //read_mem.Dump(std::cout);
  remove(output_file_path.c_str());
  delete image_printer;
}

CASE ("Test write register to image file, PrintRegistersImage")
{
  std::map<std::string, uint64> write_thread_info;
  write_thread_info["ThreadID"] = 0;
  write_thread_info["BootPC"] = 0x80000000;
  write_thread_info["InitialPC"] = 0x80001000;

  RegisterFile* read_register_file = dynamic_cast<RegisterFile*>(register_file_top->Clone());
  read_register_file->Setup();

  RegisterFile* write_register_file = dynamic_cast<RegisterFile*>(register_file_top->Clone());
  write_register_file->Setup();

  // test Sys regsiter
  write_register_file->InitializeRegister("misa", 0xF, nullptr);
  write_register_file->InitializeRegister("mstatus", 0xffffffffffffffff, nullptr);
  write_register_file->InitializeRegister("satp", 0xffffffffffffffff, nullptr);

  // test GPR
  std::vector<uint64> x1_data;
  x1_data.push_back(0x8f2f2bfc499919f4);
  write_register_file->InitializeRegister("x1", x1_data, nullptr);

  // test floating point register
  std::vector<uint64> d0_data;
  d0_data.push_back(0xfedcba9876543210);
  write_register_file->InitializeRegister("D0", d0_data, nullptr);

  std::vector<uint64> s1_data;
  s1_data.push_back(0x59ab6f56);
  write_register_file->InitializeRegister("S1", s1_data, nullptr);

  // test vector register
  std::vector<uint64> v0_data;
  v0_data.push_back(0xfedcba9876543210);
  v0_data.push_back(0x0123456789abcdef);
  write_register_file->InitializeRegister("v0", v0_data, nullptr);

  ImageIO* image_printer = new ImageIO;
  const std::string output_file_path = "./image_printer_register_test.img";
  image_printer->PrintRegistersImage(output_file_path, write_thread_info, write_register_file);

  std::map<std::string, uint64> read_thread_info;
  read_thread_info["ThreadID"] = 0;
  image_printer->LoadRegistersImage(output_file_path, read_thread_info, read_register_file);

  PhysicalRegister* w_phy = write_register_file->PhysicalRegisterLookup("mstatus");
  PhysicalRegister* r_phy = read_register_file->PhysicalRegisterLookup("mstatus");
  EXPECT(w_phy->InitialValue(MAX_UINT64) == r_phy->InitialValue(MAX_UINT64));

  w_phy = write_register_file->PhysicalRegisterLookup("x1");
  r_phy = read_register_file->PhysicalRegisterLookup("x1");
  EXPECT(w_phy->InitialValue(MAX_UINT64) == r_phy->InitialValue(MAX_UINT64));

  w_phy = write_register_file->PhysicalRegisterLookup("f0_0");
  r_phy = read_register_file->PhysicalRegisterLookup("f0_0");
  EXPECT(w_phy->InitialValue(MAX_UINT64) == r_phy->InitialValue(MAX_UINT64));

  w_phy = write_register_file->PhysicalRegisterLookup("f1_0");
  r_phy = read_register_file->PhysicalRegisterLookup("f1_0");
  EXPECT(w_phy->InitialValue(MAX_UINT32) == r_phy->InitialValue(MAX_UINT32));

  w_phy = write_register_file->PhysicalRegisterLookup("v0_0");
  r_phy = read_register_file->PhysicalRegisterLookup("v0_0");
  EXPECT(w_phy->InitialValue(MAX_UINT64) == r_phy->InitialValue(MAX_UINT64));

  w_phy = write_register_file->PhysicalRegisterLookup("v0_1");
  r_phy = read_register_file->PhysicalRegisterLookup("v0_1");
  EXPECT(w_phy->InitialValue(MAX_UINT64) == r_phy->InitialValue(MAX_UINT64));

  remove(output_file_path.c_str());
  delete image_printer;
}
