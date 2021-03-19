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
import RandomUtils
import StateTransition
from Enums import EStateElementType, EStateTransitionType

from base.Sequence import Sequence
from base.StateTransitionHandler import StateTransitionHandler
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from riscv.Utils import LoadGPR64


#  A test StateTransitionHandler that verifies the boot process can be
#  overridden with a custom StateTransitionHandler.
class GprBootStateTransitionHandlerTest(StateTransitionHandler):
    def __init__(self, aGenThread):
        super().__init__(aGenThread)

        self.mMemBlockPtrIndex = None
        self.mMemBlockStartAddr = None
        self.mGprIndices = None
        self._mMemBlockAddrLoaded = False

    # Execute the State change represented by the StateElement. Only instances
    # of the StateElement types for which the StateTransitionHandler has been
    # registered will be passed to this method. Other StateTransitionHandlers
    # will process the other StateElement types. It is important to avoid
    # making changes to entities represented by StateElements that have
    # already been processed. Changes to entities represented by StateElements
    # that will be processed later are permitted.
    #
    #  @param aStateElem A StateElement object.
    def processStateElement(self, aStateElem):
        if aStateElem.getStateElementType() != EStateElementType.GPR:
            self.error(
                "Unexpected StateElement type %s"
                % aStateElem.getStateElementType()
            )

        # This implementation loads the memory block pointer GPR out of order,
        # so that it can be used to process all subsequent StateElements.
        if not self._mMemBlockAddrLoaded:
            load_gpr64_seq = LoadGPR64(self.genThread)
            load_gpr64_seq.load(
                self.mMemBlockPtrIndex, self.mMemBlockStartAddr
            )
            self._mMemBlockAddrLoaded = True

        gpr_index = aStateElem.getRegisterIndex()
        if gpr_index in self.mGprIndices:
            if self.getGlobalState("AppRegisterWidth") == 32:
                instr = "LW##RISCV"
            else:
                instr = "LD##RISCV"
            self.genInstruction(
                instr,
                {
                    "rd": gpr_index,
                    "rs1": self.mMemBlockPtrIndex,
                    "simm12": (gpr_index * 8),
                    "NoRestriction": 1,
                },
            )
        elif gpr_index != self.mMemBlockPtrIndex:
            load_gpr64_seq = LoadGPR64(self.genThread)
            load_gpr64_seq.load(gpr_index, aStateElem.getValues()[0])

        return True


# This test verifies that StateElements are merged correctly with current
# State values when expected.
class MainSequence(Sequence):
    def generate(self, **kargs):
        state_trans_handler = GprBootStateTransitionHandlerTest(self.genThread)
        StateTransition.registerStateTransitionHandler(
            state_trans_handler,
            EStateTransitionType.Boot,
            (EStateElementType.GPR,),
        )

        gpr_indices = self.getRandomGPRs(
            RandomUtils.random32(2, 15), exclude="0"
        )
        state_trans_handler.mMemBlockPtrIndex = gpr_indices[0]
        state_trans_handler.mGprIndices = set(gpr_indices[1:])

        # Initialize a memory block for the StateTransitionHandler to use to
        # load the GPR boot values
        mem_block_size = 32 * 8
        mem_block_start_addr = self.genVA(
            Size=mem_block_size, Align=8, Type="D"
        )
        self.initializeRegister(("x%d" % gpr_indices[0]), mem_block_start_addr)
        state_trans_handler.mMemBlockStartAddr = mem_block_start_addr

        for gpr_index in gpr_indices[1:]:
            gpr_entry_addr = mem_block_start_addr + gpr_index * 8
            gpr_val = RandomUtils.random64()
            self.initializeMemory(
                addr=gpr_entry_addr,
                bank=0,
                size=8,
                data=gpr_val,
                is_instr=False,
                is_virtual=True,
            )
            self.initializeRegister(("x%d" % gpr_index), gpr_val)

        if self.getGlobalState("AppRegisterWidth") == 32:
            instructions = (
                "ADD##RISCV",
                "LW##RISCV",
                "LUI##RISCV",
                "SW##RISCV",
                "SLLI#RV32I#RISCV",
                "SRLI#RV32I#RISCV",
            )
        else:
            instructions = (
                "ADD##RISCV",
                "ADDW##RISCV",
                "LD##RISCV",
                "LUI##RISCV",
                "SD##RISCV",
                "SLLI#RV64I#RISCV",
                "SRLI#RV64I#RISCV",
            )

        for _ in range(RandomUtils.random32(100, 200)):
            self.genInstruction(self.choice(instructions))


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
