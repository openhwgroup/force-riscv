#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0
#  (the "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
# OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from Constraint import ConstraintSet

from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.EnvRISCV import GlobalInitSeqRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


#  This class partitions the threads using the random strategy.
class ThreadPartitionGlobalInitSeq(GlobalInitSeqRISCV):
    def setupThreadGroup(self):
        # Allocate one thread into one group randomly; the other threads remain
        # free
        self.partitionThreadGroup("Random", group_num=1, group_size=1)


#  This test verifies the random partition strategy. This test assumes at least
#  8 total threads are used.
class MainSequence(Sequence):
    def generate(self, **kargs):
        group_id = self.getThreadGroupId()
        if group_id == 0:
            thread_groups = self.queryThreadGroup(0)
            if len(thread_groups) != 1:
                self.error(
                    "Unexpected thread group count; Expected=%d, Actual=%d"
                    % (1, len(thread_groups))
                )

            thread_constr = ConstraintSet(thread_groups[0][2])
            if thread_constr.size() != 1:
                self.error(
                    "Unexpected group thread count; Expected=%d, Actual=%d"
                    % (1, thread_constr.size())
                )

            expected_free_thread_count = self.getThreadNumber() - 1
            free_threads = self.getFreeThreads()
            if len(free_threads) != expected_free_thread_count:
                self.error(
                    "Unexpected free thread count; Expected=%d, Actual=%d"
                    % (expected_free_thread_count, len(free_threads))
                )

            # Allocate two threads into one group randomly
            self.partitionThreadGroup("Random", group_num=1, group_size=2)
            expected_free_thread_count -= 2

            thread_groups = self.queryThreadGroup()
            if len(thread_groups) != 2:
                self.error(
                    "Unexpected thread group count; Expected=%d, Actual=%d"
                    % (2, len(thread_groups))
                )

            thread_constr_0 = ConstraintSet(thread_groups[0][2])
            if thread_constr_0.size() != 1:
                self.error(
                    "Unexpected group thread count; Expected=%d, Actual=%d"
                    % (1, thread_constr_0.size())
                )

            thread_constr_1 = ConstraintSet(thread_groups[1][2])
            if thread_constr_1.size() != 2:
                self.error(
                    "Unexpected group thread count; Expected=%d, Actual=%d"
                    % (2, thread_constr_1.size())
                )

            free_threads = self.getFreeThreads()
            if len(free_threads) != expected_free_thread_count:
                self.error(
                    "Unexpected free thread count; Expected=%d, Actual=%d"
                    % (expected_free_thread_count, len(free_threads))
                )

            free_thread_sample = self.sampleFreeThreads(5)
            group_id = thread_groups[1][0]
            self.setThreadGroup(group_id, "Endless Loop", free_thread_sample)
            expected_free_thread_count = expected_free_thread_count + 2 - 5

            thread_groups = self.queryThreadGroup(group_id)
            if len(thread_groups) != 1:
                self.error(
                    "Unexpected thread group count; Expected=%d, Actual=%d"
                    % (1, len(thread_groups))
                )

            thread_constr = ConstraintSet(thread_groups[0][2])
            if thread_constr.size() != 5:
                self.error(
                    "Unexpected group thread count; Expected=%d, Actual=%d"
                    % (5, thread_constr.size())
                )

            free_threads = self.getFreeThreads()
            if len(free_threads) != expected_free_thread_count:
                self.error(
                    "Unexpected free thread count; Expected=%d, Actual=%d"
                    % (expected_free_thread_count, len(free_threads))
                )

    # Return a comma-separated string of a random sample of free thread IDs.
    #
    #  @param aSampleSize The number of thread IDs to return.
    def sampleFreeThreads(self, aSampleSize):
        free_threads = self.getFreeThreads()
        free_thread_sample = self.sample(free_threads, aSampleSize)
        return ",".join([str(thread_id) for thread_id in free_thread_sample])


GlobalInitialSequenceClass = ThreadPartitionGlobalInitSeq
MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
