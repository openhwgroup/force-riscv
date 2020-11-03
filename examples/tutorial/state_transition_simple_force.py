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
from DV.riscv.trees.instruction_tree import RV_G_map
from State import State
import RandomUtils
import StateTransition

# This test specifies a State with memory and GPR values. It then transitions to that State and uses
# the GPRs as base registers to load the memory values.
class MainSequence(Sequence):

    # Main entry point into the test template. FORCE will invoke the generate() method to start
    # processing the test template.
    def generate(self, **kargs):
        # Get 4 random GPR indices, excluding 0. We use the first GPR as a destination register for
        # the LD instructions and the other 3 GPRs to hold addresses of the memory locations
        # specified in the State.
        gpr_indices = self.getRandomGPRs(4, exclude='0')
        for _ in range(3):
            state = self.createState(gpr_indices)

            self.genRandomInstructions()

            StateTransition.transitionToState(state)

            self.genLoadInstructions(gpr_indices)

            self.genRandomInstructions()

    # Create State with specific memory and GPR values.
    def createState(self, gprIndices):
        state = State()

        for i in range(1, 4):
            # Get a random 8-byte virtual address for data
            mem_start_addr = self.genVA(Size=8, Align=8, Type='D')

            # Specify the value of i for the memory location
            state.addMemoryStateElement(mem_start_addr, 8, i)

            gpr_index = gprIndices[i]
            gpr_name = 'x%d' % gpr_index

            # Specify the memory address as the value of the GPR
            state.addRegisterStateElement(gpr_name, [mem_start_addr])

        return state

    # Generate a random number of a wide variety of instructions.
    def genRandomInstructions(self):
        for _ in range(RandomUtils.random32(5, 10)):
            instr = RV_G_map.pick(self.genThread)
            self.genInstruction(instr)

    # Generate instructions to load from the State's memory locations.
    def genLoadInstructions(self, gprIndices):
        for i in range(1, 4):
            # The destination register receives the memory values 1, 2, 3 that we specified as part
            # of the State
            self.genInstruction('LD##RISCV', {'rd': gprIndices[0], 'rs1': gprIndices[i], 'simm12': 0, 'NoPreamble': 1})


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
