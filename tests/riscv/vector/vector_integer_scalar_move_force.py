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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from VectorTestSequence import VectorTestSequence

## This test verifies that randomly generated vector integer scalar move instructions can execute.
class MainSequence(VectorTestSequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = (
            'VMV.S.X##RISCV',
            'VMV.X.S##RISCV',
        )

    ## Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
