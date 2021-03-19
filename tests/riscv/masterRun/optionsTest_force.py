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
from DV.riscv.trees.instruction_tree import (
    ALU_Int_All_instructions,
    ALU_Int32_All_instructions,
)
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


class MyMainSequence(Sequence):
    def generate(self, **kargs):
        loopCount, valid = self.getOption("loopCount")

        if not valid:
            self.error(
                ">>>>>  No 'loopCount' option was specified.  "
                "Value is {}.".format(loopCount)
            )
        else:
            self.notice(
                ">>>>>  Value specified for 'loopCount' option is:  "
                "{}".format(loopCount)
            )

        instrs = (
            ALU_Int32_All_instructions
            if self.getGlobalState("AppRegisterWidth") == 32
            else ALU_Int_All_instructions
        )

        for _ in range(loopCount):
            instr = self.pickWeighted(instrs)
            self.genInstruction(instr)


MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
