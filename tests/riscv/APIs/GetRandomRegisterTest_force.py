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
from base.TestUtils import assert_equal
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


class MainSequence(Sequence):
    def generate(self, **kargs):

        free_regs_int = self.getRandomRegisters(10, "GPR", "31")
        free_regs_float = self.getRandomRegisters(10, "FPR", "31")

        for reg in free_regs_int:
            self.notice(">>>>>>>>>>>   Int reg id:  {}".format(reg))

        for reg in free_regs_float:
            self.notice(">>>>>>>>>>>   Float reg id:  {}".format(reg))

        for rs1 in free_regs_int:
            record_id = self.genInstruction("ADD##RISCV", {"rs1": rs1})
            instr_obj = self.queryInstructionRecord(record_id)
            source_regs = instr_obj["Srcs"]

            if "rs1" in source_regs:
                self.notice(
                    ">>>>>>>>>>>>  Generated instruction and rs1:  "
                    "{}      {}".format(instr_obj["Name"], source_regs["rs1"])
                )
            else:
                self.notice(">>>>>>>>>>>>   RS1 Not Found.   ")

            assert_equal(
                source_regs["rs1"],
                rs1,
                "The generated instruction does not have the requested " "index value for rs1.",
            )

        for rs1 in free_regs_float:
            record_id = self.genInstruction("FADD.D##RISCV", {"rs1": rs1})
            instr_obj = self.queryInstructionRecord(record_id)
            source_regs = instr_obj["Srcs"]

            if "rs1" in source_regs:
                self.notice(
                    ">>>>>>>>>>>>  Generated instruction and rs1:  "
                    "{}      {}".format(instr_obj["Name"], source_regs["rs1"])
                )
            else:
                self.notice(">>>>>>>>>>>>   RS1 Not Found.   ")

            assert_equal(
                source_regs["rs1"],
                rs1,
                "The generated instruction does not have the requested " "index value for rs1.",
            )


# Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

# Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

# Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
