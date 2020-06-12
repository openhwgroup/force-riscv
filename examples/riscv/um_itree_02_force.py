#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from base.InstructionMap import InstructionMap
from DV.riscv.trees.instruction_tree import *

class F100Sequence(Sequence):
    """
    This test template is a simple example which generates 100 instructions relying on 
    FORCE-RISCV's capabilities to randomly choose instructions, operands and operand
    values.  The set of instructions and the weighting (for frequency of selection) is
    defined in the ALU_Int32_instructions instruction tree dictionary - which is coded
    in py/DV/riscv/trees/instruction_tree.py.
    """

    def generate(self, **kargs):

        for _ in range(100):
            the_instruction = self.pickWeighted(ALU_Float_Double_instructions)
            self.genInstruction(the_instruction)




## Points to the MainSequence defined in this file
MainSequenceClass = F100Sequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

