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
#include <Random.h>
#include <Defines.h>
#include <pybind11/pybind11.h>
#include <pybind11/eval.h>
#include <pybind11/stl.h>

// A command line utility that exposes the Force way of generating a 64 bit seed.
// To be used by the regression test system.
using namespace Force;
namespace py = pybind11;

uint64 seedgen(){
  Random::Initialize();

  Random* myRandomInstance = Random::Instance();

  uint64 mySeedValue = 0;
  mySeedValue = myRandomInstance->RandomSeed();

  Random::Destroy();

  return mySeedValue;
}

PYBIND11_MODULE(seedgen, m) {
  m.doc() = "pybind seedgen plugin";
  m.def("seedgen", &seedgen, "A function that generates a 64bit random seed in the same manner used in Force.");
}
