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


# This test verifies that generating compressed load and store
# instructions without preamble with uninitialized registers doesn't fail.
class MainSequence(Sequence):
    def generate(self, **kargs):

        instructions = (
            "C.FLD##RISCV",
            "C.FSD##RISCV",
            "C.LD##RISCV",
            "C.LW##RISCV",
            "C.SD##RISCV",
            "C.SW##RISCV",
        )

        if self.getGlobalState("AppRegisterWidth") == 32:
            instructions = (
                "C.FLD##RISCV",
                "C.FSD##RISCV",
                "C.LW##RISCV",
                "C.SW##RISCV",
            )

        for _ in range(50):
            instr_id = self.genInstruction(
                self.choice(instructions), {"NoPreamble": 1}
            )

            if instr_id:
                instr_record = self.queryInstructionRecord(instr_id)

                if (instr_record["Addressing"]["Base"][0] < 8) or (
                    instr_record["Addressing"]["Base"][0] > 15
                ):
                    self.error(
                        "Expected a base register index between 8 and 15; "
                        "Actual=%d" % instr_record["Addressing"]["Base"][0]
                    )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
