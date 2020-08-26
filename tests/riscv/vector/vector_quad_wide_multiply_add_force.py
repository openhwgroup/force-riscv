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

## This test verifies that vector register operands with different layouts in quad-widening
# instructions don't overlap.
class MainSequence(VectorTestSequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mInstrList = (
            'VQMACC.VV##RISCV',
            'VQMACC.VX##RISCV',
            'VQMACCSU.VV##RISCV',
            'VQMACCSU.VX##RISCV',
            'VQMACCU.VV##RISCV',
            'VQMACCU.VX##RISCV',
            'VQMACCUS.VX##RISCV',
        )

    ## Return a list of test instructions to randomly choose from.
    def _getInstructionList(self):
        return self._mInstrList

    ## Verify additional aspects of the instruction generation and execution.
    #
    #  @param aInstr The name of the instruction.
    #  @param aInstrRecord A record of the generated instruction.
    def _performAdditionalVerification(self, aInstr, aInstrRecord):
        vd_val = aInstrRecord['Dests']['vd']
        vs1_val = aInstrRecord['Srcs'].get('vs1')
        vs2_val = aInstrRecord['Srcs']['vs2']
        if vs1_val and (vd_val == (vs1_val & 0x1C)):
            self.error('Instruction %s used overlapping source and destination registers of different formats' % aInstr)

        if vd_val == (vs2_val & 0x1C):
            self.error('Instruction %s used overlapping source and destination registers of different formats' % aInstr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
