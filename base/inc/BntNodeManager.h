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
#ifndef Force_BntNodeManager_H
#define Force_BntNodeManager_H

#include <vector>

#include "Defines.h"
#include "Object.h"

namespace Force {

  class BntNode;

  /*!
    \class BntNodeManager
    \brief class to manage all bnt nodes.
  */

  class BntNodeManager : public Object {
  public:
    BntNodeManager() : Object(), mBntNodes(), mpHotSpeculativeBntNode(nullptr), mSpeculativeBntNodes() { } //!< Default Constructor
    Object* Clone() const override; //!< Return a cloned object of the same type and same contents of the object.
    const std::string ToString() const override; //!< Return a string describing the current state of the object.
    const char* Type() const override; //!< Return a string describing the actual type of the Object
    ~BntNodeManager(); //!< Destructor

    void SaveBntNode(BntNode* pBntNode); //!< Save Bnt node to the lists
    void SwapBntNodes(std::vector<BntNode*>& rSwapVec); //!< swap Bnt nodes
    BntNode* GetHotSpeculativeBntNode() const { return mpHotSpeculativeBntNode; } //!< get hot speculative Bnt node
    void PopSpeculativeBntNode(); //!< pop speculative Bnt node from the stack and set to be hot.

    ASSIGNMENT_OPERATOR_ABSENT(BntNodeManager);
  protected:
    BntNodeManager(const BntNodeManager& rOther); //!< Copy constructor
  protected:
    std::vector<BntNode* > mBntNodes; //!< container for non-speculative Bnt nodes
    BntNode* mpHotSpeculativeBntNode; //!< hot speculative Bnt node
    std::vector<BntNode* > mSpeculativeBntNodes; //!< the container for history speculative Bnt nodes
  };

}

#endif
