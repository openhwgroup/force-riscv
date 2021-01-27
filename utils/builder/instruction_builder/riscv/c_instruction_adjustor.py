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
from abc import ABC, abstractmethod
from shared.instruction_file import InstructionFile
from adjustor_utilities import *
from instruction_adjustor import InstructionAdjustor
import copy
import re

class C_InstructionAdjustor(InstructionAdjustor):

    def __init__(self):
        super().__init__()


    def adjust_instruction_by_format(self, aInstr):

        self.isLoadStoreOp = False
        self.isFloatOp = False
        self.isWordOp = False
        self.isReadOp = False

        if aInstr.name in ['C.LW',  'C.LD',  'C.SW',  'C.SD', \
                           'C.FLW', 'C.FLD', 'C.FSW', 'C.FSD', \
                           'C.LWSP',  'C.LDSP',  'C.SWSP',  'C.SDSP', \
                           'C.FLWSP', 'C.FLDSP', 'C.FSWSP', 'C.FSDSP', ]:   self.isLoadStoreOp = True
        if aInstr.name in ['C.FLW',   'C.FLD',   'C.FSW',   'C.FSD', \
                           'C.FLWSP', 'C.FLDSP', 'C.FSWSP', 'C.FSDSP', ]:   self.isFloatOp = True
        if aInstr.name in ['C.LW',   'C.SW',   'C.FLW',   'C.FSW', \
                           'C.LWSP', 'C.SWSP', 'C.FLWSP', 'C.FSWSP']:        self.isWordOp = True
        if aInstr.name in ['C.LW',  'C.LD',  'C.LWSP',  'C.LDSP', \
                           'C.FLW', 'C.FLD', 'C.FLWSP', 'C.FLDSP']:         self.isReadOp = True

        self.regType = 'FPR' if self.isFloatOp else 'GPR'
        self.size =  4 if self.isWordOp else 8
        self.scale = 2 if self.isWordOp else 3

        #print(">>>>>>>  Processing this instruction: {}".format(aInstr.name))

        # Process all of the operands for this instruction
        # - extra attributes for the operands are added as needed to get the 
        #   final format in the c_instructions*.xml files.
        for opr in aInstr.operands:
            #print(">>>>>>>  Processing this operand: {}".format(opr.name))
            if   opr.name in ["rs1", "rs1'"]:       self.adjust_rs1(aInstr, opr)
            elif opr.name in ["rs2", "rs2'"]:       self.adjust_rs2(aInstr, opr)
            elif opr.name in ["rd", "rd'"]:         self.adjust_rd(aInstr, opr)
            elif opr.name.startswith('imm'):        self.adjust_imm(aInstr, opr)
            elif opr.name.startswith('simm'):       self.adjust_simm(aInstr, opr)
            elif opr.name == "shamt":               self.adjust_imm(aInstr, opr)
            else:
                pass
                #print('operand not parsed for {}, operand: {}'.format(aInstr.name, opr.name))


        # Create the asm record that goes for the instruction
        # - This must be called before any addressing operand or implied 
        #   operands are added.
        # - 3 operand ld/st ops need op1 and op2 explicitly ordered:
        #     loads:    opcode  rd', rs1', imm5
        #     stores:   opcode  rs2', rs1', imm5
        if aInstr.name in ['C.LW', 'C.LD', 'C.FLW', 'C.FLD',
                           'C.SW', 'C.SD', 'C.FSW', 'C.FSD']:
            if len(aInstr.operands) == 3:
                tmp = list()
                for opr in aInstr.operands:
                    tmp.append(opr)
                for opr in tmp:
                    if opr.name == "rd'":    aInstr.operands[0] = opr     # loads
                    if opr.name == "rs1'":   aInstr.operands[1] = opr     # both
                    if opr.name == "imm5":   aInstr.operands[2] = opr     # both
                    if opr.name == "rs2'":   aInstr.operands[0] = opr     # stores
            else:
                raise BuilderException("Instruction {} should have 3 operands, " + \
                    "but has {}".format(aInstr.name, len(aInstr.operands)))

        gen_asm_operand(aInstr)


        # Finish processing for each instruction
        # - Create implied register operands
        # - Create addressing operands (ld/st and branch/jump)
        # - set the 'class' attribute for the instruction 
        opr_adjustor = OperandAdjustor(aInstr)

        if aInstr.name in ['C.BEQZ', 'C.BNEZ']:
            aInstr.iclass = 'BranchInstruction'
            # Adding addressing operands: simm8, x0 which is implicitly read
            add_cond_branch_operand(aInstr, 'simm8', 1, 'CompressedConditionalBranchOperandRISCV')
            opr_adjustor.add_implied_register("x0", "GPR", "Read", 1)

        if aInstr.name in ['C.J', 'C.JAL']:
            aInstr.iclass = 'BranchInstruction'
            # Adding addressing operand
            add_pc_rel_branch_operand(aInstr, 'simm11', 1)

        if aInstr.name in ['C.JR', 'C.JALR']:
            aInstr.iclass = 'BranchInstruction'
            # Adding addressing operand
            add_reg_branch_operand(aInstr, 'rs1', 0)

        if aInstr.name in ['C.JAL', 'C.JALR']:
            # Adding implicit write of the link register
            opr_adjustor.add_implied_register("x1", "GPR", "Write", 1)

        if aInstr.name in ['C.ADDI4SPN']:
            opr_adjustor.add_implied_register("x2", "GPR", "Read", 2)

        if aInstr.name in ['C.ADDI16SP']:
            opr_adjustor.add_implied_register("x2", "GPR", "ReadWrite", 2)

        if self.isLoadStoreOp:
            aInstr.iclass = 'LoadStoreInstruction'
            mem_access = 'Read' if self.isReadOp else 'Write'
            # Non-stack-pointer loads/stores
            if opr.name == 'imm5':
                add_bols_addr_operand(aInstr, "rs1'", "imm5", mem_access, self.size, self.scale)
            # Stack-pointer loads/stores
            else:
                opr_adjustor.add_implied_register("x2", "GPR", "Read", 1)
                add_bols_addr_operand(aInstr, "x2", "imm6", mem_access, self.size, self.scale)


    # Methods to process the specific operands are below.

    def adjust_rs1(self, aInstr, aOpr):

        # C ext instructions that use "rs1":  C.JR, C.JALR
        if 'rs1' == aOpr.name:         
            adjust_gpr_operand(aOpr, 'Read', 'GPR', 0, 'Nonzero GPRs')      # the self.size is not used

        # C ext instructions that use "rs1'" independent of "rd'": C.BEQZ, C.BNEZ, C.FLD, C.FSD,
        #   C.LW, C.SW, C.LD, C.SD, C.FLW, C.FSW
        else:
            adjust_gpr_operand(aOpr, 'Read', 'GPR', self.size, 'Prime GPRs')


    def adjust_rs2(self, aInstr, aOpr):

        # C ext instructions that use "rs2":  C.ADD, C.MV, C.FSDSP, C.SWSP, C.SDSP
        if "rs2" == aOpr.name:
            if aInstr.name in ['C.ADD', 'C.MV']:
                adjust_gpr_operand(aOpr, 'Read', self.regType, self.size, 'Nonzero GPRs')
            # The other C ext instructions using "rs2".
            else:
                adjust_gpr_operand(aOpr, 'Read', self.regType, self.size)
            
        # C ext instructions that use"rs2'":  C.ADDW, C.SD, C.SUBW, C.AND, C.OR, C.SUB,
        #    C.SW, C.XOR, C.FSD, C.FSW
        else:
            if self.isFloatOp:
                arch = 32 if aInstr.name == 'C.FSW' else 64
                choices = 'Prime {}-bit SIMD/FP registers'.format(arch)
                adjust_gpr_operand(aOpr, 'Read', self.regType, self.size, choices)
            else:
                adjust_gpr_operand(aOpr, 'Read', self.regType, self.size, 'Prime GPRs')
                

    def adjust_rd(self, aInstr, aOpr):
        
        # C ext instructions that use "rd":  C.ADD, C.ADDI, C.ADDIW, C.SLLI, C.SLLI64
        #    C.LI, C.LWSP, C.MV, C.FLWSP, C.FLDSP, C.LUI 
        if "rd" == aOpr.name:
            if self.isFloatOp:
                arch = 32 if aInstr.name == 'C.FLWSP' else 64
                choices = '{}-bit SIMD/FP registers'.format(arch)
                adjust_gpr_operand(aOpr, 'Write', self.regType, self.size, choices)
            elif aInstr.name == 'C.LUI':
                adjust_gpr_operand(aOpr, 'Write', self.regType, self.size, 'GPRs not x0, x2')
            elif aInstr.name in ['C.LI', 'C.LWSP', 'C.LDSP', 'C.MV']:
                adjust_gpr_operand(aOpr, 'Write', self.regType, self.size, 'Nonzero GPRs')
            else:
                adjust_gpr_operand(aOpr, 'ReadWrite', self.regType, self.size, 'Nonzero GPRs')

        # C ext instructions that use "rd'":  C.AND, C.ANDI, C.OR, C.SRAI, C.SRAI64,
        #    C.SRLI, C.SRLI64, C.SUB, C.XOR, C.ADDW, C.SUBW, C.LW, C.ADDI4SPN, C.FLW, C.FLD
        else:
            if self.isFloatOp:
                arch = 32 if aInstr.name == 'C.FLW' else 64
                choices = 'Prime {}-bit SIMD/FP registers'.format(arch)
                adjust_gpr_operand(aOpr, 'Write', self.regType, self.size, choices)
            elif aInstr.name in ['C.LW', 'C.LD', 'C.ADDI4SPN']:
                adjust_gpr_operand(aOpr, 'Write', self.regType, self.size, 'Prime GPRs')
            else:
                adjust_gpr_operand(aOpr, 'ReadWrite', self.regType, self.size, 'Prime GPRs')


    def adjust_imm(self, aInstr, aOpr):

        # C ext instructions that use "imm5", "imm6", "imm8" or "shamt" operands processed here.
        #    "imm5":  C.LW, C.SW, C.FLW, C.FSW, C.FLD, C.FSD, C.SD, C.LD
        #    "imm6":  C.ADDI16SP, C.ANDI, C.LI, C.ADDIW, C.ADDI, C.LUI,
        #             C.LWSP, C.FLWSP, C.FLDSP, C.LDSP, C.SWSP, C.FSDSP,
        #             C.SDSP, C.FSWSP
        #    "imm8":  C.ADDI4SPN
        #    "shamt": C.SLLI, C.SRAI, C.SRLI

        if aInstr.name in ['C.ADDI4SPN', 'C.ADDI16SP', 'C.ADDI', 'C.LUI', 'C.SLLI', 'C.SRAI', 'C.SRLI']:
            aOpr.set_attribute('class', 'ImmediateExcludeOperand')
            aOpr.set_attribute('exclude', '0')


    def adjust_simm(self, aInstr, aOpr):

        # C ext instructions that use "simm8" or "simm11" operands processed here.
        #   "simm8":  C.BEQZ, C.BNEZ
        #   "simm11": C.J, C.JAL

        aOpr.set_attribute('class', 'SignedImmediateOperand')
            
