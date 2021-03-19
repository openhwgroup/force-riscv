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


# This test generates a virtual address in a different privilege level from
# that in which it is used. The genVA() method should respect the privilege
# level specified rather than the one in which the address was generated.
class MainSequence(Sequence):
    def generate(self, **kargs):
        self.systemCall({"PrivilegeLevel": 1})

        instr = "LD##RISCV"
        size = 8

        if self.getGlobalState("AppRegisterWidth") == 32:
            instr = "LW##RISCV"
            size = 4

        # Generate a target address for a U Mode instruction from S Mode
        target_addr = self.genVA(
            Size=size, Align=size, Type="D", PrivilegeLevel=0
        )

        self.systemCall({"PrivilegeLevel": 0})

        self.genInstruction(instr, {"LSTarget": target_addr})
        self._assertNoPageFault(target_addr)

        # Generate a target address for an S Mode instruction from U Mode
        target_addr = self.genVA(
            Size=size, Align=size, Type="D", PrivilegeLevel=1
        )

        self.systemCall({"PrivilegeLevel": 1})

        self.genInstruction(instr, {"LSTarget": target_addr})
        self._assertNoPageFault(target_addr)

    # Fail if a load page fault occurred.
    #
    #  @param aTargetAddr The target address of the previous load instruction.
    def _assertNoPageFault(self, aTargetAddr):
        if self.queryExceptionRecordsCount(13) > 0:
            self.error(
                "A LD instruction targeting 0x%x triggered an unexpected "
                "page fault" % aTargetAddr
            )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
