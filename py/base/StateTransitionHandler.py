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
from base.Sequence import Sequence
from riscv.Utils import LoadGPR64
from Enums import EEndianness

## This class generates instructions to update the system State according to the StateElements
# provided to the processStateElement() and processStateElements() methods. These methods are
# intended to be overriden by subclasses in order to specify exactly how the State is updated.
class StateTransitionHandler(Sequence):

    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self.mStateTransType = None
        self._mArbitraryGprs = None

    ## Execute the State change represented by the StateElement. Only instances of the StateElement
    # types for which the StateTransitionHandler has been registered will be passed to this method.
    # Other StateTransitionHandlers will process the other StateElement types. It is important to
    # avoid making changes to entities represented by StateElements that have already been
    # processed. Changes to entities represented by StateElements that will be processed later are
    # permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        return False

    ## Execute the State changes represented by the StateElements. Only instances of the
    # StateElement types for which the StateTransitionHandler has been registered will be passed to
    # this method. Other StateTransitionHandlers will process the other StateElement types. It is
    # important to avoid making changes to entities represented by StateElements that have already
    # been processed. Changes to entities represented by StateElements that will be processed later
    # are permitted.
    #
    #  @param aStateElems A list of all StateElement objects of a particular type.
    def processStateElements(self, aStateElems):
        for state_elem in aStateElems:
            self.processStateElement(state_elem)

    ## Return randomly-selected indices of GPRs that have no assigned values in the State being
    # transitioned to. These GPRs may be modified freely to assist with the processing of
    # StateElements. This method returns None if there is an insufficient number of GPRs to satisfy
    # the request.
    #
    #  @param aGprCount The number of GPRs requested.
    #  @param aExclude A sequence of GPR indices to exclude from selection.
    def getArbitraryGprs(self, aGprCount, aExclude=None):
        arbitrary_gpr_indices = self.getAllArbitraryGprs(aExclude)
        if aGprCount <= len(arbitrary_gpr_indices):
            arbitrary_gpr_indices = self.sample(arbitrary_gpr_indices, aGprCount)
        else:
            arbitrary_gpr_indices = None

        return arbitrary_gpr_indices

    ## Return all available indices of GPRs that have no assigned values in the State being
    # transitioned to. These GPRs may be modified freely to assist with the processing of
    # StateElements.
    #
    #  @param aExclude A sequence of GPR indices to exclude from selection.
    def getAllArbitraryGprs(self, aExclude=None):
        arbitrary_gpr_indices = self._mArbitraryGprIndices
        if aExclude is not None:
            arbitrary_gpr_indices = frozenset(arbitrary_gpr_indices)
            arbitrary_gpr_indices = tuple(arbitrary_gpr_indices.difference(aExclude))

        return arbitrary_gpr_indices

    ## Specify a list of indicies of GPRs that have no assigned values in the State being
    # transitioned to.
    #
    #  @param aArbitraryGprIndices A list of indices of GPRs that have no assigned values in the
    #       State being transitioned to.
    def setArbitraryGprs(self, aArbitraryGprIndices):
        self._mArbitraryGprIndices = aArbitraryGprIndices

    ## Initialize a block of memory with the values contained in the specified StateElements.
    # Generate instructions to set a GPR to point to the start of the memory block. The memory block
    # is initialized with ascending addresses corresponding to ascending indices in the StateElement
    # list provided. For StateElements with multiple value elements, the value elements will be
    # arranged such that a single load instruction from the first memory address for that
    # StateElement will load the value elements in the correct order given the current endianness
    # configuration.
    #
    #  @param aMemBlockPtrIndex The index of a GPR that should be set to point to the memory block.
    #  @param aStateElems A list of StateElement objects with values that need to be placed into the
    #       memory block.
    def initializeMemoryBlock(self, aMemBlockPtrIndex, aStateElems):
        mem_block_size = 0
        alignment = 0
        for state_elem in aStateElems:
            byte_count = len(state_elem.getValues()) * 8
            mem_block_size += byte_count

            # Align the memory block to the largest StateElement size
            if byte_count > alignment:
                alignment = byte_count

        mem_block_start_addr = self.genVA(Size=mem_block_size, Align=alignment, Type='D')
        self._initializeMemoryWithStateElementValues(mem_block_start_addr, aStateElems)

        load_gpr64_seq = LoadGPR64(self.genThread)
        load_gpr64_seq.load(aMemBlockPtrIndex, mem_block_start_addr)

    ## Initialize a prepared block of memory with the values contained in the specified
    # StateElements. The memory block that begins as the specified starting address is assumed to be
    # large enough to hold all StateElement values.
    #
    #  @param aMemBlockStartAddr The starting virtual address of the memory block.
    #  @param aStateElems A list of StateElement objects with values that need to be placed into the
    #       memory block.
    def _initializeMemoryWithStateElementValues(self, aMemBlockStartAddr, aStateElems):
        endianness = EEndianness(self.getPEstate('Endianness'))
        cur_addr = aMemBlockStartAddr
        for state_elem in aStateElems:
            values = state_elem.getValues()

            if endianness == EEndianness.BigEndian:
                values = reversed(values)

            for value in values:
                self.initializeMemory(addr=cur_addr, bank=0, size=8, data=value, is_instr=False, is_virtual=True)
                cur_addr += 8
