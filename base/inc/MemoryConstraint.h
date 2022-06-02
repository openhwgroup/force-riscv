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
#ifndef Force_MemoryConstraint_H
#define Force_MemoryConstraint_H

#include <map>

#include "Defines.h"
#include "Enums.h"
#include "Object.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class ConstraintSet;

  /*!
    \class LargeConstraintSet
    \brief A wrapper class around large ConstraintSet object to cache updates in order to improve efficiency.  Keep this class here so the normal Usable call can be inlined.
  */
  class LargeConstraintSet {
  public:
    LargeConstraintSet(); //!< Constructor.
    ~LargeConstraintSet(); //!< Destructor.

    LargeConstraintSet* Clone() const { return new LargeConstraintSet(*this); } //!< Clone the LargeConstraintSet object.
    ConstraintSet* GetConstraintSet(); //!< Return ConstraintSet with cached changes committed.
    void Initialize(const ConstraintSet& rInitConstrs); //!< Initialize LargeConstraintSet.
    void Clear(); //!< Clear the LargeConstraintSet.
    void AddRange(uint64 lower, uint64 upper); //!< Add a value range to the LargeConstraintSet.
    void SubRange(uint64 lower, uint64 upper); //!< Subtract a value range from the LargeConstraintSet.
    void MergeConstraintSet(const ConstraintSet& rConstrSet); //!< Merge a ConstraintSet object.
    void SubConstraintSet(const ConstraintSet& rConstrSet); //!< Subtract a ConstraintSet from the LargeConstraintSet.

    ASSIGNMENT_OPERATOR_ABSENT(LargeConstraintSet);
  private:
    LargeConstraintSet(const LargeConstraintSet& rOther); //!< Copy constructor.
    inline bool SomethingCached() const { return (ELargeConstraintSetState(mState) != ELargeConstraintSetState::Clean); } //!< Return true if the ConstraintSet has cached updates.
    void CommitCachedAdds(); //!< Called to commit cached additions, if any.
    inline bool AddCached() const { return ((mState & ELargeConstraintSetStateBaseType(ELargeConstraintSetState::AddCached)) > 0); } //! Check if there is addtractions cached.
    inline void SetAddCached() { mState |= ELargeConstraintSetStateBaseType(ELargeConstraintSetState::AddCached); } //!< Set to indicate there is addtractions cached.
    inline void ClearAddCached() { mState &= ~ELargeConstraintSetStateBaseType(ELargeConstraintSetState::AddCached); } //!< Clear addtractions cached state.
    inline bool SubCached() const { return ((mState & ELargeConstraintSetStateBaseType(ELargeConstraintSetState::SubCached)) > 0); } //!< Check if there is subtractions cached.
    inline void SetSubCached() { mState |= ELargeConstraintSetStateBaseType(ELargeConstraintSetState::SubCached); } //!< Set to indicate there is subtractions cached.
    inline void ClearSubCached() { mState &= ~ELargeConstraintSetStateBaseType(ELargeConstraintSetState::SubCached); } //!< Clear subtractions cached state.
    void CommitCachedSubs(); //!< Called to commit cached subtractions, if any.
  private:
    ConstraintSet* mpConstraintSet; //!< Pointer to main ConstraintSet object.
    ConstraintSet* mpCachedAdds; //!< Pointer to ConstraintSet to be added to the main ConstraintSet object.
    ConstraintSet* mpCachedSubs; //!< Pointer to ConstraintSet to be subtracted from main ConstraintSet object.
    ELargeConstraintSetStateBaseType mState; //!< Update state of the LargeConstraintSet object.
  };

  class AddressReuseMode;

  /*!
    \class MemoryConstraint
    \brief Class holding constraints for various memory access types.

    This class maintains used addresses for some memory access types to allow them to be reused for subsequent accesses.
   */
  class MemoryConstraint : public Object {
  public:
    MemoryConstraint(); //!< Default constructor.
    MemoryConstraint(const MemoryConstraint& rOther); //!< Copy constructor.
    virtual ~MemoryConstraint(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(MemoryConstraint);

    const std::string ToString() const override { return Type(); } //!< Return a string describing the current state of the Object.

    void Initialize(const ConstraintSet& constrSet); //!< Set initial constraint and set to initialized state.
    virtual void Uninitialize(); //!< Clear constraints and set to uninitialized state.
    inline bool IsInitialized() const { return mInitialized; } //!< Returns true if MemoryConstraint is initialized; false otherwise.
    void MarkUsed(cuint64 startAddress, cuint64 endAddress); //!< Mark an address range as used for all memory and access types.
    void MarkUsed(const ConstraintSet& constrSet); //!< Mark the addresses specified by the constraint set as used for all memory and access types.
    void MarkUsedForType(cuint64 startAddress, cuint64 endAddress, const EMemDataType memDataType, const EMemAccessType memAccessType, cuint32 threadId); //!< Mark an address range as used for a given memory and access type.
    void MarkShared(cuint64 startAddress, cuint64 endAddress); //!< Mark an address range as shared.
    void MarkShared(const ConstraintSet& constrSet); //!< Mark the addresses specified by the constraint set as shared.
    void UnmarkUsed(cuint64 startAddress, cuint64 endAddress); //!< Mark the addresses specified by the constraint set as unused for all memory and access types.
    void UnmarkUsed(const ConstraintSet& constrSet); //!< Mark the addresses specified by the constraint set as unused for all memory and access types.
    inline const ConstraintSet* Usable() const { return mpUsable->GetConstraintSet(); } //!< Return the most restrictive usable memory constraint.
    const ConstraintSet* Shared() const { return mpShared->GetConstraintSet(); } //!< Return the shared memory constraint.
    void ApplyToConstraintSet(const EMemDataType memDataType, const EMemAccessType memAccessType, cuint32 threadId, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const; //!< Apply the appropriate constraints to the specified constraint set.
    void ReplaceUsableInRange(uint64 lower, uint64 upper, ConstraintSet& rReplaceConstr); //!< Replace the range with translated new ranges.
  protected:
    virtual void MarkDataUsedForType(cuint64 startAddress, cuint64 endAddress, const EMemAccessType memAccessType, cuint32 threadId) = 0; //!< Mark a data address range as used for a given access type.
    virtual const ConstraintSet* DataReadUsed(cuint32 threadId) const = 0; //!< Return used addresses for data reads.
    virtual const ConstraintSet* DataWriteUsed(cuint32 threadId) const = 0; //!< Return used addresses for data writes.
  private:
    void ApplyToDataConstraintSet(const EMemAccessType memAccessType, cuint32 threadId, const AddressReuseMode& rAddrReuseMode, ConstraintSet* constrSet) const; //!< Apply the appropriate constraints to the specified data address constraint set.
    void ApplyToNonDataConstraintSet(ConstraintSet* constrSet) const; //!< Apply the appropriate constraints to the specified non-data address constraint set.
  private:
    LargeConstraintSet* mpUsable; //!< General usable memory constraint.
    LargeConstraintSet* mpShared; //!< Shared addresses.
    bool mInitialized; //!< Indicates whether MemoryConstraint is initialized.
  };

  /*!
    \class SingleThreadMemoryConstraint
    \brief Class holding memory constraints for a single thread.
   */
  class SingleThreadMemoryConstraint : public MemoryConstraint {
  public:
    explicit SingleThreadMemoryConstraint(cuint32 threadId); //!< Constructor.
    SingleThreadMemoryConstraint(const SingleThreadMemoryConstraint& rOther); //!< Copy constructor.
    ~SingleThreadMemoryConstraint() override; //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(SingleThreadMemoryConstraint);

    Object* Clone() const override { return new SingleThreadMemoryConstraint(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "SingleThreadMemoryConstraint"; } //!< Return a string describing the actual type of the Object.

    void Uninitialize() override; //!< Clear constraints and set to uninitialized state.
  protected:
    void MarkDataUsedForType(cuint64 startAddress, cuint64 endAddress, const EMemAccessType memAccessType, cuint32 threadId) override; //!< Mark a data address range as used for a given access type.
    const ConstraintSet* DataReadUsed(cuint32 threadId) const override; //!< Return used addresses for data reads.
    const ConstraintSet* DataWriteUsed(cuint32 threadId) const override; //!< Return used addresses for data writes.
  private:
    cuint32 mThreadId; //!< Thread ID associated with the memory constraints.
    LargeConstraintSet* mpDataReadUsed; //!< Used addresses for data reads.
    LargeConstraintSet* mpDataWriteUsed; //!< Used addresses for data writes.
  };

  /*!
    \class MultiThreadMemoryConstraint
    \brief Class holding memory constraints for multiple threads.
   */
  class MultiThreadMemoryConstraint : public MemoryConstraint {
  public:
    explicit MultiThreadMemoryConstraint(cuint32 threadCount); //!< Constructor.
    MultiThreadMemoryConstraint(const MultiThreadMemoryConstraint& rOther); //!< Copy constructor.
    ~MultiThreadMemoryConstraint() override; //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(MultiThreadMemoryConstraint);

    Object* Clone() const override { return new MultiThreadMemoryConstraint(*this); } //!< Return a cloned Object of the same type and same contents as the Object being cloned.
    const char* Type() const override { return "MultiThreadMemoryConstraint"; } //!< Return a string describing the actual type of the Object.

    void Uninitialize() override; //!< Clear constraints and set to uninitialized state.
  protected:
    void MarkDataUsedForType(cuint64 startAddress, cuint64 endAddress, const EMemAccessType memAccessType, cuint32 threadId) override; //!< Mark a data address range as used for a given access type.
    const ConstraintSet* DataReadUsed(cuint32 threadId) const override; //!< Return used addresses for data reads.
    const ConstraintSet* DataWriteUsed(cuint32 threadId) const override; //!< Return used addresses for data writes.
  private:
    std::map<cuint32, LargeConstraintSet*> mDataReadUsedByThread; //!< Used addresses for data reads by thread ID.
    std::map<cuint32, LargeConstraintSet*> mDataWriteUsedByThread; //!< Used addresses for data writes by thread ID.
  };

}

#endif
