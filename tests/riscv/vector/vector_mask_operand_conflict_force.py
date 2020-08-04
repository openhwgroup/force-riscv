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
from base.Sequence import Sequence
import RandomUtils

## This test verifies that masked vector instructions do not set v0 as the destination operand.
class MainSequence(Sequence):

    def generate(self, **kargs):
        instructions = [
            'VADD.VI##RISCV',
            'VADD.VV##RISCV',
            'VADD.VX##RISCV',
            'VAND.VI##RISCV',
            'VAND.VV##RISCV',
            'VAND.VX##RISCV',
            'VDOT.VV##RISCV',
            'VDOTU.VV##RISCV',
            'VID.V##RISCV',
            'VMADC.VI##RISCV',
            'VMADC.VV##RISCV',
            'VMADC.VX##RISCV',
            'VMAX.VV##RISCV',
            'VMAX.VX##RISCV',
            'VMAXU.VV##RISCV',
            'VMAXU.VX##RISCV',
            'VMIN.VV##RISCV',
            'VMIN.VX##RISCV',
            'VMINU.VV##RISCV',
            'VMINU.VX##RISCV',
            'VMSBC.VV##RISCV',
            'VMSBC.VX##RISCV',
            'VMSEQ.VI##RISCV',
            'VMSEQ.VV##RISCV',
            'VMSEQ.VX##RISCV',
            'VMSGT.VI##RISCV',
            'VMSGT.VX##RISCV',
            'VMSGTU.VI##RISCV',
            'VMSGTU.VX##RISCV',
            'VMSLE.VI##RISCV',
            'VMSLE.VV##RISCV',
            'VMSLE.VX##RISCV',
            'VMSLEU.VI##RISCV',
            'VMSLEU.VV##RISCV',
            'VMSLEU.VX##RISCV',
            'VMSLT.VV##RISCV',
            'VMSLT.VX##RISCV',
            'VMSLTU.VV##RISCV',
            'VMSLTU.VX##RISCV',
            'VMSNE.VI##RISCV',
            'VMSNE.VV##RISCV',
            'VMSNE.VX##RISCV',
            'VOR.VI##RISCV',
            'VOR.VV##RISCV',
            'VOR.VX##RISCV',
            'VRSUB.VI##RISCV',
            'VRSUB.VX##RISCV',
            'VSADD.VI##RISCV',
            'VSADD.VV##RISCV',
            'VSADD.VX##RISCV',
            'VSADDU.VI##RISCV',
            'VSADDU.VV##RISCV',
            'VSADDU.VX##RISCV',
            'VSLIDEDOWN.VI##RISCV',
            'VSLIDEDOWN.VX##RISCV',
            'VSLL.VI##RISCV',
            'VSLL.VV##RISCV',
            'VSLL.VX##RISCV',
            'VSMUL.VV##RISCV',
            'VSMUL.VX##RISCV',
            'VSRA.VI##RISCV',
            'VSRA.VV##RISCV',
            'VSRA.VX##RISCV',
            'VSRL.VI##RISCV',
            'VSRL.VV##RISCV',
            'VSRL.VX##RISCV',
            'VSSRA.VI##RISCV',
            'VSSRA.VV##RISCV',
            'VSSRA.VX##RISCV',
            'VSSRL.VI##RISCV',
            'VSSRL.VV##RISCV',
            'VSSRL.VX##RISCV',
            'VSSUB.VV##RISCV',
            'VSSUB.VX##RISCV',
            'VSSUBU.VV##RISCV',
            'VSSUBU.VX##RISCV',
            'VSUB.VV##RISCV',
            'VSUB.VX##RISCV',
            'VXOR.VI##RISCV',
            'VXOR.VV##RISCV',
            'VXOR.VX##RISCV',
        ]

        for _ in range(RandomUtils.random32(500, 1000)):
            instr = self.choice(instructions)
            instr_id = self.genInstruction(instr)

            instr_record = self.queryInstructionRecord(instr_id)
            if instr_record is None:
                self.error('Instruction %s did not generate correctly' % instr)

            vm_val = instr_record['Imms']['vm']
            vd_val = instr_record['Dests']['vd']
            if (vm_val == 0) and (vd_val == 0):
                self.error('Instruction %s is masked with v0 as the destination register' % instr)

            illegal_instr_count = self.queryExceptionRecordsCount(0x2)
            if illegal_instr_count != 0:
                self.error('Instruction %s did not execute correctly' % instr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
