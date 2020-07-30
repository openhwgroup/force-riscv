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
from base.InstructionMap import InstructionMap
from DV.riscv.trees.instruction_tree import *

import RandomUtils

class MainSequence(Sequence):

    def generate(self, **kargs):
        for i in range(2):
            instruction_group = RV_A_instructions
            
            for _ in range( RandomUtils.random32(1,10) ):
                the_instruction = self.pickWeighted(instruction_group)
                self.genInstruction(the_instruction)
            
            if RandomUtils.random32(0,1) == 1:
                self.genInstruction('ECALL##RISCV')
            else:
                self.genInstruction('EBREAK##RISCV')


def gen_thread_initialization(gen_thread):
    (delegate_opt, valid) = gen_thread.getOption("DelegateExceptions")
    if valid and delegate_opt == 1: 
        delegation_enables = ChoicesModifier(gen_thread)
        weightDict = { "0x0":0, "0x1":50 }
        delegation_enables.modifyRegisterFieldValueChoices( 'medeleg.Breakpoint', weightDict )
        delegation_enables.modifyRegisterFieldValueChoices( 'medeleg.Environment call from U-mode', weightDict )
        delegation_enables.commitSet()

    (paging_opt, valid) = gen_thread.getOption("PagingDisabled")
    if valid and paging_opt == 1: 
        gen_thread.initializeRegister(name='satp', value=0, field='MODE')
    
GenThreadInitialization = gen_thread_initialization


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
