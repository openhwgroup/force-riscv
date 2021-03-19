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
import RandomUtils
from Enums import EStateElementDuplicateMode
from State import State

from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


# This test attempts to add StateElements to a State. There is no direct
# mechanism for retrieving the StateElements after they have been added, so
# this test merely ensures the method calls don't crash or fail.
class MainSequence(Sequence):
    def generate(self, **kargs):
        state = State()
        state.setDuplicateMode(EStateElementDuplicateMode.Replace)

        mem_start_addr = (RandomUtils.random64(0, 0xFFFFFFFFFFFF) >> 3) << 3
        mem_val = RandomUtils.random64()
        state.addMemoryStateElement(mem_start_addr, 8, mem_val)

        mem_values = []
        for _ in range(RandomUtils.random32(1, 64)):
            mem_values.append(RandomUtils.random32(0, 0xFF))

        mem_start_addr = RandomUtils.random64(0, 0xFFFFFFFFFFFF)
        state.addMemoryStateElementsAsBytes(mem_start_addr, mem_values)

        gpr_name = "x%d" % RandomUtils.random32(0, 31)
        state.addRegisterStateElement(gpr_name, (RandomUtils.random64(),))
        fp_reg_name = "D%d" % RandomUtils.random32(0, 31)
        state.addRegisterStateElement(fp_reg_name, (RandomUtils.random64(),))

        state.addSystemRegisterStateElementByField("sstatus", "FS", 0x3)

        state.addVmContextStateElement("mstatus", "MPRV", 0x1)

        state.addPcStateElement(RandomUtils.random64(0, 0xFFFFFFFFFFFF))

        # Test creating duplicate StateElements
        state.addVmContextStateElement("mstatus", "MPRV", 0x0)
        state.setDuplicateMode(EStateElementDuplicateMode.Ignore)
        state.addRegisterStateElement("sstatus", (RandomUtils.random64(),))

        # Test merging two StateElements
        mem_start_addr = (RandomUtils.random64(0, 0xFFFFFFFFFFFF) >> 3) << 3
        mem_val = RandomUtils.random32()
        state.addMemoryStateElement(mem_start_addr, 4, mem_val)

        mem_start_addr += 4
        mem_values = []
        for _ in range(4):
            mem_values.append(RandomUtils.random32(0, 0xFF))

        state.addMemoryStateElementsAsBytes(mem_start_addr, mem_values)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
