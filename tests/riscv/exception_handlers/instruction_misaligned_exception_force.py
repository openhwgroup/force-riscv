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
from base.ChoicesModifier import ChoicesModifier
from base.InstructionMap import InstructionMap

# ********************************************************************************************
# Cause one or more misaligned instruction exceptions...
# ********************************************************************************************


class MainSequence(Sequence):

    # disable compressed instr set...

    def genEnableMisalignedBranches(self):
        # issue instructions (must be in M mode) to do so...

        if self.getPEstate("PrivilegeLevel") != 3:
            self.error("Test must be in Machine mode to change misa CSR.")

        # pick up a couple of scratch registers...

        (gpr1, gpr2) = self.getRandomRegisters(2, "GPR")

        # read misa, mask out C bit (to disable compressed instr set),
        # write misa back...

        self.genInstruction(
            "CSRRS#register#RISCV",
            {"rd": gpr1, "rs1": 0, "csr": self.getRegisterIndex("misa")},
        )

        self.genInstruction(
            "ADDI##RISCV", {"rd": gpr2, "rs1": 0, "simm12": 0x4}
        )
        self.genInstruction(
            "XORI##RISCV", {"rd": gpr2, "rs1": gpr2, "simm12": 0xFFF}
        )
        self.genInstruction(
            "AND##RISCV", {"rd": gpr1, "rs1": gpr1, "rs2": gpr2}
        )

        self.genInstruction(
            "CSRRW#register#RISCV",
            {"rd": 0, "rs1": gpr1, "csr": self.getRegisterIndex("misa")},
        )

    # generate a misaligned instruction address...

    def genMisalignedInstrAddress(self):
        target_addr = self.genVA(Type="I")

        self.notice(">>>>>  Next target addr: {:012x}".format(target_addr))

        if (target_addr & 0x3) == 0:
            # didn't get a misaligned address? help it along...
            target_addr = target_addr | self.random32(1, 3)

        self.notice(
            ">>>>>  Misaligned target addr: {:012x}".format(target_addr)
        )

        return target_addr

    # force a misaligned branch...

    def genMisalignedBranch(self, branch_instrs):
        self.genInstruction(
            self.choice(branch_instrs),
            {"BRTarget": self.genMisalignedInstrAddress()},
        )

    # if in privileged mode, attempt to generate an exception return instr,
    # with a misaligned return address...

    def genMisalignedExceptionReturn(self):
        if self.getPEstate("PrivilegeLevel") != 0:
            target_address = self.genMisalignedInstrAddress()
            if self.getPEstate("PrivilegeLevel") == 3:
                self.genInstruction("MRET##RISCV", {"epc": target_address})
            elif self.getPEstate("PrivilegeLevel") == 1:
                self.genInstruction("SRET##RISCV", {"epc": target_address})

    def generate(self, **kargs):

        self.genEnableMisalignedBranches()

        # after misalign enable, run actual test at any privilege level...
        self.systemCall({"PrivilegeLevel": self.choice((0, 1, 3))})

        # pick from unconditional branch instrs...
        branch_instrs = ["JAL##RISCV", "JALR##RISCV"]

        # fill with other random instrs...
        if self.getGlobalState("AppRegisterWidth") == 32:
            random_instructions = [
                "ADD##RISCV",
                "SRLI#RV32I#RISCV",
                "ADDI##RISCV",
                "SLLI#RV32I#RISCV",
                "LUI##RISCV",
            ]
        else:
            random_instructions = [
                "ADDW##RISCV",
                "SRLI#RV64I#RISCV",
                "ADDI##RISCV",
                "SLLI#RV64I#RISCV",
                "LUI##RISCV",
            ]

        for _ in range(100):
            self.genMisalignedBranch(branch_instrs)

            for _ in range(self.random32(0, 5)):
                self.genInstruction(self.choice(random_instructions))

        # should have seen at least one misaligned exception...

        exceptions_history = self.queryExceptions(EC=0)

        print("Misaligned instruction exception records: ", exceptions_history)

        if len(exceptions_history) > 0:
            # got one...
            pass
        else:
            self.error(
                "ERROR: Test did not generate any misaligned exceptions???"
            )

        # try to issue an exception return instr, with misaligned address
        # we don't expect an exception...

        self.genMisalignedExceptionReturn()

        for _ in range(self.random32(0, 5)):
            self.genInstruction(self.choice(random_instructions))


def gen_thread_initialization(gen_thread):
    (delegate_opt, valid) = gen_thread.getOption("DelegateExceptions")
    if valid and delegate_opt == 1:
        delegation_enables = ChoicesModifier(gen_thread)
        weightDict = {"0x0": 0, "0x1": 50}
        delegation_enables.modifyRegisterFieldValueChoices(
            "medeleg.Instruction address misaligned", weightDict
        )
        delegation_enables.commitSet()

    (paging_opt, valid) = gen_thread.getOption("PagingDisabled")
    if valid and paging_opt == 1:
        gen_thread.initializeRegister(name="satp", value=0, field="MODE")


GenThreadInitialization = gen_thread_initialization

MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
