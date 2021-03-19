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
from base.ChoicesModifier import ChoicesModifier
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


#  This test verifies that vector register operands with different layouts
#  in quad-widening
# instructions don't overlap.
class MainSequence(VectorTestSequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = (
            "VQMACC.VV##RISCV",
            "VQMACC.VX##RISCV",
            "VQMACCSU.VV##RISCV",
            "VQMACCSU.VX##RISCV",
            "VQMACCU.VV##RISCV",
            "VQMACCU.VX##RISCV",
            "VQMACCUS.VX##RISCV",
        )

    # Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        choices_mod = ChoicesModifier(self.genThread)

        choice_weights = {
            "0x0": 10,
            "0x1": 10,
            "0x2": 0,
            "0x3": 0,
            "0x4": 0,
            "0x5": 0,
            "0x6": 0,
            "0x7": 0,
        }
        choices_mod.modifyRegisterFieldValueChoices(
            "vtype.VSEW", choice_weights
        )

        # Ensure vector register group size is no more than 2, as larger values
        # are not legal for quad-widening instructions
        vlmul_choice_weights = {
            "0x0": 10,
            "0x1": 10,
            "0x2": 0,
            "0x3": 0,
            "0x4": 0,
            "0x5": 10,
            "0x6": 10,
            "0x7": 10,
        }
        choices_mod.modifyRegisterFieldValueChoices(
            "vtype.VLMUL", vlmul_choice_weights
        )

        choices_mod.commitSet()

    # Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    # Verify additional aspects of the instruction generation and execution.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrRecord A record of the generated instruction.
    def _performAdditionalVerification(self, aInstr, aInstrRecord):
        vd_val = aInstrRecord["Dests"]["vd"]
        vs1_val = aInstrRecord["Srcs"].get("vs1")
        if vs1_val:
            self.assertNoRegisterOverlap(
                aInstr, vd_val, vs1_val, aRegCountMultipleA=4
            )

        vs2_val = aInstrRecord["Srcs"]["vs2"]
        self.assertNoRegisterOverlap(
            aInstr, vd_val, vs2_val, aRegCountMultipleA=4
        )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
