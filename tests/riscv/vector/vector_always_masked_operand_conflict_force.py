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

## This test verifies that vector instructions that are always masked or treated as masked do not
# set v0 as the destination operand.
class MainSequence(VectorTestSequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = (
            'VADC.VIM##RISCV',
            'VADC.VVM##RISCV',
            'VADC.VXM##RISCV',
            'VFMERGE.VFM##RISCV',
            'VMERGE.VIM##RISCV',
            'VMERGE.VVM##RISCV',
            'VMERGE.VXM##RISCV',
            'VSBC.VVM##RISCV',
            'VSBC.VXM##RISCV',
        )

    ## Return the maximum number of test instructions to generate.
    def _getMaxInstructionCount(self):
        return 1000

    ## Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    ## Verify additional aspects of the instruction generation and execution.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrRecord A record of the generated instruction.
    def _performAdditionalVerification(self, aInstr, aInstrRecord):
        vd_val = aInstrRecord['Dests']['vd']
        if vd_val == 0:
            self.error('Instruction %s is masked with v0 as the destination register' % aInstr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
