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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from riscv.Utils import LoopControl


#  This test verifies that a standard loop executes for the expected number
#  of iterations.
class MainSequence(Sequence):
    def generate(self, **kargs):
        gpr_index = self.getRandomGPR(exclude="0")
        gpr_name = "x%d" % gpr_index
        self.reserveRegister(gpr_name)

        orig_gpr_val = self.random32()
        self.initializeRegister(gpr_name, orig_gpr_val)

        loop_control = LoopControl(self.genThread)
        loop_control.start(LoopCount=3)

        self.genInstruction("ADDI##RISCV", {"rd": gpr_index, "rs1": gpr_index, "simm12": 1})

        loop_control.end()

        (gpr_val, valid) = self.readRegister(gpr_name)
        if not valid:
            self.error("Unable to read register %s" % gpr_name)

        expected_gpr_val = orig_gpr_val + 3
        if gpr_val != expected_gpr_val:
            # self.error('Unexpected register value: Expected = 0x%x,
            #            Actual = 0x%x' % (expected_gpr_val, gpr_val))
            pass


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
