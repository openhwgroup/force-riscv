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
#include <AddressTableManagerRISCV.h>
#include <AddressTable.h>
#include <Generator.h>
#include <Register.h>
#include <RecoveryAddressGeneratorRISCV.h>
#include <MemoryManager.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <Constraint.h>
#include <PaGenerator.h>
#include <Log.h>

using namespace std;

namespace Force {

  AddressTableManagerRISCV::AddressTableManagerRISCV(const AddressTableManagerRISCV& rOther)
    : AddressTableManager(rOther)
  {
  }

  void AddressTableManagerRISCV::Setup(Generator* pGen)
  {
    AddressTableManager::Setup(pGen);

    for (AddressTable* addr_table : GetAddressTables())
    {
      addr_table->SetRecoveryAddressGenerator(new RecoveryAddressGeneratorRISCV(pGen));
    }

    const Generator* generator = GetGenerator();
    auto reg_file = generator->GetRegisterFile();
    auto phys_ptr = reg_file->PhysicalRegisterLookup("privilege");
    LOG(notice) << "{AddressTableManagerRISCV} trying to get privilege physical register" << endl;
    if(phys_ptr == nullptr)
    {	
	LOG(notice) << "Can't find privilege"<< endl;
    }
    auto conf_ptr = dynamic_cast<ConfigureRegister*>(phys_ptr);
    if(conf_ptr == nullptr)
    {	
	LOG(notice) << "Can't cast privilege"<< endl;
    }
    else
    {
      conf_ptr->SignUp(this);
    }
  }

  void AddressTableManagerRISCV::GetReloadRegisters(cuint32 targetMemBank, cuint32 targetEl, map<string, uint64>& rReloadMap) const
  {
    const AddressTable* addr_table = GetAddressTable(0);

    auto addr_reg_ptr = addr_table->GetTableIndexRegister();
    uint64 cur_addr = addr_table->GetCurrentAddress() + 8;
    if (not addr_reg_ptr->IsInitialized()) {
      LOG(fail) << "{AddressTableManagerRISCV::GetReloadRegisters} address table recovery register not initialized" << endl;
      FAIL("uninitialized-recovery-register");
    }
    if (addr_reg_ptr->Value() != cur_addr)
    {
      rReloadMap.insert(pair<string, uint64>(addr_reg_ptr->Name(), cur_addr));
    }
  }

  void AddressTableManagerRISCV::HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload)
  {
    UpdateCurrentAddressTable();
  }

  void AddressTableManagerRISCV::UpdateCurrentAddressTable()
  {
    SetCurrentAddressTableByBank(EMemBankType(0));
  }

  uint64 AddressTableManagerRISCV::GenerateNewTable(uint64 baseAddress, uint64 tableSize, EMemBankType bankType)
  {
    uint64 ret_pa = baseAddress;

    MemoryManager* mem_man = mpGenerator->GetMemoryManager();
    if (ret_pa == 0x0)
    {
      const ConstraintSet* usable_constr  = mem_man->GetMemoryBank(EMemBankTypeBaseType(bankType))->Usable();

      PaGenerator pa_gen(usable_constr);
      ConstraintSet min_ps_constr(mpGenerator->GetVariable("System Physical Address Range", EVariableType::String));
      uint32 align = 8;
      bool is_instr = false;
      ret_pa = pa_gen.GenerateAddress(align, tableSize, is_instr, &min_ps_constr);
    }

    LOG(notice) << "{AddressTableRISCV::GenerateNewTable} size 0x" << hex << tableSize << " base address 0x" << ret_pa << endl;

    char print_buffer[32];
    snprintf(print_buffer, 32, "AddressTable%d", NewTableId());
    mpGenerator->ReserveMemory(print_buffer, ret_pa, tableSize, EMemBankTypeBaseType(bankType), false);

    // Memory manager pushes back and owns the physical region at this point
    uint64 max_pa = ret_pa + (tableSize - 1);
    auto mem_man_phys_region = new PhysicalRegion(print_buffer, ret_pa, max_pa, EPhysicalRegionType::AddressTable, bankType, EMemDataType::Data);
    mem_man->AddPhysicalRegion(mem_man_phys_region);

    // VM mapper deletes the physical region once mapped
    VmManager* vm_manager = mpGenerator->GetVmManager();
    VmRegime* vm_regime = vm_manager->CurrentVmRegime();
    if (vm_regime->DefaultMemoryBank() == bankType)
    {
      VmMapper* vm_mapper = vm_manager->CurrentVmMapper();
      auto vm_mapper_phys_region = new PhysicalRegion(print_buffer, ret_pa, max_pa, EPhysicalRegionType::AddressTable, bankType, EMemDataType::Data);
      vm_mapper->AddPhysicalRegion(vm_mapper_phys_region, true);
    }

    return ret_pa;
  }
}
