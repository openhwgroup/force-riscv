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
#ifndef Force_VmControlBlock_H
#define Force_VmControlBlock_H

#include "Defines.h"
#include "Enums.h"
#include "Object.h"
#include "VmContextParameter.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class Generator;
  class RegisterReload;

  /*!
    \class VmControlBlock
    \brief Base class for both paging and non-paging mode VM control block.
  */
  class VmControlBlock : public VmContext {
  public:
    Object *          Clone()    const override { return new VmControlBlock(*this); } //!< Clone VmControlBlock object.
    const std::string ToString() const override; //!< Return a string describing the VmControlBlock object.
    const char*       Type()     const override { return "VmControlBlock"; }

    VmControlBlock(EPrivilegeLevelType privType, EMemBankType memType); //!< Constructor with priv type and memory bank given.
    ~VmControlBlock() override; //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(VmControlBlock);

    virtual void Setup(Generator* pGen); //!< Setup the VmControlBlock object.
    virtual void Initialize();           //!< Initialize VmControlBlock object.
    virtual bool Validate(std::string& rErrMsg) const; //!< Return true only if all initialized context are valid.

    EPrivilegeLevelType PrivilegeLevel()     const  { return mPrivilegeLevel; }     //!< Return exception level of the VM control block.
    EMemBankType        DefaultMemoryBank()  const  { return mDefaultMemoryBank; }  //!< Return default memory bank.
    bool                IsBigEndian()        const  { return mBigEndian; }          //!< Return whether page table data is big endian.
    uint32              Stage()              const  { return mStage; }              //!< Return translation stage.
    uint64              MaxPhysicalAddress() const  { return mMaxPhysicalAddress; } //!< Return maximum supported physical address.
    RegisterReload*     GetRegisterReload() const;                                  //!< Get a register reload object.
  protected:
    VmControlBlock(); //!< Default constructor.
    VmControlBlock(const VmControlBlock& rOther); //!< Copy constructor.

    virtual bool             GetBigEndian()          const { return false; }        //!< Return big-endian attribute.
    virtual uint64           GetMaxPhysicalAddress() const { return MAX_UINT64; }   //!< Return maximum physical address.
    virtual void FillRegisterReload(RegisterReload* pRegContext) const;             //!< Fill register reload context.
    virtual const std::string AdditionalAttributesString() const { return ""; }     //!< Additional attribute string.

  protected:
    Generator* mpGenerator; //!< Pointer to generator.
    EPrivilegeLevelType mPrivilegeLevel;     //!< Exception level of the VM regime.
    EMemBankType        mDefaultMemoryBank;  //!< Memory bank for default table lookup.
    bool                mBigEndian;          //!< If page table data is in big endian format.
    uint32              mStage;              //!< Translation stage.
    uint64              mMaxPhysicalAddress; //!< Maximum supported physical address.
  };

}

#endif
