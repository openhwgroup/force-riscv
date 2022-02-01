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

        gpr_index_1 = self.getRandomGPR()
        gpr_name_1 = "x%d" % gpr_index_1
        for _ in range(200):

            self.notice(">>>>>>>>>>>>   Entering section for reserveRegister.")
            self.reserveRegister(gpr_name_1, "Read")
            is_read_res = self.isRegisterReserved(gpr_name_1, "Read")
            if not is_read_res:
                self.error("Register %s: Read Reserved = %s" % (gpr_name_1, is_read_res))

            self.reserveRegister(gpr_name_1, "Write")
            is_write_res = self.isRegisterReserved(gpr_name_1, "Write")
            if not is_write_res:
                self.error("Register %s: Write Reserved = %s" % (gpr_name_1, is_write_res))

            self.unreserveRegister(gpr_name_1, "Read")
            is_read_res = self.isRegisterReserved(gpr_name_1, "Read")
            if is_read_res:
                self.error("Register %s: Read Reserved = %s" % (gpr_name_1, is_read_res))

            is_write_res = self.isRegisterReserved(gpr_name_1, "Write")
            if not is_write_res:
                self.error("Register %s: Write Reserved = %s" % (gpr_name_1, is_write_res))

            self.unreserveRegister(gpr_name_1, "Write")
            is_write_res = self.isRegisterReserved(gpr_name_1, "Write")
            if is_write_res:
                self.error("Register %s: Write Reserved = %s" % (gpr_name_1, is_write_res))

            self.initializeRegister(gpr_name_1, 0x123)

            # Test the reserveRegisterByIndex
            self.notice(">>>>>>>>>>>>   Entering section for reserveRegisterByIndex.")
            gpr_index_2 = self.getRandomGPR(exclude=("%d" % gpr_index_1))
            gpr_name_2 = "x%d" % gpr_index_2
            self.reserveRegisterByIndex(64, gpr_index_2, "GPR", "ReadWrite")
            is_read_write_res = self.isRegisterReserved(gpr_name_2, "ReadWrite")
            if not is_read_write_res:
                self.error(
                    "Register %s: ReadWrite Reserved = %s" % (gpr_name_2, is_read_write_res)
                )

            self.unreserveRegisterByIndex(64, gpr_index_2, "GPR", "ReadWrite")
            is_read_write_res = self.isRegisterReserved(gpr_name_2, "ReadWrite")
            if is_read_write_res:
                self.error(
                    "Register %s: ReadWrite Reserved = %s" % (gpr_name_2, is_read_write_res)
                )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
