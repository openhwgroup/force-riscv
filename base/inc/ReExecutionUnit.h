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
#ifndef Force_ReExecutionUnit_H
#define Force_ReExecutionUnit_H

#include <Defines.h>

namespace Force {

  class ConstraintSet;

  /*!
    \class ReExecutionUnit
    \brief Handles re-execution related functionalities.
  */

  class ReExecutionUnit {
  public:
    ReExecutionUnit(); //!< Constructor.
    COPY_CONSTRUCTOR_ABSENT(ReExecutionUnit);
    virtual ~ReExecutionUnit(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(ReExecutionUnit);
    void SetId(uint32 id) { mId = id; } //!< Set unit ID.
    uint32 Id() const { return mId; } //!< Return unit ID.
    void SetBeginAddress(uint64 beginAddr) { mBeginAddress = beginAddr; } //!< Set begin address.
    uint64 BeginAddress() const { return mBeginAddress; } //!< Return begin address.
    void SetEndAddress(uint64 endAddr) { mEndAddress = endAddr; } //!< Set end address
    uint64 EndAddress() const { return mEndAddress; } //!< Return end address.
  protected:
    uint32 mId; //!< Id of the unit.
    uint64 mBeginAddress; //!< Begin address.
    uint64 mEndAddress; //!< End address.
    ConstraintSet* mpVaRanges; //!< Virtual addresses of the loop body.
    ConstraintSet* mpPaRanges; //!< Physical addresses of the loop body.
  };

  /*!
    \class Loop
    \brief Handles loop related functionalities.
  */

  class Loop : public ReExecutionUnit {
  public:
    Loop() : ReExecutionUnit(), mReconvergeAddress(0), mPostLoopAddress(0) { } //!< Default constructor.
    void SetPostLoopAddress(uint64 addr) { mPostLoopAddress = addr; } //!< Set post loop address.
    void SetReconvergeAddress(uint64 addr) { mReconvergeAddress = addr; } //!< Set loop reconverge address.
    uint64 PostLoopAddress() const { return mPostLoopAddress; } //!< Return post loop address.
    uint64 ReconvergeAddress() const { return mReconvergeAddress; } //!< Return loop reconverge address.
    COPY_CONSTRUCTOR_ABSENT(Loop);
    SUBCLASS_DESTRUCTOR_DEFAULT(Loop); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(Loop);
  protected:
    uint64 mReconvergeAddress; //!< Loop reconverge address.
    uint64 mPostLoopAddress; //!< Post loop address.
  };

  /*!
    \class LinearBlock
    \brief Handles linear block related functionalities.
  */

  class LinearBlock : public ReExecutionUnit {
  public:
    DEFAULT_CONSTRUCTOR_DEFAULT(LinearBlock); //!< Constructor.
    COPY_CONSTRUCTOR_ABSENT(LinearBlock);
    SUBCLASS_DESTRUCTOR_DEFAULT(LinearBlock); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(LinearBlock);
  };

}

#endif
