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
from riscv.PrivilegeLevel import PrivilegeLevelRISCV
from riscv.exception_handlers.ExceptionHandlerContext import RegisterCallRole


# using defined subroutine register usage, setup set of registers used within
# the Page Fault Exception handler and its subrodinate routines...
class PageFaultExceptionHandlerRegisters:
    def __init__(self, handler_context):
        # values for these registers are loaded by the exception dispatcher:
        priv_level_reg_index = handler_context.getScratchRegisterIndices(
            RegisterCallRole.PRIV_LEVEL_VALUE
        )
        cause_reg_index = handler_context.getScratchRegisterIndices(
            RegisterCallRole.CAUSE_VALUE
        )
        ec_code_reg_index = handler_context.getScratchRegisterIndices(
            RegisterCallRole.EC_VALUE
        )
        # subroutines are allowed two 'argument' registers, for both passing
        # arguments to subroutines, and for returning values from subroutines:
        (
            pte_addr_reg_index,
            pte_reg_index,
        ) = handler_context.getScratchRegisterIndices(
            RegisterCallRole.ARGUMENT, 2
        )
        # currently there are as many as three scratch registers allocated for
        # subroutines:
        (
            stval_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
        ) = handler_context.getScratchRegisterIndices(
            RegisterCallRole.TEMPORARY, 3
        )
        # these register will need to be saved restored by the 'top level',
        # ie, by the exception handler itself:
        (
            self.prev_priv_level_reg_index,
            self.pte_level_reg_index,
            self.scratch_reg3_index,
        ) = handler_context.getScratchRegisterIndices(
            RegisterCallRole.CALLEE_SAVED, 3
        )

        self.mUsedRegisterIndices = {
            # register index
            # privilege level fault taken at
            "priv_level": priv_level_reg_index,
            # cause register value at time of fault - indicates which
            # interrupt or fault
            "cause": cause_reg_index,
            # cause register value after interrupt bit stripped off.
            # indicates which fault type
            "ec_code": ec_code_reg_index,
            # privilege level fault was taken from
            "prev_priv_level": self.prev_priv_level_reg_index,
            # faulting address
            "stval": stval_reg_index,
            # after table walk, address of page table entry that needs
            # corrections
            "pte_addr": pte_addr_reg_index,
            #   pte value again, for pte that needs corrections
            "pte_value": pte_reg_index,
            # table-walk level where fault occurred - zero indicates fault was
            # on leaf pte
            "pte_level": self.pte_level_reg_index,
            "scratch_reg": scratch_reg_index,
            # scratch registers
            "scratch_reg2": scratch_reg2_index,
            "scratch_reg3": self.scratch_reg3_index,
            # 'piggy back' sign bit arg onto pte-addr arg
            "sign_bit": pte_addr_reg_index,
            # use 1st subroutine argument for return code too
            "rcode": pte_addr_reg_index,
        }

    def RegisterIndex(self, rname):
        try:
            return self.mUsedRegisterIndices[rname]
        except KeyError:
            raise KeyError(
                "INTERNAL ERROR: "
                "PageFaultExceptionHandlerRegisters/RegisterIndex, register "
                "'%s' not found?" % rname
            )

    def RegisterSet(self, reg_names):
        rset = []
        for rname in reg_names:
            rset.append(self.RegisterIndex(rname))
        return rset

    # return list of register (indices) for registers the handler must
    # save/restore:
    def RegistersToSave(self):
        return [
            self.prev_priv_level_reg_index,
            self.pte_level_reg_index,
            self.scratch_reg3_index,
        ]


class PageFaultExceptionHandlerRISCV(ReusableSequence):
    def generateHandler(self, **kwargs):
        self.notice(
            "[PageFaultExceptionHandlerRISC] generating 'comprehensive' page "
            "fault handler..."
        )

        try:
            handler_context = kwargs["handler_context"]
            self.mHandlerStack = handler_context.mStack
        except KeyError:
            self.error(
                "INTERNAL ERROR: one or more arguments to "
                "PageFaultHandlerRISCV generate method missing."
            )

        self.debug(
            "[PageFaultExceptionHandlerRISC] handler address: 0x%x"
            % self.getPEstate("PC")
        )

        handler_regs = PageFaultExceptionHandlerRegisters(
            handler_context
        )  # set of register (indices) this handler will use

        self.privilegeLevel = handler_context.mPrivLevel
        priv_level = PrivilegeLevelRISCV[self.privilegeLevel]

        self.mAssemblyHelper.clearLabels("PageFaultExceptionFaultHandler")

        self.mHandlerStack.newStackFrame(
            handler_regs.RegistersToSave()
        )  # top-level handler must save/restore any registers used
        # that are not defined/managed in the handler context

        # is the faulting address properly sign extended?...

        self.mAssemblyHelper.logDebugSymbol("PAGE_FAULT_HANDLER entered...")

        if self.getGlobalState("AppRegisterWidth") == 32:
            # not relevant for Sv32...
            pass
        else:
            self.callRoutine("CheckFaultAddress")  # returns 0 if successful

            self.mAssemblyHelper.logDebugSymbol(
                "Return from CheckFaultAddress."
            )

            (rcode_reg_index, scratch_reg_index) = handler_regs.RegisterSet(
                ["rcode", "scratch_reg"]
            )
            self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 0)
            self.mAssemblyHelper.genConditionalBranchToLabel(
                rcode_reg_index,
                scratch_reg_index,
                26,
                "EQ",
                "PAGE_FAULT_HANDLER_EXIT",
            )

        # faulting address seems to be okay. lets move on...

        # need to perform a table walk in order to know at what page table
        # level the fault occurred...

        handler_subroutine_generator = (
            handler_context.mMemBankHandlerRegistry.mHandlerSubroutineGenerator
        )

        self.mAssemblyHelper.logDebugSymbol("Calling TableWalk...")

        pte_level_reg_index = handler_regs.RegisterIndex("pte_level")
        ec_code_reg_index = handler_regs.RegisterIndex("ec_code")

        handler_subroutine_generator.callRoutine("TableWalk")

        self.mAssemblyHelper.genMoveRegister(
            pte_level_reg_index, ec_code_reg_index
        )

        # restore 'error code' register value:
        cause_reg_index = handler_regs.RegisterIndex("cause")
        self.mAssemblyHelper.genMoveRegister(
            ec_code_reg_index, cause_reg_index
        )

        self.mAssemblyHelper.logDebugSymbol("Return from TableWalk.")

        self.mAssemblyHelper.logDebugSymbol(
            "After TableWalk,pte level reg: x%d" % pte_level_reg_index
        )

        # after table-walk, we now know what page table level the error
        # occurred on, and thus the page-table-entry type, and can proceed
        # with identifying/correcting the fault, if possible...

        (pte_reg_index, pet_level_reg_index) = handler_regs.RegisterSet(
            ["pte_value", "pte_level"]
        )
        self.mAssemblyHelper.logDebugSymbol(
            "Calling ClearPageFault... pte reg: x%d, pte level x%d"
            % (pte_reg_index, pte_level_reg_index)
        )

        self.callRoutine("ClearPageFault")

        self.mAssemblyHelper.logDebugSymbol("Returned from ClearPageFault.")

        if self.getGlobalState("AppRegisterWidth") != 32:
            self.mAssemblyHelper.addLabel("PAGE_FAULT_HANDLER_EXIT")

        # restore 'handler-saved' registers,
        self.mHandlerStack.freeStackFrame()

        # return
        self.mAssemblyHelper.genReturn()

    # tell sequence generator about subroutine(s) this handler needs...
    # mtv 01/05/2021 - add switch to exclude CheckFaultAddress if RV32
    def getPrerequisiteRoutineNames(self, aRoutineName):
        if aRoutineName == "Handler":
            if self.getGlobalState("AppRegisterWidth") == 32:
                return ["ClearPageFault"]
            else:
                return ["CheckFaultAddress", "ClearPageFault"]
        else:
            return tuple()

    #   Generate code to check/correct page fault due to input address not
    #   correctly sign extended...

    def generateCheckFaultAddress(self, **kwargs):
        self.notice(
            "[PageFaultExceptionHandlerRISC] generating code to clear page "
            "fault condition/address sign extension..."
        )

        try:
            handler_context = kwargs["handler_context"]
        except KeyError:
            self.error(
                "INTERNAL ERROR: one or more arguments to "
                "generateCheckFaultAddress generate method missing."
            )

        # page-table, and its support subroutines, all use the same set of
        # register indices...

        handler_regs = PageFaultExceptionHandlerRegisters(
            handler_context
        )  # set of register (indices) this handler will use

        self.debug(
            "[PageFaultExceptionHandlerRISC] clear page fault code address: "
            "0x%x" % self.getPEstate("PC")
        )

        self.mAssemblyHelper.clearLabels("CheckFaultAddress")

        self.mAssemblyHelper.logDebugSymbol("CheckFaultAddress entered...")

        (
            sign_bit_reg_index,
            stval_reg_index,
            rcode_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
            addr_mask_reg_index,
            priv_level_reg_index,
        ) = handler_regs.RegisterSet(
            [
                "sign_bit",
                "stval",
                "rcode",
                "scratch_reg",
                "scratch_reg2",
                "scratch_reg3",
                "priv_level",
            ]
        )

        self.privilegeLevel = handler_context.mPrivLevel
        priv_level = PrivilegeLevelRISCV[self.privilegeLevel]

        # retreive fault address based on current privilege level...
        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 3)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, priv_level_reg_index, 6, "NE", "S PRIV"
        )
        self.mAssemblyHelper.genReadSystemRegister(stval_reg_index, "mtval")
        self.mAssemblyHelper.genRelativeBranchToLabel(4, "M PRIV")
        self.mAssemblyHelper.addLabel("S PRIV")
        self.mAssemblyHelper.genReadSystemRegister(stval_reg_index, "stval")
        self.mAssemblyHelper.addLabel("M PRIV")

        # determine where the sign bit should be... - store result in
        # sign_reg_index
        self.mAssemblyHelper.genMoveImmediate(
            sign_bit_reg_index, 1
        )  # will hold index of sign bit
        self.mAssemblyHelper.genMoveImmediate(
            addr_mask_reg_index, 0
        )  # will hold
        self.mAssemblyHelper.genNotRegister(
            addr_mask_reg_index, addr_mask_reg_index
        )  # address mask
        self.mAssemblyHelper.genReadSystemRegister(
            scratch_reg_index, "satp"
        )  # isolate
        self.mAssemblyHelper.genShiftRightImmediate(
            scratch_reg_index, 60
        )  # satp.mode field
        self.mAssemblyHelper.genMoveImmediate(
            scratch_reg2_index, 8
        )  # if xatp.mode == 8, then sv39, else sv48
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, scratch_reg2_index, 8, "NE", "SV48"
        )

        self.mAssemblyHelper.genShiftLeftImmediate(
            sign_bit_reg_index, 38
        )  # for sv39, sign bit is bit 38,
        self.mAssemblyHelper.genShiftLeftImmediate(
            addr_mask_reg_index, 38
        )  # address mask bits are 63..38
        self.mAssemblyHelper.genRelativeBranchToLabel(6, "CHECK_SIGN_BIT")

        self.mAssemblyHelper.addLabel("SV48")
        self.mAssemblyHelper.genShiftLeftImmediate(
            sign_bit_reg_index, 47
        )  # for sv48, sign bit is bit 47,
        self.mAssemblyHelper.genShiftLeftImmediate(
            addr_mask_reg_index, 47
        )  # address mask bits are 63..47

        self.mAssemblyHelper.addLabel("CHECK_SIGN_BIT")
        # check the sign bit...

        # if the sign bit is zero:
        self.genInstruction(
            "AND##RISCV",
            {
                "rd": scratch_reg_index,
                "rs1": stval_reg_index,
                "rs2": sign_bit_reg_index,
            },
        )
        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 0)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, scratch_reg2_index, 16, "NE", "CHECK_ONE"
        )

        # then the upper bits of address should also be zero...
        self.genInstruction(
            "AND##RISCV",
            {
                "rd": scratch_reg_index,
                "rs1": stval_reg_index,
                "rs2": addr_mask_reg_index,
            },
        )
        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 0)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, scratch_reg2_index, 4, "NE", "CLEAR_ADDR_BITS"
        )
        self.mAssemblyHelper.genRelativeBranchToLabel(
            28, "NO_STVAL_UPDATE"
        )  # address is okay

        # clear upper address bits...
        self.mAssemblyHelper.addLabel("CLEAR_ADDR_BITS")
        self.mAssemblyHelper.genNotRegister(
            addr_mask_reg_index, addr_mask_reg_index
        )
        self.genInstruction(
            "AND##RISCV",
            {
                "rd": stval_reg_index,
                "rs1": stval_reg_index,
                "rs2": addr_mask_reg_index,
            },
        )
        self.mAssemblyHelper.genRelativeBranchToLabel(8, "UPDATE_STVAL")

        # upper bits of address should be one...
        self.mAssemblyHelper.addLabel("CHECK_ONE")
        self.genInstruction(
            "AND##RISCV",
            {
                "rd": scratch_reg_index,
                "rs1": stval_reg_index,
                "rs2": addr_mask_reg_index,
            },
        )
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, addr_mask_reg_index, 18, "EQ", "NO_STVAL_UPDATE"
        )

        # set upper address bits...
        self.genInstruction(
            "OR##RISCV",
            {
                "rd": stval_reg_index,
                "rs1": stval_reg_index,
                "rs2": addr_mask_reg_index,
            },
        )

        # was able to correct the fault address. write corrected address,
        # clear return code...
        self.mAssemblyHelper.addLabel("UPDATE_STVAL")
        # write back fault address based on current privilege level...
        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 3)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, priv_level_reg_index, 6, "NE", "S PRIV W"
        )
        self.mAssemblyHelper.genWriteSystemRegister("mtval", stval_reg_index)
        self.mAssemblyHelper.genRelativeBranchToLabel(4, "M PRIV W")
        self.mAssemblyHelper.addLabel("S PRIV W")
        self.mAssemblyHelper.genWriteSystemRegister("stval", stval_reg_index)
        self.mAssemblyHelper.addLabel("M PRIV W")

        self.mAssemblyHelper.genMoveImmediate(rcode_reg_index, 0)
        self.mAssemblyHelper.genReturn()

        self.mAssemblyHelper.addLabel("NO_STVAL_UPDATE")
        self.mAssemblyHelper.genMoveImmediate(rcode_reg_index, 1)
        self.mAssemblyHelper.genReturn()

    # ClearPageFault
    #   Generate code to clear condition that caused Page Fault Exception...

    def generateClearPageFault(self, **kwargs):
        self.notice(
            "[PageFaultExceptionHandlerRISC] generating code to clear page"
            "fault condition..."
        )

        try:
            handler_context = kwargs["handler_context"]
        except KeyError:
            self.error(
                "INTERNAL ERROR: one or more arguments to"
                "ClearPageFaultException generate method missing."
            )

        handler_regs = PageFaultExceptionHandlerRegisters(
            handler_context
        )  # set of register (indices) this handler will use

        self.debug(
            "[PageFaultExceptionHandlerRISC] clear page fault code address: "
            "0x%x" % self.getPEstate("PC")
        )

        self.mAssemblyHelper.clearLabels("ClearPageFaultException")

        (
            priv_level_reg_index,
            ec_code_reg_index,
            pte_reg_index,
            pte_level_reg_index,
            pte_addr_reg_index,
            stval_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
            prev_priv_level_reg_index,
            scratch_reg3_index,
        ) = handler_regs.RegisterSet(
            [
                "priv_level",
                "ec_code",
                "pte_value",
                "pte_level",
                "pte_addr",
                "stval",
                "scratch_reg",
                "scratch_reg2",
                "prev_priv_level",
                "scratch_reg3",
            ]
        )

        self.privilegeLevel = handler_context.mPrivLevel
        priv_level = PrivilegeLevelRISCV[self.privilegeLevel]

        self.mAssemblyHelper.logDebugSymbol(
            "ClearPageFault entered, pte addr reg: x%d pte reg: x%d, pte"
            "level reg: x%d"
            % (pte_addr_reg_index, pte_reg_index, pte_level_reg_index)
        )

        # get faulting address...

        self.mAssemblyHelper.genReadSystemRegister(
            stval_reg_index, ("%stval" % priv_level.name.lower())
        )

        # isolate previous (the faulting) privilege level field...

        self.mAssemblyHelper.genReadSystemRegister(
            prev_priv_level_reg_index, ("%sstatus" % priv_level.name.lower())
        )
        self.mAssemblyHelper.genShiftRightImmediate(
            prev_priv_level_reg_index, 11
        )
        self.mAssemblyHelper.genAndImmediate(prev_priv_level_reg_index, 3)

        # 1st check that updates pte 'wins'...

        # V-bit clear? - if so, set it and return...

        self.mAssemblyHelper.genOrImmediate(
            scratch_reg_index, 1, pte_reg_index
        )
        self.mAssemblyHelper.genConditionalBranchToLabel(
            pte_reg_index, scratch_reg_index, 6, "EQ", "NOT_V_BIT"
        )
        # write updated pte and return...
        self.genWritePTE(pte_addr_reg_index, scratch_reg_index)
        self.mAssemblyHelper.genReturn()
        self.mAssemblyHelper.addLabel("NOT_V_BIT")

        # index > 0 and R/W/X bits clear? - table pointer pte...

        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 0)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            pte_level_reg_index, scratch_reg_index, 10, "EQ", "NOT_TABLE_PTE"
        )  # br if pte level == 0
        self.mAssemblyHelper.genAndImmediate(
            scratch_reg_index, 0xE, pte_reg_index
        )  # isolate R/W/X bits    # br if R/W/X != 0
        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 0)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, scratch_reg2_index, 4, "NE", "NOT_TABLE_PTE"
        )

        # table pte - HOW DID WE GET HERE? WE ONLY EXPECT ACCESS FAULT ON
        # TABLE PTE???

        self.mAssemblyHelper.genReturn()
        self.mAssemblyHelper.addLabel("NOT_TABLE_PTE")

        self.mAssemblyHelper.genOrImmediate(
            scratch_reg_index, 2, pte_reg_index
        )
        self.mAssemblyHelper.genConditionalBranchToLabel(
            pte_reg_index, scratch_reg_index, 24, "EQ", "NOT_R_BIT"
        )
        # write updated pte, issue sfence...
        self.genWritePTE(pte_addr_reg_index, scratch_reg_index)
        self.genSFENCE(
            stval_reg_index,
            pte_level_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
            "RBIT_CLEAR",
        )
        self.mAssemblyHelper.genReturn()

        self.mAssemblyHelper.addLabel("NOT_R_BIT")

        # Store and W-bit clear? - if so, set it, issue sfence, and return...

        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 15)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            ec_code_reg_index, scratch_reg_index, 28, "NE", "NOT_W_BIT"
        )
        self.mAssemblyHelper.genOrImmediate(
            scratch_reg_index, 4, pte_reg_index
        )
        self.mAssemblyHelper.genConditionalBranchToLabel(
            pte_reg_index, scratch_reg_index, 24, "EQ", "NOT_W_BIT"
        )
        # write updated pte, issue sfence...
        self.genWritePTE(pte_addr_reg_index, scratch_reg_index)
        self.genSFENCE(
            stval_reg_index,
            pte_level_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
            "WBIT_CLEAR",
        )
        self.mAssemblyHelper.genReturn()

        self.mAssemblyHelper.addLabel("NOT_W_BIT")

        # instr and X-bit clear? - if so, set it, issue sfence, and return...

        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 12)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            ec_code_reg_index, scratch_reg_index, 28, "NE", "NOT_X_BIT"
        )
        self.mAssemblyHelper.genOrImmediate(
            scratch_reg_index, 8, pte_reg_index
        )
        self.mAssemblyHelper.genConditionalBranchToLabel(
            pte_reg_index, scratch_reg_index, 24, "EQ", "NOT_X_BIT"
        )
        # write updated pte, issue sfence...
        self.genWritePTE(pte_addr_reg_index, scratch_reg_index)
        self.genSFENCE(
            stval_reg_index,
            pte_level_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
            "XBIT_CLEAR",
        )
        self.mAssemblyHelper.genReturn()

        self.mAssemblyHelper.addLabel("NOT_X_BIT")

        # user-mode and U-bit clear? - if so, set it, issue sfence, and return

        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 0)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            prev_priv_level_reg_index,
            scratch_reg_index,
            28,
            "NE",
            "NOT_UBIT_USER_MODE",
        )
        self.mAssemblyHelper.genOrImmediate(
            scratch_reg_index, 0x10, pte_reg_index
        )
        self.mAssemblyHelper.genConditionalBranchToLabel(
            pte_reg_index, scratch_reg_index, 24, "EQ", "NOT_U_BIT_USER_MODE"
        )
        # write updated pte, issue sfence...
        self.genWritePTE(pte_addr_reg_index, scratch_reg_index)
        self.genSFENCE(
            stval_reg_index,
            pte_level_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
            "UBIT_CLEAR",
        )
        self.mAssemblyHelper.genReturn()

        self.mAssemblyHelper.addLabel("NOT_UBIT_USER_MODE")

        # is A-bit clear? - if so, set it and return...

        self.mAssemblyHelper.genOrImmediate(
            scratch_reg_index, 0x40, pte_reg_index
        )
        self.mAssemblyHelper.genConditionalBranchToLabel(
            pte_reg_index, scratch_reg_index, 24, "EQ", "NOT_A_BIT"
        )
        # write updated pte, issue sfence...
        self.genWritePTE(pte_addr_reg_index, scratch_reg_index)
        self.genSFENCE(
            stval_reg_index,
            pte_level_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
            "ABIT_CLEAR",
        )
        self.mAssemblyHelper.genReturn()
        self.mAssemblyHelper.addLabel("NOT_A_BIT")

        # is Store and D-bit clear? - if so, set it and return...

        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 15)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            ec_code_reg_index, scratch_reg_index, 28, "NE", "NOT_D_BIT"
        )
        self.mAssemblyHelper.genOrImmediate(
            scratch_reg_index, 0x80, pte_reg_index
        )
        self.mAssemblyHelper.genConditionalBranchToLabel(
            pte_reg_index, scratch_reg_index, 24, "EQ", "NOT_D_BIT"
        )
        # write updated pte, issue sfence...
        self.genWritePTE(pte_addr_reg_index, scratch_reg_index)
        self.genSFENCE(
            stval_reg_index,
            pte_level_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
            "DBIT_CLEAR",
        )
        self.mAssemblyHelper.genReturn()
        self.mAssemblyHelper.addLabel("NOT_D_BIT")

        # supervisor-mode, load/store, U-bit set, sstatus.sum bit clear?...

        #    supervisor mode?...
        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 1)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            prev_priv_level_reg_index,
            scratch_reg_index,
            62,
            "NE",
            "NOT_UBIT_SUPER_MODE",
        )

        #    instr?...
        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 12)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            ec_code_reg_index,
            scratch_reg_index,
            30,
            "NE",
            "NOT_UBIT_SUPER_MODE_INSTR",
        )

        #    instr, U bit set?...
        self.mAssemblyHelper.genOrImmediate(
            scratch_reg_index, 0x10, pte_reg_index
        )
        self.mAssemblyHelper.genConditionalBranchToLabel(
            pte_reg_index, scratch_reg_index, 54, "NE", "NOT_UBIT_SUPER_MODE"
        )
        self.mAssemblyHelper.genXorImmediate(
            scratch_reg_index, 0x10, pte_reg_index
        )

        # write updated pte (with U-bit clear), issue sfence...
        self.genWritePTE(pte_addr_reg_index, scratch_reg_index)
        self.genSFENCE(
            stval_reg_index,
            pte_level_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
            "UBIT_SET_INSTR",
        )
        self.mAssemblyHelper.genReturn()

        # must be load/store...
        self.mAssemblyHelper.addLabel("NOT_UBIT_SUPER_MODE_INSTR")
        #    U bit set?...
        self.mAssemblyHelper.genOrImmediate(
            scratch_reg_index, 0x10, pte_reg_index
        )
        self.mAssemblyHelper.genConditionalBranchToLabel(
            pte_reg_index, scratch_reg_index, 26, "NE", "NOT_UBIT_SUPER_MODE"
        )
        self.mAssemblyHelper.genXorImmediate(
            scratch_reg_index, 0x10, pte_reg_index
        )
        # write updated pte (with U-bit clear), issue sfence...
        self.genWritePTE(pte_addr_reg_index, scratch_reg_index)
        self.genSFENCE(
            stval_reg_index,
            pte_level_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
            "UBIT_SET_DATA",
        )
        self.mAssemblyHelper.genReturn()

        self.mAssemblyHelper.addLabel("NOT_UBIT_SUPER_MODE")

    # superpage table address misaligned?...

    # pte level > 0?...
    self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 0)
    self.mAssemblyHelper.genConditionalBranchToLabel(
        pte_level_reg_index,
        scratch_reg_index,
        34,
        "EQ",
        "NOT_SUPER_PAGE_FIXUP",
    )

    # use pte level to generate superpage address offset mask...

    # Sv32:       ppn[0] is bits 19..10
    # Sv39, Sv48: ppn[0] is bits 18..10
    if self.getGlobalState("AppRegisterWidth") == 32:
        self.mAssemblyHelper.genMoveImmediate(
            scratch_reg2_index, 0x3FF
        )  # for Sv32, each pte.ppn field is ten bits
    else:
        self.mAssemblyHelper.genMoveImmediate(
            scratch_reg2_index, 0x1FF
        )  # else each pte.ppn field is nine bits
    self.mAssemblyHelper.genShiftLeftImmediate(
        scratch_reg2_index, 10, scratch_reg2_index
    )
    self.mAssemblyHelper.genMoveRegister(scratch_reg_index, scratch_reg2_index)
    self.mAssemblyHelper.genMoveImmediate(scratch_reg3_index, 1)
    self.mAssemblyHelper.genConditionalBranchToLabel(
        pte_level_reg_index,
        scratch_reg3_index,
        14,
        "EQ",
        "SUPER_PAGE_FIXUP",
    )

    # Sv39, Sv48: ppn[1] is bits 27..19
    self.mAssemblyHelper.genShiftLeftImmediate(scratch_reg2_index, 9)
    self.mAssemblyHelper.genOrRegister(
        scratch_reg_index, scratch_reg_index, scratch_reg2_index
    )
    self.mAssemblyHelper.genMoveImmediate(scratch_reg3_index, 2)
    self.mAssemblyHelper.genConditionalBranchToLabel(
        pte_level_reg_index,
        scratch_reg3_index,
        6,
        "EQ",
        "SUPER_PAGE_FIXUP",
    )

    # Sv48: ppn[2] is bits 36..28
    self.mAssemblyHelper.genShiftLeftImmediate(scratch_reg2_index, 9)
    self.mAssemblyHelper.genOrRegister(
        scratch_reg_index, scratch_reg_index, scratch_reg2_index
    )

    # is the (superpage) pte indeed mis-aligned?...
    self.mAssemblyHelper.addLabel("SUPER_PAGE_FIXUP")
    self.mAssemblyHelper.genNotRegister(
        scratch_reg_index
    )  # mask of all pte bits but level - 1 PPNs
    self.mAssemblyHelper.genAndRegister(
        scratch_reg_index, pte_reg_index, scratch_reg_index
    )
    self.mAssemblyHelper.genConditionalBranchToLabel(
        pte_reg_index, scratch_reg_index, 6, "EQ", "NOT_SUPER_PAGE_FIXUP"
    )
    # write updated pte...
    self.genWritePTE(pte_addr_reg_index, scratch_reg_index)
    self.mAssemblyHelper.genReturn()

    self.mAssemblyHelper.addLabel("NOT_SUPER_PAGE_FIXUP")

    # ERROR IF UNABLE TO CORRECT FAULT...
    self.mAssemblyHelper.genReturn()


# generate code to write PTE to memory...


def genWritePTE(self, pte_addr_reg_index, pte_value_reg_index):
    if self.getGlobalState("AppRegisterWidth") == 32:
        self.genInstruction(
            "SW##RISCV",
            {
                "rs1": pte_addr_reg_index,
                "rs2": pte_value_reg_index,
                "simm12": 0,
                "NoRestriction": 1,
            },
        )
    else:
        self.genInstruction(
            "SD##RISCV",
            {
                "rs1": pte_addr_reg_index,
                "rs2": pte_value_reg_index,
                "simm12": 0,
                "NoRestriction": 1,
            },
        )


# generate code to setup/issue sfence.vma instruction, to in effect cause
# a tlb flush for a faulting address. we ASSUME a leaf PTE


def genSFENCE(
    self,
    vaddr_reg_index,
    pte_value_reg_index,
    scratch_reg_index,
    scratch_reg2_index,
    label_prefix,
):
    # global bit is bit 5 of pte value...
    self.mAssemblyHelper.genAndImmediate(
        scratch_reg_index, 0x20, pte_value_reg_index
    )
    self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 0x20)
    self.mAssemblyHelper.genConditionalBranchToLabel(
        scratch_reg_index,
        scratch_reg2_index,
        6,
        "NE",
        "%s_SFENCE_NON_GLOBAL" % label_prefix,
    )

    # flush tlb entry - global...
    self.genInstruction(
        "SFENCE.VMA##RISCV",
        {"rs1": vaddr_reg_index, "rs2": 0, "NoRestriction": 1},
    )
    self.mAssemblyHelper.genRelativeBranchToLabel(
        10, "%s_SFENCE_EXIT" % label_prefix
    )

    self.mAssemblyHelper.addLabel("%s_SFENCE_NON_GLOBAL" % label_prefix)

    # flush tlb entry - non-global so use asid...

    self.mAssemblyHelper.genReadSystemRegister(scratch_reg_index, "satp")

    # isolate satp.asid field...

    if self.getGlobalState("AppRegisterWidth") == 32:
        self.mAssemblyHelper.genShiftLeftImmediate(scratch_reg_index, 1)
        self.mAssemblyHelper.genShiftRightImmediate(scratch_reg_index, 23)
    else:
        self.mAssemblyHelper.genShiftLeftImmediate(scratch_reg_index, 4)
        self.mAssemblyHelper.genShiftRightImmediate(scratch_reg_index, 48)

    self.genInstruction(
        "SFENCE.VMA##RISCV",
        {
            "rs1": vaddr_reg_index,
            "rs2": scratch_reg_index,
            "NoRestriction": 1,
        },
    )

    self.mAssemblyHelper.addLabel("%s_SFENCE_EXIT" % label_prefix)
