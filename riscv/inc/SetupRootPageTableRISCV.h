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
#ifndef Force_SetupRootPageTableRISCV_H
#define Force_SetupRootPageTableRISCV_H

#include <VmasControlBlock.h>
#include <vector>
#include <Register.h>
#include <MemoryManager.h>
#include <PageTable.h>

namespace Force {

  /*!
    \class SetupRootPageTableRISCV
    \brief Base class for Setting up root page table.   : public VmasControlBlock
  */
  class SetupRootPageTableRISCV {
  public:
    SetupRootPageTableRISCV(); //!< Default constructor.
    ~SetupRootPageTableRISCV(); //!< Destructor.
    uint64 SetupRootPageTable(uint32 tableSize, MemoryManager* pMemMgr, EMemBankType bankType, const RegisterFile* pRegFile, const std::string& regName, const ConstraintSet* usable); //!< setup root table register.

  private:
    SetupRootPageTableRISCV(const SetupRootPageTableRISCV& rOther) { }
  };
}

#endif //Force_SetupRootPageTableRISCV_H
