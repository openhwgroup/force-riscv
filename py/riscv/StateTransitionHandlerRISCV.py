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
from base.StateTransitionHandler import StateTransitionHandler
from riscv.Utils import LoadGPR64
from Enums import EStateElementType, EStateTransitionType
import UtilityFunctions

## This class generates instructions to update the system State according to the MemoryStateElements
# provided to the processStateElement() and processStateElements() methods.
class MemoryStateTransitionHandlerRISCV(StateTransitionHandler):

    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self._mHelperGprSet = StateTransitionHelperGprSet(self)

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        if aStateElem.getStateElementType() != EStateElementType.Memory:
            return False

        (base_reg_index, mem_val_reg_index) = self._mHelperGprSet.acquireHelperGprs(2)

        load_gpr64_seq = LoadGPR64(self.genThread)
        load_gpr64_seq.load(base_reg_index, aStateElem.getStartAddress())
        load_gpr64_seq.load(mem_val_reg_index, aStateElem.getValues()[0])
        self.genInstruction('SD##RISCV', {'rs1': base_reg_index, 'rs2': mem_val_reg_index, 'simm12': 0, 'NoRestriction': 1})

        self._mHelperGprSet.releaseHelperGprs()

        return True

    ## Execute the State changes represented by the StateElements. Only instances of the
    # StateElement types for which the StateTransitionHandler has been registered will be passed to
    # this method. Other StateTransitionHandlers will process the other StateElement types. It is
    # important to avoid making changes to entities represented by StateElements that have already
    # been processed. Changes to entities represented by StateElements that will be processed later
    # are permitted.
    #
    #  @param aStateElems A list of all StateElement objects of a particular type.
    def processStateElements(self, aStateElems):
        if aStateElems[0].getStateElementType() != EStateElementType.Memory:
            self.error('This StateTransitionHandler can only process StateElements of type %s' % EStateElementType.Memory)

        (base_reg_index, mem_val_reg_index) = self._mHelperGprSet.acquireHelperGprs(2)

        load_gpr64_seq = LoadGPR64(self.genThread)
        offset_limit = 2 ** 11
        base_addr = -offset_limit
        for state_elem in aStateElems:
            load_gpr64_seq.load(mem_val_reg_index, state_elem.getValues()[0])

            # This is a minor optimization for the likely case in which the starting addresses of
            # consecutive State Elements are close together. Instead of loading the base register
            # from scratch, we can offset from its current value.
            offset = state_elem.getStartAddress() - base_addr
            if (offset < -offset_limit) or (offset >= offset_limit):
                base_addr = state_elem.getStartAddress()
                load_gpr64_seq.load(base_reg_index, base_addr)
                offset = 0

            self.genInstruction('SD##RISCV', {'rs1': base_reg_index, 'rs2': mem_val_reg_index, 'simm12': offset, 'NoRestriction': 1})

        self._mHelperGprSet.releaseHelperGprs()


## This class generates instructions to update the system State according to the vector register
# StateElements provided to the processStateElement() and processStateElements() methods.
class VectorRegisterStateTransitionHandlerRISCV(StateTransitionHandler):

    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self._mHelperGprSet = StateTransitionHelperGprSet(self)

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        if aStateElem.getStateElementType() != EStateElementType.VectorRegister:
            return False

        (base_reg_index,) = self._mHelperGprSet.acquireHelperGprs(1)

        # TODO(Noah): Finish implementing this method when the VL1R.V instruction can be generated
        # and simulated successfully.

        # TODO(Noah): Retrieve the vector register length from an API when said API is created. For
        # now, we assume vector registers are 128 bits.

        # target_addr = self.genVA(Size=16, Align=16, Type='D')
        #load_gpr64_seq = LoadGPR64(self.genThread)
        #load_gpr64_seq.load(base_reg_index, target_addr)
        #self.genInstruction('VL1R.V##RISCV', {'rd': aStateElem.getRegisterIndex(), 'rs1': base_reg_index, 'NoRestriction': 1})

        self._mHelperGprSet.releaseHelperGprs()

        return True

    ## Execute the State changes represented by the StateElements. Only instances of the
    # StateElement types for which the StateTransitionHandler has been registered will be passed to
    # this method. Other StateTransitionHandlers will process the other StateElement types. It is
    # important to avoid making changes to entities represented by StateElements that have already
    # been processed. Changes to entities represented by StateElements that will be processed later
    # are permitted.
    #
    #  @param aStateElems A list of all StateElement objects of a particular type.
    def processStateElements(self, aStateElems):
        if aStateElems[0].getStateElementType() != EStateElementType.VectorRegister:
            self.error('This StateTransitionHandler can only process StateElements of type %s' % EStateElementType.VectorRegister)

        # TODO(Noah): Finish implementing this method when the VL1R.V instruction can be generated
        # and simulated successfully.

        # The logic below assumes there will never be more than 2,048 vector register
        # StateElements
        if len(aStateElems) > 2048:
            self.error('Unexpected number of vector register StateElements: %d' % len(aStateElems))

        #(mem_block_ptr_index,) = self._mHelperGprSet.acquireHelperGprs(1)
        #self.initializeMemoryBlock(mem_block_ptr_index, aStateElems)

        #offset = 0
        #for state_elem in aStateElems:
            #self.genInstruction('VL1R.V##RISCV', {'rd': aStateElem.getRegisterIndex(), 'rs1': base_reg_index, 'NoRestriction': 1})
            #offset += 8

        #self._mHelperGprSet.releaseHelperGprs()


## This class generates instructions to update the system State according to the system register
# StateElements provided to the processStateElement() and processStateElements() methods.
class SystemRegisterStateTransitionHandlerRISCV(StateTransitionHandler):

    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self._mHelperGprSet = StateTransitionHelperGprSet(self)

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        if aStateElem.getStateElementType() != EStateElementType.SystemRegister:
            return False

        (reg_val_gpr_index, scratch_gpr_index) = self._mHelperGprSet.acquireHelperGprs(2, aValidate=False)

        load_gpr64_seq = LoadGPR64(self.genThread)
        load_gpr64_seq.load(reg_val_gpr_index, aStateElem.getValues()[0])
        if aStateElem.getName() == 'vtype':
            self._processVtypeStateElement(reg_val_gpr_index)
        elif aStateElem.getName() == 'vl':
            self._processVlStateElement(aStateElem, reg_val_gpr_index, scratch_gpr_index)
        else:
            self.genInstruction('CSRRW#register#RISCV', {'rd': 0, 'rs1': reg_val_gpr_index, 'csr': aStateElem.getRegisterIndex()})

        self._mHelperGprSet.releaseHelperGprs()

        return True

    ## Execute the State changes represented by the StateElements. Only instances of the
    # StateElement types for which the StateTransitionHandler has been registered will be passed to
    # this method. Other StateTransitionHandlers will process the other StateElement types. It is
    # important to avoid making changes to entities represented by StateElements that have already
    # been processed. Changes to entities represented by StateElements that will be processed later
    # are permitted.
    #
    #  @param aStateElems A list of all StateElement objects of a particular type.
    def processStateElements(self, aStateElems):
        if aStateElems[0].getStateElementType() != EStateElementType.SystemRegister:
            self.error('This StateTransitionHandler can only process StateElements of type %s' % EStateElementType.SystemRegister)

        (mem_block_ptr_index, reg_val_gpr_index, scratch_gpr_index) = self._mHelperGprSet.acquireHelperGprs(3)
        self.initializeMemoryBlock(mem_block_ptr_index, aStateElems)
        (mem_block_ptr_val, _) = self.readRegister('x%d' % mem_block_ptr_index)

        load_gpr64_seq = LoadGPR64(self.genThread)
        offset_limit = 2 ** 11
        offset = 0
        for state_elem in aStateElems:
            # This is a minor optimization that offsets from the memory block pointer when possible
            # rather than adjusting its value
            if offset >= offset_limit:
                mem_block_ptr_val += offset
                load_gpr64_seq.load(mem_block_ptr_index, mem_block_ptr_val)
                offset = 0

            self.genInstruction('LD##RISCV', {'rd': reg_val_gpr_index, 'rs1': mem_block_ptr_index, 'simm12': offset, 'NoRestriction': 1})
            if state_elem.getName() == 'vtype':
                self._processVtypeStateElement(reg_val_gpr_index)
            elif state_elem.getName() == 'vl':
                self._processVlStateElement(state_elem, reg_val_gpr_index, scratch_gpr_index)
            else:
                self.genInstruction('CSRRW#register#RISCV', {'rd': 0, 'rs1': reg_val_gpr_index, 'csr': state_elem.getRegisterIndex()})

            offset += 8

        self._mHelperGprSet.releaseHelperGprs()

    def _processVtypeStateElement(self, aRegValGprIndex):
        # Set rd and rs1 to x0 to preserve the value of vl
        self.genInstruction('VSETVL##RISCV', {'rd': 0, 'rs1': 0, 'rs2': aRegValGprIndex})

    def _processVlStateElement(self, aStateElem, aRegValGprIndex, aScratchGprIndex):
        # Read the current value of vtype and pass it to VSETVL in order to preserve the value of
        # vtype
        self.genInstruction('CSRRS#register#RISCV', {'rd': aScratchGprIndex, 'rs1': 0, 'csr': state_elem.getRegisterIndex()})
        self.genInstruction('VSETVL##RISCV', {'rd': 0, 'rs1': aRegValGprIndex, 'rs2': aScratchGprIndex})


## This class generates instructions to update the system State according to the GPR StateElements
# provided to the processStateElement() and processStateElements() methods.
class GprStateTransitionHandlerRISCV(StateTransitionHandler):

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        if aStateElem.getStateElementType() != EStateElementType.GPR:
            return False

        load_gpr64_seq = LoadGPR64(self.genThread)
        load_gpr64_seq.load(aStateElem.getRegisterIndex(), aStateElem.getValues()[0])
        return True

    ## Execute the State changes represented by the StateElements. Only instances of the
    # StateElement types for which the StateTransitionHandler has been registered will be passed to
    # this method. Other StateTransitionHandlers will process the other StateElement types. It is
    # important to avoid making changes to entities represented by StateElements that have already
    # been processed. Changes to entities represented by StateElements that will be processed later
    # are permitted.
    #
    #  @param aStateElems A list of all StateElement objects of a particular type.
    def processStateElements(self, aStateElems):
        if aStateElems[0].getStateElementType() != EStateElementType.GPR:
            self.error('This StateTransitionHandler can only process StateElements of type %s' % EStateElementType.GPR)

        # The logic below assumes there will never be more than 2,048 GPR StateElements
        if len(aStateElems) > 2048:
            self.error('Unexpected number of GPR StateElements: %d' % len(aStateElems))

        # Use the last GPR as the memory block pointer
        mem_block_ptr_index = aStateElems[-1].getRegisterIndex()
        self.initializeMemoryBlock(mem_block_ptr_index, aStateElems)

        offset = 0
        for state_elem in aStateElems[:-1]:
            self.genInstruction('LD##RISCV', {'rd': state_elem.getRegisterIndex(), 'rs1': mem_block_ptr_index, 'simm12': offset, 'NoRestriction': 1})
            offset += 8

        # Load the last StateElement value into the memory block pointer register as the last step
        self.genInstruction('LD##RISCV', {'rd': mem_block_ptr_index, 'rs1': mem_block_ptr_index, 'simm12': offset, 'NoRestriction': 1})


## This class generates instructions to update the system State according to the VM context
# StateElements provided to the processStateElement() and processStateElements() methods.
class VmContextStateTransitionHandlerRISCV(StateTransitionHandler):

    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self._mHelperGprSet = StateTransitionHelperGprSet(self)

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        if aStateElem.getStateElementType() != EStateElementType.VmContext:
            return False

        (reg_val_gpr_index,) = self._mHelperGprSet.acquireHelperGprs(1)

        # Ensure the register is initialized before attempting to read it
        self.randomInitializeRegister(aStateElem.getRegisterName())
        (reg_val, _) = self.readRegister(aStateElem.getRegisterName())

        reg_val = combineRegisterValueWithFieldValue(self, aStateElem.getRegisterName(), reg_val, aStateElem.getRegisterFieldName(), aStateElem.getValues()[0])

        load_gpr64_seq = LoadGPR64(self.genThread)
        load_gpr64_seq.load(reg_val_gpr_index, reg_val)
        self.genInstruction('CSRRW#register#RISCV', {'rd': 0, 'rs1': reg_val_gpr_index, 'csr': self.getRegisterIndex(aStateElem.getRegisterName())})
        self.updateVm()

        self._mHelperGprSet.releaseHelperGprs()

        return True

    ## Execute the State changes represented by the StateElements. Only instances of the
    # StateElement types for which the StateTransitionHandler has been registered will be passed to
    # this method. Other StateTransitionHandlers will process the other StateElement types. It is
    # important to avoid making changes to entities represented by StateElements that have already
    # been processed. Changes to entities represented by StateElements that will be processed later
    # are permitted.
    #
    #  @param aStateElems A list of all StateElement objects of a particular type.
    def processStateElements(self, aStateElems):
        if aStateElems[0].getStateElementType() != EStateElementType.VmContext:
            self.error('This StateTransitionHandler can only process StateElements of type %s' % EStateElementType.VmContext)

        (reg_val_gpr_index,) = self._mHelperGprSet.acquireHelperGprs(1)

        # Consolidate StateElements by register, so we only need to set each register once
        reg_fields = self._groupStateElementFieldsByRegister(aStateElems)

        load_gpr64_seq = LoadGPR64(self.genThread)
        for (reg_name, (reg_field_names, reg_field_values)) in reg_fields.items():
            # Ensure the register is initialized before attempting to read it
            self.randomInitializeRegister(reg_name)
            (reg_val, _) = self.readRegister(reg_name)

            for (i, reg_field_name) in enumerate(reg_field_names):
                reg_val = combineRegisterValueWithFieldValue(self, reg_name, reg_val, reg_field_name, reg_field_values[i])

            load_gpr64_seq.load(reg_val_gpr_index, reg_val)
            self.genInstruction('CSRRW#register#RISCV', {'rd': 0, 'rs1': reg_val_gpr_index, 'csr': self.getRegisterIndex(reg_name)})
            self.updateVm()

        self._mHelperGprSet.releaseHelperGprs()

    ## Create a dictionary of register names mapped to a tuple of a list of register field names and
    # a list of register field values.
    #
    #  @param aStateElems A list of all VmContextStateElement objects.
    def _groupStateElementFieldsByRegister(self, aStateElems):
        reg_fields = {}
        for state_elem in aStateElems:
            if state_elem.getRegisterName() in reg_fields:
                reg_fields_entry = reg_fields[state_elem.getRegisterName()]
                reg_fields_entry[0].append(state_elem.getRegisterFieldName())
                reg_fields_entry[1].append(state_elem.getValues()[0])
            else:
                reg_fields[state_elem.getRegisterName()] = ([state_elem.getRegisterFieldName()], [state_elem.getValues()[0]])

        return reg_fields


## This class generates instructions to update the system State according to the privlege level
# StateElements provided to the processStateElement() method. processStateElements() is not
# overriden because only one privilege level StateElement can be specified per State.
class PrivilegeLevelStateTransitionHandlerRISCV(StateTransitionHandler):

    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self._mHelperGprSet = StateTransitionHelperGprSet(self)

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        if aStateElem.getStateElementType() != EStateElementType.PrivilegeLevel:
            return False

        target_priv_level = aStateElem.getValues()[0]
        if target_priv_level != self.getPEstate('PrivilegeLevel'):
            self.systemCall({'PrivilegeLevel': target_priv_level})
        # Else we're already at the target privilege level, so there is nothing to do

        return True


## This class generates instructions to update the system State according to the PC StateElements
# provided to the processStateElement() method. processStateElements() is not overriden because only
# one PC StateElement can be specified per State.
class PcStateTransitionHandlerRISCV(StateTransitionHandler):

    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self._mHelperGprSet = StateTransitionHelperGprSet(self)

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        if aStateElem.getStateElementType() != EStateElementType.PC:
            return False

        target_addr = aStateElem.getValues()[0]

        # Try using a PC-relative branch first to limit the number of generated instructions; fall
        # back to a register branch if that fails.
        (branch_offset, is_valid, num_hw) = self.getBranchOffset(self.getPEstate('PC'), target_addr, 20, 1)
        if is_valid:
            self.genInstruction('JAL##RISCV', {'rd': 0, 'simm20': branch_offset, 'NoRestriction': 1})
        else:
            (branch_gpr_index,) = self._mHelperGprSet.acquireHelperGprs(1)

            load_gpr = LoadGPR64(self.genThread)
            load_gpr.load(branch_gpr_index, target_addr)
            self.genInstruction('JALR##RISCV', {'rd': 0, 'rs1': branch_gpr_index, 'simm12': 0, 'NoRestriction': 1})

            self._mHelperGprSet.releaseHelperGprs()

        return True


## This class generates instructions to update the system State according to the floating point
# register StateElements provided to the processStateElement() and processStateElements() methods.
class FloatingPointRegisterStateTransitionHandlerRISCV(StateTransitionHandler):

    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self._mHelperGprSet = StateTransitionHelperGprSet(self)

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        if aStateElem.getStateElementType() != EStateElementType.FloatingPointRegister:
            return False

        (reg_val_gpr_index,) = self._mHelperGprSet.acquireHelperGprs(1)

        # TODO(Noah): Handle Q regisers when the Q extension is supported.
        load_gpr64_seq = LoadGPR64(self.genThread)
        load_gpr64_seq.load(reg_val_gpr_index, aStateElem.getValues()[0])
        self.genInstruction('FMV.D.X##RISCV', {'rd': aStateElem.getRegisterIndex(), 'rs1': reg_val_gpr_index})

        self._mHelperGprSet.releaseHelperGprs()

        return True

    ## Execute the State changes represented by the StateElements. Only instances of the
    # StateElement types for which the StateTransitionHandler has been registered will be passed to
    # this method. Other StateTransitionHandlers will process the other StateElement types. It is
    # important to avoid making changes to entities represented by StateElements that have already
    # been processed. Changes to entities represented by StateElements that will be processed later
    # are permitted.
    #
    #  @param aStateElems A list of all StateElement objects of a particular type.
    def processStateElements(self, aStateElems):
        if aStateElems[0].getStateElementType() != EStateElementType.FloatingPointRegister:
            self.error('This StateTransitionHandler can only process StateElements of type %s' % EStateElementType.FloatingPointRegister)

        # The logic below assumes there will never be more than 2,048 floating point register
        # StateElements
        if len(aStateElems) > 2048:
            self.error('Unexpected number of floating point register StateElements: %d' % len(aStateElems))

        (mem_block_ptr_index,) = self._mHelperGprSet.acquireHelperGprs(1)
        self.initializeMemoryBlock(mem_block_ptr_index, aStateElems)

        offset = 0
        for state_elem in aStateElems:
            # TODO(Noah): Handle Q regisers when the Q extension is supported.
            self.genInstruction('FLD##RISCV', {'rd': state_elem.getRegisterIndex(), 'rs1': mem_block_ptr_index, 'simm12': offset, 'NoRestriction': 1})
            offset += 8

        self._mHelperGprSet.releaseHelperGprs()


## This class assists with allocating temporary GPRs for use in StateTransitionHandler methods. It
# uses arbitrary GPRs where possible. However, it can also return non-arbitrary GPRs and reset those
# GPRs when they are no longer needed, avoiding any permanent State modifications.
class StateTransitionHelperGprSet(object):

    def __init__(self, aStateTransHandler):
        self._mStateTransHandler = aStateTransHandler
        self._mArbitraryGprIndices = []
        self._mNonArbitraryGprIndices = []
        self._mNonArbitraryGprOrigValues = []

    ## Return GPRs that can be used to help effect a StateTransition. Arbitrary GPRs are returned
    # when possible. This call must be followed by a call to releaseHelperGprs().
    #
    #  @param aGprCount The number of GPRs requested.
    #  @param aValidate Verify the StateTransition type supports using non-arbitrary GPRs.
    def acquireHelperGprs(self, aGprCount, aValidate=True):
        if self._mNonArbitraryGprIndices:
            self.error('acquireHelperGprs() cannot be called without a matching call to releaseHelperGprs()')

        self._mArbitraryGprIndices = self._mStateTransHandler.getArbitraryGprs(aGprCount, aExclude=(0,1,2))
        if self._mArbitraryGprIndices is None:
            # TODO(Noah): Implement a better solution to having insufficient arbitrary GPRs during a
            # Boot StateTransition when such a solution can be devised. Ideally we would like to
            # restore non-arbitrary GPRs with their initial values rather than their current values
            # during a Boot StateTransition, but there is no convenient way to get that information
            # here at present. We expect that we only need one arbitrary GPR other than for system
            # registers, but the system registers are always loaded before the GPRs. The last GPR in
            # the boot sequence provides the arbitrary GPR we need.
            if aValidate:
                self._validateInsufficientArbitaryGprs(aGprCount)

            self._mArbitraryGprIndices = self._mStateTransHandler.getAllArbitraryGprs(aExclude=(0,1,2))
            remaining_gpr_count = aGprCount - len(self._mArbitraryGprIndices)
            excluded_regs = '0,1,2,%s' % ','.join(str(index) for index in self._mArbitraryGprIndices)
            self._mNonArbitraryGprIndices = self._mStateTransHandler.getRandomGPRs(remaining_gpr_count, exclude=excluded_regs, no_skip=True)

        for non_arbitrary_gpr_index in self._mNonArbitraryGprIndices:
            (gpr_val, _) = self._mStateTransHandler.readRegister('x%d' % non_arbitrary_gpr_index)
            self._mNonArbitraryGprOrigValues.append(gpr_val)

        helper_gpr_indices = list(self._mArbitraryGprIndices)
        helper_gpr_indices.extend(self._mNonArbitraryGprIndices)
        return helper_gpr_indices

    ## Indicate any acquired helper GPRs are no longer needed. Instructions are generated to reset
    # the values of any non-arbitrary GPRs.
    def releaseHelperGprs(self):
        load_gpr64_seq = LoadGPR64(self._mStateTransHandler.genThread)
        for (i, non_arbitrary_gpr_index) in enumerate(self._mNonArbitraryGprIndices):
            load_gpr64_seq.load(non_arbitrary_gpr_index, self._mNonArbitraryGprOrigValues[i])

        self._mNonArbitraryGprOrigValues = []
        self._mNonArbitraryGprIndices = []
        self._mArbitraryGprIndices = []

    ## Fail if the StateTransition type does not support proceeding without the requested number
    # arbitrary GPRs available.
    #
    #  @param aGprCount The number of GPRs requested.
    def _validateInsufficientArbitaryGprs(self, aGprCount):
        if self._mStateTransHandler.mStateTransType == EStateTransitionType.Boot:
            self._mStateTransHandler.error('Boot StateTransitions must have at least %d arbitary GPR(s) available' % aGprCount)


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
