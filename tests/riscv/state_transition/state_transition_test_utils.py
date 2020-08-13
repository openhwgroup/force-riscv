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
from riscv.Utils import LoadGPR64
from Enums import EStateElementType
import RandomUtils
import UtilityFunctions

## Add randomly-generated MemoryStateElements to the State. Return a list of MemoryStateElement
# starting address and value pairs.
#
#  @param aSequence A Sequence object.
#  @param aState The State to which the MemoryStateElements should be added.
#  @param aStateElemCount The number of MemoryStateElements to add.
#  @param aPrioirtyMin A lower bound on the priority of the MemoryStateElements.
#  @param aPriorityMax An upper bound on the priority of the MemoryStateElements.
def addRandomMemoryStateElements(aSequence, aState, aStateElemCount, aPriorityMin=1, aPriorityMax=100):
    expected_mem_state_data = []
    for _ in range(aStateElemCount):
        mem_start_addr = aSequence.genVA(Size=8, Align=8, Type='D')
        mem_val = RandomUtils.random64()
        aState.addMemoryStateElement(mem_start_addr, 8, mem_val, aPriority=RandomUtils.random32(aPriorityMin, aPriorityMax))
        expected_mem_state_data.append((mem_start_addr, mem_val))

    return expected_mem_state_data


## Add randomly-generated vector register StateElements to the State. Return a list of vector
# register StateElement name and value pairs.
#
#  @param aSequence A Sequence object.
#  @param aState The State to which the vector register StateElements should be added.
#  @param aStateElemCount The number of vector register StateElements to add.
#  @param aPrioirtyMin A lower bound on the priority of the vector register StateElements.
#  @param aPriorityMax An upper bound on the priority of the vector register StateElements.
def addRandomVectorRegisterStateElements(aSequence, aState, aStateElemCount, aPriorityMin=1, aPriorityMax=100):
    expected_vec_reg_state_data = []
    vec_reg_indices = aSequence.sample(range(0, 32), aStateElemCount)
    for vec_reg_index in vec_reg_indices:
        vec_reg_name = 'v%d' % vec_reg_index

        vec_reg_values = []

        # TODO(Noah): Retrieve the vector register length from an API when said API is created. For
        # now, we assume vector registers are 512 bits.
        for _ in range(8):
            vec_reg_values.append(RandomUtils.random64())

        aState.addRegisterStateElement(vec_reg_name, vec_reg_values, aPriority=RandomUtils.random32(aPriorityMin, aPriorityMax))
        expected_vec_reg_state_data.append((vec_reg_name, vec_reg_values))

    return expected_vec_reg_state_data


## Add randomly-generated GPR StateElements to the State. Return a list of GPR StateElement name and
# value pairs.
#
#  @param aSequence A Sequence object.
#  @param aState The State to which the GPR StateElements should be added.
#  @param aStateElemCount The number of GPR StateElements to add.
#  @param aPrioirtyMin A lower bound on the priority of the GPR StateElements.
#  @param aPriorityMax An upper bound on the priority of the GPR StateElements.
def addRandomGprStateElements(aSequence, aState, aStateElemCount, aPriorityMin=1, aPriorityMax=100):
    expected_gpr_state_data = []
    gpr_indices = aSequence.getRandomGPRs(aStateElemCount, exclude='0')
    for gpr_index in gpr_indices:
        gpr_name = 'x%d' % gpr_index
        gpr_val = RandomUtils.random64()
        aState.addRegisterStateElement(gpr_name, (gpr_val,), aPriority=RandomUtils.random32(aPriorityMin, aPriorityMax))
        expected_gpr_state_data.append((gpr_name, gpr_val))

    return expected_gpr_state_data


## Add a randomly-generated PcStateElement to the State. Return the PcStateElement value.
#
#  @param aSequence A Sequence object.
#  @param aState The State to which the PcStateElement should be added.
#  @param aPrioirtyMin A lower bound on the priority of the PcStateElement.
#  @param aPriorityMax An upper bound on the priority of the PcStateElement.
def addRandomPcStateElement(aSequence, aState, aPriorityMin=1, aPriorityMax=100):
    pc_val = aSequence.genVA(Size=4, Align=4, Type='I')
    aState.addPcStateElement(pc_val, aPriority=RandomUtils.random32(aPriorityMin, aPriorityMax))
    return pc_val


## Add randomly-generated floating point register StateElements to the State. Return a list of
# floating point register StateElement name and value pairs.
#
#  @param aSequence A Sequence object.
#  @param aState The State to which the floating point register StateElements should be added.
#  @param aStateElemCount The number of floating point register StateElements to add.
#  @param aPrioirtyMin A lower bound on the priority of the floating point register StateElements.
#  @param aPriorityMax An upper bound on the priority of the floating point register StateElements.
def addRandomFloatingPointRegisterStateElements(aSequence, aState, aStateElemCount, aPriorityMin=1, aPriorityMax=100):
    expected_fp_reg_state_data = []
    fp_reg_indices = aSequence.sample(range(0, 32), aStateElemCount)
    for fp_reg_index in fp_reg_indices:
        fp_reg_name_prefix = aSequence.choice(('S', 'D'))
        fp_reg_name = '%s%d' % (fp_reg_name_prefix, fp_reg_index)

        if fp_reg_name_prefix == 'S':
            fp_reg_val = RandomUtils.random32()
        else:
            fp_reg_val = RandomUtils.random64()

        aState.addRegisterStateElement(fp_reg_name, (fp_reg_val,), aPriority=RandomUtils.random32(aPriorityMin, aPriorityMax))
        expected_fp_reg_state_data.append((fp_reg_name, fp_reg_val))

    return expected_fp_reg_state_data


## Return the value of a register with the specified field value inserted at the appropriate place.
#
#  @param aSequence A Sequence object.
#  @param aRegName The name of the register.
#  @param aRegVal The current value of the register.
#  @param aRegFieldName The name of the register field.
#  @param aRegFieldVal The desired value of the register field.
def combineRegisterValueWithFieldValue(aSequence, aRegName, aRegVal, aRegFieldName, aRegFieldVal):
    (reg_field_mask, reg_field_reverse_mask) = aSequence.getRegisterFieldMask(aRegName, [aRegFieldName])
    reg_field_shift = UtilityFunctions.lowestBitSet(reg_field_mask)
    reg_field_val = (aRegFieldVal << reg_field_shift) & reg_field_mask
    reg_val = (aRegVal & reg_field_reverse_mask) | reg_field_val

    return reg_val


## Assert that the StateTransition brought the system to the specified State. This method may
# generate some instructions that modify the values of GPRs.
#
#  @param aSequence A Sequence object.
#  @param aExpectedStateData A dictionary mapping a StateElement type to a list of expected State
#       values for that StateElement type.
def verifyState(aSequence, aExpectedStateData):
    expected_state_data = aExpectedStateData.copy()

    # Verify PC and GPR State data first, as verifying some of the other State data may generate
    # instructions using GPRs to retrieve the values.
    if EStateElementType.PC in expected_state_data:
        _verifyPcState(aSequence, expected_state_data[EStateElementType.PC])
        del expected_state_data[EStateElementType.PC]

    if EStateElementType.GPR in expected_state_data:
        _verifyRegisterState(aSequence, expected_state_data[EStateElementType.GPR])
        del expected_state_data[EStateElementType.GPR]

    for (state_elem_type, state_elem_data) in expected_state_data.items():
        if state_elem_type == EStateElementType.Memory:
            _verifyMemoryState(aSequence, state_elem_data)
        elif state_elem_type == EStateElementType.SystemRegister:
            _verifyRegisterState(aSequence, state_elem_data)
        elif state_elem_type == EStateElementType.VectorRegister:
            _verifyVectorRegisterState(aSequence, state_elem_data)
        elif state_elem_type == EStateElementType.VmContext:
            _verifyVmContextState(aSequence, state_elem_data)
        elif state_elem_type == EStateElementType.PrivilegeLevel:
            _verifyPrivilegeLevelState(aSequence, state_elem_data)
        elif state_elem_type == EStateElementType.FloatingPointRegister:
            _verifyRegisterState(aSequence, state_elem_data)
        else:
            aSequence.error('Unexpected StateElement type %s' % state_elem_type)


## Fail if the valid flag is false.
#
#  @param aSequence A Sequence object.
#  @param aRegName The index of the register.
#  @param aValid A flag indicating whether the specified register has a valid value.
def assertValidRegisterValue(aSequence, aRegName, aValid):
    if not aValid:
        aSequence.error('Value for register %s is invalid' % aRegName)


## Assert that the actual register State matches the expected register State.
#
#  @param aSequence A Sequence object.
#  @param aExpectedRegStateData A list of expected register State values.
def _verifyRegisterState(aSequence, aExpectedRegStateData):
    for (reg_name, expected_reg_val) in aExpectedRegStateData:
        (reg_val, valid) = aSequence.readRegister(reg_name)
        assertValidRegisterValue(aSequence, reg_name, valid)

        if reg_val != expected_reg_val:
            aSequence.error('Value of register %s does not match the specified State. Expected=0x%x, Actual=0x%x' % (reg_name, expected_reg_val, reg_val))


## Assert that the actual memory State matches the expected memory State.
#
#  @param aSequence A Sequence object.
#  @param aExpectedMemStateData A list of expected memory State values.
def _verifyMemoryState(aSequence, aExpectedMemStateData):
    load_gpr64_seq = LoadGPR64(aSequence.genThread)
    (base_reg_index, mem_val_reg_index) = aSequence.getRandomGPRs(2, exclude='0')
    for (mem_start_addr, expected_mem_val) in aExpectedMemStateData:
        load_gpr64_seq.load(base_reg_index, mem_start_addr)
        aSequence.genInstruction('LD##RISCV', {'rd': mem_val_reg_index, 'rs1': base_reg_index, 'simm12': 0, 'NoRestriction': 1})

        mem_val_reg_name = 'x%d' % mem_val_reg_index
        (mem_val, valid) = aSequence.readRegister(mem_val_reg_name)
        assertValidRegisterValue(aSequence, mem_val_reg_name, valid)
        if mem_val != expected_mem_val:
            aSequence.error('Value at address 0x%x does not match the specified State. Expected=0x%x, Actual=0x%x' % (mem_start_addr, expected_mem_val, mem_val))


## Assert that the actual vector register State matches the expected vector register State.
#
#  @param aSequence A Sequence object.
#  @param aExpectedRegStateData A list of expected vector register State values.
def _verifyVectorRegisterState(aSequence, aExpectedRegStateData):
    for (reg_name, expected_reg_values) in aExpectedRegStateData:
        reg_index = int(reg_name[1:])
        reg_info = aSequence.getRegisterInfo(reg_name, reg_index)

        # TODO(Noah): Determine another way to retrieve the vector register value when the VL1R.V
        # instruction can be generated and simulated successfully. getRegisterInfo() doesn't return
        # the register value.
        reg_values = reg_info['Value']

        if reg_values != expected_reg_values:
            aSequence.error('Value of vector register %s does not match the specified State. Expected=%s, Actual=%s' % (reg_name, expected_reg_values, reg_values))


## Assert that the actual VM context State matches the expected VM context State.
#
#  @param aSequence A Sequence object.
#  @param aExpectedVmContextStateData A list of expected VM context State values.
def _verifyVmContextState(aSequence, aExpectedVmContextStateData):
    for (reg_name, reg_field_name, expected_reg_field_val) in aExpectedVmContextStateData:
        (reg_field_val, valid) = aSequence.readRegister(reg_name, reg_field_name)
        assertValidRegisterValue(aSequence, reg_name, valid)

        if reg_field_val != expected_reg_field_val:
            aSequence.error('Value of VM context parameter %s.%s does not match the specified State. Expected=0x%x, Actual=0x%x' % (reg_name, reg_field_name, expected_reg_field_val, reg_field_val))


## Assert that the actual privilege level State matches the expected privilege level State.
#
#  @param aSequence A Sequence object.
#  @param aExpectedPrivilegeLevelStateData The expected privilege level.
def _verifyPrivilegeLevelState(aSequence, aExpectedPrivLevelStateData):
    priv_level = aSequence.getPEstate('PrivilegeLevel')
    if priv_level != aExpectedPrivLevelStateData:
        aSequence.error('Current privilege level does not match the specified State. Expected=%d, Actual=%d' % (aExpectedPrivLevelStateData, priv_level))


## Assert that the actual PC State matches the expected PC State.
#
#  @param aSequence A Sequence object.
#  @param aExpectedPcStateData The expected PC value.
def _verifyPcState(aSequence, aExpectedPcStateData):
    pc_val = aSequence.getPEstate('PC')
    if pc_val != aExpectedPcStateData:
        aSequence.error('Value of PC does not match the specified State. Expected=0x%x, Actual=0x%x' % (aExpectedPcStateData, pc_val))
