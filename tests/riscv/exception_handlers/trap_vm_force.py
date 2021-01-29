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

from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


# exercize mstatus.tvm (trap virtual memory mgmt ops from Supervisor mode)


class MyMainSequence(Sequence):
    def generate(self, **kargs):
        # confirm that test is running in supervisor mode...

        if self.getPEstate("PrivilegeLevel") != 1:
            self.error("ERROR: Test needs to start in Supervisor mode")

        # we are in supervisor mode...

        # read or write access to satp should raise illegal instruction
        # exception...

        (gpr1, gpr2) = self.getRandomRegisters(2, "GPR")
        if RandomUtils.random32(0, 1) == 1:
            self.genInstruction(
                "CSRRS#register#RISCV",
                {"rd": gpr1, "rs1": 0, "csr": self.getRegisterIndex("satp")},
            )
        else:
            self.genInstruction(
                "CSRRW#register#RISCV",
                {"rd": 0, "rs1": gpr1, "csr": self.getRegisterIndex("satp")},
            )

        # confirm that illegal instruction exception occurred...

        exceptions_history = self.queryExceptions(EC="2")
        if len(exceptions_history) == 1:
            # illegal instruction exception did occur...
            self.notice(
                "Supervisor access to satp CSR when mstatus.TVM is set DID "
                "raise illegal instruction exception, as expected"
            )
            pass
        else:
            self.error(
                "ERROR: Access to satp CSR from supervisor mode did NOT "
                "raise illegal instruction exception ???"
            )

        # generation of SFENCE.VMA instruction should raise illegal instruction
        # exception...

        self.genInstruction("SFENCE.VMA##RISCV", {"rs1": gpr1, "rs2": gpr2})

        # confirm that illegal instruction exception occurred...

        exceptions_history = self.queryExceptions(EC="2")
        if len(exceptions_history) == 2:
            self.notice(
                "Execution of SFENCE.VMA from supervisor mode, when "
                "mstatus.TSR is set DID raise illegal instruction exception, "
                "as expected"
            )
            # illegal instruction exception did occur...
            pass
        else:
            self.error(
                "ERROR: Execution of SFENCE.VMA instruction in supervisor "
                "mode did NOT raise illegal instruction exception ???"
            )

        # done.


def gen_thread_initialization(gen_thread):
    gen_thread.initializeRegister(
        name="mstatus", value=1, field="TVM"
    )  # set 'Trap Virtual Memory',
    gen_thread.initializeRegister(
        name="mstatus", value=1, field="TSR"
    )  # 'Trap SRET'


GenThreadInitialization = gen_thread_initialization

MainSequenceClass = MyMainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
