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
#ifndef Force_ExceptionManager_H
#define Force_ExceptionManager_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <vector>
#include <map>
#include <Constraint.h>
#include <Log.h>

namespace Force {

  class Memory;
  class MemoryInitRecord;
  class Generator;
  class ConstraintSet;
  class PageTableConstraint;
  class PhysicalPageManager;
  class GenPageRequest;
  class Page;
  class PageSizeInfo;
  class MemoryReservation;
  class PhysicalRegion;
  class PaTuple;

  /*!
    \class MemoryManager
    \brief Manager that records the starting and ending addresses of all the exception handlers
  */

  class ExceptionManager {
  public:
    typedef uint64 BankNumber;
    typedef uint64 StartAddress;
    typedef uint64 EndAddress;

    /* Used to relate a starting address to an ending address. Used as a component in other structures below */
    typedef std::pair<StartAddress, EndAddress> AddressBoundary;
    /* Used as a list of starting and ending addresses. Used as a component in other structures below */
    typedef std::vector<AddressBoundary> ListOfAddressBoundaries;
    /* used to map handler starting and ending addresses to their handler name */
    typedef std::map<EExceptionClassType, AddressBoundary> NameMappedHandlerAddressBoundaries;

    /* Used to map user addresses and exception addresses to the proper bank number */
    typedef std::vector<ConstraintSet*> ExceptionAreaAddrConstraints;
    /* used to map handler names and their boundaries to the proper bank number */
    typedef std::vector<NameMappedHandlerAddressBoundaries> HandlerNameMappedBoundaries;

    static void Initialize();  //!< Initialization interface.
    static void Destroy();     //!< Destruction clean up interface.
    inline static ExceptionManager* Instance() { return mspExceptionManager; } //!< Access MemoryManager instance.

    bool IsReturningToUser(const PaTuple &returnAddress); //!< Returns true if the target of the current ERET is user code.

    uint64 ExceptionVectorBaseAddress(EExceptionVectorType vectorType) const; //!< return the Exception handler vector base address.

    void SetExceptionVectorBaseAddress(EExceptionVectorType vectorType, uint64 address); //!< set the Exception handler vector base address.

    void AddHandlerBounds(uint64 bank_number, const ListOfAddressBoundaries &bounds); //!< add the exception boundaries

    void RecordSpecificHandlerBoundaries(BankNumber bankNumber, EExceptionClassType exceptClass, AddressBoundary handlerBoundary);
    HandlerNameMappedBoundaries *GetHandlerAddresses() { return mpHandlerSpecificBoundaries; }
  private:
    ExceptionManager(); //!< Default constructor.
    ~ExceptionManager(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(ExceptionManager);
    COPY_CONSTRUCTOR_ABSENT(ExceptionManager);
    void Setup();
  private:
    static ExceptionManager* mspExceptionManager; //!< Pointer to singleton ExceptionManager object.

    ExceptionAreaAddrConstraints *mpHandlerAddressConstraints; //!< Contains the address boundaries mapped by bank number
    HandlerNameMappedBoundaries *mpHandlerSpecificBoundaries;
    std::vector<uint64> mExceptionVectorBases;
  };

}

#endif
