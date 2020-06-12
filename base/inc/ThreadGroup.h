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
#ifndef Force_ThreadGroup_H
#define Force_ThreadGroup_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

#include <string>
#include <map>
#include <vector>
#include <list>

namespace Force {

  class ConstraintSet;
  class PartitionArgument;
  class ThreadGroupPartitioner;

   /*!
    \class ThreadGroup
    \brief the class for thread group.
   */
  class ThreadGroup {
  public:
    explicit ThreadGroup(uint32 id); //!< constructor
    ThreadGroup(uint32 id, const std::string& job, const ConstraintSet& threads); //!< constructor
    virtual ~ThreadGroup(); //!< Destructor

    DEFAULT_CONSTRUCTOR_ABSENT(ThreadGroup);
    COPY_CONSTRUCTOR_ABSENT(ThreadGroup);
    ASSIGNMENT_OPERATOR_ABSENT(ThreadGroup);

    void SetThreadGroup(const std::string& job, const ConstraintSet& threads); //!< set thread group, clear old ones if there have.
    inline const ConstraintSet* GetThreads() const { return mpThreads; } //!< get threads
    inline uint32 GetId() const { return mId;} //!< get group id
    inline const std::string& GetJob() const { return mJob; } //!< get job
    const std::string ToString() const; //!< thread group string
  private:
    uint32 mId; //!< group id
    std::string mJob;  //!< group job
    ConstraintSet* mpThreads; //!< thread ids belonging to the group
  };

  /*!
    \ class ThreadGroupModerator
    \ brief the moderator for thread groups
  */
  class ThreadGroupModerator {
  public:
    ThreadGroupModerator(uint32 numChip, uint32 numCore, uint32 numThread); //!< constructor
    virtual ~ThreadGroupModerator(); // Destructor

    DEFAULT_CONSTRUCTOR_ABSENT(ThreadGroupModerator);
    COPY_CONSTRUCTOR_ABSENT(ThreadGroupModerator);
    ASSIGNMENT_OPERATOR_ABSENT(ThreadGroupModerator);

    void PartitionThreadGroup(EPartitionThreadPolicy policy, const PartitionArgument* pPartitionArg = nullptr); //!< partition thread groups by policy
    void SetThreadGroup(uint32 id, const std::string& job, const ConstraintSet& threads); //!< set the specified group
    void QueryThreadGroup(uint32 groupId, std::vector<ThreadGroup* >& threadGroups) const; //!< Query thread group
    void QueryThreadGroup(const std::string& job, std::vector<ThreadGroup* >& threadGroups) const; //!< Query thread group by job.
    void GetFreeThreads(std::vector<uint32>& freeThreads) const; //!< get free threads
    uint32 GetThreadGroupId(uint32 threadId) const; //!< get group id the thread belongs to
  protected:
    void Setup(uint32 numChip, uint32 numCore, uint32 numThread); //!< do some set up
    void MapThreads(const ThreadGroup* group); //!< map threads on the specific group
    void UnmapThreads(const ThreadGroup* group); //!< do unmap threads on the specific group
  private:
    std::map<uint32, ThreadGroup*> mThreadGroups; //!< the container for thread groups
    std::map<uint32, uint32> mThreadGroupIDs; //!< the map from thread id to group id
    ConstraintSet* mpFreeThreads; //!< free threads

    std::vector<ThreadGroupPartitioner* > mThreadGroupPartitioners; //!< the container for all kinds of thread group partitioners
    uint32 mThreadsNum; //!< total threads number
  };

}
#endif
