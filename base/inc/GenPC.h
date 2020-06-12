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
#ifndef Force_GenPC_H
#define Force_GenPC_H

#include <Defines.h>
#include <Object.h>

namespace Force {

  class TranslationRange;
  class Generator;
  class VmMapper;
  class ConstraintSet;
  class PaTuple;

  /*!
    \class GenPC
    \brief Data structure managing each individual PE's PC vicinity properties.
  */

  class GenPC : public Object {
  public:
    Object* Clone() const override; //!< Clone a GenPC object of the same type.
    const std::string ToString() const override; //!< Return a string describing the current state of the GenPC object.
    const char* Type() const override { return "GenPC"; } //!< Return object type in a C string.

    GenPC(); //!< Default constructor.
    explicit GenPC(uint64 alignmentMask); //!< Constructor with PC alignment mask specified
    ~GenPC(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenPC);

    void SetInstructionSpace(uint32 bytes); //!< Set current instruction space.
    inline uint64 Value() const { return mValue; } //!< Return current PC value.
    inline uint64 LastValue() const { return mLastValue; }
    inline void Advance(uint32 bytes) { mLastValue = mValue; mValue += bytes; mPaValid = false; } //!< Advance PC value.
    inline void Set(uint64 value) { mLastValue = mValue; mValue = value; mPaValid = false; } //!< Set new PC value.
    inline void SetAligned(uint64 value) { mLastValue = mValue; mValue = value & mAlignMask; mPaValid = false; } //!< Set new aligned PC value.
    inline uint32 InstructionSpace() const { return mInstructionSpace; } //!< Return instruction space size.
    void Update(uint32 iSpace); //!< Update GenPC object, called when virtual memory system is updated.
    void MapPC(Generator* pGen); //!< Map the current PC.
    uint64 GetPA(Generator* pGen, uint32& bank, bool& fault); //!< Get PA for the current PC.
    void GetPA(Generator* pGen, PaTuple& rPaTuple); //!< Get PA for the current PC.
    void SetAlignMask(uint64 alignMask) { mAlignMask = alignMask; } //!< Set PC alignment mask.
  private:
    GenPC(const GenPC& rOther); //!< Copy constructor.
  private:
    TranslationRange* mpPcTransRange; //!< The TranslationRange covers current PC.
    TranslationRange* mpCrossOverRange; //!< The cross over range of PC vicinity if applicable.
    uint64 mValue; //!< The current PC value of the PE.
    uint64 mLastValue; //!< The last PC value of the PE.
    uint64 mAlignMask; //!< The alignment mask for PC setting from simulator.
    uint64 mPaStart1; //!< Starting physical address for the PC vicinity, part1.
    uint64 mPaEnd1; //!< Ending physical address for the PC vicinity, part1
    uint64 mPaStart2; //!< Starting physical address for the PC vicinity, part1.
    uint64 mPaEnd2; //!< Ending physical address for the PC vicinity, part1
    uint32 mInstructionSpace; //!< Instruction space.
    uint32 mPaBank1; //!< Memory bank of PA vicinity part 1.
    uint32 mPaBank2; //!< Memory bank of PA vicinity part 2.
    bool mPaValid; //!< Indicate if PA values are valid.
    bool mPaPart2Valid; //!< Indicate if PA vicinity part 2 is valid.
  };

}

#endif
