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
#  don't overlap.
class MainSequence(VectorTestSequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = (
            "VNSRA.WI##RISCV",
            "VNSRA.WV##RISCV",
            "VNSRA.WX##RISCV",
            "VNSRL.WI##RISCV",
            "VNSRL.WV##RISCV",
            "VNSRL.WX##RISCV",
            "VWADD.VV##RISCV",
            "VWADD.VX##RISCV",
            "VWADD.WV##RISCV",
            "VWADD.WX##RISCV",
            "VWADDU.VV##RISCV",
            "VWADDU.VX##RISCV",
            "VWADDU.WV##RISCV",
            "VWADDU.WX##RISCV",
            "VWMACC.VV##RISCV",
            "VWMACC.VX##RISCV",
            "VWMACCSU.VV##RISCV",
            "VWMACCSU.VX##RISCV",
            "VWMACCU.VV##RISCV",
            "VWMACCU.VX##RISCV",
            "VWMACCUS.VX##RISCV",
            "VWMUL.VV##RISCV",
            "VWMUL.VX##RISCV",
            "VWMULSU.VV##RISCV",
            "VWMULSU.VX##RISCV",
            "VWMULU.VV##RISCV",
            "VWMULU.VX##RISCV",
            "VWSUB.VV##RISCV",
            "VWSUB.VX##RISCV",
            "VWSUB.WV##RISCV",
            "VWSUB.WX##RISCV",
            "VWSUBU.VV##RISCV",
            "VWSUBU.VX##RISCV",
            "VWSUBU.WV##RISCV",
            "VWSUBU.WX##RISCV",
        )

    # Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        choices_mod = ChoicesModifier(self.genThread)

        choice_weights = {
            "0x0": 10,
            "0x1": 10,
            "0x2": 10,
            "0x3": 0,
            "0x4": 0,
            "0x5": 0,
            "0x6": 0,
            "0x7": 0,
        }
        choices_mod.modifyRegisterFieldValueChoices(
            "vtype.VSEW", choice_weights
        )

        # Ensure vector register group size is no more than 4, as larger values
        # are not legal for widening and narrowing instructions
        vlmul_choice_weights = {
            "0x0": 10,
            "0x1": 10,
            "0x2": 10,
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
        vd_val = aInstrRecord["Dests"]["vd"]
        vs1_val = aInstrRecord["Srcs"].get("vs1")
        vs2_val = aInstrRecord["Srcs"]["vs2"]
        if aInstr.startswith("VW"):
            if vs1_val:
                self.assertNoRegisterOverlap(
                    aInstr, vd_val, vs1_val, aRegCountMultipleA=2
                )

            if ".W" not in aInstr:
                self.assertNoRegisterOverlap(
                    aInstr, vd_val, vs2_val, aRegCountMultipleA=2
                )
        elif aInstr.startswith("VN"):
            self.assertNoRegisterOverlap(
                aInstr, vd_val, vs2_val, aRegCountMultipleB=2
            )
        else:
            self.error("Unexpected instruction %s" % aInstr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
