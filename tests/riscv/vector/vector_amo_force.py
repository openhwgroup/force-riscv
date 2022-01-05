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
from VectorTestSequence import VectorLoadStoreTestSequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.ChoicesModifier import ChoicesModifier


# This test verifies that vector atomic instructions can be generated and executed successfully.
class MainSequence(VectorLoadStoreTestSequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = (
            "VAMOADDEI16.V#No register write#RISCV",
            "VAMOADDEI16.V#Register write#RISCV",
            "VAMOADDEI32.V#No register write#RISCV",
            "VAMOADDEI32.V#Register write#RISCV",
            "VAMOADDEI64.V#No register write#RISCV",
            "VAMOADDEI64.V#Register write#RISCV",
            "VAMOADDEI8.V#No register write#RISCV",
            "VAMOADDEI8.V#Register write#RISCV",
            "VAMOANDEI16.V#No register write#RISCV",
            "VAMOANDEI16.V#Register write#RISCV",
            "VAMOANDEI32.V#No register write#RISCV",
            "VAMOANDEI32.V#Register write#RISCV",
            "VAMOANDEI64.V#No register write#RISCV",
            "VAMOANDEI64.V#Register write#RISCV",
            "VAMOANDEI8.V#No register write#RISCV",
            "VAMOANDEI8.V#Register write#RISCV",
            "VAMOMAXEI16.V#No register write#RISCV",
            "VAMOMAXEI16.V#Register write#RISCV",
            "VAMOMAXEI32.V#No register write#RISCV",
            "VAMOMAXEI32.V#Register write#RISCV",
            "VAMOMAXEI64.V#No register write#RISCV",
            "VAMOMAXEI64.V#Register write#RISCV",
            "VAMOMAXEI8.V#No register write#RISCV",
            "VAMOMAXEI8.V#Register write#RISCV",
            "VAMOMAXUEI16.V#No register write#RISCV",
            "VAMOMAXUEI16.V#Register write#RISCV",
            "VAMOMAXUEI32.V#No register write#RISCV",
            "VAMOMAXUEI32.V#Register write#RISCV",
            "VAMOMAXUEI64.V#No register write#RISCV",
            "VAMOMAXUEI64.V#Register write#RISCV",
            "VAMOMAXUEI8.V#No register write#RISCV",
            "VAMOMAXUEI8.V#Register write#RISCV",
            "VAMOMINEI16.V#No register write#RISCV",
            "VAMOMINEI16.V#Register write#RISCV",
            "VAMOMINEI32.V#No register write#RISCV",
            "VAMOMINEI32.V#Register write#RISCV",
            "VAMOMINEI64.V#No register write#RISCV",
            "VAMOMINEI64.V#Register write#RISCV",
            "VAMOMINEI8.V#No register write#RISCV",
            "VAMOMINEI8.V#Register write#RISCV",
            "VAMOMINUEI16.V#No register write#RISCV",
            "VAMOMINUEI16.V#Register write#RISCV",
            "VAMOMINUEI32.V#No register write#RISCV",
            "VAMOMINUEI32.V#Register write#RISCV",
            "VAMOMINUEI64.V#No register write#RISCV",
            "VAMOMINUEI64.V#Register write#RISCV",
            "VAMOMINUEI8.V#No register write#RISCV",
            "VAMOMINUEI8.V#Register write#RISCV",
            "VAMOOREI16.V#No register write#RISCV",
            "VAMOOREI16.V#Register write#RISCV",
            "VAMOOREI32.V#No register write#RISCV",
            "VAMOOREI32.V#Register write#RISCV",
            "VAMOOREI64.V#No register write#RISCV",
            "VAMOOREI64.V#Register write#RISCV",
            "VAMOOREI8.V#No register write#RISCV",
            "VAMOOREI8.V#Register write#RISCV",
            "VAMOSWAPEI16.V#No register write#RISCV",
            "VAMOSWAPEI16.V#Register write#RISCV",
            "VAMOSWAPEI32.V#No register write#RISCV",
            "VAMOSWAPEI32.V#Register write#RISCV",
            "VAMOSWAPEI64.V#No register write#RISCV",
            "VAMOSWAPEI64.V#Register write#RISCV",
            "VAMOSWAPEI8.V#No register write#RISCV",
            "VAMOSWAPEI8.V#Register write#RISCV",
            "VAMOXOREI16.V#No register write#RISCV",
            "VAMOXOREI16.V#Register write#RISCV",
            "VAMOXOREI32.V#No register write#RISCV",
            "VAMOXOREI32.V#Register write#RISCV",
            "VAMOXOREI64.V#No register write#RISCV",
            "VAMOXOREI64.V#Register write#RISCV",
            "VAMOXOREI8.V#No register write#RISCV",
            "VAMOXOREI8.V#Register write#RISCV",
        )

    # Set up the environment prior to generating the test instructions.
    def _setUpTest(self):
        choices_mod = ChoicesModifier(self.genThread)

        # Ensure element size is at least 32 because smaller sizes are not legal
        choice_weights = {
            "0x0": 0,
            "0x1": 0,
            "0x2": 10,
            "0x3": 10,
            "0x4": 0,
            "0x5": 0,
            "0x6": 0,
            "0x7": 0,
        }

        # 64-bit elements are illegal in 32-bit mode for vector atomic instructions
        if self.getGlobalState("AppRegisterWidth") == 32:
            choice_weights["0x3"] = 0

        choices_mod.modifyRegisterFieldValueChoices("vtype.VSEW", choice_weights)

        choices_mod.commitSet()

    # Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
