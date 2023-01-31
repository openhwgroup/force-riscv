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
#include "DataStation.h"

#include "Log.h"

using namespace std;

/*!
  \file DataStation.cc
  \brief Code for data station
*/

namespace Force {

  DataStation* DataStation::mspDataStation = nullptr;

  void DataStation::Initialize()
  {
    if (nullptr == mspDataStation) {
      mspDataStation = new DataStation();
    }
  }

  void DataStation::Destroy()
  {
    delete mspDataStation;
    mspDataStation = nullptr;
  }

  DataStation::DataStation()
  :  mObjects(), mCurId(1)
  // make sure 0 is invalid record id
  {
  }

  DataStation::~DataStation()
  {
    for (auto &item : mObjects)
      delete item.second;
  }

  Object* DataStation::Get(uint64 id)
  {
    auto it = mObjects.find(id);
    if (it == mObjects.end()) {
      LOG(notice) << "{DataStation::Get} can't find object with given id:" << id << endl;
      return nullptr;
    }
    else
      return it->second;
  }

  uint64 DataStation::Add(Object* obj)
  {
    uint64 id = mCurId;
    mObjects[id] = obj;
    mCurId++;
    // note as of now, simply increment the id and will not re-claim already assigned id
    // to make the operation (code) simple. 64-bit should give us plenty record ids
    // even we don't claim back the id where object is removed.

    return id;
  }

  void DataStation::Remove(uint64 id)
  {
    auto it = mObjects.find(id);
    if (it == mObjects.end()) {
      LOG(fail) << "{DataStation::Remove} can't find object with given id:" << id << endl;
      FAIL("invalid-id-remove-datastation");
      return;
    }
    else
    {
      LOG(notice) << "{DataStation::Remove} remove the object with given id:" << id << endl;
      delete it->second;
      mObjects.erase(it);
    }
  }

}
