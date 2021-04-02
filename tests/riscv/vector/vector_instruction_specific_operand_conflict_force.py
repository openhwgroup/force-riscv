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


#  This test verifies that instructions with specific operand overlap
#  restrictions are generated
# legally.
class MainSequence(VectorTestSequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = (
            "VCOMPRESS.VM##RISCV",
            "VFSLIDE1UP.VF##RISCV",
            "VIOTA.M##RISCV",
            "VMSBF.M##RISCV",
            "VMSIF.M##RISCV",
            "VMSOF.M##RISCV",
            "VRGATHER.VI##RISCV",
            "VRGATHER.VV##RISCV",
            "VRGATHER.VX##RISCV",
            "VSLIDE1UP.VX##RISCV",
            "VSLIDEUP.VI##RISCV",
            "VSLIDEUP.VX##RISCV",
        )

    # Return the maximum number of test instructions to generate.
    def _getMaxInstructionCount(self):
        return 1000

    # Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    # Verify additional aspects of the instruction generation and execution.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrRecord A record of the generated instruction.
    def _performAdditionalVerification(self, aInstr, aInstrRecord):
        vs2_val = aInstrRecord["Srcs"]["vs2"]
        vd_val = aInstrRecord["Dests"]["vd"]
        if vs2_val == vd_val:
            self.error(
                "Instruction %s used overlapping source and destination "
                "registers" % aInstr
            )

        vs1_val = aInstrRecord["Srcs"].get("vs1")
        if vs1_val and (vs1_val == vd_val):
            self.error(
                "Instruction %s used overlapping source and destination "
                "registers" % aInstr
            )

        vm_val = aInstrRecord["Imms"].get("vm")
        if vm_val and ((vm_val == 0) and (vd_val == 0)):
            self.error(
                "Instruction %s is masked with v0 as the destination "
                "register" % aInstr
            )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
