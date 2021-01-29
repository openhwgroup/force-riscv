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
#include <InstructionResults.h>
#include <Generator.h>
#include <Instruction.h>
#include <Record.h>
#include <Config.h>
#include <UtilityFunctions.h>
#include <GenPC.h>
#include <Log.h>

#include <algorithm>
#include <sstream>
// C++UP accumulate defined in numeric
#include <numeric>

using namespace std;

/*!
  \file InstructionResults.cc
  \brief Code for managing generated instruction results
*/

namespace Force {

  InstructionResultsBank::InstructionResultsBank(const InstructionResultsBank& rOther)
    : Object(rOther), mBank(rOther.mBank), mResults()
  {
    // do not copy mResults contents.
  }

  InstructionResultsBank::~InstructionResultsBank()
  {
    for (auto & map_item : mResults) {
      delete map_item.second;
    }
  }

  Object* InstructionResultsBank::Clone() const
  {
    return new InstructionResultsBank(*this);
  }

  const string InstructionResultsBank::ToString() const
  {
    return "InstructionResultsBank";
  }

  void InstructionResultsBank::AddInstruction(uint64 pa, Instruction* instr)
  {
    auto map_finder = mResults.find(pa);
    if (map_finder != mResults.end()) {
      LOG(fail) << "Adding Instruction \"" << instr->FullName() << "\"result to bank " << mBank << " address 0x" << hex << pa << " that has been occupied by another instruction \"" << map_finder->second->FullName() << "\"." << endl;
      FAIL("duplicated-instruction-at-pa");
    }
    mResults[pa] = instr;
  }

  uint32 InstructionResultsBank::GetInstructionCount() const
  {
    return mResults.size();
  }

  const Instruction* InstructionResultsBank::LookupInstruction(uint64 address) const
  {
    auto map_finder = mResults.find(address);
    if (map_finder == mResults.end()) {
      LOG(fail) << "Can't find instruction at [" << mBank << "] 0x" << hex << address << endl;
      FAIL("Can't find instruction");
    }
    return map_finder->second;
  }

  ThreadInstructionResults::ThreadInstructionResults(uint32 numBanks)
    : Object(), mBanks(), mCurBank(0), mCurAddr(0), mCurAddrValid(false), mRecordId()
  {
    for (uint32 i = 0; i < numBanks; ++ i) {
      mBanks.push_back(new InstructionResultsBank(i));
    }
  }

  ThreadInstructionResults::ThreadInstructionResults(const ThreadInstructionResults& rOther)
    : Object(rOther), mBanks(), mCurBank(rOther.mCurBank), mCurAddr(rOther.mCurAddr), mCurAddrValid(rOther.mCurAddrValid), mRecordId(rOther.mRecordId)
  {
    transform(rOther.mBanks.cbegin(), rOther.mBanks.cend(), back_inserter(mBanks),
      [](const InstructionResultsBank* pResultBank) { return dynamic_cast<InstructionResultsBank*>(pResultBank->Clone()); });
  }

  ThreadInstructionResults::~ThreadInstructionResults()
  {
    for (auto result_bank : mBanks) {
      delete result_bank;
    }

  }

  Object* ThreadInstructionResults::Clone() const
  {
    return new ThreadInstructionResults(*this);
  }

  const string ThreadInstructionResults::ToString() const
  {
    return "ThreadInstructionResults";
  }

  void ThreadInstructionResults::AddInstruction(uint32 bank, uint64 pa, Instruction* instr)
  {
    mBanks[bank]->AddInstruction(pa, instr);
    // update current instruction bank and address
    mCurBank = bank;
    mCurAddr = pa;
    mCurAddrValid = true;
  }

  bool ThreadInstructionResults::Commit(Generator* gen, Instruction* instr)
  {
    auto gen_pc = gen->GetGenPC();
    uint32 bank = 0;
    bool fault = false;
    uint64 pa = gen_pc->GetPA(gen, bank, fault);

    LOG(debug) << "[ThreadInstructionResults::Commit] PC LA/PA: 0x" << std::hex << gen_pc->Value() << "/0x" << pa << std::dec << std::endl;

    string instr_text;
    if (Config::Instance()->OutputAssembly()) {
      instr_text = instr->AssemblyText();
    }
    else {
      instr_text = instr->FullName();
    }

    //TODO: identify why this is forced here.
    //fault = false;

    //if (fault) {
      //LOG(notice) << "{ThreadInstructionResults::Commit} Fault committing instruction \"" << instr_text << "\" at 0x" << hex << gen_pc->Value() << " as address error." << endl;
      //return false;
    //}
    LOG(notice) << "Committing instruction \"" << instr_text << "\" at 0x" << hex << gen_pc->Value() << "=>["<< bank <<"]0x" << pa << " (0x" << instr->Opcode() <<") gen("<< gen->ThreadId() << ")" << endl;
    AddInstruction(bank, pa, instr);
    uint32 instr_size = instr->ByteSize();
    MemoryInitRecord* mem_init_data = gen->GetRecordArchive()->GetMemoryInitRecord(gen->ThreadId(), instr_size, instr->ElementSize(), EMemDataType::Instruction);
    mem_init_data->SetData(pa, bank, instr->Opcode(), instr_size, gen->IsInstructionBigEndian());
    gen->InitializeMemory(mem_init_data);
    return true;
  }


  void ThreadInstructionResults::GenSummary()
  {
     uint32 myTotal = 0;

     LOG(notice) << "Thread Instruction Summary" << endl;
     for (auto bank = mBanks.begin(); bank != mBanks.end(); ++bank)
     {
       uint32 myBankTotal = (*bank)->GetInstructionCount();
       myTotal += myBankTotal;

       LOG(notice) << "Thread "
                   << EMemBankType_to_string(static_cast<EMemBankType>((*bank)->Bank()))
                   << " Instructions Generated: "
                   << dec << myBankTotal
                   << endl;
     }

     LOG(notice) << "Thread Instructions Generated: " << dec << myTotal << endl;
  }

  const Instruction* ThreadInstructionResults::LookupInstruction(uint32 bank, uint64 address) const
  {
    return mBanks[bank]->LookupInstruction(address);
  }

  const std::map<uint64, Instruction* >& ThreadInstructionResults::GetInstructions(uint32 bank) const
  {
    return mBanks[bank]->GetInstructions();
  }

  uint32 ThreadInstructionResults::GetInstructionCount(cuint32 bank) const
  {
    return mBanks[bank]->GetInstructionCount();
  }

  uint32 ThreadInstructionResults::GetBankCount() const
  {
    return mBanks.size();
  }

  void ThreadInstructionResults::GetCurrentInstructionRecordId (std::string& rec_id)
  {
    stringstream ss;

    if (mCurAddrValid) {
      ss << "0x" << std::hex << mCurBank;
      ss << "#";
      ss << "0x" << std::hex << mCurAddr;
    }
    mRecordId = ss.str();
    rec_id = mRecordId;
  }

  void ThreadInstructionResults::InvalidCurrentBankAddress()
  {
    mCurAddrValid = false;
    mCurBank = 0;
    mCurAddr = 0;
  }

  InstructionResults* InstructionResults::mspInstructionResults = nullptr;

  InstructionResults::InstructionResults()
    : mThreadInstructionResults()
  {
  }

  void InstructionResults::Initialize()
  {
    if (mspInstructionResults == nullptr) {
      mspInstructionResults = new InstructionResults();
    }
    else {
      LOG(fail) << "InstructionResults instance has already been initialized.";
      FAIL("reinitialize-instruction-results");
   }
  }

  void InstructionResults::Destroy()
  {
    if (mspInstructionResults != nullptr) {
      delete mspInstructionResults;
      mspInstructionResults = nullptr;
    }
    else {
      LOG(fail) << "InstructionResults has already been destroyed or was never initialized.";
      FAIL("redestroy-instruction-results");
    }
  }

  void InstructionResults::AddThreadResults(const ThreadInstructionResults* pThreadInstructionResults)
  {
    mThreadInstructionResults.push_back(pThreadInstructionResults);
  }

  void InstructionResults::GenSummary()
  {
    if (mThreadInstructionResults.empty()) {
      LOG(fail) << "Unable to count generated instructions.";
      FAIL("instruction-count-failure");
    }

    LOG(notice) << "Instruction Summary" << endl;

    uint32 instruction_total = 0;

   // This logic assumes that all ThreadInstructionResults objects utilize the same number of banks. I don't know of
   // any reason that would fail to be the case, but this logic will need to be adjusted if the number of banks ever
   // varies.
    for (uint32 i = 0; i < mThreadInstructionResults[0]->GetBankCount(); i++) {
      uint32 bank_instruction_total = accumulate(mThreadInstructionResults.begin(), mThreadInstructionResults.end(), uint32(0),
        [i](cuint32 total, const ThreadInstructionResults* pThreadInstructionResults) { return total + pThreadInstructionResults->GetInstructionCount(i); });

      instruction_total += bank_instruction_total;

      LOG(notice) << EMemBankType_to_string(EMemBankType(i))
                  << " Instructions Generated: "
                  << dec << bank_instruction_total
                  << endl;
    }

    LOG(notice) << "Total Instructions Generated: " << dec << instruction_total << endl;
  }

}
