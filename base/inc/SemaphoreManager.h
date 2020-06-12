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
#ifndef Force_SemaphoreManager_H
#define Force_SemaphoreManager_H

#include <Defines.h>

#include <string>
#include <map>

namespace Force {

  class Generator;
  class Semaphore;

  /*!
    \class SemaphoreManager
    \brief class to manage semaphores.
   */
  class SemaphoreManager {
  public:
    SemaphoreManager(); //constructor
    virtual ~SemaphoreManager(); // destructor

    bool GenSemaphore(Generator *pGen, const std::string& name, uint64 counter, uint32 bank, uint32 size, uint64& address, bool& reverseEndian); //!< generate a semaphore

  protected:
    std::map<std::string, Semaphore*> mSemaphores; //!< the container for semaphores
  };

}
#endif
