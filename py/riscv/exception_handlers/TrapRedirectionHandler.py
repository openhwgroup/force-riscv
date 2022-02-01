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

from base.exception_handlers.ReusableSequence import ReusableSequence
from riscv.exception_handlers.ExceptionHandlerContext import RegisterCallRole


#  Generate instructions to redirect exceptions to (next) lower privilege.
#
#  Notes:
#    1. Redirects from Machine mode to Supervisor mode.
#    2. Does NOT redirect traps from machine mode.
#
#  Assumptions:
#    1. Intended to handle synchronous exceptions ONLY.


class TrapRedirectionHandlerRISCV(ReusableSequence):
    def __init__(self, aGenThread, aFactory, aStack):
        super().__init__(aGenThread, aFactory, aStack)

        # these two variables are not used in this handler, but could be
        # queried by the ThreadHandlerSetRISCV::getScratchRegisterSets
        # method, depending upon which exception gets redirected...

        self.mDataBlockAddrRegIndex = None
        self.mActionCodeRegIndex = None

    def generateHandler(self, **kwargs):
        try:
            handler_context = kwargs["handler_context"]
        except KeyError:
            self.error(
                "INTERNAL ERROR: one or more arguments to "
                "TrapRedirectionHandlerRISCV generate method missing."
            )

        self.debug(
            "[TrapRedirectionHandlerRISCV] generate handler address: 0x%x" % self.getPEstate("PC")
        )

        priv_level_reg_index = handler_context.getScratchRegisterIndices(
            RegisterCallRole.PRIV_LEVEL_VALUE
        )

        (
            self.mDataBlockAddrRegIndex,
            self.mActionCodeRegIndex,
        ) = handler_context.getScratchRegisterIndices(RegisterCallRole.ARGUMENT, 2)

        (
            scratch_reg_index,
            scratch_reg2_index,
            scratch_reg3_index,
        ) = handler_context.getScratchRegisterIndices(RegisterCallRole.TEMPORARY, 3)

        # if the exception was taken in machine mode, then skip it...

        self.genGetPreviousPrivilegeMode(scratch_reg_index)
        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 3)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, scratch_reg2_index, 22, "NE", "DO_REDIRECT"
        )
        self.mAssemblyHelper.genIncrementExceptionReturnAddress(
            scratch_reg_index, priv_level_reg_index
        )
        self.mAssemblyHelper.genReturn()

        self.mAssemblyHelper.addLabel("DO_REDIRECT")

        # copy the exception context...

        self.copyCSR(
            "sepc", "mepc", scratch_reg_index
        )  # copy address of instruction that raised the exception
        self.copyCSR("scause", "mcause", scratch_reg_index)  # copy Exception Cause value
        self.copyCSR(
            "stval", "mtval", scratch_reg_index
        )  # (for page fault exception) copy faulting address
        self.copyCSR(
            "mepc", "stvec", scratch_reg_index
        )  # Machine mode 'return address' will be Supervisor Mode Base Address

        # setup to cause machine-mode return from interrupt to in effect switch
        # to supervisor-mode...

        self.genSetupSmodeReturn(scratch_reg_index, scratch_reg2_index, scratch_reg3_index)

        self.mAssemblyHelper.genReturn()

    # get previous privilege level - isolate mstatus.mpp
    #
    #  @param aScratchReg1Index - Scratch register index.

    def genGetPreviousPrivilegeMode(self, aScratchRegIndex):
        self.mAssemblyHelper.genReadSystemRegister(aScratchRegIndex, "mstatus")  # read mstatus,
        self.mAssemblyHelper.genShiftRightImmediate(aScratchRegIndex, 11)  # isolate
        self.mAssemblyHelper.genAndImmediate(aScratchRegIndex, 3)  # mstatus.mpp

    # Setup for mret to S-mode
    #
    # Manipulate privilege levels in mstatus, as follows:
    #
    #             what:                how  (copy bits...)
    #   ----------------------------   ------------------------
    #   mstatus.spp <--- mstatus.mpp   mstatus[8] = mstatus[11]
    #
    #
    #
    #   mstatus.mpp <--- 1             mstatus[12,11] = 1
    #
    #
    #  @param aMstatusTmp       - Scratch register index
    #  @param aScratchReg1Index - Scratch register index.
    #  @param aScratchReg2Index - Scratch register index.
    #
    #                            notes
    # ----------------------------------------------------------------
    # since we're in M-mode, mstatus.mpp holds the M-mode previous
    # privilege privilege level, ie, the privilege level the exception
    # was taken at. copy this value into mstatus.spp (S-mode
    # previous privilege level)
    # make the M-mode previous privilege level == S-mode, so that
    # issueing an MRET instruction will in effect, return to S-mode

    def genSetupSmodeReturn(self, aMstatusTmp, aScratchRegIndex, aScratchReg2Index):
        self.mAssemblyHelper.genReadSystemRegister(aMstatusTmp, "mstatus")  # read mstatus

        self.mAssemblyHelper.genShiftRightImmediate(aScratchRegIndex, 11, aMstatusTmp)  # isolate
        self.mAssemblyHelper.genAndImmediate(aScratchRegIndex, 3)  # mstatus.mpp,
        self.mAssemblyHelper.genShiftLeftImmediate(
            aScratchRegIndex, 8
        )  # shift left to bit 8 (mstatus.spp),
        #
        self.mAssemblyHelper.genMoveImmediate(
            aScratchReg2Index, 0x400
        )  # set mstatus.mpp = 1 (mpp will be S-mode)
        self.mAssemblyHelper.genShiftLeftImmediate(aScratchReg2Index, 1)  #
        self.mAssemblyHelper.genOrRegister(aScratchRegIndex, aScratchReg2Index)  #

        self.mAssemblyHelper.genMoveImmediate(
            aScratchReg2Index, 0x19
        )  # from mstatus current value,
        self.mAssemblyHelper.genShiftLeftImmediate(aScratchReg2Index, 8)  # clear mpp, spp fields
        self.mAssemblyHelper.genNotRegister(aScratchReg2Index)  #
        self.mAssemblyHelper.genAndRegister(aMstatusTmp, aScratchReg2Index)  #

        self.mAssemblyHelper.genOrRegister(
            aMstatusTmp, aScratchRegIndex
        )  # form updated mstatus value,
        self.mAssemblyHelper.genWriteSystemRegister(
            "mstatus", aMstatusTmp
        )  # write mstatus back...

    # Generate an instruction to copy the contents of one system register, to
    # another.
    #
    #  @param aSysRegName The name of the system register to write to.
    #  @param aSrcRegIndex The index of the register containing the value to
    #       write to the system register.
    #  @param aDestCSR         - The name of the destination system register.
    #  @param aSrcCSR          - The name of the source system register.
    #  @param aScratchRegIndex - Scratch register index.

    def copyCSR(self, aDestCSR, aSrcCSR, aScratchRegIndex):
        self.mAssemblyHelper.genReadSystemRegister(aScratchRegIndex, aSrcCSR)
        self.mAssemblyHelper.genWriteSystemRegister(aDestCSR, aScratchRegIndex)
