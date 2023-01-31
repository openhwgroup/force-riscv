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
#ifndef Force_OperandRequest_H
#define Force_OperandRequest_H

#include "Defines.h"
#include "Object.h"

namespace Force {

  class ConstraintSet;

  /*!
    \class OperandRequest
    \breif A request to the test generator thread to generate an instruction.
  */
  class OperandRequest : public Object {
  public:
    ASSIGNMENT_OPERATOR_ABSENT(OperandRequest);
    OperandRequest(const std::string& name, uint64 value); //!< Constructor with name and value given.
    OperandRequest(const std::string& name, const std::string& valueStr); //!< Constructor with name and value string given.
    ~OperandRequest(); //!< Destructor.
    Object* Clone() const override;  //!< Return a cloned OperandRequest object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the OperandRequest object.
    const char* Type() const override { return "OperandRequest"; } //!< Return type of the OperandRequest object.

    const std::string& Name() const { return mName; } //!< Return operand name.
    void SetValueRequest(uint64 value); //!< Set value request of the operand.
    void SetValueRequest(const std::string& valueStr); //!< Set value request of the operand in string format.
    inline void SetApplied() const {mApplied = true;} //!< the request is applied
    inline bool IsApplied() const {return mApplied; }//!< return applied status
    inline void SetIgnored() const {mIgnored = true; } //!< the request is ignored
    inline bool IsIgnored() const { return mIgnored; } //!< return ignored status
    const ConstraintSet* GetValueConstraint() const { return mpValueConstraint; } //!< Return pointer to value constraint, if any.
  protected:
    OperandRequest(const OperandRequest& rOther); //!< Copy constructor.
  protected:
    std::string mName; //!< Operand name.
    ConstraintSet* mpValueConstraint; //!< Constraint on operand value selection.
    mutable bool mApplied; //!< whether the request is applied or not
    mutable bool mIgnored; //!< whether the request is ignored or not
  };

}

#endif
