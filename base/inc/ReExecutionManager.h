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
#ifndef Force_ReExecutionManager_H
#define Force_ReExecutionManager_H

#include <Defines.h>
#include <Object.h>
#include <vector>

namespace Force {

  class ReExecutionUnit;
  class LinearBlock;

  /*!
    \class ReExecutionManager
    \brief Class managing re-execution of generated code.
  */

  class ReExecutionManager : public Object {
  public:
    Object * Clone() const override { return new ReExecutionManager(*this); } //!< Clone ReExecutionManager object.
    const std::string ToString() const override { return Type(); } //!< Return a string describing the ReExecutionManager object.
    const char* Type() const override { return "ReExecutionManager"; } //!< Return ReExecutionManager object type.

    ReExecutionManager(); //!< Default constructor.
    ~ReExecutionManager(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(ReExecutionManager);
    uint32 StartLoop(uint64 loopBackAddr); //!< Start a new loop.
    void EndLoop(uint32 loopId, uint64 loopEndAddr); //!< End the current loop.
    void ReportPostLoopAddress(uint32 loopId, uint64 postLoopAddr); //!< Report post loop address for the specified loop.
    void ReportLoopReconvergeAddress(uint32 loopId, uint64 loopReconvergeAddr); //!< Report loop reconverge address for the specified loop.
    uint64 PostLoopAddress() const; //!< Return post loop address of the current loop.
    uint64 LoopReconvergeAddress() const; //!< Return loop reconverge address of the current loop.
    uint32 StartLinearBlock(uint64 startAddr); //!< Start a new linear block.
    uint64 EndLinearBlock(uint32 blockId, uint64 blockEndAddr); //!< End the current linear block.
  protected:
    ReExecutionManager(const ReExecutionManager& rOther); //!< Copy constructor.
    void PushActiveGenUnit(ReExecutionUnit* pNewUnit); //!< Push a new unit.
    void PopActiveGenUnit(uint64 endAddr); //!< Pop a active unit.
  protected:
    ReExecutionUnit* mpCurrentGenUnit; //!< Pointer to the current ReExecutionUnit being generated, if any.
    LinearBlock* mpCurrentGenBlock; //!< Current linear block being generated, if any.
    std::vector<ReExecutionUnit* > mActiveGenUnits; //!< Container for currently active re-execution units being generated.
    std::vector<ReExecutionUnit* > mUnits; //!< Container for ReExecutionUnit objects.
    std::vector<LinearBlock* > mBlocks; //!< Container for LinearBlock objects.
  };

}

#endif
