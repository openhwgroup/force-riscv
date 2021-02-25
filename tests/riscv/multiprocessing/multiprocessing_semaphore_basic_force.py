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

from DV.riscv.trees.instruction_tree import (
    BranchJump_map,
    RV_G_map,
    RV32_G_map,
)
from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenSemaphoreRISCV import GenSemaphore32RISCV
from riscv.GenSemaphoreRISCV import GenSemaphore64RISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from riscv.Utils import LoadGPR64


# This test is intended to be run with mulitple threads to verify basic
# functionality of semaphores.
class MainSequence(Sequence):
    def generate(self, **kargs):
        target_addr_32 = self._genSharedVa("Shared Address 32")
        target_addr_64 = self._genSharedVa("Shared Address 64")
        sema32 = GenSemaphore32RISCV(self.genThread, "32-bit Semaphore", 1)
        if self.getGlobalState("AppRegisterWidth") == 64:
            sema64 = GenSemaphore64RISCV(self.genThread, "64-bit Semaphore", 1)

        for _ in range(3):
            if self.getGlobalState("AppRegisterWidth") == 64:
                (sema, target_addr) = self.choice(
                    ((sema32, target_addr_32), (sema64, target_addr_64))
                )
            else:
                (sema, target_addr) = (sema32, target_addr_32)

            with sema:
                self._testExclusiveStoreLoad(target_addr)

        if self.getGlobalState("AppRegisterWidth") == 64:
            sema64.cleanUp()
        sema32.cleanUp()

    # Generate a shared PA if one has not already been generated. Then,
    # generate a VA from the shared PA.
    #
    #  @param aAddrName An identifier for the shared address.
    def _genSharedVa(self, aAddrName):
        shared_phys_addr = 0
        with self.threadLockingContext():
            if self.hasSharedThreadObject(aAddrName):
                shared_phys_addr = self.getSharedThreadObject(aAddrName)
            else:
                shared_phys_addr = self.genPA(
                    Size=8, Align=8, Type="D", Shared=1
                )
                self.setSharedThreadObject(aAddrName, shared_phys_addr)

        return self.genVAforPA(
            Size=8, Align=8, Type="D", PA=shared_phys_addr, CanAlias=1
        )

    # Verify that a stored value can be subsequently retrieved by a load from
    # the same address. This method assumes execution in the context of a
    # semaphore.
    #
    #  @param aTargetAddr The target address for the store and load
    #       instructions.
    def _testExclusiveStoreLoad(self, aTargetAddr):
        src_reg_index = self.getRandomGPR(exclude="0")
        self.reserveRegister("x%d" % src_reg_index)

        if self.getGlobalState("AppRegisterWidth") == 64:
            load_gpr64_seq = LoadGPR64(self.genThread)
            test_val = RandomUtils.random64()
            load_gpr64_seq.load(src_reg_index, test_val)
            instr = "SD##RISCV"
        else:
            load_gpr32_seq = LoadGPR64(self.genThread)
            test_val = RandomUtils.random32()
            load_gpr32_seq.load(src_reg_index, test_val)
            instr = "SW##RISCV"
        self.genInstruction(
            instr, {"rs2": src_reg_index, "LSTarget": aTargetAddr}
        )

        self.unreserveRegister("x%d" % src_reg_index)

        if self.getGlobalState("AppRegisterWidth") == 32:
            instr_map = RV32_G_map - BranchJump_map
        else:
            instr_map = RV_G_map - BranchJump_map

        for _ in range(RandomUtils.random32(5, 10)):
            instr = instr_map.pick(self.genThread)
            self.genInstruction(instr)

        if self.getGlobalState("AppRegisterWidth") == 32:
            instr = "LW##RISCV"
        else:
            instr = "LD##RISCV"
        instr_id = self.genInstruction(instr, {"LSTarget": aTargetAddr})

        instr_record = self.queryInstructionRecord(instr_id)
        dest_reg_index = instr_record["Dests"]["rd"]
        if dest_reg_index != 0:
            dest_reg_name = "x%d" % dest_reg_index
            (dest_reg_val, valid) = self.readRegister(dest_reg_name)
            self._assertValidRegisterValue(dest_reg_name, valid)

            gen_mode = self.getPEstate("GenMode")
            no_iss = gen_mode & 0x1
            if (no_iss != 1) and (dest_reg_val != test_val):
                self.error(
                    "Unexpected destination register value; Expected=0x%x, "
                    "Actual=0x%x" % (test_val, dest_reg_val)
                )

    # Fail if the valid flag is false.
    #
    #  @param aRegName The name of the register.
    #  @param aValid A flag indicating whether the specified register has a
    #  valid value.
    def _assertValidRegisterValue(self, aRegName, aValid):
        if not aValid:
            self.error("Value for register %s is invalid" % aRegName)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
