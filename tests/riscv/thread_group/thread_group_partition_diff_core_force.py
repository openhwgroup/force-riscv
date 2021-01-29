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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from riscv.EnvRISCV import GlobalInitSeqRISCV
from Constraint import ConstraintSet

## This class partitions the threads using the different core strategy.
class ThreadPartitionGlobalInitSeq(GlobalInitSeqRISCV):

    def setupThreadGroup(self):
        self.partitionThreadGroup('DiffCore')

## This test verifies the different core partition strategy. This test assumes exactly 2 threads per
# core are used. We expect no two threads in a given thread group to belong to the same core.
class MainSequence(Sequence):

    def generate(self, **kargs):
        thread_groups = self.queryThreadGroup()
        if len(thread_groups) != 2:
            self.error('Unexpected thread group count; Expected=%d, Actual=%d' % (2, len(thread_groups)))

        total_thread_count = self.getThreadNumber()
        for thread_group in thread_groups:
            thread_constr = ConstraintSet(thread_group[2])

            if thread_constr.size() != (total_thread_count // 2):
                self.error('Unexpected group thread count; Expected=%d, Actual=%d' % ((total_thread_count // 2), thread_constr.size()))

        for thread_id in range(0, self.getThreadNumber(), 2):
            group_id_a = self.getThreadGroupId(thread_id)
            group_id_b = self.getThreadGroupId(thread_id + 1)

            if group_id_a == group_id_b:
                self.error('Expected Thread %d (Group ID %d) to have a different group ID from Thread %d (Group ID %d)' % (thread_id, group_id_a, (thread_id + 1), group_id_b))

        free_threads = self.getFreeThreads()
        if len(free_threads) != 0:
            self.error('Unexpected free thread count; Expected=%d, Actual=%d' % (0, len(free_threads)))


GlobalInitialSequenceClass = ThreadPartitionGlobalInitSeq
MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
