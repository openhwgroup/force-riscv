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
#ifndef Force_RestoreLoop_H
#define Force_RestoreLoop_H

#include <Notify.h>
#include <NotifyDefines.h>
#include <ResourcePeState.h>
#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <map>
#include <set>
#include <stack>
#include <vector>

namespace Force {

  class IncrementalResourcePeStateStack;
  class GenRequest;

  /*!
    \class RestoreLoop
    \brief Class to represent state restore loops.
  */
  class RestoreLoop {
  public:
    RestoreLoop(cuint32 loopRegIndex, cuint32 branchRegIndex, cuint32 simCount, cuint32 restoreCount, const std::set<ERestoreExclusionGroup>& rRestoreExclusions, cuint64 loopBackAddress, cuint32 loopId, Generator* pGenerator); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(RestoreLoop);
    SUPERCLASS_DESTRUCTOR_DEFAULT(RestoreLoop); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(RestoreLoop);

    virtual void BeginLoop() = 0; //!< Mark the start of a state restore loop.
    virtual void EndLoop() = 0; //!< Mark the end of a state restore loop.
    void PushResourcePeState(const ResourcePeState* pState); //!< Record resource state.
    virtual void GenerateRestoreInstructions() = 0; //!< Generate instructions to restore the state to that prior to the start of a loop iteration.
    void SetRestoreStartAddress(cuint64 restoreStartAddr); //!< Set address of the start of the restore instructions.
    uint32 GetLoopId() const; //!< Get loop ID.
    uint64 GetLoopBackAddress() const; //!< Get address of the start of the loop.
    uint64 GetRestoreStartAddress() const; //!< Get address of the start of the restore instructions.
    bool OnFirstRestoreIteration() const; //!< Return whether the loop is currently executing its first restore iteration.
    bool OnLastRestoreIteration() const; //!< Return whether the loop is currently executing its last restore iteration.
    bool HasFinishedRestoreIterations() const; //!< Return whether the loop has finished executing all restore iterations.
  protected:
    void CommitRestoreInstructions(std::vector<GenRequest*>& rRequestSeq); //!< Finalize generation of restore instructions.
    IncrementalResourcePeStateStack* GetResourcePeStateStack(const ERestoreGroup restoreGroup);
    virtual ERestoreGroup GetRestoreGroup(const ResourcePeState* pState) const = 0; //!< Get restore group for specified resource state.
    bool IsExcluded(const ERestoreExclusionGroup restoreGroup) const; //!< Return whether the specified restore group has been excluded from restore instruction generation.
    uint32 GetLoopRegisterIndex() const; //!< Get the index of the loop count register.
    uint32 GetBranchRegisterIndex() const; //!< Get the index of the branch register.
    uint32 GetSimulationCount() const; //!< Get the number of times the loop will execute.
    uint32 GetCurrentRestoreCount() const; //!< Get the number of times restore instructions have been generated.
  protected:
    Generator* mpGenerator; //!< Pointer to generator.
  private:
    const std::set<ERestoreExclusionGroup> mRestoreExclusions; //!< Set of restore groups that have been excluded from restore instruction generation.
    cuint32 mLoopRegIndex; //!< Index of the loop count register
    cuint32 mBranchRegIndex; //!< Index of the branch register
    cuint64 mLoopBackAddr; //!< Address of the start of the loop
    cuint32 mSimCount; //!< Number of times the loop will execute
    cuint32 mEndRestoreCount; //!< Number of times to generate restore instructions
    uint32 mCurRestoreCount; //!< Number of times restore instructions have been generated
    uint32 mLoopId; //!< Loop ID
    uint64 mRestoreStartAddr; //!< Address of the start of the restore instructions
    std::map<ERestoreGroup, IncrementalResourcePeStateStack> mResourcePeStateGroups; //!< Preserved state organized by restore group
  };

  /*!
    \class RestoreLoopManager
    \brief Class to manage state restore loops.
  */
  class RestoreLoopManager : public NotificationReceiver {
  public:
    explicit RestoreLoopManager(Generator* pGenerator); //!< Default constructor
    COPY_CONSTRUCTOR_ABSENT(RestoreLoopManager);
    ~RestoreLoopManager() override; //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(RestoreLoopManager);

    void BeginLoop(cuint32 loopRegIndex, cuint32 simCount, cuint32 restoreCount, const std::set<ERestoreExclusionGroup>& rRestoreExclusions); //!< Mark the start of a state restore loop.
    void EndLoop(cuint32 loopId); //!< Mark the end of a state restore loop.
    void GenerateRestoreInstructions(cuint32 loopId); //!< Generate instructions to restore the state to that prior to the start of a loop iteration.
    void PushResourcePeState(const ResourcePeState* pState); //!< Record resource state.
    uint32 GetCurrentLoopId() const; //!< Get ID of the current loop.
    uint64 GetCurrentLoopBackAddress() const; //!< Get address of the start of the current loop.
    uint32 GetBranchRegisterIndex() const; //!< Get the index of the branch register.
  protected:
    void HandleNotification(const NotificationSender* sender, ENotificationType eventType, Object* pPayload) override; //!< Handle a notification.
    virtual RestoreLoop* CreateRestoreLoop(cuint32 loopRegIndex, cuint32 branchRegIndex, cuint32 simCount, cuint32 restoreCount, const std::set<ERestoreExclusionGroup>& rRestoreExclusions, cuint64 loopBackAddress, cuint32 loopId) const = 0; //!< Create a RestoreLoop.
    Generator* GetGenerator() const; //!< Get instruction generator.
    void SetBranchRegisterIndex(cuint32 branchRegIndex); //!< Set the index of the branch register.
    virtual void ReserveBranchRegister() = 0; //!< Reserve the branch register.
    virtual void UnreserveBranchRegister() = 0; //!< Unreserve the branch register.
  private:
    void ValidateExceptionMode(); //!< Fail if generator is running in an unsupported exception mode.
    void HandlePcUpdate(cuint64 curPc); //!< Respond to PCUpdate notification.
    void BeginNextIteration(); //!< Start the next loop iteration.
    void BeginNextRestoreIteration(); //!< Start the next iteration of restore instructions.
    void BeginNestedLoop(); //!< Start executing a loop that has been previously generated within the current loop.
    void EndNestedLoop(); //!< Stop executing a loop that has been previously generated within the current loop.
    void FinalizeCurrentLoop(); //!< Wrap up the currently executing loop; should be called when execution passes the end of the loop.
    bool IsNestedLoopStartAddress(cuint64 addr) const; //!< Return true if the specified address marks the beginning of a previously-generated nested loop.
    bool IsNestedLoopEndAddress(cuint64 addr) const; //!< Return true if the specified address marks the beginning of a previously-generated nested loop.
    void AddNestedLoopAddresses(cuint64 nestedLoopStartAddr, cuint64 nestedLoopEndAddr); //!< Record the start and end addresses for previously-generated nested loop.
    RestoreLoop* GetCurrentRestoreLoop() const; //!< Get the currently executing loop.
  private:
    Generator* mpGenerator; //!< Instruction generator
    std::stack<RestoreLoop*> mRestoreLoops; //!< Active restore loops
    std::map<uint32, std::set<uint64>> mNestedLoopStartAddresses; //!< Map from loop ID to a set of previously-generated nested loop start addresses
    std::map<uint32, std::set<uint64>> mNestedLoopEndAddresses; //!< Map from loop ID to a set of previously-generated nested loop end addresses
    uint32 mBranchRegIndex; //!< Index of the branch register
  };

  /*!
    \class RestoreLoopManagerRepository
    \brief Class to track RestoreLoopManagers for different threads.
  */
  class RestoreLoopManagerRepository {
  public:
    RestoreLoopManagerRepository(); //!< Default constructor
    COPY_CONSTRUCTOR_ABSENT(RestoreLoopManagerRepository);
    ~RestoreLoopManagerRepository(); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(RestoreLoopManagerRepository);

    static void Initialize(); //!< Create RestoreLoopManagerRepository instance.
    static void Destroy(); //!< Destroy RestoreLoopManagerRepository instance.
    inline static RestoreLoopManagerRepository* Instance() { return mspRestoreLoopManagerRepository; } //!< Access RestoreLoopManagerRepository instance.
    void AddRestoreLoopManager(cuint32 threadId, RestoreLoopManager* pRestoreLoopManager);
    RestoreLoopManager* GetRestoreLoopManager(cuint32 threadId) const;
  private:
    static RestoreLoopManagerRepository* mspRestoreLoopManagerRepository; //!< Static pointer to RestoreLoopManagerRepository
    std::map<uint32, RestoreLoopManager*> mRestoreLoopManagers; //!< RestoreLoopManagers mapped by thread ID
  };

}

#endif  // Force_RestoreLoop_H
