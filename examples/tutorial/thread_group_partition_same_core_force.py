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
from DV.riscv.trees.instruction_tree import ALU_Int64_map
from DV.riscv.trees.instruction_tree import LDST_Int_map
from DV.riscv.trees.instruction_tree import RV_G_map
import RandomUtils

# This class partitions the threads such that each group consists of the threads belonging to a
# given core.
class ThreadPartitionGlobalInitSeq(GlobalInitSeqRISCV):

    def setupThreadGroup(self):
        self.partitionThreadGroup('SameCore')


# This test assigns different tasks to different groups of threads.
class MainSequence(Sequence):

    def generate(self, **kargs):
        if self.getThreadGroupId() == 0:
            # Thread Group 0 generates random integer data processing instructions
            for _ in range(RandomUtils.random32(50, 100)):
                instr = ALU_Int64_map.pick(self.genThread)
                self.genInstruction(instr)
        elif self.getThreadGroupId() == 1:
            # Thread Group 1 generates random integer load/store instructions
            for _ in range(RandomUtils.random32(50, 100)):
                instr = LDST_Int_map.pick(self.genThread)

                # The NoPreamble flag avoids generating instructions to load a value into the base
                # register prior to generating the load or store instruction
                self.genInstruction(instr, {'NoPreamble': 1})
        elif self.getThreadGroupId() == 2:
            # Thread Group 2 randomly selects from a large collection of instructions
            for _ in range(RandomUtils.random32(50, 100)):
                instr = RV_G_map.pick(self.genThread)
                self.genInstruction(instr)
        else:
            # The remaining thread groups generate load/store instructions targeting a shared memory
            # location. A thread locking context permits only one thread at a time to execute until
            # the executing thread exits the context. This ensures only one thread generates the
            # shared physical address and the remaining threads use it.
            shared_phys_addr = 0
            shared_phys_addr_name = 'Shared PA'
            with self.threadLockingContext():
                if not self.hasSharedThreadObject(shared_phys_addr_name):
                    shared_phys_addr = self.genPA(Size=8, Align=8, Type='D', Shared=1)
                    self.setSharedThreadObject(shared_phys_addr_name, shared_phys_addr)
                else:
                    shared_phys_addr = self.getSharedThreadObject(shared_phys_addr_name)

            target_addr = self.genVAforPA(Size=8, Align=8, Type='D', PA=shared_phys_addr)
            for _ in range(RandomUtils.random32(10, 20)):
                instr = LDST_Int_map.pick(self.genThread)
                self.genInstruction(instr, {'LSTarget': target_addr})


GlobalInitialSequenceClass = ThreadPartitionGlobalInitSeq
MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
