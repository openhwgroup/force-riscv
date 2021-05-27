#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
# OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from base.exception_handlers.ReusableSequence import ReusableSequence
from riscv.exception_handlers.ExceptionHandlerContext import RegisterCallRole
from riscv.exception_handlers.ExceptionHandlerStack import (
    ExceptionHandlerStackRISCV,
)
from riscv.PrivilegeLevel import PrivilegeLevelRISCV
from riscv.Utils import LoopControl


# using defined subroutine register usage, setup set of registers used
# within the Access Exception handler and its subordinate routines...
class AccessExceptionHandlerRegisters:
    def __init__(self, handler_context):
        # at handler entry, values for these first three registers are
        # already loaded, by the exception dispatcher:
        priv_level_reg_index = handler_context.getScratchRegisterIndices(
            RegisterCallRole.PRIV_LEVEL_VALUE
        )
        cause_reg_index = handler_context.getScratchRegisterIndices(RegisterCallRole.CAUSE_VALUE)
        ec_code_reg_index = handler_context.getScratchRegisterIndices(RegisterCallRole.EC_VALUE, 1)
        # these two registers are used for subroutine arguments:
        (
            pmp_entry_reg_index,
            prev_addrlo_reg_index,
        ) = handler_context.getScratchRegisterIndices(RegisterCallRole.ARGUMENT, 2)

        rcode_reg_index = pmp_entry_reg_index  # use 1st subroutine argument,
        # for return code too

        # three scratch registers are allocated for subroutines:
        (
            fault_addr_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
        ) = handler_context.getScratchRegisterIndices(RegisterCallRole.TEMPORARY, 3)

        # these register will need to be saved restored by the 'top level',
        # ie, by the exception handler itself:
        self.mSaveRegs = handler_context.getScratchRegisterIndices(
            RegisterCallRole.CALLEE_SAVED, 9
        )

        self.mUsedRegisterIndices = {
            "priv_level": priv_level_reg_index,
            "cause": cause_reg_index,
            "ec_code": ec_code_reg_index,
            "pmp_index": pmp_entry_reg_index,
            "prev_addrlo": prev_addrlo_reg_index,
            "rcode": rcode_reg_index,
            "fault_addr": fault_addr_reg_index,
            "scratch_reg": scratch_reg_index,
            "scratch_reg2": scratch_reg2_index,
            "pmpcfg": self.mSaveRegs[0],
            "pmp_entry_value": self.mSaveRegs[1],
            "cfgval": self.mSaveRegs[2],
            "addr_lo": self.mSaveRegs[3],
            "addr_hi": self.mSaveRegs[4],
            "scratch_reg3": self.mSaveRegs[5],
            "scratch_reg4": self.mSaveRegs[6],
            "scratch_reg5": self.mSaveRegs[7],
            "scratch_reg6": self.mSaveRegs[8],
        }

    def RegisterIndex(self, rname):
        try:
            return self.mUsedRegisterIndices[rname]
        except KeyError:
            self.error(
                "INTERNAL ERROR: AccessExceptionHandlerRegisters"
                "/RegisterIndex, register '%s' not found?" % rname
            )

    def RegisterSet(self, reg_names):
        rset = []
        for rname in reg_names:
            rset.append(self.RegisterIndex(rname))
        return rset

    # return list of register (indices) for registers the handler must
    # save/restore:
    def RegistersToSave(self):
        return self.mSaveRegs


# Access Fault exception handler


class AccessExceptionHandlerRISCV(ReusableSequence):
    def __init__(self, aGenThread, aFactory, aStack):
        super().__init__(aGenThread, aFactory, aStack)
        self.mHandlerStack = None
        self.privilegeLevel = None

    def generateHandler(self, **kwargs):
        try:
            handler_context = kwargs["handler_context"]
            self.mHandlerStack = handler_context.mStack
        except KeyError:
            self.error(
                "INTERNAL ERROR: one or more arguments to "
                "AccessExceptionHandlerRISCV generate method missing."
            )

        self.debug(
            "[AccessExceptionHandlerRISCV] generate handler address: 0x%x" % self.getPEstate("PC")
        )

        handler_regs = AccessExceptionHandlerRegisters(
            handler_context
        )  # set of register (indices) this handler will use

        self.mAssemblyHelper.clearLabels("AccessExceptionHandlerRISCV")

        self.mHandlerStack.newStackFrame(
            handler_regs.RegistersToSave()
        )  # top-level handler must save/restore any registers used
        # that are not defined/managed in the handler context

        # call access exceptions 'main' subroutine here...

        if self.hasGeneratedRoutine("ClearAccessFault"):
            self.callRoutine("ClearAccessFault")
        else:
            self.error("INTERNAL ERROR: ClearAccessFault subroutine has NOT generated?")

        # restore 'handler-saved' registers
        self.mHandlerStack.freeStackFrame()

        # check return code, skip instruction that caused the exception on
        # non-zero return code...
        (
            rcode_reg_index,
            scratch_reg_index,
            priv_level_reg_index,
        ) = handler_regs.RegisterSet(["rcode", "scratch_reg", "priv_level"])

        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 0)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            rcode_reg_index, scratch_reg_index, 20, "EQ", "HNDLR_EXIT"
        )

        # failed to clear condition that caused the exception...

        self.mAssemblyHelper.genIncrementExceptionReturnAddress(
            scratch_reg_index, priv_level_reg_index
        )

        self.mAssemblyHelper.addLabel("HNDLR_EXIT")
        self.mAssemblyHelper.genReturn()

    def getPrerequisiteRoutineNames(self, aRoutineName):
        self.notice("[getPrerequisiteRoutineNames] routine-name: '%s'" % aRoutineName)
        if aRoutineName == "Handler":
            return ["ClearAccessFault"]
        elif aRoutineName == "ClearAccessFault":
            return ["PMPfixup", "GetNAPOTaddrRange", "FixPMPentry"]
        else:
            return tuple()

    def generateClearAccessFault(self, **kwargs):
        self.notice("[generateClearAccessFault] entered...")
        try:
            handler_context = kwargs["handler_context"]
        except KeyError:
            self.error(
                "INTERNAL ERROR: one or more arguments to "
                "generateClearAccessFault generate method missing."
            )

        handler_regs = AccessExceptionHandlerRegisters(
            handler_context
        )  # set of register (indices) this handler will use

        self.mAssemblyHelper.clearLabels("ClearAccessException")

        (
            rcode_reg_index,
            ec_code_reg_index,
            priv_level_reg_index,
            fault_addr_reg_index,
            pmp_reg_index,
            pmp_entry_value_reg_index,
            prev_addrlo_reg_index,
            scratch_reg_index,
        ) = handler_regs.RegisterSet(
            [
                "rcode",
                "ec_code",
                "priv_level",
                "fault_addr",
                "pmp_index",
                "pmp_entry_value",
                "prev_addrlo",
                "scratch_reg",
            ]
        )

        self.privilegeLevel = handler_context.mPrivLevel
        priv_level = PrivilegeLevelRISCV[self.privilegeLevel]

        # get faulting address...

        self.mAssemblyHelper.genReadSystemRegister(
            fault_addr_reg_index, ("%stval" % priv_level.name.lower())
        )

        self.mAssemblyHelper.logDebugSymbol("Current privilege level: %s" % priv_level.name)

        # if not at machine level, then bail...

        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 3)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            priv_level_reg_index, scratch_reg_index, 6, "EQ", "IN_MACHINE_MODE"
        )
        self.mAssemblyHelper.genMoveImmediate(
            rcode_reg_index, 1
        )  # non-zero return code indicates failure
        self.mAssemblyHelper.genReturn()

        self.mAssemblyHelper.addLabel("IN_MACHINE_MODE")

        # locate/fixup matching PMP entry or bail - 1st fixup 'wins'...

        # for TOR mode (assuming there are one or more entries config'd
        # with Tor mode), 'starting low address (previous TOR mode entry
        # high address)' is zero
        self.mAssemblyHelper.genMoveImmediate(prev_addrlo_reg_index, 0)

        for i in range(16):
            case_label = "CHECK_PMP_ENTRY_%d" % (i + 1)
            lock_label = "PMP_ENTRY_LOCKED_%d" % (i + 1)
            self.mAssemblyHelper.genMoveImmediate(pmp_reg_index, i)
            self.mAssemblyHelper.genReadSystemRegister(
                pmp_entry_value_reg_index, ("pmpaddr%d" % i)
            )
            self.callRoutine("PMPfixup")
            # zero return code indicates success...
            self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 0)
            self.mAssemblyHelper.genConditionalBranchToLabel(
                rcode_reg_index, scratch_reg_index, 4, "NE", case_label
            )
            self.mAssemblyHelper.genReturn()
            self.mAssemblyHelper.addLabel(case_label)
            # return code == 2 indicates a matching pmp entry was found,
            # but it was locked, so no need to continue...
            self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 2)
            self.mAssemblyHelper.genConditionalBranchToLabel(
                rcode_reg_index, scratch_reg_index, 4, "NE", lock_label
            )
            self.mAssemblyHelper.genReturn()
            self.mAssemblyHelper.addLabel(lock_label)

        # failed to locate suitable PMP entry...

        self.mAssemblyHelper.genMoveImmediate(rcode_reg_index, 1)
        self.mAssemblyHelper.genReturn()

    def generatePMPfixup(self, **kwargs):
        """
        for indexed PMP entry, see if address range covers
        the faulting address. If so, attempt to update
        the PMP entry permissions to allow the access.

        pmp_reg_index          - pmp index 0-15
        prev_addrlo_reg_index  - for TOR mode, holds address high value for
                                 previous PMP entry (used as address lo
                                 value, this PMP entry)
        fault_addr_reg_index   - faulting address value

        return code of zero indicates success.
        """
        self.notice("[generatePMPfixup] entered...")
        try:
            handler_context = kwargs["handler_context"]
        except KeyError:
            self.error(
                "INTERNAL ERROR: one or more arguments to generatePMPfixup() method missing."
            )

        handler_regs = AccessExceptionHandlerRegisters(
            handler_context
        )  # set of register (indices) this handler will use

        (
            rcode_reg_index,
            ec_code_reg_index,
            priv_level_reg_index,
            fault_addr_reg_index,
            pmp_reg_index,
            prev_addrlo_reg_index,
            scratch_reg_index,
        ) = handler_regs.RegisterSet(
            [
                "rcode",
                "ec_code",
                "priv_level",
                "fault_addr",
                "pmp_index",
                "prev_addrlo",
                "scratch_reg",
            ]
        )

        # subroutine arguments...
        (
            fault_addr_reg_index,
            pmp_reg_index,
            prev_addrlo_reg_index,
        ) = handler_regs.RegisterSet(["fault_addr", "pmp_index", "prev_addrlo"])

        # more reg indices...
        (
            pmpcfg_reg_index,
            cfgval_reg_index,
            pmp_entry_value_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
        ) = handler_regs.RegisterSet(
            [
                "pmpcfg",
                "cfgval",
                "pmp_entry_value",
                "scratch_reg",
                "scratch_reg2",
            ]
        )

        # pmp entry index...

        self.mAssemblyHelper.clearLabels("PMPfixup")

        # return code register...
        rcode_reg_index = handler_regs.RegisterIndex("rcode")

        self.mAssemblyHelper.genMoveImmediate(
            rcode_reg_index, 1
        )  # will clear return code if successful

        # read pmpcfg reg based on index, isolate pmpcfg[i]...

        self.genGetIndexedPMPcfg(
            cfgval_reg_index,
            pmp_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
        )

        # isolate Mode bits...
        self.mAssemblyHelper.genShiftRightImmediate(scratch_reg_index, 3, cfgval_reg_index)
        self.mAssemblyHelper.genAndImmediate(scratch_reg_index, 3)

        # ASSUME: While a particular PMP entry R,W,X values may not be
        #         correct, the Mode and address ranges, have been setup
        #         correctly.  The handler cannot be responsible for
        #         randomizing physical memory address ranges.  The handler
        #         cannot determine (easily) what mode was intended.

        # ASSUME: All bytes of a data access will fall within a correctly
        #         configured PMP entries, ie, the handler will check only
        #         that the starting address, ie, the faulting address,
        #         falls within a valid PMP entry.

        # if mode is 0 (PMP entry is disabled), then bail...
        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 0)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, scratch_reg2_index, 4, "NE", "TRY_TOR_MODE"
        )
        self.mAssemblyHelper.genReturn()  # this pmp entry IS disabled

        # TOR mode?...
        self.mAssemblyHelper.addLabel("TRY_TOR_MODE")
        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 1)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, scratch_reg2_index, 24, "NE", "NOT_TOR_MODE"
        )
        # check address range for this mode...
        # save previous tor 'address lo' in a scratch register.
        # update address lo value...
        self.mAssemblyHelper.genMoveRegister(scratch_reg_index, prev_addrlo_reg_index)
        self.mAssemblyHelper.genMoveRegister(prev_addrlo_reg_index, pmp_entry_value_reg_index)
        # TOR address lo (previous TOR PMP entry address) <= faulting
        # address <= this TOR PMP entry address
        self.mAssemblyHelper.genConditionalBranchToLabel(
            fault_addr_reg_index,
            scratch_reg_index,
            16,
            "LT",
            "TOR_MODE_RETURN",
        )  # fault addr < previous lo addr
        self.mAssemblyHelper.genConditionalBranchToLabel(
            prev_addrlo_reg_index,
            fault_addr_reg_index,
            14,
            "LT",
            "TOR_MODE_RETURN",
        )  # hi addr < fault addr
        # address range match. fixup R/W/X depending on access type...
        self.callRoutine("FixPMPentry")  # fix the entry,
        self.mAssemblyHelper.genMoveImmediate(
            rcode_reg_index, 0
        )  # match: fault address is within this TOR mode entry address range

        self.mAssemblyHelper.addLabel("TOR_MODE_RETURN")
        self.mAssemblyHelper.genReturn()

        self.mAssemblyHelper.addLabel("NOT_TOR_MODE")

        # NA4 mode?...
        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 2)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, scratch_reg2_index, 24, "NE", "NOT_NA4_MODE"
        )
        # check address range for this mode - NA4 is four byte address range.
        # use indexed pmpaddr value, shifted left two bits as base address
        self.mAssemblyHelper.genShiftLeftImmediate(
            scratch_reg_index, pmp_entry_value_reg_index, 2
        )  # form addr lo value
        self.mAssemblyHelper.genConditionalBranchToLabel(
            fault_addr_reg_index,
            scratch_reg_index,
            18,
            "LT",
            "NA4_MODE_RETURN",
        )
        self.mAssemblyHelper.genAddImmediate(
            scratch_reg_index, 3
        )  # add max offset to form addr hi value
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index,
            fault_addr_reg_index,
            14,
            "LT",
            "NA4_MODE_RETURN",
        )
        # address range match. fixup R/W/X depending on access type...
        self.callRoutine("FixPMPentry")  # fix the entry,
        self.mAssemblyHelper.genMoveImmediate(
            rcode_reg_index, 0
        )  # match: fault address is within this NA4 mode entry address range

        self.mAssemblyHelper.addLabel("NA4_MODE_RETURN")
        self.mAssemblyHelper.genReturn()

        self.mAssemblyHelper.addLabel("NOT_NA4_MODE")

        # must be NAPOT mode...

        # check address range for NAPOT mode...

        # address range lo/hi returned in addr_lo_reg_index,addr_hi_reg_index
        # scratch_reg1_index,scratch_reg2_index - used by GetNAPOTaddrRange

        self.callRoutine("GetNAPOTaddrRange")  # get address range, NAPOT mode

        (addr_lo_reg_index, addr_hi_reg_index) = handler_regs.RegisterSet(["addr_lo", "addr_hi"])

        # address range hi will be zero on error...
        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 0)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            addr_hi_reg_index, scratch_reg_index, 18, "NE", "NAPOT_MODE_RETURN"
        )

        self.mAssemblyHelper.genConditionalBranchToLabel(
            fault_addr_reg_index,
            addr_lo_reg_index,
            16,
            "LT",
            "NAPOT_MODE_RETURN",
        )
        self.mAssemblyHelper.genConditionalBranchToLabel(
            addr_hi_reg_index,
            fault_addr_reg_index,
            14,
            "LT",
            "NAPOT_MODE_RETURN",
        )

        # match: fault address is within this NAPOT mode entry address range
        self.callRoutine("FixPMPentry")  # fix the entry,
        self.mAssemblyHelper.genMoveImmediate(rcode_reg_index, 0)  # clear return code,

        self.mAssemblyHelper.addLabel("NAPOT_MODE_RETURN")
        self.mAssemblyHelper.genReturn()

    # read pmpcfg reg based on index, isolate pmpcfg[i]...

    def genGetIndexedPMPcfg(
        self,
        cfgval_reg_index,
        pmp_reg_index,
        scratch_reg_index,
        scratch_reg2_index,
    ):
        self.mAssemblyHelper.genMoveRegister(
            scratch_reg_index, pmp_reg_index
        )  # will use scratch reg to index into pmpcfg
        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 8)
        self.mAssemblyHelper.genConditionalBranch(pmp_reg_index, scratch_reg2_index, 6, "GE")
        # if pmp_index < 8:
        self.mAssemblyHelper.genReadSystemRegister(cfgval_reg_index, "pmpcfg0")
        self.mAssemblyHelper.genRelativeBranch(12)
        # else:
        self.mAssemblyHelper.genReadSystemRegister(cfgval_reg_index, "pmpcfg2")
        self.mAssemblyHelper.genShiftRightImmediate(
            scratch_reg_index, 1
        )  # when pmp-index >= 8, divide by 2

        self.mAssemblyHelper.genShiftLeftImmediate(scratch_reg_index, 3)  # multiply by 8
        self.genShiftRightRegister(
            cfgval_reg_index, cfgval_reg_index, scratch_reg_index
        )  # shift pmpcfg val right to indexed cfg
        self.mAssemblyHelper.genAndImmediate(cfgval_reg_index, 0xFF)  # isolate indexed cfg

    def genShiftLeftRegister(self, aDestRegIndex, aSrcRegIndex1, aSrcRegIndex2):
        self.genInstruction(
            "SLL##RISCV",
            {"rd": aDestRegIndex, "rs1": aSrcRegIndex1, "rs2": aSrcRegIndex2},
        )

    def genShiftRightRegister(self, aDestRegIndex, aSrcRegIndex1, aSrcRegIndex2):
        self.genInstruction(
            "SRL##RISCV",
            {"rd": aDestRegIndex, "rs1": aSrcRegIndex1, "rs2": aSrcRegIndex2},
        )

    def genXorRegister(self, aDestRegIndex, aSrcRegIndex1, aSrcRegIndex2):
        self.genInstruction(
            "XOR##RISCV",
            {"rd": aDestRegIndex, "rs1": aSrcRegIndex1, "rs2": aSrcRegIndex2},
        )

    def generateGetNAPOTaddrRange(self, **kwargs):
        """for NAPOT mode, form address range..."""
        self.notice("[generateGetNAPOTaddrRange] entered...")
        try:
            handler_context = kwargs["handler_context"]
        except KeyError:
            self.error(
                "INTERNAL ERROR: one or more arguments to "
                "generateGetNAPOTaddrRange generate method missing."
            )

        handler_regs = AccessExceptionHandlerRegisters(
            handler_context
        )  # set of register (indices) this handler will use

        (
            fault_addr_reg_index,
            pmp_reg_index,
            pmp_entry_value_reg_index,
        ) = handler_regs.RegisterSet(["fault_addr", "pmp_index", "pmp_entry_value"])

        (
            addr_lo_reg_index,
            addr_hi_reg_index,
            scratch_reg1_index,
            scratch_reg2_index,
        ) = handler_regs.RegisterSet(["addr_lo", "addr_hi", "scratch_reg", "scratch_reg2"])

        (
            offset_reg_index,
            zero_bit_reg_index,
            addr_base_reg_index,
            loop_reg_index,
        ) = handler_regs.RegisterSet(
            ["scratch_reg3", "scratch_reg4", "scratch_reg5", "scratch_reg6"]
        )

        self.mAssemblyHelper.genMoveImmediate(
            addr_hi_reg_index, 0
        )  # addr_hi_reg value will be zero if fault address not in this
        # address range

        self.mAssemblyHelper.genMoveImmediate(
            offset_reg_index, 7
        )  # smallest offset range is 2**8 - 1
        self.mAssemblyHelper.genMoveImmediate(zero_bit_reg_index, 1)  # zero bit starts here
        self.mAssemblyHelper.genMoveRegister(
            addr_base_reg_index, pmp_entry_value_reg_index
        )  # form starting addr_base from pmpAddrX,
        self.mAssemblyHelper.genShiftLeftImmediate(
            addr_base_reg_index, 2
        )  # shifted left by 2 (minimal offset is 2**8 - 1)

        # for 64 bit address, with minimal granule size of 3 bits...
        # loop:
        self.mAssemblyHelper.genMoveImmediate(loop_reg_index, 60)  # loop 60 times max

        # break if addr_base & zero_bit == 0:
        self.mAssemblyHelper.genAndRegister(
            scratch_reg1_index, zero_bit_reg_index, addr_base_reg_index
        )
        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 0)
        self.mAssemblyHelper.genConditionalBranch(scratch_reg1_index, scratch_reg2_index, 18, "EQ")

        self.mAssemblyHelper.genShiftLeftImmediate(zero_bit_reg_index, 1)  # zerobit<<1
        self.mAssemblyHelper.genShiftLeftImmediate(offset_reg_index, 1)  # offset =
        self.mAssemblyHelper.genAddImmediate(offset_reg_index, 1)  # (offset<<1)+1
        self.mAssemblyHelper.genShiftLeftImmediate(addr_base_reg_index, 1)  # addr_base << 1

        self.mAssemblyHelper.genSubRegister(loop_reg_index, 1)
        self.mAssemblyHelper.genConditionalBranch(loop_reg_index, scratch_reg2_index, -16, "NE")
        # loop ends

        # if we get here, then error...
        self.mAssemblyHelper.genMoveImmediate(addr_hi_reg_index, 1)  # set address hi to zero
        self.mAssemblyHelper.genReturn()  # for fail...

        self.mAssemblyHelper.genConditionalBranch(scratch_reg1_index, scratch_reg2_index, 12, "NE")

        # but if we get here, then located the zero bit and can correctly
        # form address range...

        # addr_lo = addr_base & not offset_rval:
        self.mAssemblyHelper.genMoveImmediate(scratch_reg1_index, 0)  # load scratch reg
        self.mAssemblyHelper.genNotRegister(
            scratch_reg1_index, scratch_reg1_index
        )  # with all ones,
        self.genXorRegister(
            scratch_reg1_index, scratch_reg1_index, offset_reg_index
        )  # xor with offset,
        self.mAssemblyHelper.genAndRegister(
            addr_lo_reg_index, addr_base_reg_index, scratch_reg1_index
        )  # then use to strip off low order bits to yield addr base lo
        # addr_hi = addr_lo + offset_rval:
        self.mAssemblyHelper.genAndRegister(
            addr_hi_reg_index, addr_base_reg_index, offset_reg_index
        )
        self.mAssemblyHelper.genReturn()

    def generateFixPMPentry(self, **kwargs):
        """address range match. fixup A/R/W/X depending on access type..."""
        self.notice("[generateFixPMPentry] entered...")
        try:
            handler_context = kwargs["handler_context"]
        except KeyError:
            self.error(
                "INTERNAL ERROR: one or more arguments to generateFixPMPentry"
                "generate method missing."
            )

        handler_regs = AccessExceptionHandlerRegisters(
            handler_context
        )  # set of register (indices) this handler will use

        (
            ec_code_reg_index,
            fault_addr_reg_index,
            pmp_reg_index,
            rcode_reg_index,
        ) = handler_regs.RegisterSet(["ec_code", "fault_addr", "pmp_index", "rcode"])

        (
            pmpcfg_reg_index,
            cfgval_reg_index,
            scratch_reg_index,
            scratch_reg2_index,
        ) = handler_regs.RegisterSet(["pmpcfg", "cfgval", "scratch_reg", "scratch_reg2"])

        # if PMP entry is locked, then cannot change...
        self.mAssemblyHelper.genAndImmediate(
            scratch_reg_index, 0x80, cfgval_reg_index
        )  # bit 7 is Lock bit.
        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 0x80)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            scratch_reg_index, scratch_reg2_index, 6, "NE", "ENTRY_NOT_LOCKED"
        )
        self.mAssemblyHelper.genMoveImmediate(
            rcode_reg_index, 2
        )  # set rcode to indicate that a matching pmp entry is locked.
        self.mAssemblyHelper.genReturn()  # entry WAS locked

        self.mAssemblyHelper.addLabel("ENTRY_NOT_LOCKED")

        # check the R/W/X bits...

        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 1)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            ec_code_reg_index, scratch_reg_index, 6, "NE", "NOT_INSTR"
        )
        self.mAssemblyHelper.genOrImmediate(cfgval_reg_index, 4)  # set X bit
        self.mAssemblyHelper.genRelativeBranchToLabel(12, "FORMAT_WRITE_PMP_ENTRY")
        self.mAssemblyHelper.addLabel("NOT_INSTR")
        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 5)  # set R bit
        self.mAssemblyHelper.genConditionalBranchToLabel(
            ec_code_reg_index, scratch_reg_index, 6, "NE", "NOT_LOAD"
        )
        self.mAssemblyHelper.genOrImmediate(cfgval_reg_index, 1)
        self.mAssemblyHelper.genRelativeBranchToLabel(4, "FORMAT_WRITE_PMP_ENTRY")
        self.mAssemblyHelper.addLabel("NOT_LOAD")

        self.mAssemblyHelper.genOrImmediate(cfgval_reg_index, 3)  # set R and W bits for store

        self.mAssemblyHelper.addLabel("FORMAT_WRITE_PMP_ENTRY")

        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 0xFF)
        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 8)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            pmp_reg_index, scratch_reg2_index, 8, "GE", "POSITION_CFG"
        )
        # if pmp_index < 8:
        self.mAssemblyHelper.genShiftLeftImmediate(scratch_reg2_index, 8, pmp_reg_index)
        self.mAssemblyHelper.genRelativeBranchToLabel(4, "POSITION_CFG")
        # else:
        self.mAssemblyHelper.genShiftLeftImmediate(scratch_reg2_index, 4, pmp_reg_index)

        self.mAssemblyHelper.addLabel("POSITION_CFG")
        self.genShiftLeftRegister(
            cfgval_reg_index, cfgval_reg_index, scratch_reg2_index
        )  # left shift cfg value,
        self.genShiftLeftRegister(
            scratch_reg_index, scratch_reg_index, scratch_reg2_index
        )  # mask, to position in pmpcfg reg
        self.mAssemblyHelper.genNotRegister(
            scratch_reg_index, scratch_reg2_index
        )  # invert cfg value mask
        self.mAssemblyHelper.genAndRegister(
            pmpcfg_reg_index, scratch_reg_index
        )  # use (inverted) cfg value mask to clear old cfg value
        self.mAssemblyHelper.genOrRegister(
            pmpcfg_reg_index, cfgval_reg_index
        )  # then or in new cfg value

        self.mAssemblyHelper.genMoveImmediate(scratch_reg2_index, 8)
        self.mAssemblyHelper.genConditionalBranch(pmp_reg_index, scratch_reg2_index, 6, "GE")
        # if pmp_index < 8:
        self.mAssemblyHelper.genWriteSystemRegister("pmpcfg0", pmpcfg_reg_index)
        self.mAssemblyHelper.genRelativeBranch(4)
        # else:
        self.mAssemblyHelper.genWriteSystemRegister("pmpcfg2", pmpcfg_reg_index)

        # if we got here, then located and corrected a PMP entry, for the
        # faulting address. can safely return...
        self.mAssemblyHelper.genMoveImmediate(rcode_reg_index, 0)
        self.mAssemblyHelper.genReturn()
