#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from VectorTestSequence import VectorTestSequence

## This test verifies that whole register load and store instructions can be generated and executed
# successfully.
class MainSequence(VectorTestSequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        # TODO(Noah): Add additional load/store whole register instructions when they are supported
        # by Handcar.
        self._mInstrList = (
            'VL1R.V##RISCV',
            'VS1R.V##RISCV',
        )

    ## Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    ## Get allowed exception codes.
    def _getAllowedExceptionCodes(self):
        allowed_except_codes = set()

        # TODO(Noah): Remove the line below permitting store page fault exceptions when the page
        # descriptor generation is improved. Currently, we are generating read-only pages for load
        # instructions, which is causing subsequent store instructions to the same page to fault.
        allowed_except_codes.add(0xF)

        return allowed_except_codes


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
