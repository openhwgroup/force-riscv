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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence

class MainSequence(Sequence):

    def generate(self, **kargs):
        instr_list = ["ADDW##", "SRLI#RV32I#", "ADDI##", "SLLI#RV32I#", "LUI##"] # ["JAL", "BEQ"]

        for instr in instr_list:
            self.genInstruction("%sRISCV" % instr)

        # limit range due to spike standlone memory foot print limitation
        self.genInstruction("LB##RISCV", {"LSTarget":"0x80000000-0x83ffffff"})
        self.genInstruction("SB##RISCV", {"LSTarget":"0x80000000-0x83ffffff"})

## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
