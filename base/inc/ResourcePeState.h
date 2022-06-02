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
#ifndef Force_ResourcePeState_H
#define Force_ResourcePeState_H

#include <functional>
#include <string>
#include <vector>

#include "Defines.h"
#include "Enums.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class Generator;
  class SimAPI;
  class PhysicalRegister;
  class ResourcePeState;
  class ResourceDependence;

  /*!
    \class ResourcePeStateStack
    \brief Base for recording Resource Pe State Stack.
  */

  class ResourcePeStateStack {
  public:
    explicit ResourcePeStateStack(EResourcePeStateType type) : mResourcePeStates(), mStateType(type) { } //!< Constructor
    virtual ~ResourcePeStateStack(); //!< Destructor
    void PushResourcePeState(const ResourcePeState* pState); //!< push resource pe state to stacks
    virtual bool RecoverResourcePeStates(Generator* pGen, SimAPI* pSim); //!< recover resource pe states on both FORCE and ISS side
    bool IsEmpty() const; //!< whether the stack is empty

    ASSIGNMENT_OPERATOR_ABSENT(ResourcePeStateStack);
    COPY_CONSTRUCTOR_ABSENT(ResourcePeStateStack);
  protected:
    bool IsDuplicated(const ResourcePeState* pState) const; //!< whether the state was pushed
  protected:
    std::vector<const ResourcePeState* > mResourcePeStates; //!< The container for resource PE states
    EResourcePeStateType mStateType; //!< resource state type
  };

  using RestoreFunction = std::function<void(const std::vector<const ResourcePeState*>&)>;

  /*!
    \class IncrementalResourcePeStateStack
    \brief Class for restoring state in several iterations.
  */

  class IncrementalResourcePeStateStack : public ResourcePeStateStack {
  public:
    explicit IncrementalResourcePeStateStack(EResourcePeStateType type); //!< Constructor
    ~IncrementalResourcePeStateStack(); //!< Destructor
    COPY_CONSTRUCTOR_ABSENT(IncrementalResourcePeStateStack);
    ASSIGNMENT_OPERATOR_ABSENT(IncrementalResourcePeStateStack);

    bool RecoverResourcePeStates(Generator* pGen, SimAPI* pSim) override; //!< recover resource pe states on both FORCE and ISS side
    void SetRestoreFunction(RestoreFunction restoreFunction); //!< Specify a function to call to restore state.
  private:
    RestoreFunction mRestoreFunction;
    size_t mNextEndIndex;
  };

  /*!
    \class ResourcePeState
    \brief Base for recording Resource Pe State.
  */

  class ResourcePeState {
  public:
    ResourcePeState() { } //!< Default constructor
    virtual const std::string ToString() const = 0; //!< Return a string describing resource Pe state.
    virtual const char* Type() const = 0; //!< Return a string describing the actual type of resource state.
    virtual bool DoStateRecovery(Generator* pGen, SimAPI* pSim) const = 0; //!< The Interface to recover resource state
    virtual bool IsIdenticalState(const ResourcePeState* pState) const = 0; //!< whether the state is Identical or not
    virtual EResourcePeStateType GetStateType() const = 0; //!< Return resource state type.

     /*!
      Templated function so that a derived class can conveniently cast base class to the desired derived class type.
      For example, ImmediateOperandConstraint* imm_constraint = opr_constraint->CastInstance<ImmediateOperandConstraint>();
     */
    template<typename T>
      T* CastInstance() const
      {
        T* cast_instance = dynamic_cast<T* >(this);
        return cast_instance;
      }

    virtual ~ResourcePeState() { } //!< virtual destructor
  };

  /*!
    \class RegisterPeState
    \brief class for recording Register State.
  */

  class RegisterPeState : public ResourcePeState {
  public:
    RegisterPeState(PhysicalRegister* pPhysicalRegister, uint64 mask, uint64 value)
    : ResourcePeState(), mpPhysicalRegister(pPhysicalRegister), mMask(mask), mValue(value) { } //!< Constructor
    RegisterPeState() : ResourcePeState(), mpPhysicalRegister(nullptr), mMask(0), mValue(0) { } //!< Default Constructor
    const std::string ToString() const override; //!< Return a string describing resource state.
    const char* Type() const override; //!< Return a string describing the actual type of resource state.
    bool DoStateRecovery(Generator* pGen, SimAPI* pSim) const override; //!< The Interface to recover resource state
    bool IsIdenticalState(const ResourcePeState* pState) const override; //!< whether the state is same or not
    EResourcePeStateType GetStateType() const override; //!< Return resource state type.

    inline const PhysicalRegister* GetPhysicalRegister() const { return mpPhysicalRegister; } //!< get the pointer to physical register
    inline uint64 GetValue() const { return mValue; } //!< get register value

    ASSIGNMENT_OPERATOR_ABSENT(RegisterPeState);
    COPY_CONSTRUCTOR_ABSENT(RegisterPeState);
  private:
    PhysicalRegister* mpPhysicalRegister; //!< The pointer to physical register
    uint64 mMask; //!< value mask
    uint64 mValue; //!< register value
  };

  /*!
    \class PCPeState
    \brief class for recording PC State.
  */

  class PCPeState : public RegisterPeState {
  public:
    explicit PCPeState(uint64 pc) : RegisterPeState(), mPC(pc) { } //!< Constructor
    const std::string ToString() const override; //!< Return a string describing resource state.
    bool DoStateRecovery(Generator* pGen, SimAPI* pSim) const override; //!< The Interface to recover resource state
    bool IsIdenticalState(const ResourcePeState* pState) const override; //!< whether the state is same or not
    inline uint64 GetPC() const { return mPC; } //!< get pc value

    ASSIGNMENT_OPERATOR_ABSENT(PCPeState);
    COPY_CONSTRUCTOR_ABSENT(PCPeState);
  protected:
    uint64 mPC; //!< PC value
  };

   /*!
    \class MemoryPeState
    \brief class for recording Memory Pe State.
  */

  class MemoryPeState : public ResourcePeState {
  public:
    MemoryPeState(uint32 bank, uint64 physicalAddress, uint64 virtualAddress)
      : ResourcePeState(), mBank(bank), mPhysicalAddress(physicalAddress), mVirtualAddress(virtualAddress) { } //!< Constructor
    SUPERCLASS_DESTRUCTOR_DEFAULT(MemoryPeState); //!< Destructor
    const std::string ToString() const override; //!< Return a string describing resource state.
    const char* Type() const override; //!< Return a string describing the actual type of resource state.
    bool IsIdenticalState(const ResourcePeState* pState) const override; //!< whether the state is same or not
    EResourcePeStateType GetStateType() const override; //!< Return resource state type.

    inline uint32 GetBank() const { return mBank; } //!< return bank
    inline uint64 GetPhysicalAddress() const { return mPhysicalAddress; } //!< return physical Address
    inline uint64 GetVirtualAddress() const { return mVirtualAddress; } //!< return virtual Address
    virtual uint64 GetData() const = 0; //!< Return the data contained in memory.

    ASSIGNMENT_OPERATOR_ABSENT(MemoryPeState);
    COPY_CONSTRUCTOR_ABSENT(MemoryPeState);
  protected:
    uint32 mBank; //!< memory bank
    uint64 mPhysicalAddress; //!< physical address
    uint64 mVirtualAddress; //!< virtual address
  };

   /*!
    \class ByteMemoryPeState
    \brief class for recording byte-sized segments of Memory Pe State.
  */

  class ByteMemoryPeState : public MemoryPeState {
  public:
    ByteMemoryPeState(uint32 bank, uint64 physicalAddress, uint64 virtualAddress, unsigned char dataByte)
      : MemoryPeState(bank, physicalAddress, virtualAddress), mData(dataByte) { } //!< Constructor
    SUBCLASS_DESTRUCTOR_DEFAULT(ByteMemoryPeState); //!< Destructor
    const char* Type() const override; //!< Return a string describing the actual type of resource state.
    bool DoStateRecovery(Generator* pGen, SimAPI* pSim) const override; //!< The Interface to recover resource state
    uint64 GetData() const override; //!< Return the data contained in memory.

    ASSIGNMENT_OPERATOR_ABSENT(ByteMemoryPeState);
    COPY_CONSTRUCTOR_ABSENT(ByteMemoryPeState);
  private:
    const unsigned char mData; //!< the data contained in memory
  };

   /*!
    \class BlockMemoryPeState
    \brief class for recording larger than byte-sized segements of Memory Pe State.
  */

  class BlockMemoryPeState : public MemoryPeState {
  public:
    BlockMemoryPeState(uint32 bank, uint64 virtualAddress, uint64 data, uint32 blockSize)
      : MemoryPeState(bank, 0, virtualAddress), mData(data), mBlockSize(blockSize) { } //!< Constructor
    SUBCLASS_DESTRUCTOR_DEFAULT(BlockMemoryPeState); //!< Destructor
    const char* Type() const override; //!< Return a string describing the actual type of resource state.
    bool DoStateRecovery(Generator* pGen, SimAPI* pSim) const override; //!< The Interface to recover resource state
    bool IsIdenticalState(const ResourcePeState* pState) const override; //!< whether the state is same or not
    uint64 GetData() const override; //!< Return the data contained in memory.
    uint32 GetBlockSize() const; //!< Return the number of bytes recorded.
    static constexpr uint32 GetMaxBlockSize() { return sizeof(uint64); } //!< Return maximum number of bytes that can be recorded.

    ASSIGNMENT_OPERATOR_ABSENT(BlockMemoryPeState);
    COPY_CONSTRUCTOR_ABSENT(BlockMemoryPeState);
  private:
    cuint64 mData; //!< The data contained in memory.
    cuint32 mBlockSize; //!< The number of bytes recorded up to a maximum of sizeof(uint64).
  };

  /*!
    \class DependencePeState
    \brief class for recording register dependence Pe State.
  */
  class DependencePeState : public ResourcePeState {
  public:
    explicit DependencePeState(const ResourceDependence* pDependence) : ResourcePeState(), mpDependence(pDependence) { } //!< copy construtor

    const std::string ToString() const override; //!< Return a string describing resource state.
    const char* Type() const override; //!< Return a string describing the actual type of resource state.
    bool DoStateRecovery(Generator* pGen, SimAPI* pSim) const override; //!< The Interface to recover resource state
    bool IsIdenticalState(const ResourcePeState* pState) const override; //!< whether the state is same or not
    EResourcePeStateType GetStateType() const override; //!< Return resource state type.

    ASSIGNMENT_OPERATOR_ABSENT(DependencePeState);
    COPY_CONSTRUCTOR_ABSENT(DependencePeState);
  protected:
    mutable const ResourceDependence *mpDependence; //!< the pointer to dependence
  };

}

#endif
