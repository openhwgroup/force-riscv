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
#include "BntNodeManager.h"

#include <sstream>

#include "BntNode.h"
#include "Log.h"

/*!
  \file BntNodeManager.cc
  \brief Code manage bnt node
*/

using namespace std;

namespace Force {
  Object* BntNodeManager::Clone() const
  {
    return new BntNodeManager(*this);
  }

  const std::string BntNodeManager::ToString() const
  {
    std::stringstream sstream;

    sstream << Type() << endl;
    if (mpHotSpeculativeBntNode != nullptr)
      sstream << "Hot Bnt node: " <<  mpHotSpeculativeBntNode->ToString() << endl;
    sstream << "History Bnt node:" << endl;
    for (auto bnt_node : mSpeculativeBntNodes)
      sstream << bnt_node->ToString() << endl;

    return sstream.str();
  }

  const char* BntNodeManager::Type() const
  {
    return "BntNodeManager";
  }

  BntNodeManager::~BntNodeManager()
  {
    if (nullptr != mpHotSpeculativeBntNode || !mSpeculativeBntNodes.empty()) {
      LOG(fail) << "{BntNodeManager::~BntNodeManager} dangling BntNode pointer." << endl;
      FAIL("dangling-bnt-node-pointer");
    }
  }

  void BntNodeManager::SaveBntNode(BntNode* pBntNode)
  {
    //<< "{BntNodeManager::SaveBntNode} Save Bnt node:" <<pBntNode->ToString() << endl;

    if (pBntNode->IsSpeculative()) {
      if (mpHotSpeculativeBntNode != nullptr)
        mSpeculativeBntNodes.push_back(mpHotSpeculativeBntNode);

      mpHotSpeculativeBntNode = pBntNode;
    }
    else {
      mBntNodes.push_back(pBntNode);
    }
  }

  void BntNodeManager::SwapBntNodes(std::vector<BntNode*>& rSwapVec)
  {
    rSwapVec.swap(mBntNodes);
  }

  void BntNodeManager::PopSpeculativeBntNode()
  {
    if (mpHotSpeculativeBntNode == nullptr) {
      LOG(fail) << "{BntNodeManager::PopSpeculativeBntNode} Not hot Bnt node" << endl;
      FAIL("No-hot-BntNode");
    }

    delete mpHotSpeculativeBntNode;
    if (mSpeculativeBntNodes.empty())
      mpHotSpeculativeBntNode = nullptr;
    else {
      mpHotSpeculativeBntNode = mSpeculativeBntNodes.back();
      mSpeculativeBntNodes.pop_back();
    }
  }

  BntNodeManager::BntNodeManager(const BntNodeManager& rOther) : Object(rOther), mBntNodes(), mpHotSpeculativeBntNode(nullptr), mSpeculativeBntNodes()
  {

  }

}
