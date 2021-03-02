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
from DV.riscv.trees.instruction_tree import ALU_Int32_instructions
from DV.riscv.trees.instruction_tree import ALU_Float_Double_instructions


class MyMainSequence(Sequence):
    def generate(self, **kwargs):

        for _ in range(10):

            instr_rec_id1 = self.genInstruction("ADD##RISCV")

            # The object returned is a dictionary with the values
            # that correspond to some keys themselves being
            # dictionaries
            instr_obj = self.queryInstructionRecord(instr_rec_id1)

            # The key "Dests" indicates the write target of the
            # instruction.
            target_regs = instr_obj["Dests"]  # get the dict of target regs
            target_reg_id = target_regs["rd"]  # get the id for the "rd" reg
            self.notice(
                ">>>>>>>>  Key:  {}     Value:  {}".format("rd", target_reg_id)
            )

            if target_reg_id == 0:
                continue  # targed reg id of 0 does
            # not write a reg, so skip

            # generate new instruction with rs1 = target_reg of previous
            # instruction
            instr_rec_id2 = self.genInstruction(
                "SUB##RISCV", {"rs1": target_reg_id}
            )

            # extract the rs1 reg id from the generated instruction
            instr_obj2 = self.queryInstructionRecord(instr_rec_id2)
            source_regs = instr_obj2["Srcs"]
            source_reg_id = source_regs["rs1"]

            self._check_dependency(source_reg_id, target_reg_id)

    def _check_dependency(self, source_reg_id, target_reg_id):
        # make sure the read-after-write dependency was generated
        if source_reg_id == target_reg_id:
            self.notice(
                ">>>>>>>>>>  It worked!  Target reg of the first \
                    instruction is feeding the source reg of the second \
                    instruction."
            )
        else:
            self.notice(
                ">>>>>>>>>>  FAIL!  Source reg id: {}    \
                    Target reg id: {}".format(
                    source_reg_id, target_reg_id
                )
            )


#  Points to the MainSequence defined in this file
MainSequenceClass = MyMainSequence

#  Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

#  Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
