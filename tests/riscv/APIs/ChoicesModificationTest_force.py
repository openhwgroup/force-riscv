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
from base.ChoicesModifier import *
from DV.riscv.trees.instruction_tree import ALU_Int_All_instructions

## My Operand Choices Modifier class
#  the sub ChoicesModifier class for an example

class MyOperandChoicesModifier(ChoicesModifier):
    def __init__(self, gen_thread):
        super(MyOperandChoicesModifier, self).__init__(gen_thread, "MyOperandChoicesModifier")

    def update(self, **kwargs):
        for arg in kwargs:
            modifies = kwargs[arg]
            for key in modifies :
                self.modifyOperandChoices(key, modifies[key])

class MainSequence(Sequence):

    def generate(self, **kargs):

        my_choices_mod = MyOperandChoicesModifier(self.genThread)
        dict1 = {'GPRs' : {'x7' : 200, 'x15' : 200} }
        apply_id1 =  my_choices_mod.apply(myarg1 = dict1)

        self.notice("Applied choices modifications")
        self.dumpPythonObject(my_choices_mod.getChoicesTreeInfo('GPRs', 'OperandChoices'))

        for _ in range(50):
            instr = self.pickWeighted(ALU_Int_All_instructions)
            self.genInstruction('ADD##RISCV')

        my_choices_mod.revert(apply_id1)

        self.notice("Reverted choices modifications")
        self.dumpPythonObject(my_choices_mod.getChoicesTreeInfo('GPRs', 'OperandChoices'))

        for _ in range(50):
            instr = self.pickWeighted(ALU_Int_All_instructions)
            self.genInstruction('ADD##RISCV')




## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

