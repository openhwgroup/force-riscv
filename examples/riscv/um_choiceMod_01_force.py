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
from DV.riscv.trees.instruction_tree import RV32_G_instructions
from DV.riscv.trees.instruction_tree import RV_G_instructions
from base.ChoicesModifier import ChoicesModifier
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


class MyMainSequence(Sequence):

    # Change the "choices" settings at the start of the test by using
    # the gen_thread_initialization entry point.

    def generate(self, **kwargs):

        usage_count = {}  # used to tally of reg usage

        for _ in range(1000):

            # the selection of GPRs to use for generateion will be
            # affected by the fact that the weightings were changed
            # in the gen_thread_initialization function.
            if self.getGlobalState("AppRegisterWidth") == 32:
                instr = self.pickWeighted(RV32_G_instructions)
            else:
                instr = self.pickWeighted(RV_G_instructions)

            instr_rec_id = self.genInstruction(instr)

            # get the indexes for the GPRs that were used.
            instr_obj = self.queryInstructionRecord(instr_rec_id)

            if ("Dests" in instr_obj) and ("rd" in instr_obj["Dests"]):
                rd_index = instr_obj["Dests"]["rd"]
                rd_name = "x{}".format(rd_index)
                if rd_name in usage_count:
                    usage_count[rd_name] += 1
                else:
                    usage_count[rd_name] = 1  # first time only

            if "Srcs" in instr_obj.keys():
                if "rs1" in instr_obj["Srcs"]:
                    rs1_index = instr_obj["Srcs"]["rs1"]
                    rs1_name = "x{}".format(rs1_index)
                    if rs1_name in usage_count:
                        usage_count[rs1_name] += 1
                    else:
                        usage_count[rs1_name] = 1
                if "rs2" in instr_obj["Srcs"]:
                    rs2_index = instr_obj["Srcs"]["rs2"]
                    rs2_name = "x{}".format(rs2_index)
                    if rs2_name in usage_count:
                        usage_count[rs2_name] += 1
                    else:
                        usage_count[rs2_name] = 1

        # print the usage count for each GPR
        # check results in gen.log
        # frequency of x10-x13 should be higher based on the choices
        # modification if enough instructions are generated
        self.notice(">>>>>>>>>>  Usage count for all GPRs  <<<<<<<<<<<<")
        for reg_name in sorted(usage_count):
            self.notice(
                ">>>>>>>>>>  Usage count for {}:  {}".format(
                    reg_name, usage_count[reg_name]
                )
            )


# Modify GPR weightings.  A higher weighting means force-riscv is more likely
# to choose that value during a weighted random selection when generating
# a new instruction.
# This will be run before the MainSequence.generate()
def gen_thread_initialization(gen_thread):
    choices_mod = ChoicesModifier(gen_thread)

    # Increase the likelihood of using GPRs  x10, x11, x12 and x13 by
    # increasing the weighting.  The default weighting in the
    # operand_choices.xml file is 10 for each GPR.
    choices_mod.modifyOperandChoices(
        "GPRs", {"x10": 40, "x11": 40, "x12": 60, "x13": 60}
    )

    choices_mod.commitSet()


# Enable the GenThreadInitialization entry point
GenThreadInitialization = gen_thread_initialization

#  Points to the MainSequence defined in this file
MainSequenceClass = MyMainSequence

#  Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

#  Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
