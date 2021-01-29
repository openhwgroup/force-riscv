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
from base.StateTransitionHandler import StateTransitionHandler
import state_transition_test_utils
from Enums import EStateElementType, EStateTransitionType
from State import State
import RandomUtils
import StateTransition
import UtilityFunctions

## A test StateTransitionHandler that verifies the StateElements it is passed have been merged with
# current State data.
class StateTransitionHandlerTest(StateTransitionHandler):

    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self.mExpectedStateData = None

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        state_elem_type = aStateElem.getStateElementType()
        if state_elem_type == EStateElementType.Memory:
            expected_mem_state_data = self.mExpectedStateData[state_elem_type]

            if aStateElem.getStartAddress() not in expected_mem_state_data:
                self.error('MemoryStateElement has unexpected start address: 0x%x' % aStateElem.getStartAddress())
        elif state_elem_type == EStateElementType.SystemRegister:
            expected_sys_reg_state_data = self.mExpectedStateData[state_elem_type]

            sys_reg_val = expected_sys_reg_state_data[aStateElem.getName()]
            if aStateElem.getValues()[0] != sys_reg_val:
                self.error('Value for StateElement %s was not merged as expected. Expected=0x%x, Actual=0x%x' % (aStateElem.getName(), sys_reg_val, aStateElem.getValues()[0]))
        elif state_elem_type == EStateElementType.FloatingPointRegister:
            expected_fp_reg_state_data = self.mExpectedStateData[state_elem_type]

            fp_reg_name = 'D%d' % aStateElem.getRegisterIndex()
            fp_reg_val = expected_fp_reg_state_data[fp_reg_name]
            if aStateElem.getValues()[0] != fp_reg_val:
                self.error('Value for StateElement %s was not merged as expected. Expected=0x%x, Actual=0x%x' % (fp_reg_name, fp_reg_val, aStateElem.getValues()[0]))
        elif state_elem_type == EStateElementType.VectorRegister:
            expected_vec_reg_state_data = self.mExpectedStateData[state_elem_type]

            vec_reg_name = 'v%d' % aStateElem.getRegisterIndex()
            vec_reg_values = expected_vec_reg_state_data[vec_reg_name]
            if aStateElem.getValues() != vec_reg_values:
                self.error('Value for StateElement %s was not merged as expected. Expected=%s, Actual=%s' % (vec_reg_name, vec_reg_values, aStateElem.getValues()))

        return True


## This test verifies that StateElements are merged correctly with current State values when
# expected.
class MainSequence(Sequence):

    def __init__(self, aGenThread, aName=None):
        super().__init__(aGenThread, aName)

        self._mExpectedStateData = {}

    def generate(self, **kargs):
        state_trans_handler = StateTransitionHandlerTest(self.genThread)
        StateTransition.registerStateTransitionHandler(state_trans_handler, EStateTransitionType.Explicit, (EStateElementType.Memory, EStateElementType.SystemRegister, EStateElementType.FloatingPointRegister, EStateElementType.VectorRegister))

        state = self._createState()

        state_trans_handler.mExpectedStateData = self._mExpectedStateData

        StateTransition.transitionToState(state)

    ## Create a simple State to test an explicit StateTransition.
    def _createState(self):
        state = State()

        self._mExpectedStateData[EStateElementType.Memory] = self._createMemoryStateElements(state)
        self._mExpectedStateData[EStateElementType.SystemRegister] = self._createSystemRegisterStateElements(state)
        self._mExpectedStateData[EStateElementType.FloatingPointRegister] = self._createFloatingPointRegisterStateElements(state)
        self._mExpectedStateData[EStateElementType.VectorRegister] = self._createVectorRegisterStateElements(state)

        return state

    ## Add randomly-generated memory StateElements to the State. The StateElements only partially
    # specify the memory values. Return a list of expected memory StateElement name and value pairs.
    # The expected values are determined by combining what is specified in the StateElements and the
    # values currently in memory.
    #
    #  @param aState The State to which the memory StateElements should be added.
    def _createMemoryStateElements(self, aState):
        expected_mem_state_data = set()

        for _ in range(RandomUtils.random32(0, 5)):
            mem_size = RandomUtils.random32(1, 20)
            mem_start_addr = self.genVA(Size=mem_size, Align=1, Type='D')

            mem_values = []
            for _ in range(mem_size):
                mem_values.append(RandomUtils.random32(0, 0xFF))

            aState.addMemoryStateElementsAsBytes(mem_start_addr, mem_values)

            # Compute the start addresses for the resulting StateElements
            state_elem_size = 8
            aligned_mem_start_addr = UtilityFunctions.getAlignedValue(mem_start_addr, state_elem_size)
            bytes_remaining = mem_size
            chunk_size = state_elem_size - (mem_start_addr - aligned_mem_start_addr)
            while bytes_remaining > 0:
                expected_mem_state_data.add(aligned_mem_start_addr)
                aligned_mem_start_addr += state_elem_size
                bytes_remaining -= chunk_size

                # The last StateElement may contain fewer specified bytes than state_elem_size, but
                # setting chunk_size to state_elem_size will cause the loop to terminate at the
                # correct time in either case
                chunk_size = state_elem_size

        return expected_mem_state_data

    ## Add system register StateElements to the State. The StateElements only partially specify the
    # system register values. Return a list of expected system register StateElement name and value
    # pairs. The expected values are determined by combining what is specified in the StateElements
    # and the values currently in the registers.
    #
    #  @param aState The State to which the system register StateElements should be added.
    def _createSystemRegisterStateElements(self, aState):
        sys_reg_name = 'mstatus'
        aState.addSystemRegisterStateElementByField(sys_reg_name, 'MIE', 0x0)
        aState.addSystemRegisterStateElementByField(sys_reg_name, 'MPRV', 0x1)

        self.randomInitializeRegister(sys_reg_name)
        (sys_reg_val, valid) = self.readRegister(sys_reg_name)
        state_transition_test_utils.assertValidRegisterValue(self, sys_reg_name, valid)

        sys_reg_val = state_transition_test_utils.combineRegisterValueWithFieldValue(self, sys_reg_name, sys_reg_val, 'MIE', 0x0)
        sys_reg_val = state_transition_test_utils.combineRegisterValueWithFieldValue(self, sys_reg_name, sys_reg_val, 'MPRV', 0x1)

        return {sys_reg_name: sys_reg_val}

    ## Add randomly-generated floating-point register StateElements to the State. The StateElements
    # only partially specify the floating-point register values. Return a list of expected
    # floating-point register StateElement name and value pairs. The expected values are determined
    # by combining what is specified in the StateElements and the values currently in the registers.
    #
    #  @param aState The State to which the floating-point register StateElements should be added.
    def _createFloatingPointRegisterStateElements(self, aState):
        expected_fp_reg_state_data = {}

        fp_reg_count = RandomUtils.random32(0, 10)
        fp_reg_indices = self.sample(range(0, 32), fp_reg_count)
        for fp_reg_index in fp_reg_indices:
            fp_reg_name = 'S%d' % fp_reg_index
            fp_reg_val = RandomUtils.random32()
            aState.addRegisterStateElement(fp_reg_name, (fp_reg_val,))

            containing_fp_reg_name = 'D%d' % fp_reg_index
            self.randomInitializeRegister(containing_fp_reg_name)
            (orig_fp_reg_val, valid) = self.readRegister(containing_fp_reg_name)
            state_transition_test_utils.assertValidRegisterValue(self, containing_fp_reg_name, valid)

            combined_fp_reg_val = (orig_fp_reg_val & (0xFFFFFFFF << 32)) | fp_reg_val
            expected_fp_reg_state_data[containing_fp_reg_name] = combined_fp_reg_val

        return expected_fp_reg_state_data

    ## Add randomly-generated vector register StateElements to the State. The StateElements only
    # partially specify the vector register values. Return a list of expected vector register
    # StateElement name and value list pairs. The expected values are determined by combining what
    # is specified in the StateElements and the values currently in the registers.
    #
    #  @param aState The State to which the vector register StateElements should be added.
    def _createVectorRegisterStateElements(self, aState):
        expected_vec_reg_state_data = {}

        vec_reg_count = RandomUtils.random32(0, 10)
        vec_reg_indices = self.sample(range(0, 32), vec_reg_count)
        max_reg_val_count = self.getLimitValue('MaxPhysicalVectorLen') // 64
        for vec_reg_index in vec_reg_indices:
            vec_reg_values = []

            state_elem_reg_val_count = RandomUtils.random32(1, max_reg_val_count)
            for val_index in range(state_elem_reg_val_count):
                vec_reg_values.append(RandomUtils.random64())

            vec_reg_name = 'v%d' % vec_reg_index
            self.randomInitializeRegister(vec_reg_name)
            aState.addRegisterStateElement(vec_reg_name, vec_reg_values)

            for val_index in range(state_elem_reg_val_count, max_reg_val_count):
                field_name = '%s_%d' % (vec_reg_name, val_index)
                (field_val, valid) = self.readRegister(vec_reg_name, field=field_name)
                state_transition_test_utils.assertValidRegisterValue(self, vec_reg_name, valid)
                vec_reg_values.append(field_val)

            expected_vec_reg_state_data[vec_reg_name] = vec_reg_values

        return expected_vec_reg_state_data


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

