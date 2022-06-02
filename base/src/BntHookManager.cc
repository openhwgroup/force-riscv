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
#include "BntHookManager.h"

#include <algorithm>
#include <sstream>

#include "Generator.h"
#include "Log.h"

using namespace std;

namespace Force {
  BntHook::BntHook(uint64 id, const std::string& seq_name, const std::string& func_name) : mId(id), mSequenceName(seq_name), mFunctionName(func_name)
  {

  }

  BntHook::BntHook(const BntHook& rOther) : mId(rOther.mId), mSequenceName(rOther.mSequenceName), mFunctionName(rOther.mFunctionName)
  {

  }

  Object* BntHook::Clone() const
  {
    return new BntHook(*this);
  }

  const std::string BntHook::ToString() const
  {
    stringstream out_stream;

    out_stream << Type() << " id: " << mId << ", sequence name: " << mSequenceName << ", function name: " << mFunctionName << " ";
    return out_stream.str();
  }

  BntHookManager::~BntHookManager()
  {
    delete mpBntHook;

    for (auto bnt : mBntHookStack)
      delete bnt;
  }

  BntHookManager::BntHookManager(const BntHookManager& rOther): BntHook(rOther), mId(rOther.mId), mpBntHook(nullptr), mBntHookStack()
  {
    mpBntHook = dynamic_cast<const BntHook*>(rOther.mpBntHook->Clone());

    transform(rOther.mBntHookStack.cbegin(), rOther.mBntHookStack.cend(), back_inserter(mBntHookStack),
      [](const BntHook* pBntHook) { return dynamic_cast<const BntHook*>(pBntHook->Clone()); });
  }

  Object* BntHookManager::Clone() const
  {
    return new BntHookManager(*this);
  }

  const std::string BntHookManager::ToString() const
  {
    stringstream out_stream;

    out_stream << Type() << " ";
    for (auto bnt : mBntHookStack)
      out_stream << bnt->ToString();

    if (mpBntHook)
      out_stream  << mpBntHook->ToString();

    return out_stream.str();
  }

  void BntHookManager::Setup(const Generator* pGen)
  {
    mpBntHook = new BntHook(AllocateId(), "default","defaultFunction");
  }

  void BntHookManager::PushBntHook(const BntHook& bntHook)
  {
    mBntHookStack.push_back(mpBntHook);

    auto seq_name = bntHook.SequenceName();
    if (seq_name == "")
      seq_name = mpBntHook->SequenceName(); // use sequence name last time

    auto func_name = bntHook.FunctionName();
    if (func_name == "")
      func_name = mpBntHook->FunctionName();

    mpBntHook = new BntHook(bntHook.Id(), seq_name, func_name);
  }
  void BntHookManager::RevertBntHook(uint64 bntId)
  {
    if (mpBntHook == nullptr) {
      LOG(fail) << "{BntHookManager::RevertBntHook} no Bnt hook to revert" << endl;
      FAIL("no-Bnt-Hook");
    }
    if (bntId > mpBntHook->Id()) {
      LOG(fail) << "{BntHookManager::RevertBntHook} invalid bnt ID: 0x" << hex << bntId << endl;
      FAIL("invalid-Bnt-id");
    }
    if (bntId == 0)  // default Id, revert to last one
      bntId = mpBntHook->Id();

    while (bntId <= mpBntHook->Id()) {
      delete mpBntHook;
      mpBntHook = nullptr;
      if (mBntHookStack.empty())
        break;
      mpBntHook = mBntHookStack.back();
      mBntHookStack.pop_back();
    }

  }

}
