#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from base.Sequence import Sequence
import riscv.PcConfig as PcConfig

class ThreadSplitterSequence(Sequence):

    def __init__(self, genThread, numberCores, numberThreads):
        super().__init__(genThread)

        self.numberCores = numberCores
        self.numberThreads = numberThreads

    def generate(self, **kwargs):
        with ThreadSplitterContextManager(self):
            # TODO add MP support
            pc = PcConfig.get_base_boot_pc()
            (skip_boot, skip_boot_valid) = self.getOption("SkipBootCode") #TODO allow for granular control of skip boot code/skip thread splitter code
            if skip_boot_valid and skip_boot == 1:
                pc = PcConfig.get_base_initial_pc()

            self.genInstruction('JAL##RISCV', {'rd':0, 'NoBnt':1, 'BRTarget':pc})  # Brach to calculated address

    # Compute the thread ID according to the following expression: chip_id * num_cores * num_threads + core_id * num_threads + core_thread_id
    def extractThreadId(self):
        return 0 # TODO

class ThreadSplitterContextManager:
    def __init__(self, sequence):
        self.sequence = sequence
        self.origPc = None

    def __enter__(self):
        self.sequence.genThread.modifyGenMode('NoEscape,SimOff')
        self.origPc = self.sequence.getPEstate('PC')
        self.sequence.setPEstate('PC', PcConfig.get_reset_pc())
        return self

    def __exit__(self, *unused):
        self.sequence.setPEstate('PC', self.origPc)
        self.sequence.genThread.revertGenMode('NoEscape,SimOff')
        return False
