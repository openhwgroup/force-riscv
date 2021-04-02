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


# ******************************************************************
#   Generate Floating point instructions
#
# ******************************************************************


class MainSequence(Sequence):
    def generate(self, **kargs):

        instrCount = (
            10  # How many instructions should be generated in the test case.
        )

        # "ALL" indicates randomly select from the set of instructions in
        # the instructionSubset.
        # Or you can...
        # specify a single instruction type to be generated in the test case
        # Find the correct string specifying the instruction you want
        # in the <force checkout base dir>/Force/riscv/arch_data/
        # riscv_instruction.xml file.

        instrType = "ALL"

        # if you specify instrType="ALL", instructions will be generated from
        # this set of instructions
        instructionSubset = {
            "FMUL.D##RISCV": 20,
            "FMUL.S##RISCV": 20,
            "FDIV.D##RISCV": 20,
            "FDIV.S##RISCV": 20,
            "FMADD.D##RISCV": 20,
            "FMADD.S##RISCV": 20,
            "FADD.D##RISCV": 20,
            "FADD.S##RISCV": 20,
        }

        # gen each instruction
        for i in range(instrCount):

            # if ctrl file instrType option=ALL, randomly pick each instruction
            if instrType == "ALL":

                # Pick a random instruction from the instructionSubset
                theInstruction = self.pickWeighted(instructionSubset)
                self.genInstruction(theInstruction)

            else:

                theInstruction = instrType
                self.genInstruction(theInstruction)


# Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

# Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

# Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
