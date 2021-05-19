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
import RandomUtils

from base.ChoicesModifier import ChoicesModifier
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from riscv.ModifierUtils import PageFaultModifier


# This test verifies that the PageFaultModifier does not unintentionally induce
# access faults on branch instructions.
class MainSequence(Sequence):
    def generate(self, **kwargs):
        page_fault_mod = PageFaultModifier(self.genThread, self.getGlobalState("AppRegisterWidth"))
        page_fault_mod.apply(**{"All": 1})

        # The PageFaultModifier alters the page size choices, so we need to
        # set the desired values after applying the page fault modifications
        choices_mod = ChoicesModifier(self.genThread)
        satp_info = self.getRegisterInfo("satp", self.getRegisterIndex("satp"))
        if satp_info["Width"] == 32:
            choices_mod.modifyPagingChoices("Page size#4K granule#S#stage 1", {"4K": 1, "4M": 10})
        else:
            choices_mod.modifyPagingChoices(
                "Page size#4K granule#S#stage 1",
                {"4K": 1, "2M": 10, "1G": 10, "512G": 10},
            )

        choices_mod.commitSet()

        instruction_list = ("JAL##RISCV", "JALR##RISCV")
        for _ in range(RandomUtils.random32(2, 5)):
            self.genInstruction(self.choice(instruction_list))

        page_fault_mod.revert()

        if self.queryExceptionRecordsCount(1) != 0:
            self.error("Unexpected instruction access fault.")


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
