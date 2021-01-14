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
from DV.riscv.trees.instruction_tree import RV_G_instructions
from DV.riscv.trees.instruction_tree import RV32_G_instructions
from DV.riscv.trees.instruction_tree import ALU_Float_All_map
from DV.riscv.trees.instruction_tree import LDST_All_map
from DV.riscv.trees.instruction_tree import LDST32_All_map
from DV.riscv.trees.instruction_tree import RV32F_map


class MyMainSequence(Sequence):

    def generate(self, **kwargs):
        
        choices_mod = ChoicesModifier(self.genThread)

        for _ in range(10):

            # Generate instructions with the default choices settings
            # except for the paging choices for "Page size#4K granule#S#stage 1"
            # which was set before this generate was called via
            # the GenThreadInitialization below.

            for _ in range(20):
                if (self.getGlobalState('AppRegisterWidth') == 32):
                    instr = self.pickWeighted(RV32_G_instructions)
                else:
                    instr = self.pickWeighted(RV_G_instructions)
                self.genInstruction(instr)

            # Modify the choices settings
            choices_mod.modifyOperandChoices("Rounding mode", {"RNE":0, 
                    "RTZ":0, "RDN":50, "RUP":0, "RMM":0, "DYN":50})
            
            choices_mod.modifyOperandChoices("Read after write address reuse", 
                                    {"No reuse":50, "Reuse":50})
            choices_mod.commitSet()

            # generate instructions
            for _ in range(20):

                if (self.getGlobalState('AppRegisterWidth') == 32):
                    instr_mix = { RV32F_map:10,
                                  LDST32_All_map:10 }
                else:
                    instr_mix = { ALU_Float_All_map:10,
                                  LDST_All_map:10 }

                instr = self.pickWeighted(instr_mix)
                self.genInstruction(instr)
               
            # undo the choices settings - revert back to prior
            choices_mod.revert()




# Modify the Paging Choices so that the test uses only 4K, 2M, or 4M page sizes
# This will be run before the MainSequence.generate()
def gen_thread_initialization(gen_thread):

    # when satp csr width is 32 bits then Sv32 is only paging mode possible...
    satp_info = gen_thread.getRegisterInfo("satp", gen_thread.getRegisterIndex('satp') )
    rv32 = satp_info['Width'] == 32
    
    choices_mod = ChoicesModifier(gen_thread)

    if rv32:
        # Sv32 only choices...
        choices_mod.modifyPagingChoices("Page size#4K granule#S#stage 1", {"4K":10, "4M":10} )
    else:
        # Sv48 otherwise...
        choices_mod.modifyPagingChoices("Page size#4K granule#S#stage 1", {"4K":10, "2M":10, "1G":0, "512G":0} )

    choices_mod.commitSet()



# Enable the GenThreadInitialization entry point
GenThreadInitialization = gen_thread_initialization

## Points to the MainSequence defined in this file
MainSequenceClass = MyMainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

