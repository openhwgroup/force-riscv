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
#ifndef Force_ThreadGroupPartitioner_H
#define Force_ThreadGroupPartitioner_H

#include <vector>

#include "Defines.h"
#include "Enums.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class ConstraintSet;
  class ThreadGroup;

 /*!
   \ class PartitionArgument
   \ brief the structure for parition argument
  */
 class PartitionArgument {
 public:
   PartitionArgument(): mGroupNum(1), mGroupSize(0) { }
   explicit PartitionArgument(uint32 groupNum, uint32 groupSize = 0) : mGroupNum(groupNum), mGroupSize(groupSize) { }

   ASSIGNMENT_OPERATOR_DEFAULT(PartitionArgument);
   COPY_CONSTRUCTOR_DEFAULT(PartitionArgument);

   uint32 mGroupNum; //!< group number
   uint32 mGroupSize; //!< group size
 };

 /*!
   \ class ThreadGroupPartitioner
   \ brief the base class to partition thread group
  */
 class ThreadGroupPartitioner {
 public:
   DEFAULT_CONSTRUCTOR_DEFAULT(ThreadGroupPartitioner); //!< Constructor
   COPY_CONSTRUCTOR_ABSENT(ThreadGroupPartitioner);
   SUPERCLASS_DESTRUCTOR_DEFAULT(ThreadGroupPartitioner); //!< Destructor
   ASSIGNMENT_OPERATOR_ABSENT(ThreadGroupPartitioner);

   virtual void Setup(uint32 numChip, uint32 numCore, uint32 numThread) = 0; //!< Set up thread group partitioner
   virtual void DoPartition(std::vector<ThreadGroup*>& threadGroups, const PartitionArgument* pPartitionArg = nullptr ) = 0; // Do Partition
   virtual const std::string ToString() const = 0; //!< Return a string describing partitioner context

 };

 /*!
  \ class DomainThreadPartitioner
  \ brief partition the threads on the domain into the same group
 */
 class DomainThreadPartitioner : public ThreadGroupPartitioner {
 public:
   explicit DomainThreadPartitioner(EPartitionThreadPolicy policy); //!< Constructor
   COPY_CONSTRUCTOR_ABSENT(DomainThreadPartitioner);
   ~DomainThreadPartitioner() override; //!< Destructor
   ASSIGNMENT_OPERATOR_ABSENT(DomainThreadPartitioner);

   void Setup(uint32 numChip, uint32 numCore, uint32 numThread) override; //!< set up thread group partitioner
   void DoPartition(std::vector<ThreadGroup*>& threadGroups, const PartitionArgument* pPartitionArg = nullptr) override; //!< do partition
   const std::string ToString() const override; //!< Return a string describing partitioner context
 protected:
   EPartitionThreadPolicy mPolicy; //!< partition policy
   std::vector<ConstraintSet* > mThreads; //!< threads on the domain.
 };

 /*!
  \ class DiffDomainThreadPartitioner
  \ brief partition the threads on the different domain into the same group
 */
 class DiffDomainThreadPartitioner : public DomainThreadPartitioner {
 public:
   explicit DiffDomainThreadPartitioner(EPartitionThreadPolicy policy);  //!< Constructor
   COPY_CONSTRUCTOR_ABSENT(DiffDomainThreadPartitioner);
   ~DiffDomainThreadPartitioner() override; //!< Destructor
   ASSIGNMENT_OPERATOR_ABSENT(DiffDomainThreadPartitioner);

   void Setup(uint32 numChip, uint32 numCore, uint32 numThread) override; //!< set up thread group partitioner
   void DoPartition(std::vector<ThreadGroup*>& threadGroups, const PartitionArgument* pPartitionArg = nullptr) override; //!< do partition
 protected:
   std::vector<ConstraintSet* > mDiffThreads; //!< threads on the diff domain.
 };

 /*!
  \ class RandomThreadPartitioner
  \ brief partition the threads randomly into the same group
 */
 class RandomThreadPartitioner : public ThreadGroupPartitioner {
 public:
   RandomThreadPartitioner();
   COPY_CONSTRUCTOR_ABSENT(RandomThreadPartitioner);
   ~RandomThreadPartitioner() override; //!< Destructor
   ASSIGNMENT_OPERATOR_ABSENT(RandomThreadPartitioner);

   void Setup(uint32 numChip, uint32 numCore, uint32 numThread) override; //!< set up thread group partitioner
   void DoPartition(std::vector<ThreadGroup*>& threadGroups, const PartitionArgument* pPartitionArg) override; //!< do partition
   const std::string ToString() const override; //!< Return a string describing partitioner context
 protected:
   ConstraintSet* mpFreeThreads; //!< free threads to partition
   static uint32 mGroupId; //!< group id to allocate
 };

 /*!
  \ class ThreadPartitionerFactory
  \ brief the factory to create thread partitioners
 */
  class ThreadPartitionerFactory {
  public:
    static void Initialize();  //!< Initialization interface.
    static void Destroy();     //!< Destruction clean up interface.
    inline static ThreadPartitionerFactory* Instance() { return mspPartitionFactory; }
    ThreadGroupPartitioner* CreatePartitioner(EPartitionThreadPolicy policy); //!< create thread partitioner
  private:
    ThreadPartitionerFactory( ) { } //!< constructor, private
    ~ThreadPartitionerFactory( ) { } //!< destructor, private
    static ThreadPartitionerFactory* mspPartitionFactory; //!< Pointer to singleton Factory object
  };


}
#endif
