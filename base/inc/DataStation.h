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
#ifndef Force_DataStation_H
#define Force_DataStation_H

#include <Defines.h>
#include <Object.h>
#include <map>

namespace Force {

  /*!
    \class DataStation
    \brief data station
   */

  class DataStation {
  public:
    static void Initialize();  //!< Initialization interface.
    static void Destroy();     //!< Destruction clean up interface.
    inline static DataStation* Instance() { return mspDataStation; } //!< Access data station instance.

    Object* Get(uint64 id);  //!< Get object with given record id
    uint64 Add(Object* obj);   //!< Add the given object to the list and return the assigned record id
    void Remove(uint64 id);  //!< Remove object with given record id

  private:
    DataStation();  //!< Constructor, private
    ~DataStation(); //!< Destructor, private
  private:
    static DataStation* mspDataStation;  //!< Static singleton pointer to data station
    std::map<uint64, Object*> mObjects;  //!< a list of objects
    uint64 mCurId;  //!< point to next available id
  };

}

#endif
