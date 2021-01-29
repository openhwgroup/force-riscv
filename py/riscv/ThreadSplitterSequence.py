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
from base.Sequence import Sequence
from riscv.AssemblyHelperRISCV import AssemblyHelperRISCV
from riscv.Utils import LoadGPR64
import riscv.PcConfig as PcConfig


class ThreadSplitterSequence(Sequence):
    def __init__(self, genThread, numberCores, numberThreads):
        super().__init__(genThread)

        self.numberCores = numberCores
        self.numberThreads = numberThreads

    def generate(self, **kwargs):
        with ThreadSplitterContextManager(self):
            pc = PcConfig.get_base_boot_pc()
            (skip_boot, skip_boot_valid) = self.getOption("SkipBootCode")
            # TODO allow for granular control of skip boot code/skip thread
            # splitter code
            if skip_boot_valid and skip_boot == 1:
                pc = PcConfig.get_base_initial_pc()

            (
                boot_pc_reg_index,
                thread_id_reg_index,
                pc_offset_reg_index,
            ) = self.getRandomGPRs(3, exclude="0")
            assembly_helper = AssemblyHelperRISCV(self)
            assembly_helper.genReadSystemRegister(
                thread_id_reg_index, "mhartid"
            )  # Get the thread ID

            load_gpr64_seq = LoadGPR64(self.genThread)
            load_gpr64_seq.load(
                pc_offset_reg_index, PcConfig.get_boot_pc_offset()
            )
            self.genInstruction(
                "MUL##RISCV",
                {
                    "rd": pc_offset_reg_index,
                    "rs1": thread_id_reg_index,
                    "rs2": pc_offset_reg_index,
                },
            )  # Multiply the base PC offset by the thread ID

            load_gpr64_seq.load(boot_pc_reg_index, PcConfig.get_base_boot_pc())
            assembly_helper.genAddRegister(
                boot_pc_reg_index, pc_offset_reg_index
            )  # Add the thread PC offset to the base initial PC

            self.genInstruction(
                "JALR##RISCV",
                {
                    "rd": 0,
                    "rs1": boot_pc_reg_index,
                    "simm12": 0,
                    "NoBnt": 1,
                    "NoRestriction": 1,
                },
            )  # Branch to calculated address


class ThreadSplitterContextManager:
    def __init__(self, sequence):
        self.sequence = sequence
        self.origPc = None

    def __enter__(self):
        self.sequence.genThread.modifyGenMode("NoEscape,SimOff")
        self.origPc = self.sequence.getPEstate("PC")
        self.sequence.setPEstate("PC", PcConfig.get_reset_pc())
        return self

    def __exit__(self, *unused):
        self.sequence.setPEstate("PC", self.origPc)
        self.sequence.genThread.revertGenMode("NoEscape,SimOff")
        return False
