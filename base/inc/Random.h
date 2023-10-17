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
#ifndef Force_Random_H
#define Force_Random_H

#include "Defines.h"

namespace Force {

  struct RandomEngine;

  /*!
    \class Random
    \brief Random number engine.
   */

  class Random {
  public:
    static void Initialize();  //!< Initialization interface.
    static void Destroy();     //!< Destruction clean up interface.
    inline static Random* Instance() { return mspRandom; } //!< Access Random engine instance.

    void Seed(uint64 seed);    //!< Set initial seed for the random engine.
    uint64 RandomSeed() const; //!< Obtain a random initial seed if no seed is specified at the command line
    uint32 Random32(uint32 min=0, uint32 max=MAX_UINT32) const; //!< Obtain a random 32 bit integer value
    uint64 Random64(uint64 min=0, uint64 max=MAX_UINT64) const; //!< Obtain a random 64 bit integer value
    double RandomReal(double min=0.0, double max=1.0) const; //!< Obtain a random 64 bit real value
  private:
    Random();  //!< Constructor, private.
    ~Random(); //!< Destructor, private.
    COPY_CONSTRUCTOR_ABSENT(Random);
    ASSIGNMENT_OPERATOR_ABSENT(Random);
  private:
    static Random* mspRandom;  //!< Static singleton pointer to random engine.
    RandomEngine * mpRandomEngine; //!< Pointer to internal RandomEngine object.
  };

  /*!
    \class RandomURBG32
    \brief A wrapper class to provide a uniform random bit generator for STL library use.
  */

  class RandomURBG32 {
  public:
    typedef uint32 result_type; //!< Type define required by STL
    static constexpr uint32 min() { return 0; } //!< min function required by STL
    static constexpr uint32 max() { return MAX_UINT32; } //!< max function required by STL
    uint32 operator () () const { return mpRandomInstance->Random32(min(), max()); }

    explicit RandomURBG32(const Random* randomInstance) : mpRandomInstance(randomInstance) { } //!< Constructor with pointer to a Random object provieded.
  private:
    RandomURBG32() : mpRandomInstance(nullptr) { } //!< Private default constructor.
  private:
    const Random* mpRandomInstance; //!< Pointer to a Random object instance.
  };

}

#endif
