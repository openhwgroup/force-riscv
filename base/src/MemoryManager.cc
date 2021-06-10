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
#include <MemoryManager.h>
#include <Memory.h>
#include <Architectures.h>
#include <Record.h>
#include <TestIO.h>
#include <Config.h>
#include <PathUtils.h>
#include <Constraint.h>
#include <MemoryReservation.h>
#include <PhysicalPageManager.h>
#include <PageTableManager.h>
#include <VmUtils.h>
#include <PagingChoicesAdapter.h>
#include <MemoryConstraint.h>
#include <MemoryConstraintUpdate.h>
#include <AddressReuseMode.h>
#include <ImageIO.h>
#include <SymbolManager.h>
#include <MemoryTraits.h>
#include <Log.h>

#include <algorithm>
#include <sstream>

using namespace std;

namespace Force {

  MemoryBank::MemoryBank(EMemBankType bankType)
    : mpMemory(nullptr), mpBaseConstraint(nullptr), mpFree(nullptr), mpUsable(nullptr), mpPhysicalPageManager(nullptr), mpPageTableManager(nullptr), mpSymbolManager(nullptr), mpMemTraitsManager(nullptr)
  {
    mpMemory = new Memory(bankType);
    mpBaseConstraint = new ConstraintSet();

    Config* config = Config::Instance();
    mpUsable = new MultiThreadMemoryConstraint(config->NumChips() * config->NumCores() * config->NumThreads());
    mpSymbolManager = new SymbolManager(bankType);

    Architectures* arch = Architectures::Instance();
    ArchInfo* arch_info = arch->DefaultArchInfo();
    mpMemTraitsManager = new MemoryTraitsManager(arch_info->InstantiateMemoryTraitsRegistry());
  }

  MemoryBank::~MemoryBank()
  {
    delete mpMemory;
    delete mpBaseConstraint;
    delete mpFree;
    delete mpUsable;
    delete mpPhysicalPageManager;
    delete mpPageTableManager;
    delete mpSymbolManager;
    delete mpMemTraitsManager;
  }

  EMemBankType MemoryBank::MemoryBankType() const
  {
    return mpMemory->MemoryBankType();
  }

  void MemoryBank::AddMemoryRange(uint64 start, uint64 end)
  {
    mpBaseConstraint->AddRange(start, end);
  }

  void MemoryBank::SubMemoryRange(uint64 start, uint64 end)
  {
    mpBaseConstraint->SubRange(start, end);
  }

  void MemoryBank::Configure()
  {
    mpFree = new ConstraintSet(*mpBaseConstraint);
    mpUsable->Initialize(*mpBaseConstraint);
  }

  void MemoryBank::InitializeMemory(const MemoryInitRecord& memInitRecord)
  {
    uint64 address = memInitRecord.Address();
    uint32 bytes = memInitRecord.Size();
    uint64 init_end = address + (bytes - 1);

    mpMemory->Initialize(address, memInitRecord.InitData(), memInitRecord.InitAttributes(), bytes, memInitRecord.InitType());
    mpFree->SubRange(address, init_end);
    mpUsable->MarkUsedForType(address, init_end, memInitRecord.InitType(), memInitRecord.AccessType(), memInitRecord.ThreadId());

    LOG(trace) << "{MemoryBank::InitializeMemory} bank=" << EMemBankType_to_string(MemoryBankType()) << " start_addr=0x"
               << hex << address << " end_addr=0x" << init_end << endl;

    if (mpPhysicalPageManager != nullptr)
    {
      MarkUsedForTypeUpdate mem_constr_update(address, init_end, memInitRecord.InitType(), memInitRecord.AccessType(), memInitRecord.ThreadId());
      mpPhysicalPageManager->HandleMemoryConstraintUpdate(mem_constr_update);
    }
  }

  void MemoryBank::ReadMemoryPartiallyInitialized(cuint64 address, cuint32 nBytes, uint8* data) const
  {
    mpMemory->ReadPartiallyInitialized(address, nBytes, data);
  }

  void MemoryBank::WriteMemory(uint64 address, cuint8* data, uint32 nBytes)
  {
    mpMemory->Write(address, data, nBytes);
  }

  void MemoryBank::MarkShared(cuint64 address, cuint32 nBytes)
  {
    uint64 end_address = address + nBytes - 1;
    mpUsable->MarkShared(address, end_address);

    LOG(trace) << "{MemoryBank::MarkShared} bank=" << EMemBankType_to_string(MemoryBankType()) << " start_addr=0x"
               << hex << address << " end_addr=0x" << end_address << endl;

    if (mpPhysicalPageManager != nullptr)
    {
      SharedUpdate mem_constr_update(address, end_address);
      mpPhysicalPageManager->HandleMemoryConstraintUpdate(mem_constr_update);
    }
  }

  void MemoryBank::GetMemoryAttributes(cuint64 address, cuint32 nBytes, uint8* memAttrs) const
  {
    mpMemory->GetMemoryAttributes(address, nBytes, memAttrs);
  }

  uint8 MemoryBank::GetByteMemoryAttributes(cuint64 address) const
  {
    return mpMemory->GetByteMemoryAttributes(address);
  }

  const ConstraintSet* MemoryBank::Usable() const
  {
    return mpUsable->Usable();
  }

  const ConstraintSet* MemoryBank::Shared() const
  {
    return mpUsable->Shared();
  }

  const ConstraintSet* MemoryBank::Unmapped() const
  {
    return mpPhysicalPageManager->GetUsable();
  }

  void MemoryBank::ApplyUsableConstraint(const EMemDataType memDataType, const EMemAccessType memAccessType, cuint32 threadId, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const
  {
    mpUsable->ApplyToConstraintSet(memDataType, memAccessType, threadId, rAddrReuseMode, constrSet);
  }

  void MemoryBank::SetupPageTableRegion()
  {
    if (nullptr != mpPhysicalPageManager) {
      // page table constraint already setup.
      return;
    }

    //Page manager should ignore reserved memory and use mpBaseConstraint masked to the physical range to only
    //avoid allocating pages in predefined/unsupported physical memory ranges.
    Architectures* arch = Architectures::Instance();
    ArchInfo* arch_info = arch->DefaultArchInfo();
    mpPhysicalPageManager = arch_info->InstantiatePhysicalPageManager(MemoryBankType(), mpMemTraitsManager);
    uint64 pa_limit = Config::Instance()->LimitValue(ELimitType::PhysicalAddressLimit);
    ConstraintSet phys_range(0x0ull, pa_limit);
    mpPhysicalPageManager->Initialize(&phys_range, mpBaseConstraint);

    if (nullptr != mpPageTableManager) {
      return;
    }

    mpPageTableManager = new PageTableManager(MemoryBankType());
  }

  void MemoryBank::ReserveMemory(const ConstraintSet& memConstr)
  {
    mpUsable->MarkUsed(memConstr);
    if (mpPhysicalPageManager != nullptr)
    {
      mpPhysicalPageManager->SubFromBoundary(memConstr);
      if (!memConstr.IsEmpty())
      {
        for (auto& constr : memConstr.GetConstraints())
        {
          MarkUsedUpdate mem_constr_update(constr->LowerBound(), constr->UpperBound());
          mpPhysicalPageManager->HandleMemoryConstraintUpdate(mem_constr_update);
        }
      }
    }
  }

  void MemoryBank::UnreserveMemory(const ConstraintSet& memConstr)
  {
    mpUsable->UnmarkUsed(memConstr);

    if (mpPhysicalPageManager != nullptr)
    {
      mpPhysicalPageManager->AddToBoundary(memConstr);
      if (!memConstr.IsEmpty())
      {
        for (auto& constr : memConstr.GetConstraints())
        {
          UnmarkUsedUpdate mem_constr_update(constr->LowerBound(), constr->UpperBound());
          mpPhysicalPageManager->HandleMemoryConstraintUpdate(mem_constr_update);
        }
      }
    }
  }

  bool MemoryBank::AllocatePageTableBlock(uint64 align, uint64 size, const ConstraintSet* range, uint64& start)
  {
    uint64 allocated = mpPageTableManager->AllocatePageTableBlock(align, size, Usable(), range, start);
    if (allocated) {
      uint64 pt_end = start + (size - 1);
      ConstraintSet allocate_constr(start, pt_end);
      mpUsable->MarkUsed(allocate_constr);

      for (auto& constr : allocate_constr.GetConstraints())
      {
        MarkUsedUpdate mem_constr_update(constr->LowerBound(), constr->UpperBound());
        mpPhysicalPageManager->HandleMemoryConstraintUpdate(mem_constr_update);
      }
    }
    return allocated;
  }

  MemoryManager * MemoryManager::mspMemoryManager = nullptr;

  void MemoryManager::Initialize()
  {
    if (nullptr == mspMemoryManager) {
      mspMemoryManager = new MemoryManager();
      mspMemoryManager->Setup();
    }
  }

  void MemoryManager::Destroy()
  {
    delete mspMemoryManager;
    mspMemoryManager = nullptr;
  }

  MemoryManager::MemoryManager()
    : mMemoryBanks(), mConstraintConfigured(false), mReservations(), mPhysicalRegions()
  {

  }

  MemoryManager::~MemoryManager()
  {
    for (auto mem_bank : mMemoryBanks) {
      delete mem_bank;
    }

    for (auto reserv_item : mReservations) {
      delete reserv_item.second;
    }

    for (auto phys_region : mPhysicalRegions) {
      delete phys_region;
    }
  }

  void MemoryManager::Setup()
  {
    Architectures* arch_ptr = Architectures::Instance();
    list<EMemBankType> mem_banks = arch_ptr->DefaultArchInfo()->MemoryBankTypes();

    transform(mem_banks.cbegin(), mem_banks.cend(), back_inserter(mMemoryBanks),
      [](const EMemBankType bankType) { return new MemoryBank(bankType); });
  }

  void MemoryManager::BankOutOfBound(uint32 bank) const
  {
    LOG(fail) << "{MemoryManager::BankOutOfBound} accessing non existent memory bank \"" << dec << bank << "\"." << endl;
    FAIL("memory-bank-out-of-bound");
  }

  void MemoryManager::AddMemoryRange(uint32 bank, uint64 start, uint64 end)
  {
    if (mConstraintConfigured) {
      LOG(fail) << "{MemoryManager::AddMemoryRange} adding memory range after base line memory constraints have been configured." << endl;
      FAIL("modifying-memory-ranges-after-constraints-configured");
    }

    MemoryBank* mem_bank = GetMemoryBank(bank);
    mem_bank->AddMemoryRange(start, end);
  }

  void MemoryManager::SubMemoryRange(uint32 bank, uint64 start, uint64 end)
  {
    if (mConstraintConfigured) {
      LOG(fail) << "{MemoryManager::AddMemoryRange} adding memory range after base line memory constraints have been configured." << endl;
      FAIL("modifying-memory-ranges-after-constraints-configured");
    }

    MemoryBank* mem_bank = GetMemoryBank(bank);
    mem_bank->SubMemoryRange(start, end);
  }

  void MemoryManager::ConfigureMemoryBanks()
  {
    if (mConstraintConfigured) {
      LOG(fail) << "{MemoryManager::ConfigureMemoryBanks} trying to configure base line memory constraints again." << endl;
      FAIL("configuring-base-line-constraint-again");
    }
    mConstraintConfigured = true;

    for (auto mem_bank : mMemoryBanks) {
      mem_bank->Configure();
    }
  }

  void MemoryManager::InitializeMemory(const MemoryInitRecord* memInitRecord)
  {
    LOG(info) << "InitializeMemory: " << memInitRecord->ToString() << endl;
    MemoryBank* mem_bank = GetMemoryBank(memInitRecord->MemoryId());
    mem_bank->InitializeMemory(*memInitRecord);
  }

  void MemoryManager::ReadMemoryPartiallyInitialized(const PaTuple& rPaTuple, cuint32 size, uint8* memData) const
  {
    const MemoryBank* mem_bank = GetMemoryBank(rPaTuple.mBank);
    mem_bank->ReadMemoryPartiallyInitialized(rPaTuple.mAddress, size, memData);
  }

  void MemoryManager::MarkShared(const PaTuple& rPaTuple, cuint32 size)
  {
    MemoryBank* mem_bank = GetMemoryBank(rPaTuple.mBank);
    mem_bank->MarkShared(rPaTuple.mAddress, size);
  }

  void MemoryManager::GetMemoryAttributes(const PaTuple& rPaTuple, cuint32 size, uint8* memAttrs) const
  {
    const MemoryBank* mem_bank = GetMemoryBank(rPaTuple.mBank);
    mem_bank->GetMemoryAttributes(rPaTuple.mAddress, size, memAttrs);
  }

  void MemoryManager::SetupPageTableRegions()
  {
    for (auto mem_bank : mMemoryBanks) {
      mem_bank->SetupPageTableRegion();
    }
  }

  PhysicalPageManager* MemoryManager::GetPhysicalPageManager(EMemBankType memType)
  {
    MemoryBank* mem_bank = GetMemoryBank(uint32(memType));
    return mem_bank->GetPhysicalPageManager();
  }

  bool MemoryManager::AllocatePageTableBlock(EMemBankType memType, uint64 align, uint64 size, const ConstraintSet* usable, uint64& start)
  {
    MemoryBank* mem_bank = GetMemoryBank(uint32(memType));
    return mem_bank->AllocatePageTableBlock(align, size, usable, start);
  }

  PageTableManager* MemoryManager::GetPageTableManager(EMemBankType memType) const
  {
    const MemoryBank* mem_bank = GetMemoryBank(uint32(memType));
    return mem_bank->GetPageTableManager();
  }

  void MemoryManager::ReserveMemory(MemoryReservation* pMemReserv)
  {
    LOG(notice) << "{MemoryManager::ReserveMemory} " << pMemReserv->ToString() << endl;
    auto existing_reserv = mReservations.find(pMemReserv->Name());
    if (existing_reserv != mReservations.end()) {
      LOG(fail) << "{MemoryManager::ReserveMemory} duplicated reservation name: " << pMemReserv->Name() << endl;
      FAIL("duplicated-memory-reservation-name");
    }
    mReservations[pMemReserv->Name()] = pMemReserv;

    for (EMemBankTypeBaseType bank_value = 0; bank_value < EMemBankTypeSize; ++ bank_value) {
      EMemBankType bank_type = EMemBankType(bank_value);
      auto reserv_constr = pMemReserv->GetReservation(bank_type);
      if (nullptr != reserv_constr) {
        MemoryBank* mem_bank = GetMemoryBank(uint32(bank_type));
        mem_bank->ReserveMemory(*reserv_constr);
      }
    }
  }

  void MemoryManager::UnreserveMemory(MemoryReservation* pMemReserv)
  {
    LOG(notice) << "{MemoryManager::UnreserveMemory} " << pMemReserv->ToString() << endl;
    auto existing_reserv = mReservations.find(pMemReserv->Name());
    if (existing_reserv == mReservations.end()) {
      LOG(fail) << "{MemoryManager::UnreserveMemory} no reservation name: " << pMemReserv->Name() << endl;
      FAIL("no-memory-reservation-name");
    }

    for (EMemBankTypeBaseType bank_value = 0; bank_value < EMemBankTypeSize; ++ bank_value) {
      EMemBankType bank_type = EMemBankType(bank_value);
      auto reserv_constr = pMemReserv->GetReservation(bank_type);
      if (nullptr != reserv_constr) {
        MemoryBank* mem_bank = GetMemoryBank(uint32(bank_type));
        mem_bank->UnreserveMemory(*reserv_constr);
      }
    }

    delete existing_reserv->second;
    mReservations.erase(existing_reserv);

    delete pMemReserv;
  }

  void MemoryManager::AddPhysicalRegion(PhysicalRegion* pPhysRegion)
  {
    mPhysicalRegions.push_back(pPhysRegion);

    SendNotification(ENotificationType::PhysicalRegionAdded, pPhysRegion);
  }

  bool MemoryManager::PaInitialized(const PaTuple& rPaTuple, cuint32 size) const
  {
    vector<uint8> mem_attrs(size);
    GetMemoryBank(rPaTuple.mBank)->GetMemoryAttributes(rPaTuple.mAddress, size, mem_attrs.data());

    auto init_test = [](uint8 byte_mem_attrs) {
      return ((byte_mem_attrs & EMemDataTypeBaseType(EMemDataType::Init)) == EMemDataTypeBaseType(EMemDataType::Init));
    };

    return all_of(mem_attrs.cbegin(), mem_attrs.cend(), init_test);
  }

  bool MemoryManager::InstructionPaInitialized(const PaTuple& rPaTuple) const
  {
    uint32 min_instr_size = 2;
    return PaInitialized(rPaTuple, min_instr_size);
  }

  void MemoryManager::OutputTest(const std::map<uint32, Generator *>& generators, uint64 resetPC, uint32 machineType)
  {
    auto cfg_handle = Config::Instance();
    uint64 initialSeed;
    string file_base = get_file_stem(cfg_handle->TestTemplate());
    if (cfg_handle->OutputWithSeed(initialSeed)) {
      stringstream seed_stream;
      seed_stream << "0x"<< hex << initialSeed;
      file_base += "_" + seed_stream.str();
    }
    for (auto mem_bank : mMemoryBanks) {
      Memory* output_mem = mem_bank->MemoryInstance();
      if (output_mem->IsEmpty()) continue;

      string mem_bank_str = EMemBankType_to_string(output_mem->MemoryBankType());
      string output_name_base = file_base + "." + mem_bank_str;
      string output_name_elf = output_name_base + ".ELF";
      string output_name_asm = output_name_base + ".S";
      TestIO output_instance(uint32(output_mem->MemoryBankType()), output_mem, mem_bank->GetSymbolManager());
      output_instance.WriteTestElf(output_name_elf, false, resetPC, machineType);
      if (cfg_handle->OutputAssembly()) {
        output_instance.WriteTestAssembly(generators, output_name_asm);
      }
    }
  }
  void MemoryManager::OutputImage() const
  {
    for (auto mem_bank : mMemoryBanks) {
      Memory* output_mem = mem_bank->MemoryInstance();
      if (output_mem->IsEmpty()) continue;

      auto cfg_handle = Config::Instance();
      uint64 initial_seed = 0;
      string output_name_img = get_file_stem(cfg_handle->TestTemplate());
      if (cfg_handle->OutputWithSeed(initial_seed)) {
        stringstream seed_stream;
        seed_stream << "0x"<< hex << initial_seed;
        output_name_img += "_" + seed_stream.str();
      }
      string mem_bank_str = EMemBankType_to_string(output_mem->MemoryBankType());
      output_name_img += "." + mem_bank_str + ".img";

      ImageIO printer;
      printer.PrintMemoryImage(output_name_img, output_mem);
    }
  }

}
