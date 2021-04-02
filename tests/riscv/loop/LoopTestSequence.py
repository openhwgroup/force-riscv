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
from base.Sequence import Sequence
from riscv.Utils import LoopControl


#  This class provides a base sequence for testing standard loops.
class LoopTestSequence(Sequence):
    def generate(self, **kargs):
        loop_reg_index = self.getRandomGPR(exclude="0")
        loop_reg_name = "x%d" % loop_reg_index
        self.reserveRegister(loop_reg_name)

        loop_control = LoopControl(self.genThread)
        loop_control.start(
            LoopReg=loop_reg_index, LoopCount=self.random32(2, 8)
        )

        for _ in range(self.random32(500, 1000)):
            self.genInstruction(
                self.pickWeighted(self.getInstructionWeights())
            )

        loop_control.end()

        (loop_reg_val, valid) = self.readRegister(loop_reg_name)
        if not valid:
            self.error("Unable to read register %s" % loop_reg_name)

        if loop_reg_val != 0:
            # self.error('Unexpected register value: Expected = 0x0,
            #            Actual = 0x%x' % loop_reg_val)
            pass

    # Return a dictionary of names of instructions to generate in the loop
    # body with their corresponding weights.
    def getInstructionWeights(self):
        raise NotImplementedError
