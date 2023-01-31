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
#ifndef Force_GenException_H
#define Force_GenException_H

#pragma GCC visibility push(default)

#include <exception>
#include <sstream>
#include <string>

namespace Force {

  /*!
    \class GenException
    \brief Base exception class for various type of generator exceptions.
  */
  class GenException : public std::exception {
  public:
    explicit GenException(const std::string& msg); //!< Constructor with std string given.
    explicit GenException(const char* msg); //!< Constructor with C style string given.

    GenException(const GenException& rOther) noexcept; //!< Copy constructor.
    GenException& operator=(const GenException& rOther) noexcept; //!< Assignment operator.

    virtual ~GenException() noexcept; //!< Destructor.

    const char* what() const noexcept; //!< Return C style string error message.
    virtual const char* GenExceptionType() const = 0; //!< Return actual GenException type
  protected:
    std::string mMessage; //!< Exception error message.
  };

  /*!
    \class ChoicesError
    \brief Return error message for error encounterred in choices choosing.
  */
  class ChoicesError : public GenException {
  public:
    explicit ChoicesError(const std::string& msg) : GenException(msg) { } //!< Constructor with std string given.
    explicit ChoicesError(const char* msg) : GenException(msg) { } //!< Constructor with C style string given.

    ChoicesError(const ChoicesError& rOther) noexcept : GenException(rOther) { } //!< Copy constructor.
    ChoicesError& operator=(const ChoicesError& rOther) noexcept; //!< Assignment operator.

    ~ChoicesError() noexcept { } //!< Destructor.
    const char* GenExceptionType() const override { return "ChoicesError"; } //!< Return actual GenException type
  };

  /*!
    \class OperandError
    \brief Return error message for error encounterred in choices choosing.
  */
  class OperandError : public GenException {
  public:
    explicit OperandError(const std::string& msg) : GenException(msg) { } //!< Constructor with std string given.
    explicit OperandError(const char* msg) : GenException(msg) { } //!< Constructor with C style string given.

    OperandError(const OperandError& rOther) noexcept : GenException(rOther) { } //!< Copy constructor.
    OperandError& operator=(const OperandError& rOther) noexcept; //!< Assignment operator.

    ~OperandError() noexcept { } //!< Destructor.
    const char* GenExceptionType() const override { return "OperandError"; } //!< Return actual GenException type
  };

  /*!
    \class InstructionError
    \brief Return error message for error encounterred in Instructions Generating .
  */
  class InstructionError : public GenException {
  public:
    explicit InstructionError(const std::string& msg) : GenException(msg) { } //!< Constructor with std string given.
    explicit InstructionError(const char* msg) : GenException(msg) { } //!< Constructor with C style string given.

    InstructionError(const InstructionError& rOther) noexcept : GenException(rOther) { } //!< Copy constructor.
    InstructionError& operator=(const InstructionError& rOther) noexcept; //!< Assignment operator.

    ~InstructionError() noexcept { } //!< Destructor.
    const char* GenExceptionType() const override { return "InstructionError"; } //!< Return actual GenException type
  };

  /*!
    \class ConstraintError
    \brief Return error message for error encounterred in Constraint solving.
  */
  class ConstraintError : public GenException {
  public:
    explicit ConstraintError(const std::string& msg) : GenException(msg) { } //!< Constructor with std string given.
    explicit ConstraintError(const char* msg) : GenException(msg) { } //!< Constructor with C style string given.

    ConstraintError(const ConstraintError& rOther) noexcept : GenException(rOther) { } //!< Copy constructor.
    ConstraintError& operator=(const ConstraintError& rOther) noexcept; //!< Assignment operator.

    ~ConstraintError() noexcept { } //!< Destructor.
    const char* GenExceptionType() const override { return "ConstraintError"; } //!< Return actual GenException type
  };

  /*!
    \class EnumTypeError
    \brief Return error message for error encounterred in Enum conversion.
  */
  class EnumTypeError : public GenException {
  public:
    explicit EnumTypeError(const std::string& msg) : GenException(msg) { } //!< Constructor with std string given.
    explicit EnumTypeError(const char* msg) : GenException(msg) { } //!< Constructor with C style string given.

    EnumTypeError(const EnumTypeError& rOther) noexcept : GenException(rOther) { } //!< Copy constructor.
    EnumTypeError& operator=(const EnumTypeError& rOther) noexcept; //!< Assignment operator.

    ~EnumTypeError() noexcept { } //!< Destructor.
    const char* GenExceptionType() const override { return "EnumTypeError"; } //!< Return actual GenException type
  };

  /*!
    \class PagingError
    \brief Return error message for error encounterred in page selection.
  */
  class PagingError : public GenException {
  public:
    explicit PagingError(const std::string& msg) : GenException(msg) { } //!< Constructor with std string given.
    explicit PagingError(const char* msg) : GenException(msg) { } //!< Constructor with C style string given.

    PagingError(const PagingError& rOther) noexcept : GenException(rOther) { } //!< Copy constructor.
    PagingError& operator=(const PagingError& rOther) noexcept; //!< Assignment operator.

    ~PagingError() noexcept { } //!< Destructor.
    const char* GenExceptionType() const override { return "PagingError"; } //!< Return actual GenException type
  };

  /*!
    \class SimulationError
    \brief Return error message for error encounterred during simulation.
  */
  class SimulationError : public GenException {
  public:
    explicit SimulationError(const std::string& msg) : GenException(msg) { } //!< Constructor with std string given.
    explicit SimulationError(const char* msg) : GenException(msg) { } //!< Constructor with C style string given.

    SimulationError(const SimulationError& rOther) noexcept : GenException(rOther) { } //!< Copy constructor.
    SimulationError& operator=(const SimulationError& rOther) noexcept; //!< Assignment operator.

    ~SimulationError() noexcept { } //!< Destructor.
    const char* GenExceptionType() const override { return "SimulationError"; } //!< Return actual GenException type
  };


  /*!
    \class GenerationFail
    \brief Return error message for reporting a generation fail.
  */
  class GenerationFail : public GenException {
  public:
    explicit GenerationFail(const std::string& msg) : GenException(msg) { } //!< Constructor with std string given.
    explicit GenerationFail(const char* msg) : GenException(msg) { } //!< Constructor with C style string given.

    GenerationFail(const GenerationFail& rOther) noexcept : GenException(rOther) { } //!< Copy constructor.
    GenerationFail& operator=(const GenerationFail& rOther) noexcept; //!< Assignment operator.

    ~GenerationFail() noexcept { } //!< Destructor.
    const char* GenExceptionType() const override { return "GenerationFail"; } //!< Return actual GenException type
  };

  /*!
    \class RegisterError
    \brief Return error message for error encounterred in page selection.
  */
  class RegisterError : public GenException {
  public:
    explicit RegisterError(const std::string& msg) : GenException(msg) { } //!< Constructor with std string given.
    explicit RegisterError(const char* msg) : GenException(msg) { } //!< Constructor with C style string given.

    RegisterError(const RegisterError& rOther) noexcept : GenException(rOther) { } //!< Copy constructor.
    RegisterError& operator=(const RegisterError& rOther) noexcept; //!< Assignment operator.

    ~RegisterError() noexcept { } //!< Destructor.
    const char* GenExceptionType() const override { return "RegisterError"; } //!< Return actual GenException type
  };

  /*!
    \class MemoryError
    \brief Return error message for error encounterred in memory.
  */
  class MemoryError : public GenException {
  public:
    explicit MemoryError(const std::string& msg) : GenException(msg) { } //!< Constructor with std string given.
    explicit MemoryError(const char* msg) : GenException(msg) { } //!< Constructor with C style string given.

    MemoryError(const MemoryError& rOther) noexcept : GenException(rOther) { } //!< Copy constructor.
    MemoryError& operator=(const MemoryError& rOther) noexcept; //!< Assignment operator.

    ~MemoryError() noexcept { } //!< Destructor.
    const char* GenExceptionType() const override { return "MemoryError"; } //!< Return actual GenException type
  };
}

#pragma GCC visibility pop

#endif
