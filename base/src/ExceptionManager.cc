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
#include <Constraint.h>
#include <ExceptionManager.h>
#include <Generator.h>
#include <Log.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

using namespace std;


namespace Force {
  ExceptionManager * ExceptionManager::mspExceptionManager = nullptr;

  ExceptionManager::ExceptionManager() : mpHandlerAddressConstraints(nullptr), mpHandlerSpecificBoundaries(nullptr), mExceptionVectorBases()
  {
    /* Number of banks present */
    uint64 numBanks = EMemBankTypeSize;

    mpHandlerAddressConstraints = new ExceptionAreaAddrConstraints(numBanks);
    mpHandlerSpecificBoundaries = new HandlerNameMappedBoundaries(numBanks);
    mExceptionVectorBases.assign(EExceptionVectorTypeSize, 0);
  }

  /*
   *  Exception manager singleton mangement functions.
   */
  void ExceptionManager::Initialize()
  {
    if (mspExceptionManager == nullptr) {
      mspExceptionManager = new ExceptionManager();
      mspExceptionManager->Setup();
    }
  }

  void ExceptionManager::Destroy()
  {
    delete mspExceptionManager;
    mspExceptionManager = nullptr;
  }

  ExceptionManager::~ExceptionManager()
  {
    delete mpHandlerAddressConstraints;
    delete mpHandlerSpecificBoundaries;
    return;
  }

  void ExceptionManager::Setup()
  {
    return;
  }

  bool ExceptionManager::IsReturningToUser(const PaTuple &returnAddress)
  {
      /* The PaTuple has the bank number as the second element */
      uint64 bank_number = returnAddress.mBank;

      ConstraintSet *handlersInThisBank = (*mpHandlerAddressConstraints)[bank_number];

      /* If we have no handlers registered, by default return true */
      if (handlersInThisBank == nullptr) {
        LOG(notice) << "There were no handler addresses registered for NS:" << bank_number << endl;
        return true;
      }

      return !handlersInThisBank->ContainsValue(returnAddress.mAddress);
  }

  void ExceptionManager::RecordSpecificHandlerBoundaries(BankNumber bankNumber, EExceptionClassType exceptClass, AddressBoundary handlerBoundary)
  {
    (*mpHandlerSpecificBoundaries)[bankNumber][exceptClass] = handlerBoundary;
  }

  void ExceptionManager::AddHandlerBounds(uint64 bank_number, const std::vector<std::pair<uint64, uint64>> &bounds)
  {
    /* Append the new handler boundaries to what's already recorded */
    ConstraintSet *handlersInThisBank = (*mpHandlerAddressConstraints)[bank_number];

    if (handlersInThisBank == nullptr) {
      /* ListOfAddressBoundaries is typedef'd as an array of pairs */
      (*mpHandlerAddressConstraints)[bank_number] = new ConstraintSet();
      handlersInThisBank = (*mpHandlerAddressConstraints)[bank_number];
    }

    for (std::pair<uint64, uint64> handler_bound : bounds) {
        handlersInThisBank->AddRange(handler_bound.first, handler_bound.second);
    }
  }

  uint64 ExceptionManager::ExceptionVectorBaseAddress(EExceptionVectorType vectorType) const
  {
    return mExceptionVectorBases[int(vectorType)];
  }

  void ExceptionManager::SetExceptionVectorBaseAddress(EExceptionVectorType vectorType, uint64 address)
  {
    mExceptionVectorBases[int(vectorType)] = address;
  }

}
