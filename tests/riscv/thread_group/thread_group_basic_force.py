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
from riscv.GenThreadRISCV import GenThreadRISCV


#  This test verifies the default thread partiion strategy. We expect all
#  threads to be assigned to Group 0.
class MainSequence(Sequence):
    def generate(self, **kargs):
        thread_groups = self.queryThreadGroup()
        if len(thread_groups) != 1:
            self.error(
                "Unexpected thread group count; Expected=%d, Actual=%d" % (1, len(thread_groups))
            )

        total_thread_count = self.getThreadNumber()
        thread_constr = ConstraintSet(thread_groups[0][2])
        if thread_constr.size() != total_thread_count:
            self.error(
                "Unexpected group thread count; Expected=%d, Actual=%d"
                % (total_thread_count, thread_constr.size())
            )

        thread_groups = self.queryThreadGroup(1)
        if len(thread_groups) != 0:
            self.error("Unexpectedly retrieved nonexistent thread group %d" % 1)

        group_id = self.getThreadGroupId()
        if group_id != 0:
            self.error("Unexpected thread group ID; Expected=%d, Actual=%d" % (0, group_id))

        # The thread count should not be a valid thread ID; e.g. if there are
        # 16 threads, the largest valid thread ID is 15
        group_id = self.getThreadGroupId(total_thread_count)
        if group_id is not None:
            self.error(
                "Unexpected thread group ID for nonexistent thread; "
                "Expected=None, Actual=%d" % group_id
            )

        free_threads = self.getFreeThreads()
        if len(free_threads) != 0:
            self.error(
                "Unexpected free thread count; Expected=%d, Actual=%d" % (0, len(free_threads))
            )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
