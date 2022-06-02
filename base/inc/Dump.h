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
#ifndef Force_Dump_H
#define Force_Dump_H

#include "Defines.h"
#include "Enums.h"

namespace Force {

  class Scheduler;

  /*!
    \class Dump
    \brief A dumpping module
   */
  class Dump {
  public:
    static void Initialize();  //!< Initialization interface.
    static void Destroy();    //!< Destruction clean up interface.
    inline static Dump* Instance() { return mspDump; } //!< Access dump instance.
    void SetOption(const char* dumps); //!< set optons to dump
    void SetScheduler(const Scheduler* pScheduler)  { mpScheduler = pScheduler; }  //!< set scheduler to dump instruction results
    void DumpInfo(bool partial = true); //!< dump required info
  private:
    Dump( ) : mAttribute(0ull), mpScheduler(nullptr), mDumped(false), mBaseName() {}  //!< constructor, private
    virtual ~Dump( ) {} //!< destructor, private
    ASSIGNMENT_OPERATOR_ABSENT(Dump);
    COPY_CONSTRUCTOR_DEFAULT(Dump);
    static Dump *mspDump; //!< static pointer to Dump object
  private:
    void DumpPartialElf();  //!<  Dump Partial Elf
    void DumpPartialAsm();  //!< Dump partial asm
    void DumpMem(bool partial); //!< Dump memory
    void DumpPage() const; //!< Dump page
    void DumpPageAndMemoryAttributesJson() const; //!< Dump page table and memory attribute data in JSON format
    void DumpHandlerAddresses(); //!< Dump handler addresses
    void SetBaseName(); //!< set base name to dump
  private:
    uint64 mAttribute; //!< dump attribute
    const Scheduler* mpScheduler; //!< the pointer to scheduler
    bool mDumped;  //!< whether dumped or not
    std::string mBaseName; //!< base file name dumped
  };
}
#endif
