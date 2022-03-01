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
from base.Sequence import Sequence
from riscv.AssemblyHelperRISCV import AssemblyHelperRISCV
from riscv.Utils import LoadGPR64


class SystemCallSequence(Sequence):
    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self._mAssemblyHelper = AssemblyHelperRISCV(self)
        self._mDataBlockAddrRegIndex = None
        self._mActionCodeRegIndex = None
        self._mAppRegSize = 64

    def generate(self, **kwargs):
        (handlers_set_name, valid) = self.getOption("handlers_set")
        if valid and (handlers_set_name == "Fast"):
            self.error(
                "Fast exception handlers are enabled. SystemCallSequence only "
                "supports using comprehensive exception handlers."
            )

        self._mAppRegSize = self.getGlobalState("AppRegisterWidth")

        function = kwargs.setdefault("Function", "SwitchPrivilegeLevel")
        if function == "SwitchPrivilegeLevel":
            ret_code = self._switchPrivilegeLevel(kwargs)
        else:
            self.error("SystemCallSequence does not support the %s function" % function)

        self._processReturnCode(ret_code, function)

    # Execute a privilege level switch.
    #
    #  @params aParams A dictionary of optional parameters.
    #       PrivilegeLevel: The target privilege level. Permissible values are
    #           'U', 'S', 'M', 0, 1, 3 or 'Random'; defaults to 'Random'.
    #       TargetAddr: The address at which to resume execution after
    #           completing the privilege level switch. Permissible values are
    #           any 64-bit integer; defaults to a randomly generated valid
    #           instruction address.
    #       SkipAddrValidation: Specifies whether to skip validating a
    #           specified TargetAddr value. Permissible values are 0 or 1;
    #           defaults to 0. If SkipAddrValidation is 0, the TargetAddr value
    #           is checked to determine whether it is a valid instruction
    #           address. If it isn't, an alternative address is randomly
    #           generated. If SkipAddrValidation is 1, the target address value
    #           is always used as is.
    #       AddrChoicesModID: Specifies ID of choices modification set to be
    #           applied when generating the target address in the event no
    #           TargetAddr value is provided or the value provided is invalid.
    #           Permissible values are any valid choices modification set ID.
    #       InterruptMask: Value to set the xstatus.xIE field to for the target
    #           privilege level. Permissible values are 0, 1, 'Same', 'Flip' or
    #           'Random'; defaults to 'Random'.
    #       SUM: *** Not currently supported! *** Value to set the xstatus.SUM
    #           field to for the target privilege level. Permissible values are
    #           0, 1, 'Same', 'Flip' or 'Random'; defaults to 'Random'.
    #       MXR: *** Not currently supported! *** Value to set the xstatus.MXR
    #           field to for the target privilege level. Permissible values are
    #           0, 1, 'Same', 'Flip' or 'Random'; defaults to 'Random'.
    #       MPRV: *** Not currently supported! *** Value to set the
    #           mstatus.MPRV field to for the target privilege level.
    #           Permissible values are 0, 1, 'Same', 'Flip' or 'Random';
    #           defaults to 'Random'. As this field is only relevant to M mode,
    #           it is ignored if the target privilege level is not M.
    def _switchPrivilegeLevel(self, aParams):
        except_request_results = self.exceptionRequest("SystemCall", aParams)

        ret_code = except_request_results["RetCode"]
        if ret_code != 0:
            return ret_code

        self._assignRegisters(
            except_request_results["DataBlockAddrRegIndex"],
            except_request_results["ActionCodeRegIndex"],
        )

        load_gpr64_seq = LoadGPR64(self.genThread)
        load_gpr64_seq.load(
            self._mDataBlockAddrRegIndex,
            except_request_results["DataBlockAddr"],
        )

        self._genPrivilegeLevelSwitchInstructions(
            except_request_results["InstrSeqCode"],
            except_request_results["PrivilegeLevel"],
            except_request_results["TargetAddr"],
            except_request_results["IntermediateRetAddr"],
        )

        self._unassignRegisters()

        self.genSequence("UpdatePeState", {"RecordId": except_request_results["RecordId"]})
        self.updateVm()

        return ret_code

    # Output appropriate messages based on the return code. Fail if the return
    # code is not 0 and NoSkip is specified.
    #
    #  @param aRetCode The return code from the system call operation.
    #  @param aFunction The operation performed by the system call.
    def _processReturnCode(self, aRetCode, aFunction):
        if aRetCode == 0:
            self.notice("SystemCallSequence request %s processed succesfully" % aFunction)
        else:
            self.notice(
                "SystemCallSequence request %s could not be processed "
                "successfully; return code %d" % (aFunction, aRetCode)
            )

        (no_skip, valid) = self.genThread.getOption("NoSkip")
        if valid and (no_skip == 1) and (aRetCode != 0):
            self.error(
                "Unable to process SystemCallSequence request and NoSkip option was specified"
            )

    # Generate the instructions required to execute the privilege level switch.
    #
    #  @param aInstrSeqCode A value indicating the instructions that need to be
    #       generated.
    #  @param aTargetPrivLevel The target privilege level.
    #  @param aTargetAddr The address at which to resume execution after
    #       completing the privilege level switch.
    #  @param aIntermediateRetAddr The return address for the first ECALL of a
    #       2 ECALL sequence
    def _genPrivilegeLevelSwitchInstructions(
        self,
        aInstrSeqCode,
        aTargetPrivLevel,
        aTargetAddr,
        aIntermediateRetAddr,
    ):
        if aInstrSeqCode == 0:
            # Do nothing
            pass
        elif aInstrSeqCode == 1:
            # Generate 1 ECALL instruction
            action_code = 2  # Load From Data Block
            self._mAssemblyHelper.genMoveImmediate(self._mActionCodeRegIndex, action_code)
            self._genEcall()
        elif aInstrSeqCode == 2:
            # Generate xRET instruction
            self._genXRet(aTargetPrivLevel, aTargetAddr)
        elif aInstrSeqCode == 3:
            # Generate 2 ECALL instructions
            action_code = 1  # Return to S Mode
            self._mAssemblyHelper.genMoveImmediate(self._mActionCodeRegIndex, action_code)
            self._genEcall()
            self.setPEstate("PrivilegeLevel", 1)
            self.setPEstate("PC", aIntermediateRetAddr)

            # For a 2 ECALL sequence, the data block contains a return address
            # for the first ECALL as the first entry; after the first ECALL, we
            # increment the data block address pointer to allow loading the
            # data block as usual in the second ECALL
            self._genIncrementDataBlockPointer()
            action_code = 2  # Load From Data Block
            self._mAssemblyHelper.genMoveImmediate(self._mActionCodeRegIndex, action_code)
            self._genEcall()
        else:
            self.error("Unexpected InstrSeqCode value %d" % aInstrSeqCode)

    # Assign registers to be used for executing the privilege level switch so
    # that they can be referenced later.
    #
    #  @param aDataBlockRegIndex The index of the register used for storing the
    #       data block address.
    #  @param aActionCodeRegIndex The index of the register used for storing
    #       the action code.
    def _assignRegisters(self, aDataBlockRegIndex, aActionCodeRegIndex):
        self._mDataBlockAddrRegIndex = aDataBlockRegIndex
        self._mActionCodeRegIndex = aActionCodeRegIndex

    # Clear register assignments.
    def _unassignRegisters(self):
        self._mActionCodeRegIndex = None
        self._mDataBlockAddrRegIndex = None

    # Generate an ECALL instruction.
    def _genEcall(self):
        self.genInstruction("ECALL##RISCV")

    # Generate an xRET instruction.
    #
    #  @param aTargetPrivLevel The target privilege level.
    #  @param aTargetAddr The address at which to resume execution after
    #       completing the privilege level switch.
    def _genXRet(self, aTargetPrivLevel, aTargetAddr):
        priv_level = self.getPEstate("PrivilegeLevel")
        if priv_level == 1:
            priv_level_prefix = "S"
        elif priv_level == 3:
            priv_level_prefix = "M"
        else:
            self.error(
                "Unexpected request to execute xRET instruction to transition "
                "from privilege level %s to privilege level %s" % (priv_level, aTargetPrivLevel)
            )

        # The action code register is used as a scratch register here to reduce
        # the number of required registers
        self._genLoadGPR(self._mActionCodeRegIndex, self._mDataBlockAddrRegIndex, 0)
        self._mAssemblyHelper.genWriteSystemRegister(
            ("%sstatus" % priv_level_prefix.lower()), self._mActionCodeRegIndex
        )
        self._genLoadGPR(self._mActionCodeRegIndex, self._mDataBlockAddrRegIndex, 1)
        self._mAssemblyHelper.genWriteSystemRegister(
            ("%sepc" % priv_level_prefix.lower()), self._mActionCodeRegIndex
        )
        self._genLoadGPR(self._mActionCodeRegIndex, self._mDataBlockAddrRegIndex, 2)
        self._mAssemblyHelper.genWriteSystemRegister("satp", self._mActionCodeRegIndex)
        self.genInstruction(("%sRET##RISCV" % priv_level_prefix), {"NoRestriction": 1})

    # Generate an instruction to increment the data block address pointer.
    def _genIncrementDataBlockPointer(self):
        if self._mAppRegSize == 32:
            self._mAssemblyHelper.genAddImmediate(self._mDataBlockAddrRegIndex, 4)
        else:
            self._mAssemblyHelper.genAddImmediate(self._mDataBlockAddrRegIndex, 8)

    # Generate instruction to load a single GPR
    #
    # @param aDestReg - index of destination GPR
    # @param aAddrReg - index of GPR containing base address
    # @param aOffset  - offset from base address
    def _genLoadGPR(self, aDestReg, aAddrReg, aOffset=0):
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
