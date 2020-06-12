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
from DV.riscv.trees.instruction_tree import Zicsr_instructions

class MainSequence(Sequence):

    def generate(self, **kargs):

        # Check the fflags
        index = self.getRegisterIndex("fflags")
        if index != 0x1:  self.error("Index value for fflags should always be 0x1")
        reloadValue = self.getRegisterReloadValue("fflags")
        self.notice(">>>>>>>>>>>>   The fflags reload value is {:16X}".format(reloadValue))

        # Check the frm
        index = self.getRegisterIndex("frm")
        if index != 0x2:  self.error("Index value for frm should always be 0x2")
        reloadValue = self.getRegisterReloadValue("frm")
        self.notice(">>>>>>>>>>>>   The frm reload value is {:16X}".format(reloadValue))

        # Check the fcsr
        index = self.getRegisterIndex("fcsr")
        if index != 0x3:  self.error("Index value for fcsr should always be 0x3")
        reloadValue = self.getRegisterReloadValue("fcsr")
        self.notice(">>>>>>>>>>>>   The fcsr reload value is {:16X}".format(reloadValue))



        # -- Randomly generate instructions that access the fcsr and its fields --
        # csr addresses for fcsr:  0x1=fflags, 0x2=frm, 0x3=fcsr
        fcsr_addresses = [0x1, 0x2, 0x3]              
        # create a random list of csr access instructions
        list_of_csr_instructions = self.sample(list(Zicsr_instructions.keys()), 2)

        for instr in list_of_csr_instructions:
            csr_address = self.choice(fcsr_addresses)
            #self.genInstruction(instr, {'csr': csr_address})
            self.genInstruction(instr, {'csr': csr_address, 'rs1': reloadValue})






## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

