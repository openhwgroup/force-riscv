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
from DV.riscv.trees.instruction_tree import RV_G_map, RV32_G_map
import RandomUtils

## This test performs some basic checks with thread locking and shared memory.
class MainSequence(Sequence):

    def generate(self, **kargs):
        shared_phys_addresses_name = 'Shared PAs'
        with self.threadLockingContext():
            # One thread needs to generate the shared physical addresses
            if not self.hasSharedThreadObject(shared_phys_addresses_name):
                shared_phys_addresses = []

                for _ in range(3):
                    phys_addr = self.genPA(Size=8, Align=8, Type='D', Shared=1)
                    shared_phys_addresses.append(phys_addr)

                self.setSharedThreadObject(shared_phys_addresses_name, shared_phys_addresses)

        for _ in range(RandomUtils.random32(2, 5)):
            with self.threadLockingContext():
                shared_phys_addresses = self.getSharedThreadObject(shared_phys_addresses_name)
                self._genSharedLoadInstruction(self.choice(shared_phys_addresses))
                self._genRandomInstructions()

    ## Generate a load instruction to a shared address and assert that the destination register is
    # marked unpredictable.
    #
    #  @param aSharedPhysAddr A shared physical address to target with the load instruction.
    def _genSharedLoadInstruction(self, aSharedPhysAddr):
        target_addr = self.genVAforPA(Size=8, Align=8, Type='D', PA=aSharedPhysAddr)
        if (self.getGlobalState('AppRegisterWidth') == 32):
            instr_id = self.genInstruction('LW##RISCV', {'LSTarget': target_addr})
        else:
            instr_id = self.genInstruction('LD##RISCV', {'LSTarget': target_addr})

        instr_record = self.queryInstructionRecord(instr_id)
        dest_reg_index = instr_record['Dests']['rd']
        dest_reg_name = 'x%d' % dest_reg_index
        if (dest_reg_index != 0) and (not self.isRegisterReserved(dest_reg_name, access='Read', resv_type='Unpredictable')):
            self.error('Destination register %s was not marked as unpredictable' % dest_reg_name)

    ## Generate a random number of a wide variety of instructions.
    def _genRandomInstructions(self):
        for _ in range(RandomUtils.random32(0, 10)):
            if (self.getGlobalState('AppRegisterWidth') == 32):
                instr = RV32_G_map.pick(self.genThread)
            else:
                instr = RV_G_map.pick(self.genThread)
            self.genInstruction(instr)


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
