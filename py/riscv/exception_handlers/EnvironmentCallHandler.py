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


class EnvironmentCallHandlerRISCV(ReusableSequence):
    def __init__(self, aGenThread, aFactory, aStack):
        super().__init__(aGenThread, aFactory, aStack)

        self.mDataBlockAddrRegIndex = None
        self.mActionCodeRegIndex = None
        self._mAppRegSize = 64

    def generateHandler(self, **kwargs):
        try:
            handler_context = kwargs["handler_context"]
        except KeyError:
            self.error(
                "INTERNAL ERROR: one or more arguments to "
                "EnvironmentCallHandlerRISCV generate method missing."
            )

        self.debug(
            "[EnvironmentCallHandlerRISCV] generate handler address: 0x%x" % self.getPEstate("PC")
        )

        self._mAppRegSize = self.getGlobalState("AppRegisterWidth")

        self.mAssemblyHelper.clearLabels("EnvironmentCallHandlerRISCV")

        (
            _,
            self.mActionCodeRegIndex,
        ) = handler_context.getScratchRegisterIndices(RegisterCallRole.ARGUMENT, 2)
        priv_level_reg_index = handler_context.getScratchRegisterIndices(
            RegisterCallRole.PRIV_LEVEL_VALUE
        )
        scratch_reg_index = handler_context.getScratchRegisterIndices(
            RegisterCallRole.TEMPORARY, 1
        )

        # Action Code 1: Return to S Mode
        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 1)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            self.mActionCodeRegIndex,
            scratch_reg_index,
            8,
            "EQ",
            "RETURN_TO_S_MODE",
        )

        # Action Code 2: Load From Data Block
        self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 2)
        self.mAssemblyHelper.genConditionalBranchToLabel(
            self.mActionCodeRegIndex,
            scratch_reg_index,
            48,
            "EQ",
            "LOAD_FROM_DATA_BLOCK",
        )

        # All other action codes: Skip instruction and return
        self.mAssemblyHelper.genRelativeBranchToLabel(78, "SKIP_INSTRUCTION")

        self.mAssemblyHelper.addLabel("RETURN_TO_S_MODE")
        self._genReturnToSMode(handler_context)

        self.mAssemblyHelper.addLabel("LOAD_FROM_DATA_BLOCK")
        self._genLoadRegistersFromDataBlock(handler_context)

        self.mAssemblyHelper.addLabel("SKIP_INSTRUCTION")
        self.mAssemblyHelper.genIncrementExceptionReturnAddress(
            scratch_reg_index, priv_level_reg_index
        )

        self.mAssemblyHelper.addLabel("RETURN")
        self.mAssemblyHelper.genReturn()

    # Generate instructions to return to S Mode using the first data block
    # entry as the return address.
    #
    #  @param aHandlerContext The exception handler context from which
    #       register indices can be retrieved by role.
    def _genReturnToSMode(self, aHandlerContext):
        (
            self.mDataBlockAddrRegIndex,
            _,
        ) = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.ARGUMENT, 2)
        priv_level_reg_index = aHandlerContext.getScratchRegisterIndices(
            RegisterCallRole.PRIV_LEVEL_VALUE
        )
        (
            scratch_reg_index,
            xstatus_reg_index,
            inverse_mask_reg_index,
        ) = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.TEMPORARY, 3)

        for priv_level in self.mAssemblyHelper.genPrivilegeLevelInstructions(
            aPrivLevels=tuple(PrivilegeLevelRISCV)[1:],
            aInstrCountPerLevel=9,
            aScratchRegIndex=scratch_reg_index,
            aPrivLevelRegIndex=priv_level_reg_index,
        ):
            self.mAssemblyHelper.genReadSystemRegister(
                xstatus_reg_index, ("%sstatus" % priv_level.name.lower())
            )

            self.mAssemblyHelper.genMoveImmediate(scratch_reg_index, 1)
            if priv_level == PrivilegeLevelRISCV.S:
                self.mAssemblyHelper.genShiftLeftImmediate(scratch_reg_index, 8)
            elif priv_level == PrivilegeLevelRISCV.M:
                self.mAssemblyHelper.genShiftLeftImmediate(scratch_reg_index, 11)

            self.mAssemblyHelper.genNotRegister(
                inverse_mask_reg_index, aSrcRegIndex=scratch_reg_index
            )
            self.mAssemblyHelper.genAndRegister(xstatus_reg_index, inverse_mask_reg_index)
            self.mAssemblyHelper.genOrRegister(xstatus_reg_index, scratch_reg_index)
            self.mAssemblyHelper.genWriteSystemRegister(
                ("%sstatus" % priv_level.name.lower()), xstatus_reg_index
            )

            self._genLoadRegFromDataBlock(scratch_reg_index, self.mDataBlockAddrRegIndex, 0)
            self.mAssemblyHelper.genWriteSystemRegister(
                ("%sepc" % priv_level.name.lower()), scratch_reg_index
            )

        self.mAssemblyHelper.genRelativeBranchToLabel(52, "RETURN")

    # Generate instructions to load CSRs using values from the data block.
    #
    #  @param aHandlerContext The exception handler context from which
    #  register indices can be retrieved by role.
    def _genLoadRegistersFromDataBlock(self, aHandlerContext):
        # The data block should hold values for the following sequence of
        # registers: xstatus, xepc, satp, action code register, data block
        # address register
        (
            self.mDataBlockAddrRegIndex,
            _,
        ) = aHandlerContext.getScratchRegisterIndices(RegisterCallRole.ARGUMENT, 2)
        priv_level_reg_index = aHandlerContext.getScratchRegisterIndices(
            RegisterCallRole.PRIV_LEVEL_VALUE
        )
        scratch_reg_index = aHandlerContext.getScratchRegisterIndices(
            RegisterCallRole.TEMPORARY, 1
        )

        for priv_level in self.mAssemblyHelper.genPrivilegeLevelInstructions(
            aPrivLevels=tuple(PrivilegeLevelRISCV)[1:],
            aInstrCountPerLevel=4,
            aScratchRegIndex=scratch_reg_index,
            aPrivLevelRegIndex=priv_level_reg_index,
        ):
            self._genLoadRegFromDataBlock(scratch_reg_index, self.mDataBlockAddrRegIndex, 0)
            self.mAssemblyHelper.genWriteSystemRegister(
                ("%sstatus" % priv_level.name.lower()), scratch_reg_index
            )
            self._genLoadRegFromDataBlock(scratch_reg_index, self.mDataBlockAddrRegIndex, 1)
            self.mAssemblyHelper.genWriteSystemRegister(
                ("%sepc" % priv_level.name.lower()), scratch_reg_index
            )

        self._genLoadRegFromDataBlock(scratch_reg_index, self.mDataBlockAddrRegIndex, 2)
        self.mAssemblyHelper.genWriteSystemRegister("satp", scratch_reg_index)
        self._genLoadRegFromDataBlock(self.mActionCodeRegIndex, self.mDataBlockAddrRegIndex, 3)
        self._genLoadRegFromDataBlock(self.mDataBlockAddrRegIndex, self.mDataBlockAddrRegIndex, 4)

        self.mAssemblyHelper.genRelativeBranchToLabel(20, "RETURN")

    # Generate instruction to load a single GPR from the data block.
    #
    # @param aDestReg - destination GPR
    # @param aAddrReg - GPR containing base address to load from
    # @param aOffset  - offset from base address, if any
    def _genLoadRegFromDataBlock(self, aDestReg, aAddrReg, aOffset):
        if self._mAppRegSize == 32:
            self.genInstruction(
                "LW##RISCV",
                {
                    "rd": aDestReg,
                    "rs1": aAddrReg,
                    "simm12": aOffset * 4,
                    "NoRestriction": 1,
                },
            )
        else:
            self.genInstruction(
                "LD##RISCV",
                {
                    "rd": aDestReg,
                    "rs1": aAddrReg,
                    "simm12": aOffset * 8,
                    "NoRestriction": 1,
                },
            )
