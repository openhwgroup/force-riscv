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
import copy
import re
from abc import ABC, abstractmethod

from shared.instruction_file import InstructionFile

from adjustor_utilities import *


class InstructionAdjustor(ABC):

    def __init__(self):
        self.mSupportedInstrFile = InstructionFile()

    @abstractmethod
    def adjust_instruction_by_format(self, aInstr):
        pass

    def adjust_instruction(self, aInstr):
        self.adjust_instruction_by_format(aInstr)
        self.mSupportedInstrFile.add_instruction(copy.deepcopy(aInstr))


class G_InstructionAdjustor(InstructionAdjustor):

    def __init__(self):
        super().__init__()
        self._mFpRdGprPattern = re.compile(r'FLT|FLE|FEQ|FCLASS')
        self._mFpSizePattern = re.compile(r'^F[A-Z]*.(?P<size>\w*)$')
        self._mFpSrcDstPattern = re.compile(r'^F[A-Z]*.(?P<dst>\w*).'
                                            r'(?P<src>\w*)$')
        self._mLdStPattern = re.compile(r'^F{0,1}(?P<access>[LS])'
                                        r'(?P<size>[A-Z])U{0,1}$')

    def adjust_instruction_by_format(self, aInstr):
        for opr in aInstr.operands:
            if opr.name.startswith('rs'):
                self.adjust_rs(aInstr, opr)
            elif 'rm' in opr.name:
                self.adjust_rm(aInstr, opr)
            elif 'rd' in opr.name:
                self.adjust_rd(aInstr, opr)
            elif opr.name.startswith('imm'):
                self.adjust_imm(aInstr, opr)
            elif 'shamt' in opr.name:
                self.adjust_shamt(aInstr, opr)
            elif 'csr' in opr.name:
                self.adjust_csr(aInstr, opr)
            elif 'uimm' in opr.name:
                self.adjust_uimm(aInstr, opr)
            elif opr.name in ['pred', 'succ']:
                self.adjust_barrier(aInstr, opr)
            else:
                pass

        gen_asm_operand(aInstr)

        if aInstr.name.startswith('F') and not aInstr.name.startswith('FENCE'):
            aInstr.group = 'Float'

        if aInstr.name.startswith('B'):
            aInstr.iclass = 'BranchInstruction'
            add_cond_branch_operand(aInstr, 'simm12', 1)

        if aInstr.name == 'JAL':
            aInstr.iclass = 'BranchInstruction'
            add_pc_rel_branch_operand(aInstr, 'simm20', 1)

        if aInstr.name == 'JALR':
            aInstr.iclass = 'BranchInstruction'
            add_bo_branch_operand(aInstr, 'rs1', 'simm12', 0)

        if aInstr.name == 'ECALL':
            aInstr.group = 'System'
            aInstr.iclass = 'SystemCallInstruction'

        if aInstr.name in ['EBREAK', 'FENCE', 'FENCE.I']:
            aInstr.group = 'System'

        if aInstr.name.startswith('CSR'):
            aInstr.group = 'System'
            if aInstr.name.endswith('I'):
                aInstr.form = 'immediate'
            else:
                aInstr.form = 'register'

        atomic_size = 4 if aInstr.name.endswith('W') else 8
        if aInstr.name.startswith('AMO'):
            aInstr.iclass = 'LoadStoreInstruction'
            add_amo_addr_operand(aInstr, 'rs1', 'ReadWrite', atomic_size,
                                 'AtomicRW')
        if aInstr.name.startswith('LR'):
            aInstr.iclass = 'LoadStoreInstruction'
            add_amo_addr_operand(aInstr, 'rs1', 'Read', atomic_size,
                                 'Ordered')
        if aInstr.name.startswith('SC'):
            aInstr.iclass = 'UnpredictStoreInstruction'
            add_amo_addr_operand(aInstr, 'rs1', 'Write', atomic_size,
                                 'Ordered')

        ld_st_result = self._mLdStPattern.match(aInstr.name)
        if ld_st_result is not None:
            size = int_ldst_size(ld_st_result.group('size'))
            access = int_ldst_access(ld_st_result.group('access'))
            aInstr.iclass = "LoadStoreInstruction"
            add_bols_addr_operand(aInstr, 'rs1', 'simm12', access, size, 0)

    def adjust_rs(self, aInstr, aOpr):
        if 'rs3' in aOpr.name:
            adjust_gpr_operand(
                aOpr, 'Read', 'FPR', self._FpInstrSize(aInstr.name))
        elif 'rs2' in aOpr.name:
            if aInstr.name.startswith('F'):
                ld_st_result = self._mLdStPattern.match(aInstr.name)
                if ld_st_result is not None:
                    adjust_gpr_operand(
                        aOpr, 'Read', 'FPR', int_ldst_size(
                            ld_st_result.group('size')))
                else:
                    adjust_gpr_operand(
                        aOpr, 'Read', 'FPR', self._FpInstrSize(aInstr.name))
            else:
                adjust_gpr_operand(aOpr, 'Read', 'GPR', 8)
        elif 'rs1' in aOpr.name:
            ld_st_result = self._mLdStPattern.match(aInstr.name)
            if ld_st_result is not None:  # rs1 - int gpr bols addressing opr
                adjust_gpr_operand(aOpr, 'Read', 'GPR', 8, 'Nonzero GPRs')
            elif aInstr.name.startswith('AMO'):  # AMO*
                adjust_gpr_operand(aOpr, 'ReadWrite', 'GPR', 8, 'Nonzero GPRs')
            elif aInstr.name.startswith('LR'):  # Load Reserved
                adjust_gpr_operand(aOpr, 'Read', 'GPR', 8, 'Nonzero GPRs')
            elif aInstr.name.startswith('SC'):  # Store Conditional
                adjust_gpr_operand(aOpr, 'Write', 'GPR', 8, 'Nonzero GPRs')
            elif aInstr.name.startswith('FMV') or \
                    aInstr.name.startswith('FCVT'):  # FMV/FCVT rs1 op
                src_size, src_is_fp = self._FpSrcDstSizeType(aInstr, 'src')
                if src_is_fp:
                    adjust_gpr_operand(aOpr, 'Read', 'FPR', src_size)
                else:
                    adjust_gpr_operand(aOpr, 'Read', 'GPR', 8)
            elif aInstr.name.startswith('F') and not \
                    aInstr.name.startswith('FENCE'):
                # remaining fp instrs - (excluding FMV/FCVT and fp load/st)
                adjust_gpr_operand(
                    aOpr, 'Read', 'FPR', self._FpInstrSize(aInstr.name))
            else:  # remaining instrs use int gpr
                adjust_gpr_operand(aOpr, 'Read', 'GPR', 8)

    def adjust_rm(self, aInstr, aOpr):
        adjust_rm_operand(aOpr)

    def adjust_rd(self, aInstr, aOpr):
        if aInstr.name.startswith('F') and not aInstr.name.startswith('FENCE'):
            fp_rd_gpr_result = self._mFpRdGprPattern.match(aInstr.name)
            ld_st_result = self._mLdStPattern.match(aInstr.name)
            if fp_rd_gpr_result is not None:  # fp instrs with gpr target
                adjust_gpr_operand(aOpr, 'Write', 'GPR', 8)
            elif ld_st_result is not None:
                # fp ld instrs - different naming convention
                # (no .<size> which needed for catchall)
                adjust_gpr_operand(
                    aOpr, 'Write', 'FPR', int_ldst_size(
                        ld_st_result.group('size')))
            elif aInstr.name.startswith("FMV") or \
                    aInstr.name.startswith("FCVT"):  # FMV/FCVT rd op
                dst_size, dst_is_fp = self._FpSrcDstSizeType(aInstr, 'dst')
                if dst_is_fp:
                    adjust_gpr_operand(aOpr, 'Write', 'FPR', dst_size)
                else:
                    adjust_gpr_operand(aOpr, 'Write', 'GPR', 8)
            else:  # parse size for catchall FP case
                adjust_gpr_operand(
                    aOpr, 'Write', 'FPR', self._FpInstrSize(aInstr.name))
        else:
            adjust_gpr_operand(aOpr, 'Write', 'GPR', 8)

    def adjust_imm(self, aInstr, aOpr):
        if '[31:12]' in aOpr.name:
            adjust_imm_operand(aOpr, True, 20)
        elif '[11:0]' in aOpr.name:
            if aInstr.name.startswith('JALR'):
                adjust_imm_operand(aOpr, True, 12, '31-20')
            else:
                adjust_imm_operand(aOpr, True, 12)
        elif aInstr.name in 'JAL':  # imm[20|10:1|11|19:12]
            adjust_imm_operand(aOpr, True, 20, '31,19-12,20,30-21')
        elif aInstr.name.startswith('B'):
            # branch instrs imm[12|10:5]-...-imm[4:1|11]
            if '[12|10:5]' in aOpr.name:
                adjust_imm_operand(aOpr, True, 12, '31,7,30-25,11-8')
            else:  # imm[4:1|11]
                aInstr.operands.remove(aOpr)
        else:  # should be stores imm[11:5]-...-imm[4:0]
            if '[11:5]' in aOpr.name:
                adjust_imm_operand(aOpr, True, 12, '31-25,11-7')
            else:  # imm[4:0]
                aInstr.operands.remove(aOpr)

    def adjust_shamt(self, aInstr, aOpr):
        if aInstr.name in ['SLLI', 'SRAI', 'SRLI']:
            aInstr.form = 'RV32I' if aOpr.bits == '24-20' else 'RV64I'

    def adjust_csr(self, aInstr, aOpr):
        adjust_csr_operand(aOpr)

    def adjust_uimm(self, aInstr, aOpr):
        adjust_imm_operand(aOpr, False, 4)

    def adjust_barrier(self, aInstr, aOpr):
        aOpr.choices = 'Barrier option'

    def _FpInstrSize(self, aName):
        size_result = self._mFpSizePattern.match(aName)
        return fp_operand_size(size_result.group('size'))

    def _FpSrcDstSizeType(self, aInstr, aParam):
        src_dst_result = self._mFpSrcDstPattern.match(aInstr.name)
        if src_dst_result is None:
            return 0, False

        param = src_dst_result.group(aParam)
        size, is_fp = src_dst_size_regtype(aInstr, param)

        return size, is_fp
