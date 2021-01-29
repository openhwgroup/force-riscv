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
# [SCENARIO]
# This template test register dependence scenario. 

from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from base.ChoicesModifier import *
from DV.riscv.trees.instruction_tree import *

#************************************************************************
# gen a couple of instructions, restricting GP reg choices to X0..X3...
#************************************************************************

class MyDependenceChoicesModifier(ChoicesModifier):
    def __init__(self, gen_thread):
        super(MyDependenceChoicesModifier, self).__init__(gen_thread, "MyDependenceChoicesModifier")

    def update(self, **kwargs):
        for arg in kwargs:
            modifies = kwargs[arg]
            for key in modifies:
                self.modifyDependenceChoices(key, modifies[key])
                
class MainSequence(Sequence):

    def generate(self, **kargs):

        my_choices_mod = MyDependenceChoicesModifier(self.genThread)
       
        # select inter-dependence 
        dep_dicts = {'Register Dependency' : {'No dependency' : 1, 'Inter-dependency' : 10, 'Intra-dependency' : 10}, 
                              'Target Dependency' : {'Target after source' : 10, 'Target after target': 10, 'No target dependence': 1}, 
                              'Source Dependency' : {'Source after source' : 10, 'Source after target': 10, 'No source dependence' : 1},
                              'Inter-Dependency Priority' : {'The neareast': 1 , 'The farthest': 1, 'Optimal':10, 'Random':10} } 

        my_choices_mod.apply(myarg1 = dep_dicts)

        self.modifyVariable("Inter-Dependency Window", "1-5:10, 6-10:1", "Choice")

        alu_instrs = ALU_Int32_All_instructions if self.getGlobalState("AppRegisterWidth") == 32 else ALU_Int_All_instructions
        g_instrs = RV32_G_instructions if self.getGlobalState("AppRegisterWidth") == 32 else RV_G_instructions 

        for i in range(10):
            random_instr = self.pickWeighted(alu_instrs)
            self.genInstruction(random_instr)
        
        for i in range(10):
            random_instr = self.pickWeighted(g_instrs)
            self.genInstruction(random_instr)
    
        my_choices_mod.revert()


## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

