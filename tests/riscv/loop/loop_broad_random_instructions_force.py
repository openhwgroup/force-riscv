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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from LoopTestSequence import LoopTestSequence

## This test verifies that a standard loop can execute a wide variety of instructions and still
# terminate successfully.
class MainSequence(LoopTestSequence):

    def __init__(self, gen_thread, name=None):
        super().__init__(gen_thread, name)

        # TODO(Noah): Uncomment the load and store instructions when the Handcar memory
        # initialization issue is resolved.
        # TODO(Noah): Uncomment the branch instructions when running Force with co-simulation is
        # enabled for RISCV.
        
        self._mInstructionWeights = {
            'ADD##RISCV': 10,
            'ADDI##RISCV': 10,
            'ADDIW##RISCV': 10,
            'ADDW##RISCV': 10,
            'AND##RISCV': 10,
            'ANDI##RISCV': 10,
            'AUIPC##RISCV': 10,
            #'BEQ##RISCV': 10,
            #'BGE##RISCV': 10,
            #'BGEU##RISCV': 10,
            #'BLT##RISCV': 10,
            #'BLTU##RISCV': 10,
            #'BNE##RISCV': 10,
            'DIV##RISCV': 10,
            'DIVU##RISCV': 10,
            'DIVUW##RISCV': 10,
            'DIVW##RISCV': 10,
            'JAL##RISCV': 10,
            'JALR##RISCV': 10,
            #'LB##RISCV': 10,
            #'LBU##RISCV': 10,
            #'LD##RISCV': 10,
            #'LH##RISCV': 10,
            #'LHU##RISCV': 10,
            'LUI##RISCV': 10,
            #'LW##RISCV': 10,
            #'LWU##RISCV': 10,
            'MUL##RISCV': 10,
            'MULH##RISCV': 10,
            'MULHSU##RISCV': 10,
            'MULHU##RISCV': 10,
            'MULW##RISCV': 10,
            'OR##RISCV': 10,
            'ORI##RISCV': 10,
            'REM##RISCV': 10,
            'REMU##RISCV': 10,
            'REMUW##RISCV': 10,
            'REMW##RISCV': 10,
            #'SB##RISCV': 10,
            #'SD##RISCV': 10,
            #'SH##RISCV': 10,
            'SLL##RISCV': 10,
            'SLLI#RV64I#RISCV': 10,
            'SLLIW##RISCV': 10,
            'SLLW##RISCV': 10,
            'SLT##RISCV': 10,
            'SLTI##RISCV': 10,
            'SLTIU##RISCV': 10,
            'SLTU##RISCV': 10,
            'SRA##RISCV': 10,
            'SRAI#RV64I#RISCV': 10,
            'SRAIW##RISCV': 10,
            'SRAW##RISCV': 10,
            'SRL##RISCV': 10,
            'SRLI#RV64I#RISCV': 10,
            'SRLIW##RISCV': 10,
            'SRLW##RISCV': 10,
            'SUB##RISCV': 10,
            'SUBW##RISCV': 10,
            #'SW##RISCV': 10,
            'XOR##RISCV': 10,
            'XORI##RISCV': 10,
        }

        if self.getGlobalState('AppRegisterWidth') == 32:
            self._mInstructionWeights = {
                'ADD##RISCV': 10,
                'ADDI##RISCV': 10,
                'AND##RISCV': 10,
                'ANDI##RISCV': 10,
                'AUIPC##RISCV': 10,
                #'BEQ##RISCV': 10,
                #'BGE##RISCV': 10,
                #'BGEU##RISCV': 10,
                #'BLT##RISCV': 10,
                #'BLTU##RISCV': 10,
                #'BNE##RISCV': 10,
                'DIV##RISCV': 10,
                'DIVU##RISCV': 10,
                'JAL##RISCV': 10,
                'JALR##RISCV': 10,
                #'LB##RISCV': 10,
                #'LBU##RISCV': 10,
                #'LH##RISCV': 10,
                #'LHU##RISCV': 10,
                'LUI##RISCV': 10,
                #'LW##RISCV': 10,
                #'LWU##RISCV': 10,
                'MUL##RISCV': 10,
                'MULH##RISCV': 10,
                'MULHSU##RISCV': 10,
                'MULHU##RISCV': 10,
                'OR##RISCV': 10,
                'ORI##RISCV': 10,
                'REM##RISCV': 10,
                'REMU##RISCV': 10,
                #'SB##RISCV': 10,
                #'SH##RISCV': 10,
                'SLL##RISCV': 10,
                'SLLI#RV32I#RISCV': 10,
                'SLT##RISCV': 10,
                'SLTI##RISCV': 10,
                'SLTIU##RISCV': 10,
                'SLTU##RISCV': 10,
                'SRA##RISCV': 10,
                'SRAI#RV32I#RISCV': 10,
                'SRL##RISCV': 10,
                'SRLI#RV32I#RISCV': 10,
                'SUB##RISCV': 10,
                #'SW##RISCV': 10,
                'XOR##RISCV': 10,
                'XORI##RISCV': 10,
        }
        
    ## Return a dictionary of names of instructions to generate in the loop body with their
    # corresponding weights.
    def getInstructionWeights(self):
        return self._mInstructionWeights


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV

