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
#ifndef Force_VmInfo_H
#define Force_VmInfo_H

#include <map>
#include <string>

#include "Defines.h"
#include "Enums.h"
#include "VmContextParameter.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class Register;
  class Generator;

  /*!
    \class VmInfo
    \brief A class for VmMapper look up.
  */

  class VmInfo {
  public:
    VmInfo() //!< Default constructor.
      : mPrivilegeLevel(-1u), mPrivilegeLevelType(EPrivilegeLevelType(0)), mBoolAttributes(EVmInfoBoolTypeBaseType(0)), mBoolMask(EVmInfoBoolTypeBaseType(0))
      {
      }

    virtual ~VmInfo() { } //!< Destructor.

    void SetPrivilegeLevel(uint32 priv) //!< Set privilege level.

    {
      mPrivilegeLevel = priv;
      mPrivilegeLevelType = EPrivilegeLevelType(priv);
    }

    uint32 PrivilegeLevel() const { return mPrivilegeLevel; } //!< Return numeric privilege level.

    void SetPrivilegeLevelType(EPrivilegeLevelType priv)
    {
      mPrivilegeLevelType = priv;
      mPrivilegeLevel = uint32(priv);
    }

    void SetBoolAttribute(EVmInfoBoolType attrType, bool set); //!< Set VmInfo attribute value.

    inline bool BoolAttribute(EVmInfoBoolType attrType) const //!< Return VmInfo bool attribute value.
    {
      return (mBoolAttributes & EVmInfoBoolTypeBaseType(attrType)) == EVmInfoBoolTypeBaseType(attrType);
    }

    inline void Clear() //!< Clear VmInfo attributes.
    {
      mPrivilegeLevel = -1u;
      mPrivilegeLevelType = EPrivilegeLevelType(0);
      mBoolAttributes = EVmInfoBoolTypeBaseType(0);
      mBoolMask = EVmInfoBoolTypeBaseType(0);
    }

    virtual EVmRegimeType RegimeType(bool* pIsValid=nullptr) const = 0; //!< Return the current VM regime type.
    virtual const std::string ToString() const = 0; //!< Return the VmInfo in string format.
    virtual void GetCurrentStates(const Generator& rGen) = 0; //!< Obtain the current VM attributes.
    virtual bool PagingEnabled() const = 0; //!< Indicate whether paging is enabled in the specified VM.

    void GetOtherStates(const Generator& rGen); //!< Obtain other states from current PE states.
  protected:
    uint32 GetRegisterFieldState(const Register* pReg, EVmInfoBoolType attrType); //!< Get register field state.
    virtual const std::string GetRegisterNameForField(EVmInfoBoolType attrType, const Generator& rGen) const = 0; //!< Get the register name associated with the VmInfo boolean attribute.

  protected:
    uint32 mPrivilegeLevel; //!< Numeric exception level.
    EPrivilegeLevelType mPrivilegeLevelType; //!< Privilege level.
    EVmInfoBoolTypeBaseType mBoolAttributes; //!< VM attributes.
    EVmInfoBoolTypeBaseType mBoolMask; //!< Mask of attributes that's been set.
  };


}

#endif
