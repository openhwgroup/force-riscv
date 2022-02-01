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


#  This test verifies that whole register load and store instructions can be
#  generated and executed
# successfully.
class MainSequence(VectorTestSequence):
    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = (
            "VL1RE8.V##RISCV",
            "VL1RE16.V##RISCV",
            "VL1RE32.V##RISCV",
            "VL1RE64.V##RISCV",
            "VL2RE8.V##RISCV",
            "VL2RE16.V##RISCV",
            "VL2RE32.V##RISCV",
            "VL2RE64.V##RISCV",
            "VL4RE8.V##RISCV",
            "VL4RE16.V##RISCV",
            "VL4RE32.V##RISCV",
            "VL4RE64.V##RISCV",
            "VL8RE8.V##RISCV",
            "VL8RE16.V##RISCV",
            "VL8RE32.V##RISCV",
            "VL8RE64.V##RISCV",
            "VS1R.V##RISCV",
            "VS2R.V##RISCV",
            "VS4R.V##RISCV",
            "VS8R.V##RISCV",
        )

    # Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    # Get allowed exception codes.
    #
    #  @param aInstr The name of the instruction.
    def _getAllowedExceptionCodes(self, aInstr):
        allowed_except_codes = set()

        allowed_except_codes.add(0xF)

        return allowed_except_codes


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
