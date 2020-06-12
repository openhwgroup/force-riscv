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
from base.Sequence import Sequence
from base.ChoicesModifier import *
#from riscv.SystemRegisterUtils import UpdateSystemRegister
import pdb
#from riscv.ModifierUtils import PageMemoryAttributeModifier

class MyChoicesModifier(ChoicesModifier):
    def __init__(self, gen_thread):
        super().__init__(gen_thread, "MyChoicesModifier")

    def apply(self, func,**kwargs):
        for arg in kwargs:
            func(arg, kwargs[arg])
        self.commitSet()

class depSequence(Sequence): 
    def __init__(self, gen_thread, name=None):
        super().__init__(gen_thread, name)
        self.genThread = gen_thread
        self.myChoiceMod = MyChoicesModifier(self.genThread)  
        #self.memattrModify = PageMemoryAttributeModifier(self.genThread)

    def choiceMod(self, **kargs):
        generalDepDicts = {'Register Dependency' : {'No dependency' : 0, 'Inter-dependency' : 10, 'Intra-dependency' : 0}, 
                           'Target Dependency' : {'Target after source' : 10, 'Target after target': 0, 'No target dependence': 0}, 
                           'Source Dependency' : {'Source after source' : 0, 'Source after target': 10, 'No source dependence': 0},
                           'Inter-Dependency Priority' : {'The neareast': 0 , 'The farthest': 0, 'Optimal':10, 'Random':10},
                           "The Optimal Inter-Dependency Direction":{"From the nearest to the farthest":50, "From the farthest to the nearest":50},
                           } 
        self.myChoiceMod.apply(self.myChoiceMod.modifyDependenceChoices, **generalDepDicts)

        self.modifyVariable("Inter-Dependency Window", "1-30:10", "Choice")

        operandDicts = {   
                           'Use addressing preamble' : {"no":999999, "yes":1},
                           'Ordered data alignment' : {'Aligned':100, 'Unaligned':0},
                       }
        self.myChoiceMod.apply(self.myChoiceMod.modifyOperandChoices, **operandDicts)

    def alu2ldChoice(self,**kargs):
        operandDicts = {
                           "ALU result":{"Load store address":100, "Random":0},
                           "Data processing result":{"Load store address":100, "Random":0}
                       }
        self.myChoiceMod.apply(self.myChoiceMod.modifyOperandChoices, **operandDicts)

    def ld2ldChoice(self, **kargs):
        operandDicts = {
                           'Load data' : {'Load store address':100, 'Branch address':0, 'Random' :0},
                       }
        self.myChoiceMod.apply(self.myChoiceMod.modifyOperandChoices, **operandDicts)

    def ld2brChoice(self, **kargs):
        operandDicts = {
                           'Load data' : {'Load store address':0, 'Branch address':100, 'Random' :0},
                       }
        self.myChoiceMod.apply(self.myChoiceMod.modifyOperandChoices, **operandDicts)  

    def pointerchaseChoice(self, **kargs):
        generalDepDicts = {'Register Dependency' : {'No dependency' : 0, 'Inter-dependency' : 10, 'Intra-dependency' : 0}, 
                           'Target Dependency' : {'Target after source' : 0, 'Target after target': 0, 'No target dependence': 10}, 
                           'Source Dependency' : {'Source after source' : 0, 'Source after target': 10, 'No source dependence': 0},
                           'Inter-Dependency Priority' : {'The neareast': 0 , 'The farthest': 0, 'Optimal':10, 'Random':10},
                           "The Optimal Inter-Dependency Direction":{"From the nearest to the farthest":50, "From the farthest to the nearest":50},
                           } 
        self.myChoiceMod.apply(self.myChoiceMod.modifyDependenceChoices, **generalDepDicts)

        operandDicts = {   
                           'Use addressing preamble' : {"no":999999, "yes":1},
                           'Ordered data alignment' : {'Aligned':100, 'Unaligned':0},
                       }
        self.myChoiceMod.apply(self.myChoiceMod.modifyOperandChoices, **operandDicts)    
        self.ld2ldChoice()
        self.modifyVariable("Inter-Dependency Window", "1:10,2-30:0", "Choice")


    def update_sysreg(self,**kargs):  pass

    def setup(self, **kargs):  pass



