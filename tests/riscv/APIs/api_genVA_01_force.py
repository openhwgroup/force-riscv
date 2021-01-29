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

class MainSequence(Sequence):
    """Exercise different combinations of values for the parameters for the genVA instruction.
       Focus in this test is to try values of the Size, Align, and Flatmap parameters. 
       Type is always 'D'; Bank is always '0'.
    """

    def generate(self, **kargs):

        ldstr_byte_ops   = ['LB##RISCV', 'SB##RISCV']
        ldstr_half_ops   = ['LH##RISCV', 'SH##RISCV']
        ldstr_word_ops   = ['LW##RISCV', 'SW##RISCV']
        ldstr_double_ops = ['LD##RISCV', 'SD##RISCV']

        # Iterate through Size and Align values.  Force requires Align to be a power of 2.
        # This 1st block tests smaller values of size - 1 byte to 32 bytes.
        for theSize in [2 ** x for x in range(0, 6)]:

            for theAlign in [2 ** x for x in range(0, 16)]:

                if theAlign < theSize: continue

                for _ in range(2):

                    rand_VA = self.genVA(Size=theSize, Align=theAlign, Type="D", Bank=0, FlatMap=1)
                    self.notice(">>>>>> Requested Alignment:  {:6d}     Requested Size:  {:6d}     gen target VA={:12X}".format(theAlign, theSize, rand_VA))

                    instr_id = self.genInstruction(self.choice(ldstr_byte_ops), {'LSTarget':rand_VA})
                    if theAlign % 2 == 0: instr_id = self.genInstruction(self.choice(ldstr_half_ops), {'LSTarget':rand_VA})
                    if theAlign % 4 == 0: instr_id = self.genInstruction(self.choice(ldstr_word_ops), {'LSTarget':rand_VA})
                    if self.getGlobalState('AppRegisterWidth') != 32:
                        if theAlign % 8 == 0: instr_id = self.genInstruction(self.choice(ldstr_double_ops), {'LSTarget':rand_VA})
                    


        # Iterate through Size and Align values.  Force requires Align to be a power of 2.
        # This 2nd block tests larger values of size - 32K to 8M.
        for theSize in [2 ** x for x in range(15, 24)]:

            for theAlign in [2 ** x for x in range(15, 25)]:

                if theAlign < theSize: continue

                for _ in range(2):

                    rand_VA = self.genVA(Size=theSize, Align=theAlign, Type="D", Bank=0, FlatMap=0)
                    self.notice(">>>>>> Requested Alignment:  {:6d}     Requested Size:  {:6d}     gen target VA={:12X}".format(theAlign, theSize, rand_VA))

                    instr_id = self.genInstruction(self.choice(ldstr_byte_ops), {'LSTarget':rand_VA})
                    if theAlign % 2 == 0: instr_id = self.genInstruction(self.choice(ldstr_half_ops), {'LSTarget':rand_VA})
                    if theAlign % 4 == 0: instr_id = self.genInstruction(self.choice(ldstr_word_ops), {'LSTarget':rand_VA})
                    if self.getGlobalState('AppRegisterWidth') != 32:
                        if theAlign % 8 == 0: instr_id = self.genInstruction(self.choice(ldstr_double_ops), {'LSTarget':rand_VA})





MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

