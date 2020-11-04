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
from DV.riscv.trees.instruction_tree import LDST_All_map
from DV.riscv.trees.instruction_tree import LDST32_All_map
from DV.riscv.trees.instruction_tree import ALU_Int_All_map
from DV.riscv.trees.instruction_tree import ALU_Float_Double_map

class MainSequence(Sequence):

    # This test template is a simple example of build your own instruction trees.
    # Other examples can be found in py/DV/riscv/trees/instruction_tree.py.

    def generate(self, **kargs):

        for _ in range(100):

            # define a single level instruction tree and choose an instruction
            random_iset1 = { "FMADD.S#Single-precision#RISCV":10,
                            "FMAX.S##RISCV":20,
                            "FMIN.S##RISCV":30 }

            the_instruction = self.pickWeighted(random_iset1)
            self.genInstruction(the_instruction)


            # define a multilevel instruction tree and choose an instruction
            random_iset2 = { "ADD##RISCV":10,
                             "SD##RISCV":10,
                             "BEQ##RISCV":10,
                             "FMADD.D#Double-precision#RISCV":10 }

            random_iset3 = { "JAL##RISCV":70,
                             "LUI##RISCV":50,
                             "FENCE##RISCV":30,
                             "ORI##RISCV":10 }

            iset1_map = InstructionMap("random_iset1", random_iset1)
            iset2_map = InstructionMap("random_iset2", random_iset2)
            iset3_map = InstructionMap("random_iset3", random_iset3)

            random_itree = { iset1_map:10,
                             iset2_map:20,
                             iset3_map:10 }

            # pick an instruction from the hierarchical instruction tree
            picked_instr = self.pickWeighted(random_itree)
            self.genInstruction(picked_instr)


            # a hierarchical instruction tree using the maps in py/DV/riscv/trees/instruction_tree.py
            random_itree = {  LDST_All_map:10,
                              ALU_Float_Double_map:10,
                              ALU_Int_All_map:10 }

            if self.getGlobalState('AppRegisterWidth') == 32:
                random_itree = {  LDST32_All_map:10,
                                  ALU_Float_Double_map:10,
                                  ALU_Int_All_map:10 }
                
            picked_instr = self.pickWeighted(random_itree)
            self.genInstruction(picked_instr)








## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

