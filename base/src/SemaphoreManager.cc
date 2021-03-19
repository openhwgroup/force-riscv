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
#include <Generator.h>
#include <GenRequest.h>
#include <VmManager.h>
#include <SemaphoreManager.h>
#include <VmMapper.h>
#include <Log.h>

using namespace std;

/*!
  \file SemaphoreManager.cc
  \brief Code for back-end semaphore manager class
*/

namespace Force {

  /*!
    \class Semaphore
    \brief class to model back-end semaphores.
  */
  class Semaphore {
  public:
    Semaphore(const string& name, uint64 counter, uint32 bank, uint32 size) : mName(name), mCounter(counter), mSize(size), mBank(bank), mPA(0), mEndian(false)  //!< constructor
    {

    }

    ~Semaphore() //!< destructor
    {

    }

    void Generate(Generator *pGen) //!< generate a semaphore
    {
      GenPaRequest pa_req;
      auto pa_var = pGen->GetVariable("Semaphore Physical Address Range", EVariableType::String);

      pa_req.AddDetail("size", mSize);
      pa_req.AddDetail("align", mSize);
      pa_req.AddDetail("isInstr", 0);
      pa_req.AddDetail("Range", pa_var); // PA in the range valid for all PEs
      pa_req.AddDetail("Shared", true);

      // Ensure an unmapped adddres is used, so we can assign the desired memory attributes when
      // generating VAs later
      pa_req.AddDetail("ForceNewAddr", true);

      pa_req.SetBankType(EMemBankType(mBank));

      pGen->GenVmRequest(&pa_req);

      mPA = pa_req.PA();
      mEndian = pGen->IsDataBigEndian();

      pGen->InitializeMemoryWithEndian(mPA, mBank, mSize, mCounter, false, mEndian);
    }

    inline uint64 GetPA() const { return mPA; }
    inline uint32 Bank() const { return mBank; }
    inline bool IsBigEndian() const { return mEndian; }

  protected:
    string mName; //!< semaphore name
    uint64 mCounter; //!< semaphore counter
    uint32 mSize; //!< size in byte
    uint32 mBank; //!< memory bank
    uint64 mPA;   //!< physical address mapped
    bool mEndian; //!< data endian, true: big endian
  };

  SemaphoreManager::SemaphoreManager() : mSemaphores()
  {

  }

  SemaphoreManager::~SemaphoreManager()
  {
    for (auto sema: mSemaphores)
      delete sema.second;
  }

  bool SemaphoreManager::GenSemaphore(Generator *pGen, const std::string& name, uint64 counter, uint32 bank, uint32 size, uint64& address, bool& reverseEndian)
  {
    auto it = mSemaphores.find(name);
    if (it != mSemaphores.end()) {
      LOG(debug) << "{SemaphoreManager::GenSemaphore} Reuse semaphore:" << name <<endl;
      address = it->second->GetPA();
      reverseEndian = it->second->IsBigEndian() != pGen->IsDataBigEndian();

      return address <= pGen->GetVmManager()->CurrentVmMapper()->MaxPhysicalAddress();
    }

    Semaphore *pSema = new Semaphore(name, counter, bank, size);
    pSema->Generate(pGen);
    mSemaphores[name] = pSema;

    LOG(debug) << "{SemaphoreManager::GenSemaphore} Create semaphore:" << name << ", initial value:" << counter <<
      ", physical address:0x" << hex << pSema->GetPA() << "["<< pSema->Bank() << "]" << endl;

    address = pSema->GetPA();
    reverseEndian = false;
    return true;
  }

}
