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
#include <GenRequestQueue.h>
#include <GenRequest.h>
#include <Log.h>

#include <sstream>

/*!
  \file GenRequestQueue.cc
  \brief Code managing GenRequestQueue, a queue of GenRequest objects.
 */

using namespace std;

namespace Force {

  GenRequestQueue::GenRequestQueue() : mRequestQueue()
  {
  }

  GenRequestQueue::GenRequestQueue(const GenRequestQueue& rOther)
    : Object(rOther), mRequestQueue()

  {
  }

  GenRequestQueue::~GenRequestQueue()
  {
    for (auto list_item : mRequestQueue) {
      delete list_item;
    }
  }

  Object* GenRequestQueue::Clone() const
  {
    return new GenRequestQueue(*this);
  }

  const std::string GenRequestQueue::ToString() const
  {
    stringstream out_stream;
    for (auto item_ptr : mRequestQueue) {
      out_stream << item_ptr->ToString() << endl;
    }

    return out_stream.str();
  }

  void GenRequestQueue::PrependRequest(GenRequest* genRequest)
  {
    mRequestQueue.push_front(genRequest);
  }

  void GenRequestQueue::PrependRequests(vector<GenRequest* >& requests)
  {
    mRequestQueue.insert(mRequestQueue.begin(), requests.begin(), requests.end());
    requests.clear();
  }

  GenRequestQueue::ConstGRequestQIter GenRequestQueue::StartRound()
  {
    return mRequestQueue.begin();
  }

  bool GenRequestQueue::RoundFinished(const ConstGRequestQIter& rRoundEndIter) const
  {
    return (mRequestQueue.begin() == rRoundEndIter);
  }

  GenRequest* GenRequestQueue::PopFront()
  {
    GenRequest* gen_front = mRequestQueue.front();
    mRequestQueue.pop_front();
    return gen_front;
  }

}
