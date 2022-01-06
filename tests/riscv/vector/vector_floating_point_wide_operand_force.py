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
from base.ChoicesModifier import ChoicesModifier


# This test verifies that vector widening and narrowing floating-point instructions can be
# generated and executed successfully.
class MainSequence(VectorTestSequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = (
            "VFNCVT.F.F.W##RISCV",
            "VFNCVT.F.X.W##RISCV",
            "VFNCVT.F.XU.W##RISCV",
            "VFNCVT.ROD.F.F.W##RISCV",
            "VFNCVT.RTZ.X.F.W##RISCV",
            "VFNCVT.RTZ.XU.F.W##RISCV",
            "VFNCVT.X.F.W##RISCV",
            "VFNCVT.XU.F.W##RISCV",
            "VFNMACC.VF##RISCV",
            "VFNMACC.VV##RISCV",
            "VFNMADD.VF##RISCV",
            "VFNMADD.VV##RISCV",
            "VFNMSAC.VF##RISCV",
            "VFNMSAC.VV##RISCV",
            "VFNMSUB.VF##RISCV",
            "VFNMSUB.VV##RISCV",
            "VFWADD.VF##RISCV",
            "VFWADD.VV##RISCV",
            "VFWADD.WF##RISCV",
            "VFWADD.WV##RISCV",
            "VFWCVT.F.F.V##RISCV",
            "VFWCVT.F.X.V##RISCV",
            "VFWCVT.F.XU.V##RISCV",
            "VFWCVT.RTZ.X.F.V##RISCV",
            "VFWCVT.RTZ.XU.F.V##RISCV",
            "VFWCVT.X.F.V##RISCV",
            "VFWCVT.XU.F.V##RISCV",
            "VFWMACC.VF##RISCV",
            "VFWMACC.VV##RISCV",
            "VFWMSAC.VF##RISCV",
            "VFWMSAC.VV##RISCV",
            "VFWMUL.VF##RISCV",
            "VFWMUL.VV##RISCV",
            "VFWNMACC.VF##RISCV",
            "VFWNMACC.VV##RISCV",
            "VFWNMSAC.VF##RISCV",
            "VFWNMSAC.VV##RISCV",
            "VFWREDOSUM.VS##RISCV",
            "VFWREDUSUM.VS##RISCV",
            "VFWSUB.VF##RISCV",
            "VFWSUB.VV##RISCV",
            "VFWSUB.WF##RISCV",
            "VFWSUB.WV##RISCV",
        )

    # Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        choices_mod = ChoicesModifier(self.genThread)

        # Ensure element width is at least 16, as 8-bit floating point operations are illegal
        vsew_choice_weights = {
            "0x0": 0,
            "0x1": 10,
            "0x2": 10,
            "0x3": 0,
            "0x4": 0,
            "0x5": 0,
            "0x6": 0,
            "0x7": 0,
        }
        choices_mod.modifyRegisterFieldValueChoices("vtype.VSEW", vsew_choice_weights)

        # Ensure vector register group size is no more than 4, as larger values are not legal for
        # widening and narrowing instructions
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
        choices_mod.modifyRegisterFieldValueChoices("vtype.VLMUL", vlmul_choice_weights)

        choices_mod.commitSet()

    # Return the maximum number of test instructions to generate.
    def _getMaxInstructionCount(self):
        return 1000

    # Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
