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
#include "GenException.h"

#include "Log.h"

using namespace std;

namespace Force {

  GenException::GenException(const std::string& msg)
    : exception(), mMessage(msg)
  {
  }

  GenException::GenException(const char* msg)
    : exception(), mMessage(msg)
  {
  }

  GenException::GenException(const GenException& rOther) noexcept
    : exception(rOther), mMessage(rOther.mMessage)
  {
  }

  GenException& GenException::operator=(const GenException& rOther) noexcept
  {
    if (this != &rOther) {
      mMessage = rOther.mMessage;
    }

    return *this;
  }

  GenException::~GenException() noexcept
  {
  }

  const char* GenException::what() const noexcept
  {
    return mMessage.c_str();
  }

  ChoicesError& ChoicesError::operator=(const ChoicesError& rOther) noexcept
  {
    GenException::operator=(rOther);

    return *this;
  }

  OperandError& OperandError::operator=(const OperandError& rOther) noexcept
  {
    GenException::operator=(rOther);

    return *this;
  }

  InstructionError& InstructionError::operator=(const InstructionError& rOther) noexcept
  {
    GenException::operator=(rOther);

    return *this;
  }

  ConstraintError& ConstraintError::operator=(const ConstraintError& rOther) noexcept
  {
    GenException::operator=(rOther);

    return *this;
  }

  EnumTypeError& EnumTypeError::operator=(const EnumTypeError& rOther) noexcept
  {
    GenException::operator=(rOther);

    return *this;
  }

  PagingError& PagingError::operator=(const PagingError& rOther) noexcept
  {
    GenException::operator=(rOther);

    return *this;
  }

  RegisterError& RegisterError::operator=(const RegisterError& rOther) noexcept
  {
    GenException::operator=(rOther);

    return *this;
  }

  MemoryError& MemoryError::operator=(const MemoryError& rOther) noexcept
  {
    GenException::operator=(rOther);

    return *this;
  }

}
