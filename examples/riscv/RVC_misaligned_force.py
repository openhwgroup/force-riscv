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

# *********************************************************************************
# generate N misaligned random RVC load/stores...
# *********************************************************************************

# Riscv RVC load/store instrs, word/dword accesses:
RVC_load_store_instructions = {
    "C.FLD##RISCV": 10,
    "C.LD##RISCV": 10,
    "C.LW##RISCV": 10,
    "C.FLDSP##RISCV": 10,
    "C.LDSP##RISCV": 10,
    "C.LWSP##RISCV": 10,
    "C.FSD##RISCV": 10,
    "C.SD##RISCV": 10,
    "C.SW##RISCV": 10,
    "C.FSDSP##RISCV": 10,
    "C.SDSP##RISCV": 10,
    "C.SWSP##RISCV": 10,
}


class MyMainSequence(Sequence):
    def generate(self, **kargs):
        for _ in range(100):
            # pick random RVC load/store instruction...
            instr = self.pickWeighted(RVC_load_store_instructions)
            # pick a random address aligned to a page boundary,
            # then (re)align that address close to the end of the page,
            # on half-word boundary. should yield a fair amount of misaligned
            # load/stores...
            target_addr = self.genVA(Align=0x1000) | 0xFFE
            self.notice(
                ">>>>>  Instruction: {} Target addr: {:012x}".format(
                    instr, target_addr
                )
            )
            self.genInstruction(instr, {"LSTarget": target_addr})


MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
