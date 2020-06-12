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
#ifndef Force_ExceptionContext_H
#define Force_ExceptionContext_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

namespace Force {

  class Generator;
  class RegisterFile;
  class Instruction;

  /*!
    \class ExceptionContext
    \brief A class that can be instaniated at the exception site to gather exception related information for diagnosis.
  */
  class ExceptionContext {
  public:
    ExceptionContext(const Generator* pGen, EExceptionClassType exceptClass); //!< Constructor.
    virtual ~ExceptionContext(); //!< Desctructor.
    virtual std::string ToString() const; //!< Return string description of the exception context object.

    virtual std::string GetExceptionCategory(EExceptionClassType exceptClass) const { return "exception"; } //!< Return exception category string.
    void UpdateContext(); //!< Update exception source information.
    std::string GetSourceInstructionName(); //!< Return instruction name at the exception source location.
    ASSIGNMENT_OPERATOR_ABSENT(ExceptionContext);
    DEFAULT_CONSTRUCTOR_ABSENT(ExceptionContext);
    COPY_CONSTRUCTOR_ABSENT(ExceptionContext);
  protected:
    virtual void UpdateSourcePrivilegeLevel() { } //!< Update source exception level.
    virtual void UpdateSourceAddress() { } //!< Update exception source address.
    void UpdateSourceInstruction(); //!< Update source instruction information.
    bool UpdatePhysicalAddress(); //!< Update physical address for the exception source location.
  protected:
    EPrivilegeLevelType mServicePrivilegeLevel; //!< Exception level servicing the exception.
    EExceptionClassType mExceptionClass; //!< Exception class.
    EPrivilegeLevelType mSourcePrivilegeLevel; //!< Source exception level.
    uint64 mPreferredReturnAddress; //!< Preferred return-to virtual address.
    uint64 mExceptionAddress; //!< Virtual address of the exception.
    bool mContextUpdated; //!< Indicate whether exception context has been updated.
    bool mAddressAccurate; //!< Indicates if the exception address is valid.
    uint64 mPhysicalAddress; //!< Physical address for the exception address.
    EMemBankType mMemoryBank; //!< Memory bank for physical address.
    bool mPaValid; //!< Indicate if the physical address is valid.
    const Generator* mpGenerator; //!< Const pointer to Generator object.
    const RegisterFile* mpRegisterFile; //!< Const pointer to RegisterFile object.
    const Instruction* mpSourceInstruction; //!< Pointer to instruction at the exception source location.
  };

}

#endif
