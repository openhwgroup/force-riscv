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
#include <Dump.h>
#include <Log.h>
#include <Config.h>
#include <PathUtils.h>
#include <StringUtils.h>
#include <GenException.h>
#include <MemoryManager.h>
#include <ExceptionManager.h>
#include <Memory.h>
#include <TestIO.h>
#include <Scheduler.h>
#include <Generator.h>
#include <VmManager.h>
#include <iomanip>
#include <fstream>

using namespace std;

namespace Force {

  Dump* Dump::mspDump = nullptr;

  void Dump::Initialize()
  {
    if (nullptr == mspDump) {
      mspDump = new Dump();
    }
  }

  void Dump::Destroy()
  {
    delete mspDump;
    mspDump = nullptr;
  }

  void Dump::SetOption(const char* dumps)
  {
    const string strDump(dumps);
    StringSplitter sp(strDump, ',');

    while (!sp.EndOfString()) {
      auto strDumpOption =sp.NextSubString();
      try {
        auto dumpType = string_to_EDumpType(strDumpOption);
        mAttribute |= 1 << uint32(dumpType);
      }
      catch (const EnumTypeError& err) {
        LOG(fail) << "{Dump::SetOption} invalid option \"" << strDumpOption << "\", support dump option is : Asm, Elf, Mem, Page, Handlers" << endl;
        ::exit(1);
      }
    }
  }

  void Dump::DumpInfo(bool partial)
  {
    if (mDumped)
      return;
    bool fail_only = mAttribute & (1 << uint32(EDumpType::FailOnly));
    if (!partial && fail_only)
      return;
    mDumped = true;
    SetBaseName();
    if (mAttribute & (1 << uint32(EDumpType::Asm)) && partial)
      DumpPartialAsm();
    if (mAttribute & ( 1 << uint32(EDumpType::Elf)) && partial)
      DumpPartialElf();
    if (mAttribute & (1 << uint32(EDumpType::Mem)))
      DumpMem(partial);
    if (mAttribute & (1 << uint32(EDumpType::Page)))
      DumpPage();
    if (mAttribute & (1 << uint32(EDumpType::PageMemAttrJSON)))
      DumpPageAndMemoryAttributesJson();
    if (mAttribute & (1 << uint32(EDumpType::Handlers)))
      DumpHandlerAddresses();
  }

  void Dump::DumpHandlerAddresses()
  {
    auto const exceptionManager = ExceptionManager::Instance();
    auto const BankMappedBoundaries = exceptionManager-> GetHandlerAddresses();

    uint64 numBanks = EMemBankTypeSize;
    ofstream currentHandlerFileStream;

    for (uint64 currentBank = 0; currentBank < numBanks; currentBank++) {
      string bank_name = EMemBankType_to_string((EMemBankType) currentBank);
      string currentBankFileName = "Handlers" + bank_name + ".txt";

      auto const currentBankHandlerBoundaries = (*BankMappedBoundaries)[currentBank];

      currentHandlerFileStream.open(currentBankFileName);
      if (currentHandlerFileStream.bad()) {
          LOG(fail) << "Can't open file " << currentBankFileName << endl;
          FAIL("Can't open file");
      }


      /* For this bank, iterate all the handlers and write the addresses to file */
      for (auto item : currentBankHandlerBoundaries) {
        string handlerName;
        try {
          handlerName = EExceptionClassType_to_string(item.first);
        } catch (const EnumTypeError& err) {
          /* This is a reserved error code. */
          handlerName = "RSVD ( " + std::to_string((int) item.first) + " )";
        }
        uint64 startAddress = item.second.first;
        uint64 endAddress = item.second.second;
        currentHandlerFileStream << setw(30) << handlerName << setw(10) << " : ( 0x" << hex << startAddress << ", 0x" << hex << endAddress << " );" << endl;
      }

        currentHandlerFileStream.close();
    }
  }

  void Dump::DumpPartialElf()
  {
    uint64 resetPC = Config::Instance()->GetGlobalStateValue(EGlobalStateType::ResetPC);
    uint32 machine_type = Config::Instance()->GetGlobalStateValue(EGlobalStateType::ElfMachine);

    auto const memManager = MemoryManager::Instance();
    uint32 num_banks = memManager->NumberBanks();
    for ( unsigned bank = 0 ; bank < num_banks; bank ++) {
      auto mem_bank = memManager->GetMemoryBank(bank);
      Memory* output_mem = mem_bank->MemoryInstance();
      if (output_mem->IsEmpty()) continue;

      string mem_bank_str = EMemBankType_to_string(output_mem->MemoryBankType());
      string output_name_base = mBaseName + "_Partial." + mem_bank_str;
      string output_name_elf = output_name_base + ".ELF";
      TestIO output_instance(uint32(output_mem->MemoryBankType()), output_mem, mem_bank->GetSymbolManager());
      output_instance.WriteTestElf(output_name_elf, false, resetPC, machine_type);
    }
  }

  void Dump::DumpPartialAsm()
  {
    auto const memManager = MemoryManager::Instance();
    uint32 num_banks = memManager->NumberBanks();
    for ( unsigned bank = 0 ; bank < num_banks; bank ++) {
      auto mem_bank = memManager->GetMemoryBank(bank);
      Memory* output_mem = mem_bank->MemoryInstance();
      string mem_bank_str = EMemBankType_to_string(output_mem->MemoryBankType());
      string output_name_base = mBaseName + "_Partial." + mem_bank_str;
      string output_name_asm = output_name_base + ".S";
      TestIO output_instance(uint32(output_mem->MemoryBankType()), output_mem, mem_bank->GetSymbolManager());
      output_instance.WriteTestAssembly(mpScheduler->GetGenerators(), output_name_asm);
    }
  }

  void Dump::DumpMem(bool partial)
  {
    auto const memManager = MemoryManager::Instance();
    uint32 num_banks = memManager->NumberBanks();
    for ( unsigned bank = 0 ; bank < num_banks; bank ++) {
      Memory* output_mem = memManager->GetMemoryBank(bank)->MemoryInstance();
      if (output_mem->IsEmpty()) continue;

      string mem_bank_str = EMemBankType_to_string(output_mem->MemoryBankType());
      string strPartial = (partial) ? "_Partial." : ".";
      string output_name_base = mBaseName + strPartial + mem_bank_str;
      string output_name_img = output_name_base + ".mem";
      ofstream imgFile;
      imgFile.open(output_name_img);
      if (imgFile.bad()) {
        LOG(fail) << "Can't open file " << output_name_img << std::endl;
        FAIL("Can't open file");
      }
      output_mem->Dump(imgFile);
      imgFile.close();
    }

  }

  void Dump::DumpPage() const
  {
    auto generators = mpScheduler->GetGenerators();
    for (auto gen_item : generators) {
      gen_item.second->GetVmManager()->DumpPage(EDumpFormat::Text);
    }
  }

  void Dump::DumpPageAndMemoryAttributesJson() const
  {
    auto generators = mpScheduler->GetGenerators();
    for (auto gen_item : generators) {
      gen_item.second->GetVmManager()->DumpPage(EDumpFormat::JSON);
    }
  }

  void Dump::SetBaseName()
  {
    auto const cfg_handle = Config::Instance();
    uint64 initialSeed;
    mBaseName = get_file_stem(cfg_handle->TestTemplate());
    if (cfg_handle->OutputWithSeed(initialSeed)) {
      stringstream seed_stream;
      seed_stream << "0x"<< hex << initialSeed;
      mBaseName += "_" + seed_stream.str();
    }
  }

}
