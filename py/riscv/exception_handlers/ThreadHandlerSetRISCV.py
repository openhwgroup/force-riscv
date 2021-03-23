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
from base.exception_handlers.ThreadHandlerSet import ThreadHandlerSet
from riscv.MemoryBank import MemoryBankRISCV
from riscv.PrivilegeLevel import PrivilegeLevelRISCV
from riscv.SecurityState import SecurityStateRISCV
from riscv.exception_handlers.ExceptionClass import ExceptionClassRISCV


class ThreadHandlerSetRISCV(ThreadHandlerSet):
    def __init__(
        self, gen_thread, memBankHandlerRegistryRepo, factory, exceptionsStack
    ):
        super().__init__(
            gen_thread, memBankHandlerRegistryRepo, factory, exceptionsStack
        )

        self.currentPrivLevel = None

    # return set of scratch (gpr) registers for a handler set.
    # NOTE: call this method after handlers are generated
    def getScratchRegisterSets(self):
        scratch_register_sets = {}

        handler = self.priv_level_handler_sets[
            PrivilegeLevelRISCV.M
        ].getSynchronousExceptionHandler(
            SecurityStateRISCV.DEFAULT,
            ExceptionClassRISCV.ENV_CALL_FROM_U_MODE,
        )
        scratch_register_sets[
            "DataBlockAddrRegIndex"
        ] = handler.mDataBlockAddrRegIndex
        scratch_register_sets[
            "ActionCodeRegIndex"
        ] = handler.mActionCodeRegIndex

        return scratch_register_sets

    def getVectorBaseAddressSets(self):
        vector_base_addr_sets = {}
        for (
            (priv_level, security_state),
            vector_offset_table,
        ) in self.vector_offset_tables.items():
            if priv_level == PrivilegeLevelRISCV.M:
                priv_level_str = "MachineMode"
            elif priv_level == PrivilegeLevelRISCV.S:
                priv_level_str = "SupervisorMode"
            else:
                raise ValueError("Unexpected privilege level %s" % priv_level)

            vector_base_addr_sets[
                "%s%sVector"
                % (security_state.name.capitalize(), priv_level_str)
            ] = vector_offset_table

        return vector_base_addr_sets

    def savePrivilegeLevel(self):
        self.currentPrivLevel = self.getPEstate("PrivilegeLevel")

        self.debugPrint("SAVED PRIVILEGE-LEVEL: %d\n" % self.currentPrivLevel)

    def setPrivilegeLevel(self, newSecurityState):
        # The handlers are generated with paging disabled to avoid the cost of
        # initializing paging for tests that do not require it. We also want to
        # minimize the number of execution states that we utilize in order to
        # avoid initializing any more state than necessary. This leads
        # us to use M mode to generate the exception handlers.
        self.debugPrint("SET PRIVILEGE-LEVEL TO %s\n" % "M")

        self.setPEstate("PrivilegeLevel", 3)
        self.updateVm()

    def restorePrivilegeLevel(self):
        self.setPEstate("PrivilegeLevel", self.currentPrivLevel)
        self.updateVm()

    def getPrivilegeLevels(self):
        return tuple(PrivilegeLevelRISCV)[1:]

    def getSupportedSecurityStates(self, aPrivLevel):
        return tuple(SecurityStateRISCV)

    def getMemoryBanks(self):
        return tuple(MemoryBankRISCV)

    def getVectorTableSize(self):
        return 0x80

    def getVectorOffsetIncrement(self):
        return 0x80

    def getNextVectorBaseAddress(self, aNextAvailableAddress):
        vector_base_address = (aNextAvailableAddress + 0xFFF) & (~0xFFF)
        return vector_base_address

    def isSynchronousVectorEntry(self, aVectorOffset):
        return aVectorOffset == 0x0

    def getVectorEntryErrorCode(self):
        return 65

    # use this method to lay down a relative branch
    def genRelativeBranchAtAddr(self, br_address, br_target_address):
        save_pc = self.getPEstate("PC")
        self.setPEstate("PC", br_address)
        self.genRelativeBranch(br_target_address)
        self.setPEstate("PC", save_pc)

    def genRelativeBranch(self, br_target_address):
        (br_offset, is_valid, num_hw) = self.getBranchOffset(
            self.getPEstate("PC"), br_target_address, 20, 1
        )
        self.genInstruction(
            "JAL##RISCV", {"rd": 0, "simm20": br_offset, "NoRestriction": 1}
        )
