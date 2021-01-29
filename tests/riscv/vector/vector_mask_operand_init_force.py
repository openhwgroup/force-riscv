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
from VectorTestSequence import VectorTestSequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


#  This test verifies that masked vector instructions trigger initialization
#  of v0, the implicit mask register.
class MainSequence(VectorTestSequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = (
            "VADD.VI##RISCV",
            "VADD.VV##RISCV",
            "VADD.VX##RISCV",
            "VAND.VI##RISCV",
            "VAND.VV##RISCV",
            "VAND.VX##RISCV",
            "VDOT.VV##RISCV",
            "VDOTU.VV##RISCV",
            "VID.V##RISCV",
            "VMAX.VV##RISCV",
            "VMAX.VX##RISCV",
            "VMAXU.VV##RISCV",
            "VMAXU.VX##RISCV",
            "VMIN.VV##RISCV",
            "VMIN.VX##RISCV",
            "VMINU.VV##RISCV",
            "VMINU.VX##RISCV",
            "VMSEQ.VI##RISCV",
            "VMSEQ.VV##RISCV",
            "VMSEQ.VX##RISCV",
            "VMSGT.VI##RISCV",
            "VMSGT.VX##RISCV",
            "VMSGTU.VI##RISCV",
            "VMSGTU.VX##RISCV",
            "VMSLE.VI##RISCV",
            "VMSLE.VV##RISCV",
            "VMSLE.VX##RISCV",
            "VMSLEU.VI##RISCV",
            "VMSLEU.VV##RISCV",
            "VMSLEU.VX##RISCV",
            "VMSLT.VV##RISCV",
            "VMSLT.VX##RISCV",
            "VMSLTU.VV##RISCV",
            "VMSLTU.VX##RISCV",
            "VMSNE.VI##RISCV",
            "VMSNE.VV##RISCV",
            "VMSNE.VX##RISCV",
            "VOR.VI##RISCV",
            "VOR.VV##RISCV",
            "VOR.VX##RISCV",
            "VRSUB.VI##RISCV",
            "VRSUB.VX##RISCV",
            "VSADD.VI##RISCV",
            "VSADD.VV##RISCV",
            "VSADD.VX##RISCV",
            "VSADDU.VI##RISCV",
            "VSADDU.VV##RISCV",
            "VSADDU.VX##RISCV",
            "VSLIDEDOWN.VI##RISCV",
            "VSLIDEDOWN.VX##RISCV",
            "VSLL.VI##RISCV",
            "VSLL.VV##RISCV",
            "VSLL.VX##RISCV",
            "VSMUL.VV##RISCV",
            "VSMUL.VX##RISCV",
            "VSRA.VI##RISCV",
            "VSRA.VV##RISCV",
            "VSRA.VX##RISCV",
            "VSRL.VI##RISCV",
            "VSRL.VV##RISCV",
            "VSRL.VX##RISCV",
            "VSSRA.VI##RISCV",
            "VSSRA.VV##RISCV",
            "VSSRA.VX##RISCV",
            "VSSRL.VI##RISCV",
            "VSSRL.VV##RISCV",
            "VSSRL.VX##RISCV",
            "VSSUB.VV##RISCV",
            "VSSUB.VX##RISCV",
            "VSSUBU.VV##RISCV",
            "VSSUBU.VX##RISCV",
            "VSUB.VV##RISCV",
            "VSUB.VX##RISCV",
            "VXOR.VI##RISCV",
            "VXOR.VV##RISCV",
            "VXOR.VX##RISCV",
        )

    # Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    # Return parameters to be passed to Sequence.genInstruction().
    def _getInstructionParameters(self):
        return {"vd": "1-31", "vm": 0}

    # Verify additional aspects of the instruction generation and execution.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrRecord A record of the generated instruction.
    def _performAdditionalVerification(self, aInstr, aInstrRecord):
        for sub_index in range(8):
            v0_field_name = "v0_%d" % sub_index
            (v0_field_val, valid) = self.readRegister(
                "v0", field=v0_field_name
            )
            self.assertValidRegisterValue("v0", valid)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
