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
#include <SetupRootPageTableRISCV.h>
#include <MemoryManager.h>
#include <Log.h>
#include <PageTableManager.h>
#include <Constraint.h>

#include <sstream>

/*!
  \file SetupRootPageTableRISCV.cc
  \brief Code to set up root page table
*/

using namespace std;

namespace Force {

  SetupRootPageTableRISCV::SetupRootPageTableRISCV()
  {

  }

  SetupRootPageTableRISCV::~SetupRootPageTableRISCV()
  {

  }

  uint64 SetupRootPageTableRISCV::SetupRootPageTable(uint32 tableSize, MemoryManager* pMemMgr, EMemBankType bankType, const RegisterFile* pRegFile, const std::string& regName, const ConstraintSet* usable)
  {
    LOG(debug) << "[SetupRootPageTableRISCV::SetupRootPageTable] table-size: " << std::dec << tableSize << " (0x" << std::hex << tableSize << ")" << std::dec << " reg: " << regName << std::endl;

    uint64 root_addr = 0;
    auto reg_ptr = pRegFile->RegisterLookup(regName);
    auto field_ptr = reg_ptr->RegisterFieldLookup("PPN");

    if (!field_ptr->IsInitialized())
    {
      pMemMgr->AllocatePageTableBlock(bankType, tableSize, tableSize, usable, root_addr);
      LOG(debug) << "[SetupRootPageTableRISCV::SetupRootPageTable] initial root-address: 0x" << std::hex << root_addr << std::dec << std::endl;
      pRegFile->InitializeRegisterFieldFullValue(reg_ptr, "PPN", (root_addr >> 12)); //field is just PPN, so 44:0 need to be written ignoring the page offset (should be 0x000 for this case)
      auto reg_ptrX = pRegFile->RegisterLookup(regName);
      auto field_ptrX = reg_ptrX->RegisterFieldLookup("PPN");
      uint64 root_addrX = (field_ptrX->Value() << 12);
      LOG(debug) << "[SetupRootPageTableRISCV::SetupRootPageTable] satp.PPN: 0x" << std::hex << field_ptr->Value() << " yields table addr: 0x" << root_addrX << std::dec << std::endl;
    }
    else
    {
      root_addr = (field_ptr->Value() << 12);
      uint64 end = root_addr + (tableSize - 1);
      LOG(debug) << "[SetupRootPageTableRISCV::SetupRootPageTable] root-address: 0x" << std::hex << root_addr << " end: 0x" << end << std::dec << std::endl;
      ConstraintSet addr_constr(root_addr, end);

      if (usable->ContainsConstraintSet(addr_constr))
      {
        auto allocated_constr = pMemMgr->GetPageTableManager(bankType)->Allocated();
        if (not allocated_constr->ContainsConstraintSet(addr_constr)) // not allocated, try to reserved.
        {
          pMemMgr->AllocatePageTableBlock(bankType, tableSize, tableSize, &addr_constr, root_addr);
        }
      }
      else
      {
        LOG(fail) << "{SetupRootPageTableRISCV::SetupRootPageTable} " << regName << ".PPN is initialized outside of usable range" << endl;
        FAIL("base_addr_out_of_range");
      }
    }

    LOG(notice) << "{SetupRootPageTableRISCV::SetupRootPageTable} root_addr=0x" << hex << root_addr << endl;

    return root_addr;
  }
}
