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
from base.ChoicesModifier import ChoicesModifier
from base.TestException import *
from riscv.AssemblyHelperRISCV import AssemblyHelperRISCV
from riscv.Utils import LoadGPR64
from base.InstructionMap import InstructionMap
from DV.riscv.trees.instruction_tree import *
from DV.riscv.trees.csr_trees import *

import RandomUtils

## --------------------------------------------------------------------------------------------
## generate random instructions from RV32I instr tree...
## --------------------------------------------------------------------------------------------

my_RV32I_instructions = {
    'ADD##RISCV':10,
    'ADDI##RISCV':10,
    'AND##RISCV':10,
    'ANDI##RISCV':10,
    'OR##RISCV':10,
    'ORI##RISCV':10,
    'XOR##RISCV':10,
    'XORI##RISCV':10
    }

class MainSequence(Sequence):

    def generate(self, **kargs):
        # generate sequences of random instructions...
        self.genRandomInstrs(my_RV32I_instructions, 1, 5)
        
    ## generate some random instructions...

    def genRandomInstrs(self, instr_tree, min_cnt, max_cnt):
        for _ in range( RandomUtils.random32(min_cnt, max_cnt) ):
            the_instruction = self.pickWeighted(instr_tree)
            self.genInstruction(the_instruction)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
