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

    # Main sequence which calls the other sequences.
    def generate(self, **kwargs):
        
        i100_seq = I100Sequence(self.genThread)
        i100_seq.run()

        f100_seq = F100Sequence(self.genThread)
        f100_seq.run()




class I100Sequence(Sequence):

    # generate 100 random integer 32 ALU ops
    def generate(self, **kargs):

        for _ in range(100):
            the_instruction = self.pickWeighted(ALU_Int32_instructions)
            self.genInstruction(the_instruction)
            self.notice(">>>>>  The instruction:  {}".format(the_instruction))


class F100Sequence(Sequence):

    # Generate 100 random floating point alu ops
    def generate(self, **kargs):

        for _ in range(100):
            the_instruction = self.pickWeighted(ALU_Float_Double_instructions)
            self.genInstruction(the_instruction)
            self.notice(">>>>>  The instruction:  {}".format(the_instruction))






## Points to the MainSequence defined in this file
MainSequenceClass = MyMainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

