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
#include "Register.h"

#include "lest/lest.hpp"

#include "Architectures.h"
#include "Config.h"
#include "Log.h"
#include "ObjectRegistry.h"
#include "Random.h"
#include "RegisterRISCV.h"
#include "UnitTestUtilities.h"

#define CASE( name ) lest_CASE( specification(), name )

using namespace std;
using namespace Force;
using text = std::string;

RegisterFile* register_file_top;

lest::tests& specification()
{
  static lest::tests tests;
  return tests;
}

int main(int argc, char* argv[])
{
  //Initialize Logger + RNG
  Logger::Initialize();
  Random::Initialize();

  //arch info template will get filled with default register classes + UT regfiles names
  vector<ArchInfo* > arch_info_objs;
  arch_info_objs.push_back(new ArchInfoTest("RegisterUnitTest"));
  Architectures::Initialize();
  Architectures::Instance()->AssignArchInfoObjects(arch_info_objs);

  //Initialize Test Config Object (used for xml paths)
  Config::Initialize();
  auto config = Config::Instance();
  config->LoadConfigFile("config/register_unit.config", argv[0]);

  //Register objects needed by RegisterParser (xml parser)
  ObjectRegistry::Initialize();
  ObjectRegistry* obj_reg = ObjectRegistry::Instance();
  obj_reg->RegisterObject(new PhysicalRegister());
  obj_reg->RegisterObject(new ConfigureRegister());
  obj_reg->RegisterObject(new LargeRegister());
  obj_reg->RegisterObject(new Register());
  obj_reg->RegisterObject(new ReadOnlyRegister());
  obj_reg->RegisterObject(new ReadOnlyZeroRegister());
  obj_reg->RegisterObject(new RegisterField());
  obj_reg->RegisterObject(new RegisterFieldRes0());
  obj_reg->RegisterObject(new RegisterFieldRes1());
  obj_reg->RegisterObject(new PhysicalRegisterRazwi());

  //load test_register_file with xml files specified in register_unit.config
  std::list<std::string> files = Architectures::Instance()->DefaultArchInfo()->RegisterFiles();
  register_file_top = new RegisterFile();
  register_file_top->LoadRegisterFiles(files);
  register_file_top->Setup();

  int ret = lest::run(specification(), argc, argv);

  //Cleanup test objects
  ObjectRegistry::Destroy();
  Config::Destroy();
  Architectures::Destroy();
  Random::Destroy();
  Logger::Destroy();

  return ret;
}
