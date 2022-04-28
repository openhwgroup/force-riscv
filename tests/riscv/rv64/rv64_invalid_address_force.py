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
from base.TestUtils import assert_true
from riscv.Utils import LoadGPR64
import RandomUtils

# This test verifies that instructions with invalid target addresses are resolved in some way, such
# as by having the exception handler skip the instruction.
class MainSequence(Sequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExceptCounts = {
            0xD: 0,
            0xF: 0,
        }

    def generate(self, **kwargs):
        load_gpr64_seq = LoadGPR64(self.genThread)
        for _ in range(RandomUtils.random32(25, 50)):
            addr_size = 8
            target_addr = self.genVA(Size=addr_size, Align=addr_size, Type="D")

            # Make the address invalid by manipulating the top bits
            target_addr |= 0xF000000000000000
            target_addr &= 0xF0FFFFFFFFFFFFFF

            self.initializeMemory(
                addr=target_addr,
                bank=0,
                size=addr_size,
                data=RandomUtils.random64(),
                is_instr=False,
                is_virtual=True,
            )

            gpr_index = self.getRandomGPR(exclude="0")
            load_gpr64_seq.load(gpr_index, target_addr)

            self.genInstruction(
                self.choice(("LD##RISCV", "SD##RISCV")),
                {
                    "rd": 0,
                    "rs1": gpr_index,
                    "simm12": 0,
                    "LSTarget": target_addr,
                    "NoRestriction": 1,
                },
            )

            gen_mode = self.getPEstate("GenMode")
            no_iss = gen_mode & 0x1
            if no_iss != 1:
                assert_true(self._hasPageFaultOccurred(), "No page fault triggered")

    def _hasPageFaultOccurred(self):
        page_fault_occurred = False
        for (except_code, except_count) in self._mExceptCounts.items():
            new_except_count = self.queryExceptionRecordsCount(except_code)

            if new_except_count > except_count:
                page_fault_occurred = True
                self._mExceptCounts[except_code] = new_except_count

        return page_fault_occurred


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
