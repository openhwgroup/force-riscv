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


class MyMainSequence(Sequence):

    def generate(self, **kargs):
        
        for _ in range(20):
            
            #instr = self.pickWeighted(Zicsr_instructions)
            #reg_value = self.random32(32,63)
            #reg_index = self.getRandomGPR()
            #reg_id = "x{}".format(reg_index)
            #self.writeRegister(reg_id, reg_value)
            #self.notice(">>>>>  Instruction:  {}".format(instr))
            #instr_record_id = self.genInstruction("CSRRW#register#RISCV", {"rd":0, "rs1":reg_index, "csr":0x340})
            instr_record_id = self.genInstruction("CSRRW#register#RISCV", {"rd":0, "csr":0x140})
            instr_record_id = self.genInstruction("CSRRC#register#RISCV", {"rs1":0, "csr":0x140})




MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
