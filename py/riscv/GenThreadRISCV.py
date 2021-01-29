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
import StateTransition
from Enums import EStateElementType

import riscv.PcConfig as PcConfig
from base.GenThread import (
    GenThread,
    GenThreadSetupSequence,
)
from riscv.StateTransitionHandlerRISCV import *
from riscv.exception_handlers.ExceptionHandlerManagerRISCV import (
    ExceptionHandlerManagerRISCV,
)
from riscv.exception_handlers.ExceptionsFactoryRISCV import (
    ExceptionsFactoryRISCV,
)


class GenThreadSetupSeqRISCV(GenThreadSetupSequence):
    """Inherit from GenThreadSetupSequence, return RISCV specific per-thread
    initial information."""

    def __init__(self, gen_thread, name):
        super().__init__(gen_thread, name)

    def setupBootRegion(self):
        """Setup memory region for boot instructions"""
        self.virtualMemoryRequest(
            "PhysicalRegion",
            {
                "RegionType": "BootRegion",
                "Size": PcConfig.get_boot_region_size(),
                "Type": "I",
                "Bank": 0,
            },
        )

    def setupEssentials(self):
        """Setup essentials like key register fields, etc"""
        self.genSequence("InitialSetup")
        self.updateVm()
        self.genThread.installAddressTable()

        no_handler, valid = self.getOption("NoHandler")
        if (not valid) or (not no_handler):
            self.genThread.installHandlers()

        handler_set, valid = self.getOption("handlers_set")
        if valid:
            self.genThread.specifyExceptionHandlerSet(handler_set)

    def getInitialPC(self):
        """Return initial main test PC for the generator thread"""
        return PcConfig.get_initial_pc(self.genThread.genThreadID)

    # Return boot code address for the generator thread
    def getBootPC(self):
        return PcConfig.get_boot_pc(self.genThread.genThreadID)

    def setDefaultStateTransitionHandlers(self):
        """Set the default StateTransitionHandlers to process StateElements of
        each type."""
        StateTransition.setDefaultStateTransitionHandler(
            MemoryStateTransitionHandlerRISCV(self.genThread),
            EStateElementType.Memory,
        )
        StateTransition.setDefaultStateTransitionHandler(
            VectorRegisterStateTransitionHandlerRISCV(self.genThread),
            EStateElementType.VectorRegister,
        )
        StateTransition.setDefaultStateTransitionHandler(
            SystemRegisterStateTransitionHandlerRISCV(self.genThread),
            EStateElementType.SystemRegister,
        )
        StateTransition.setDefaultStateTransitionHandler(
            GprStateTransitionHandlerRISCV(self.genThread),
            EStateElementType.GPR,
        )
        StateTransition.setDefaultStateTransitionHandler(
            VmContextStateTransitionHandlerRISCV(self.genThread),
            EStateElementType.VmContext,
        )
        StateTransition.setDefaultStateTransitionHandler(
            PrivilegeLevelStateTransitionHandlerRISCV(self.genThread),
            EStateElementType.PrivilegeLevel,
        )
        StateTransition.setDefaultStateTransitionHandler(
            PcStateTransitionHandlerRISCV(self.genThread), EStateElementType.PC
        )
        StateTransition.setDefaultStateTransitionHandler(
            FloatingPointRegisterStateTransitionHandlerRISCV(self.genThread),
            EStateElementType.FloatingPointRegister,
        )


#  GenThreadRISCV class
#  RISCV class representing generator thread
class GenThreadRISCV(GenThread):
    def __init__(self, gen_id, interface):
        super().__init__(gen_id, interface)
        self.exceptionHandlerManager = ExceptionHandlerManagerRISCV(
            self, ExceptionsFactoryRISCV()
        )
        self.addressTableManager = None
        self.fastMode = False

    # Called to assign sequences to be run at setup stage
    def assignSetupSequences(self):
        self.addSetupSequence(GenThreadSetupSeqRISCV)

    # Called to install exception handlers.
    def installHandlers(self):
        # generate exception handlers...
        self.exceptionHandlerManager.run(
            address_table_manager=self.addressTableManager
        )

        # assign SystemCallSequence to handle system call
        from riscv.SystemCallSequence import SystemCallSequence

        self.systemCallSequence = SystemCallSequence

        # create SelfHostedDebugSequence instance...
        # TODO:
        #   from riscv.SelfHostedDebugSequence import SelfHostedDebugSequence
        # TODO: self.selfHostedDebugSequence = SelfHostedDebugSequence

    def installAddressTable(self):
        from riscv.AddressTable import AddressTableManagerRISCV

        if self.addressTableManager is None:
            self.addressTableManager = AddressTableManagerRISCV(self)
        self.addressTableManager.run()
