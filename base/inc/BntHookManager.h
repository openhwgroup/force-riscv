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
#ifndef Force_BntHookManager_H
#define Force_BntHookManager_H

#include <list>
#include <string>

#include "Defines.h"
#include "Object.h"

namespace Force {

  class Generator;

  /*!
    \class BntHook
    \brief Class holding bnt hook.
  */
  class BntHook : public Object{
  public:
    BntHook() : mId(0), mSequenceName(), mFunctionName() { } //!< default constructor
    BntHook(uint64 mId, const std::string& seq_name, const std::string& func_name);  //!< constructor
    SUBCLASS_DESTRUCTOR_DEFAULT(BntHook);
    ASSIGNMENT_OPERATOR_ABSENT(BntHook);
    Object* Clone() const override; //!<  Return a cloned object of the same type and same contents of the object.
    const std::string ToString() const override; //!< Return a string describing the current state of the object.
    const char* Type() const override { return "BntHook"; } //!< actual type of the object

    inline const std::string& SequenceName() const { return mSequenceName; } //!< Get sequence name
    inline const std::string& FunctionName() const { return mFunctionName; } //!< Get function name
    inline const uint64 Id() const { return mId; } //!< Return Id
  protected:
    BntHook(const BntHook& rOther); //!< copy constructor
  private:
    uint64 mId; //!< Bnt Id
    std::string mSequenceName; //!< sequence name
    std::string mFunctionName; //!< function name
  };

  /*!
    \class BntHookManager
    \brief Class manage bnt states
  */
  class BntHookManager : public BntHook {
  public:
    BntHookManager(): BntHook(), mId(0), mpBntHook(nullptr), mBntHookStack() { } //!< Constructor
    ~BntHookManager() override; //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(BntHookManager);
    Object* Clone() const override; //!< Return a cloned object of the same type and same contents of the object.
    const std::string ToString() const override; //!< Return a string describing the current state of the object.
    const char* Type() const override { return "BntHookManager"; } //!< actual type of the object

    void Setup(const Generator* pGen); //!< Do Some Setup
    inline const BntHook* GetBntHook() const { return mpBntHook; } //!< get current bnt hook
    void PushBntHook(const BntHook& bntHook); //!< set bnt hook
    void RevertBntHook(uint64 bntId); //!< revert bnt hook
    uint64 AllocateId() { return ++ mId; } //!< allocate Id
  protected:
    BntHookManager(const BntHookManager& rOther); //!< copy constructor

  protected:
    uint64 mId; //!< Bnt unique id
    const BntHook* mpBntHook; //!< current bnt hook
    std::list<const BntHook*> mBntHookStack; //!< history bnt hook stack
  };
}

#endif
