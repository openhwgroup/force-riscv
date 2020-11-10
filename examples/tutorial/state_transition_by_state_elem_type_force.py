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
from Enums import EStateElementType, EStateTransitionOrderMode
from State import State
import RandomUtils
import StateTransition
import UtilityFunctions

# This test specifies a State with PC, floating point register and system register values. It then
# transitions to that State by processing all elements of a given type at once. This results in
# fewer instructions being generated in many cases because the StateTransitionHandlers can choose
# instructions to process the elements in bulk.
class MainSequence(Sequence):

    # Main entry point into the test template. FORCE will invoke the generate() method to start
    # processing the test template.
    def generate(self, **kargs):
        state = self.createState()

        # We have to specify an order for all element types even if the State we create doesn't use
        # all types
        state_elem_type_order = (EStateElementType.VectorRegister, EStateElementType.Memory, EStateElementType.SystemRegister, \
                                 EStateElementType.FloatingPointRegister, EStateElementType.PredicateRegister, EStateElementType.GPR, \
                                 EStateElementType.VmContext, EStateElementType.PrivilegeLevel, EStateElementType.PC)

        StateTransition.transitionToState(state, EStateTransitionOrderMode.ByStateElementType, state_elem_type_order)

    # Create State with specific PC, floating point register and system register values.
    def createState(self):
        state = State()

        # Get a random 4-byte virtual address as a target PC value
        pc_val = self.genVA(Size=4, Align=4, Type='I')
        state.addPcStateElement(pc_val)

        # Add half of the floating point registers here and half of the floating point registers
        # below. This helps demonstrate how the transition logic consolidates the elements by type
        # regardless of the order in which they were specified.
        fp_reg_indices = self.getRandomRegisters(6, reg_type='FPR')
        for i in range(0, 3):
            fp_reg_index = fp_reg_indices[i]
            fp_reg_name = 'D%d' % fp_reg_index
            state.addRegisterStateElement(fp_reg_name, [RandomUtils.random64()])

        state.addRegisterStateElement('mscratch', [RandomUtils.random64()])
        mepc_val = UtilityFunctions.getAlignedValue(RandomUtils.random64(), 4)
        state.addRegisterStateElement('mepc', [mepc_val])

        for i in range(3, 6):
            fp_reg_index = fp_reg_indices[i]
            fp_reg_name = 'D%d' % fp_reg_index
            state.addRegisterStateElement(fp_reg_name, [RandomUtils.random64()])

        return state


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
