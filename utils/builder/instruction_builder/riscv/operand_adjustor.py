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
from shared.instruction import Operand


class OperandAdjustor(object):
    def __init__(self, instr):
        self.mInstr = instr
        self.mAsmOpCount = 0

    def adjust_reg_opr(self, oprName, regType, size, access):
        opr = self.mInstr.find_operand(oprName)
        if regType == "GPR":
            opr.choices = "GPRs"
        elif regType == "FPR":
            self.set_fpr_choices(opr, size)

        opr.access = access
        self.add_asm_op(opr)

    def set_fpr_choices(self, opr, size):
        if size == 2:
            opr.choices = "16-bit SIMD/FP registers"
        if size == 4:
            opr.choices = "32-bit SIMD/FP registers"
        elif size == 8:
            opr.choices = "64-bit SIMD/FP registers"
        elif size == 16:
            opr.choices = "128-bit SIMD/FP registers"

    def set_rd_int(self):
        rd_opr = self.mInstr.find_operand("rd")
        rd_opr.access = "Write"
        self.set_reg_int(rd_opr)

    def set_rd_hp(self):
        rd_opr = self.mInstr.find_operand("rd")
        rd_opr.access = "Write"
        self.set_reg_hp(rd_opr)

    def set_rd_sp(self):
        rd_opr = self.mInstr.find_operand("rd")
        rd_opr.access = "Write"
        self.set_reg_sp(rd_opr)

    def set_rd_dp(self):
        rd_opr = self.mInstr.find_operand("rd")
        rd_opr.access = "Write"
        self.set_reg_dp(rd_opr)

    def set_rd_qp(self):
        rd_opr = self.mInstr.find_operand("rd")
        rd_opr.access = "Write"
        self.set_reg_qp(rd_opr)

    def set_rs1_int(self):
        rs1_opr = self.mInstr.find_operand("rs1")
        rs1_opr.access = "Read"
        self.set_reg_int(rs1_opr)

    def set_rs1_int_ls_base(self):
        rs1_opr = self.mInstr.find_operand("rs1")
        self.set_reg_nonzero_int(rs1_opr)

    def set_reg_nonzero_int(self, aSrcOpr):
        aSrcOpr.type = "GPR"
        aSrcOpr.choices = "Nonzero GPRs"
        self.add_asm_op(aSrcOpr)
        
    def set_rs2_int(self):
        rs2_opr = self.mInstr.find_operand("rs2")
        rs2_opr.access = "Read"
        self.set_reg_int(rs2_opr)

    def set_rs2_int_ls_base(self):
        rs2_opr = self.mInstr.find_operand("rs2")
        self.set_reg_nonzero_int(rs2_opr)

    def set_rs3_int(self):
        rs3_opr = self.mInstr.find_operand("rs3")
        rs3_opr.access = "Read"
        self.set_reg_int(rs3_opr)

    def set_rs1_hp(self):
        rs1_opr = self.mInstr.find_operand("rs1")
        rs1_opr.access = "Read"
        self.set_reg_hp(rs1_opr)

    def set_rs1_sp(self):
        rs1_opr = self.mInstr.find_operand("rs1")
        rs1_opr.access = "Read"
        self.set_reg_sp(rs1_opr)

    def set_rs2_hp(self):
        rs2_opr = self.mInstr.find_operand("rs2")
        rs2_opr.access = "Read"
        self.set_reg_hp(rs2_opr)

    def set_rs2_sp(self):
        rs2_opr = self.mInstr.find_operand("rs2")
        rs2_opr.access = "Read"
        self.set_reg_sp(rs2_opr)

    def set_rs3_hp(self):
        rs3_opr = self.mInstr.find_operand("rs3")
        rs3_opr.access = "Read"
        self.set_reg_hp(rs3_opr)

    def set_rs3_sp(self):
        rs3_opr = self.mInstr.find_operand("rs3")
        rs3_opr.access = "Read"
        self.set_reg_sp(rs3_opr)

    def set_rs1_dp(self):
        rs1_opr = self.mInstr.find_operand("rs1")
        rs1_opr.access = "Read"
        self.set_reg_dp(rs1_opr)

    def set_rs2_dp(self):
        rs2_opr = self.mInstr.find_operand("rs2")
        rs2_opr.access = "Read"
        self.set_reg_dp(rs2_opr)

    def set_rs3_dp(self):
        rs3_opr = self.mInstr.find_operand("rs3")
        rs3_opr.access = "Read"
        self.set_reg_dp(rs3_opr)

    def set_rs1_qp(self):
        rs1_opr = self.mInstr.find_operand("rs1")
        rs1_opr.access = "Read"
        self.set_reg_qp(rs1_opr)

    def set_rs2_qp(self):
        rs2_opr = self.mInstr.find_operand("rs2")
        rs2_opr.access = "Read"
        self.set_reg_qp(rs2_opr)

    def set_rs3_qp(self):
        rs3_opr = self.mInstr.find_operand("rs3")
        rs3_opr.access = "Read"
        self.set_reg_qp(rs3_opr)

    def set_reg_int(self, aSrcOpr):
        aSrcOpr.type = "GPR"
        aSrcOpr.choices = "GPRs"
        self.add_asm_op(aSrcOpr)

    def set_reg_hp(self, aSrcOpr):
        aSrcOpr.type = "FPR"
        aSrcOpr.choices = "16-bit SIMD/FP registers"
        self.add_asm_op(aSrcOpr)

    def set_reg_sp(self, aSrcOpr):
        aSrcOpr.type = "FPR"
        aSrcOpr.choices = "32-bit SIMD/FP registers"
        self.add_asm_op(aSrcOpr)

    def set_reg_dp(self, aSrcOpr):
        aSrcOpr.type = "FPR"
        aSrcOpr.choices = "64-bit SIMD/FP registers"
        self.add_asm_op(aSrcOpr)

    def set_reg_qp(self, aSrcOpr):
        aSrcOpr.type = "FPR"
        aSrcOpr.choices = "128-bit SIMD/FP registers"
        self.add_asm_op(aSrcOpr)

    def set_imm(self, old_name, new_name, is_signed):
        imm_opr = self.mInstr.find_operand(old_name)
        imm_opr.name = new_name
        if is_signed:
            imm_opr.set_attribute("class", "SignedImmediateOperand")

        self.add_asm_op(imm_opr)

    def set_rm(self):
        rm_opr = self.mInstr.find_operand("rm")
        rm_opr.type = "Choices"
        rm_opr.choices = "Rounding mode"

    # add operand to ASM line.
    def add_asm_op(self, opr):
        if self.mAsmOpCount == 0:
            self.mInstr.asm.format += " %s"
        else:
            self.mInstr.asm.format += ", %s"

        self.mAsmOpCount += 1
        self.mInstr.asm.ops.append(opr.name)

    # add an implied register operand to the instruction.
    def add_implied_register(
        self, aOpName, aOpType, aAccessType, aInsertIndex
    ):
        implied_opr = Operand()
        implied_opr.name = aOpName
        implied_opr.type = aOpType
        implied_opr.oclass = "ImpliedRegisterOperand"
        if aAccessType:
            implied_opr.access = aAccessType
        self.mInstr.insert_operand(aInsertIndex, implied_opr)

    # merge a const field into const_bits field and delete the passed
    # in operand
    def merge_into_const_bits(self, aOtherConst):
        const_bits = self.mInstr.find_operand("const_bits")
        const_bits.merge_operand(aOtherConst)
        const_bits.update_bits_value()
        self.mInstr.operands.remove(aOtherConst)
        
    ########################################
    # C extension operands
    #
    def set_rs1p_or_rdp_int(self):
        reg_opr = self.mInstr.find_operand("rs1'/rd'")
        reg_opr.name = "rd'"
        reg_opr.access = "ReadWrite"
        self.set_reg_prime_int(reg_opr)

    def set_rdp_int(self):
        reg_opr = self.mInstr.find_operand("rd'")
        reg_opr.access = "Write"
        self.set_reg_prime_int(reg_opr)

    def set_rdp_sp(self):
        reg_opr = self.mInstr.find_operand("rd'")
        reg_opr.access = "Write"
        self.set_reg_prime_sp(reg_opr)

    def set_rdp_dp(self):
        reg_opr = self.mInstr.find_operand("rd'")
        reg_opr.access = "Write"
        self.set_reg_prime_dp(reg_opr)
        
    def set_rs2p_int(self):
        reg_opr = self.mInstr.find_operand("rs2'")
        reg_opr.access = "Read"
        self.set_reg_prime_int(reg_opr)

    def set_rs2p_sp(self):
        reg_opr = self.mInstr.find_operand("rs2'")
        reg_opr.access = "Read"
        self.set_reg_prime_sp(reg_opr)

    def set_rs2p_dp(self):
        reg_opr = self.mInstr.find_operand("rs2'")
        reg_opr.access = "Read"
        self.set_reg_prime_dp(reg_opr)
        
    def set_rs1p_int(self):
        reg_opr = self.mInstr.find_operand("rs1'")
        reg_opr.access = "Read"
        self.set_reg_prime_int(reg_opr)
        
    def set_reg_prime_int(self, aSrcOpr):
        aSrcOpr.type = "GPR"
        aSrcOpr.choices = "Prime GPRs"
        aSrcOpr.oclass = "CompressedRegisterOperandRISCV"
        self.add_asm_op(aSrcOpr)

    def set_reg_prime_sp(self, aSrcOpr):
        aSrcOpr.type = "FPR"
        aSrcOpr.choices = "Prime 32-bit SIMD/FP registers"
        self.add_asm_op(aSrcOpr)

    def set_reg_prime_dp(self, aSrcOpr):
        aSrcOpr.type = "FPR"
        aSrcOpr.choices = "Prime 64-bit SIMD/FP registers"
        self.add_asm_op(aSrcOpr)
    
    def set_reg_not02_int(self, aSrcOpr):
        aSrcOpr.type = "GPR"
        aSrcOpr.choices = "GPRs not x0, x2"
        self.add_asm_op(aSrcOpr)
        
    def merge_imm_5_4_0(self):
        self.mInstr.remove_operand("imm[5]")
        imm_4_0 = self.mInstr.find_operand("imm[4:0]")
        imm_4_0.name = "imm6"
        imm_4_0.bits = "12,6-2"
        self.add_asm_op(imm_4_0)

    def set_rd_rs1_nonzero_int(self):
        reg_opr = self.mInstr.find_operand("rs1/rd$\\neq$0")
        reg_opr.name = "rd"
        reg_opr.access = "ReadWrite"
        self.set_reg_nonzero_int(reg_opr)

    def set_rd_nonzero_int(self):
        reg_opr = self.mInstr.find_operand("rd$\\neq$0")
        reg_opr.name = "rd"
        reg_opr.access = "Write"
        self.set_reg_nonzero_int(reg_opr)

    def set_rs1_nonzero_int(self):
        reg_opr = self.mInstr.find_operand("rs1$\\neq$0")
        reg_opr.name = "rs1"
        reg_opr.access = "Read"
        self.set_reg_nonzero_int(reg_opr)
        
    def set_rs2_nonzero_int(self):
        reg_opr = self.mInstr.find_operand("rs2$\\neq$0")
        reg_opr.name = "rs2"
        reg_opr.access = "Read"
        self.set_reg_nonzero_int(reg_opr)
        
    def set_rd_not02_int(self):
        reg_opr = self.mInstr.find_operand("rd$\\neq$$\\{0,2\\}$")
        reg_opr.name = "rd"
        reg_opr.access = "Write"
        self.set_reg_not02_int(reg_opr)

    def set_nzimm(self, aSrcOpr):
        aSrcOpr.oclass = "ImmediateExcludeOperand"
        aSrcOpr.exclude = "0"
        self.add_asm_op(aSrcOpr)
        
    def merge_nzimm_17_16_12(self):
        self.mInstr.remove_operand("nzimm[17]")
        imm_16_12 = self.mInstr.find_operand("nzimm[16:12]")
        imm_16_12.name = "imm6"
        imm_16_12.bits = "12,6-2"
        self.set_nzimm(imm_16_12)

    def merge_nzimm_5_4_0(self):
        self.mInstr.remove_operand("nzimm[5]")
        imm_4_0 = self.mInstr.find_operand("nzimm[4:0]")
        imm_4_0.name = "imm6"
        imm_4_0.bits = "12,6-2"
        self.set_nzimm(imm_4_0)

    def merge_nzuimm_5_4_0(self, aNewName):
        self.mInstr.remove_operand("nzuimm[5]")
        imm_4_0 = self.mInstr.find_operand("nzuimm[4:0]")
        imm_4_0.bits = "12,6-2"
        if aNewName is not None:
            imm_4_0.name = aNewName
        else:
            imm_4_0.name = "imm6"
        self.set_nzimm(imm_4_0)
