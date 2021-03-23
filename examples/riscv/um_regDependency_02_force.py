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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


class MyMainSequence(Sequence):
    def generate(self, **kwargs):

        for _ in range(100):

            # request the id of an available GPR - avoid "x0"
            # setting reg_id variable scope outside of the for loop
            reg_id = 0
            while reg_id == 0:
                reg_id = self.getRandomGPR()

            # generate an instruction that writes that GPR
            instr_rec_id1 = self.genInstruction("ADD##RISCV", {"rd": reg_id})
            instr_rec_id2 = self.genInstruction("SUB##RISCV", {"rs2": reg_id})

            # Check the results: index of rd for id1 must match index of rs2
            # for id2
            instr_obj1 = self.queryInstructionRecord(instr_rec_id1)
            instr_obj2 = self.queryInstructionRecord(instr_rec_id2)

            instr1_rd_index = instr_obj1["Dests"]["rd"]
            instr2_rs2_index = instr_obj2["Srcs"]["rs2"]

            if instr1_rd_index == instr2_rs2_index:
                self.notice(">>>>>>>>>>  It worked.  ")
                self.notice(
                    ">>>>>>>>>>  Instr1 rd: {}   Instr2 rs2: {}".format(
                        instr1_rd_index, instr2_rs2_index
                    )
                )
            else:
                self.notice(">>>>>>>>>>>  FAIL - reg indexes did not match.")
                self.error(
                    ">>>>>>>>>>  Instr1 rd: {}   Instr2 rs2: {}".format(
                        instr1_rd_index, instr2_rs2_index
                    )
                )

            # The gen.log file can be examined to see what instructions
            # were generated and the self.notice log messages


MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
