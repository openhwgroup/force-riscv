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
from base.Env import Env, GlobalInitSequence
from riscv.ThreadSplitterSequence import ThreadSplitterSequence
import riscv.PcConfig as PcConfig
from Constraint import ConstraintSet

#  GlobalInitSeqRISCV class
# Return arch specific information to base GlobalInitSequence class


class GlobalInitSeqRISCV(GlobalInitSequence):
    def __init__(self, gen_thread, name):
        super().__init__(gen_thread, name)

    def setupResetRegion(self):
        self.setGlobalState("ResetPC", PcConfig.get_reset_pc())
        self.virtualMemoryRequest(
            "PhysicalRegion",
            {
                "RegionType": "ResetRegion",
                "Size": PcConfig.get_reset_region_size(),
                "Type": "I",
                "Bank": 0,
            },
        )

        (skip_boot, skip_boot_valid) = self.genThread.getOption("SkipBootCode")
        if skip_boot_valid and skip_boot == 1:
            self.setGlobalState("ResetPC", PcConfig.get_base_initial_pc())

    def allocateHandlerSetMemory(self):
        # Need to ensure the handler memory doesn't intersect the boot region
        handler_memory_constr = ConstraintSet(0, 0xFFFFFFFFFFFFFFFF)
        handler_memory_constr.subRange(
            PcConfig.get_base_boot_pc(),
            (
                PcConfig.get_base_boot_pc()
                + PcConfig.get_boot_region_size()
                - 1
            ),
        )

        handler_memory_size = self.getHandlerMemorySize()
        self.virtualMemoryRequest(
            "PhysicalRegion",
            {
                "RegionType": "HandlerMemory",
                "Size": handler_memory_size,
                "Align": 0x10000,
                "Type": "I",
                "Bank": 0,
                "Range": str(handler_memory_constr),
            },
        )

    def setupMemoryFillPattern(self):
        fill_pattern, valid = self.genThread.getOption("MemoryFillPattern")
        if valid:
            self.setGlobalState("MemoryFillPattern", fill_pattern)

    def getHandlerMemorySize(self):
        return 0x2000 * 3

    def setupThreadGroup(self):
        self.partitionThreadGroup("Random", group_num=1)


#  EnvRISCV class
#  RISCV environment class


class EnvRISCV(Env):
    def __init__(self, interface):
        super().__init__(interface)
        self.defaultInitSeqClass = GlobalInitSeqRISCV

    def addThreadSplitterSequence(self):
        self.afterSequences.append(
            ThreadSplitterSequence(None, self.numberCores, self.numberThreads)
        )
