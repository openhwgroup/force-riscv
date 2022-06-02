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
#ifndef Force_BntNode_H
#define Force_BntNode_H

#include <vector>

#include "Defines.h"
#include "Enums.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class Generator;
  class SimplePeState;
  class ResourcePeState;
  class ResourcePeStateStack;
  class Instruction;

  /*!
    \class BntNode
    \brief Base for recording BNT node information.
  */

  class BntNode {
  public:
    explicit BntNode(uint64 brTarget, bool taken=false, bool cond=false); //!< Constructor with parameters given.
    virtual ~BntNode(); //!< Virtual destructor.

    const std::string ToString() const; //!< Return BntNode details in a string.

    ASSIGNMENT_OPERATOR_ABSENT(BntNode);
    uint64 BranchTarget() const { return mBranchTarget; } //!< Return branch target.
    uint64 NextPC() const { return mNextPC; } //!< Return next PC address.
    void SetNextPC(uint64 nextPC) { mNextPC = nextPC; } //!< Set next PC value.
    inline bool BranchTaken() const { return (mAttributes >> uint32(EBntAttributeType::Taken)) & 1; } //!< Return whether the branch is taken.
    void SetTaken(bool taken); //!< Set the branch taken attribute.
    inline bool IsConditional() const { return (mAttributes >> uint32(EBntAttributeType::Conditional)) & 1; } //!< Return whether the branch is conditional
    void SetConditional(bool cond); //!< Set the branch conditional attribute.
    inline bool IsAccurate() const { return (mAttributes >> uint32(EBntAttributeType::Accurate)) & 1; } //!< Return whether the branch taken is accurate.
    void SetAccurate(bool accu); //!< Set that the BNT taken is accurate.
    uint64 TakenPath() const; //!< Return taken path starting address.
    uint64 NotTakenPath() const; //!< Return not taken path starting address.
    void PreserveNotTakenPath(Generator* pGen); //!< Preserve the not-taken path.
    bool PathsSame() const { return (mBranchTarget == mNextPC); } //!< Return whether the branch target and next PC are the same.
    SimplePeState* GetPeState() const { return mpPeState; } //!< Return pointer to PE state object.
    void UpdateAccurateState(Generator* pGen, uint32 instrBytes); //!< Update accurate state.
    const std::string& GetSequenceName(void) const { return mSequenceName; } //!< get sequence name
    const std::string& GetBntFunction(void) const { return mBntFunction; }
    void SetSequenceName(const std::string& seqName) {mSequenceName = seqName; } //!< set sequence name
    void SetBntFunction(const std::string& bntFunction) { mBntFunction = bntFunction; }
    virtual bool IsSpeculative() const { return false; } //!< Return whether the bnt is speculative or not
    virtual void PushResourcePeState(const ResourcePeState* pState) { } //!< An interface to push resource state
    virtual bool RecoverResourcePeStates(Generator* pGen) {  return false; } //!< An interface to recover all resource states
    virtual void SetRealPath(uint64 targetPC) { } //!< set real path which may be unaligned
    virtual uint64 RealPath() const { return TakenPath(); } //!< return real path which may be unaligned.
    virtual void ReserveTakenPath(Generator* pGen); //!< Reserve the not-taken path.
    virtual void UnreserveTakenPath(Generator* pGen); //!< Unreserve the taken path.
    virtual void RecordExecution(const Instruction* pInstr) { } //!< Record execution on a Bnt
    virtual bool ExecutionIsOverflow() { return false; } //!< whether execution on a bnt is overflow
  protected:
    BntNode() : mBranchTarget(0), mNextPC(0), mAttributes(0), mId(0),mSequenceName(), mBntFunction(), mpPeState(nullptr) { } //!< Default constructor.
    BntNode(const BntNode& rOther) : mBranchTarget(0), mNextPC(0), mAttributes(0),mId(0), mSequenceName(), mBntFunction(), mpPeState(nullptr) { } //!< Copy constructor.
    void SetBoolAttribute(EBntAttributeType attrType, bool setIt); //!< Set BNT boolean attribute.
    void SavePeState(Generator* pGen); //!< Save necessary PE states.
  protected:
    static uint32 msBntId; //!< BNT identifier.
    uint64 mBranchTarget; //!< Branch target address.
    uint64 mNextPC; //!< Next PC address.
    uint32 mAttributes; //!< BNT attributes.
    uint32 mId; //!< Bnt id
    std::string mSequenceName;  //!< bnt sequence name
    std::string mBntFunction; //!< bnt function
    SimplePeState* mpPeState; //!< Pointer to PE state object.
  };

   /*!
    \class BntNode
    \brief Base for recording BNT node information.
  */

  class SpeculativeBntNode : public BntNode {
  public:
    explicit SpeculativeBntNode(uint64 brTarget, bool taken=false, bool cond=false); //!< Constructor
    ~SpeculativeBntNode(); //!< Destructor
    bool IsSpeculative() const override { return true; } //!< Whether node is speculative or not
    void PushResourcePeState(const ResourcePeState* pState) override; //!< Push resource state
    bool RecoverResourcePeStates(Generator* pGen) override; // Recover all resource states. Return true if the recovering has el regime switch
    void SetRealPath(uint64 targetPC) override { mRealPath = targetPC; } //!< set real path which may be unaligned
    uint64 RealPath() const override { return mRealPath; } //!< return real path which may be unaligned.
    void ReserveTakenPath(Generator* pGen) override; //!< Reserve the not-taken path.
    void UnreserveTakenPath(Generator* pGen) override; //!< Unreserve the taken path.
    void RecordExecution(const Instruction* pInstr) override; //!< Record execution on a Bnt
    bool ExecutionIsOverflow() override; //!< whether execution on a bnt is overflow

    ASSIGNMENT_OPERATOR_ABSENT(SpeculativeBntNode);
  protected:
    SpeculativeBntNode(); //!< Default Construtor
    SpeculativeBntNode(const SpeculativeBntNode& rOther); //!< Copy Construtor
  protected:
    std::vector<ResourcePeStateStack* > mResourcePeStateStacks; //!< The container for all types of resource state stacks.
    uint64 mRealPath; //!< real path which may be unaligned
    uint64 mInstructions; //!< number of instructions speculated on BNT
    bool mReservedTakenPath; //!< whether reserve taken path
  };

}

#endif
