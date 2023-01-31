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
#include "Random.h"

#include <random>

#include "Log.h"

using namespace std;

/*!
  \file Random.cc
  \brief Code for random number engine
*/

namespace Force {

  /*!
    \struct RandomEngine
    \brief internal struct pointing to 32-bit and 64-bit random number engine in use
   */
  struct RandomEngine {
    RandomEngine() : mEngine32(), mEngine64() { }
    std::mt19937 mEngine32;    //!< Instance of 32-bit random number engine in use
    std::mt19937_64 mEngine64; //!< Instance of 64-bit random number engine in use
  };

  Random* Random::mspRandom = nullptr;

  void Random::Initialize()
  {
    if (nullptr == mspRandom) {
      mspRandom = new Random();
    }
  }

  void Random::Destroy()
  {
    delete mspRandom;
    mspRandom = nullptr;
  }

  Random::Random() : mpRandomEngine()
  {
    mpRandomEngine = new RandomEngine();
  }

  Random::~Random()
  {
    delete mpRandomEngine;
  }

  void Random::Seed(uint64 seed)
  {
    LOG(notice) << "Initial seed = 0x" << hex << seed << endl;
    mpRandomEngine->mEngine64.seed(seed);
    uint32 seed32 = Random64(0, UINT32_MAX);
    mpRandomEngine->mEngine32.seed(seed32);
  }

  uint64 Random::RandomSeed() const
  {
    std::random_device rd;
    uint64 ret_seed = rd();
    ret_seed <<= 32;
    ret_seed |= rd();
    return ret_seed;
  }

  uint32 Random::Random32(uint32 min, uint32 max) const
  {
    std::uniform_int_distribution<uint32> dist32(min, max);
    return dist32(mpRandomEngine->mEngine32);
  }

  uint64 Random::Random64(uint64 min, uint64 max) const
  {
    std::uniform_int_distribution<uint64> dist64(min, max);
    return dist64(mpRandomEngine->mEngine64);
  }

  double Random::RandomReal(double min, double max) const
  {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(mpRandomEngine->mEngine64);
  }

}
