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
from abc import ABC, abstractmethod
from shared.instruction_file import InstructionFile
from adjustor_utilities import *
from xml.sax.saxutils import escape
import copy
import re

class InstructionAdjustor(ABC):

    def __init__(self):
        self._mSupportedInstrFile = InstructionFile()

    @abstractmethod
    def adjust_instruction_by_format(self, aInstr):
        pass

    def adjust_instruction(self, aInstr):
        self.adjust_instruction_by_format(aInstr)
        self._mSupportedInstrFile.add_instruction(copy.deepcopy(aInstr))
        #else:
        #    print('unsupported instr: {} using format: {}'.format(instr.get_full_ID(), instr.get_format()))


class G_InstructionAdjustor(InstructionAdjustor):

    def __init__(self):
        super().__init__()
        self._mFpRdGprPattern = re.compile(r'FLT|FLE|FEQ|FCLASS')
        self._mFpSizePattern = re.compile(r'^F[A-Z]*.(?P<size>\w*)$')
        self._mFpSrcDstPattern = re.compile(r'^F[A-Z]*.(?P<dst>\w*).(?P<src>\w*)$')
        self._mLdStPattern = re.compile(r'^F{0,1}(?P<access>[LS])(?P<size>[A-Z])U{0,1}$')

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
            elif opr.name in ['pred', 'succ', 'fm']:
                self.adjust_barrier(aInstr, opr)
            else:
                pass
                #print('operand not parsed for {}, operand: {}'.format(aInstr.name, opr.name))

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
            add_amo_addr_operand(aInstr, 'rs1', 'ReadWrite', atomic_size, 'AtomicRW')
        if aInstr.name.startswith('LR'):
            aInstr.iclass = 'LoadStoreInstruction'
            add_amo_addr_operand(aInstr, 'rs1', 'Read', atomic_size, 'Ordered')
        if aInstr.name.startswith('SC'):
            aInstr.iclass = 'UnpredictStoreInstruction'
            add_amo_addr_operand(aInstr, 'rs1', 'Write', atomic_size, 'Ordered')

        ld_st_result = self._mLdStPattern.match(aInstr.name)
        if ld_st_result is not None:
            size = int_ldst_size(ld_st_result.group('size'))
            access = ldst_access(ld_st_result.group('access'))
            aInstr.iclass = "LoadStoreInstruction"
            add_bols_addr_operand(aInstr, 'rs1', 'simm12', access, size, 0)

        if aInstr.name in ['MRET', 'SRET']:
            add_ret_operand(aInstr)
            aInstr.iclass = 'RetInstruction'
            aInstr.group = 'System'
            aInstr.extension = 'RV64Priv'

        if aInstr.name == 'SFENCE.VMA':
            aInstr.group = 'System'
            aInstr.extension = 'RV64Priv'


    def adjust_rs(self, aInstr, aOpr):
        if 'rs3' in aOpr.name:
            adjust_gpr_operand(aOpr, 'Read', 'FPR', self._FpInstrSize(aInstr.name))
        elif 'rs2' in aOpr.name:
            if aInstr.name.startswith('F'):
                ld_st_result = self._mLdStPattern.match(aInstr.name)
                if ld_st_result is not None:
                    adjust_gpr_operand(aOpr, 'Read', 'FPR', int_ldst_size(ld_st_result.group('size')))
                else:
                    adjust_gpr_operand(aOpr, 'Read', 'FPR', self._FpInstrSize(aInstr.name))
            #elif aInstr.name =='SFENCE.VMA':
            #    adjust_gpr_operand(aOpr, 'Read', 'GPR', 0, 'GPRs')
            else:
                adjust_gpr_operand(aOpr, 'Read', 'GPR', 8)
        elif 'rs1' in aOpr.name:
            ld_st_result = self._mLdStPattern.match(aInstr.name)
            if ld_st_result is not None: #rs1 - int gpr bols addressing opr
                adjust_gpr_operand(aOpr, 'Read', 'GPR', 8, 'Nonzero GPRs')
            elif aInstr.name.startswith('AMO'): #AMO*
                adjust_gpr_operand(aOpr, 'ReadWrite', 'GPR', 8, 'Nonzero GPRs')
            elif aInstr.name.startswith('LR'): #Load Reserved
                adjust_gpr_operand(aOpr, 'Read', 'GPR', 8, 'Nonzero GPRs')
            elif aInstr.name.startswith('SC'): #Store Conditional
                adjust_gpr_operand(aOpr, 'Write', 'GPR', 8, 'Nonzero GPRs')
            elif aInstr.name.startswith('FMV') or aInstr.name.startswith('FCVT'): #FMV/FCVT rs1 op
                src_size, src_is_fp = self._FpSrcDstSizeType(aInstr, 'src')
                if src_is_fp:
                    adjust_gpr_operand(aOpr, 'Read', 'FPR', src_size)
                else:
                    adjust_gpr_operand(aOpr, 'Read', 'GPR', 8)
            elif aInstr.name.startswith('F') and not aInstr.name.startswith('FENCE'): #remaining fp instrs - (excluding FMV/FCVT and fp load/st)
                adjust_gpr_operand(aOpr, 'Read', 'FPR', self._FpInstrSize(aInstr.name))
            #elif aInstr.name =='SFENCE.VMA':
            #    adjust_gpr_operand(aOpr, 'Read', 'GPR', 0, 'GPRs')
            else: #remaining instrs use int gpr
                adjust_gpr_operand(aOpr, 'Read', 'GPR', 8)


    def adjust_rm(self, aInstr, aOpr):
        adjust_rm_operand(aOpr)

    def adjust_rd(self, aInstr, aOpr):
        if aInstr.name.startswith('F') and not aInstr.name.startswith('FENCE'):
            fp_rd_gpr_result = self._mFpRdGprPattern.match(aInstr.name)
            ld_st_result = self._mLdStPattern.match(aInstr.name)
            if fp_rd_gpr_result is not None: #fp instrs with gpr target
                adjust_gpr_operand(aOpr, 'Write', 'GPR', 8)
            elif ld_st_result is not None: #fp ld instrs - different naming convention (no .<size> which needed for catchall)
                adjust_gpr_operand(aOpr, 'Write', 'FPR', int_ldst_size(ld_st_result.group('size')))
            elif aInstr.name.startswith("FMV") or aInstr.name.startswith("FCVT"): #FMV/FCVT rd op
                dst_size, dst_is_fp = self._FpSrcDstSizeType(aInstr, 'dst')
                if dst_is_fp:
                    adjust_gpr_operand(aOpr, 'Write', 'FPR', dst_size)
                else:
                    adjust_gpr_operand(aOpr, 'Write', 'GPR', 8)
            else: #parse size for catchall FP case
                adjust_gpr_operand(aOpr, 'Write', 'FPR', self._FpInstrSize(aInstr.name))
        else:
            adjust_gpr_operand(aOpr, 'Write', 'GPR', 8)

    def adjust_imm(self, aInstr, aOpr):
        if '[31:12]' in aOpr.name:
            adjust_imm_operand(aOpr, True, 20)
        elif '[11:0]' in aOpr.name:
            if aInstr.name.startswith('JALR'):
                adjust_imm_operand(aOpr, True, 12, '31-20')
            if aInstr.name.startswith('FENCE'):
                adjust_imm_operand(aOpr, False, 12)
            else:
                adjust_imm_operand(aOpr, True, 12)
        elif aInstr.name in 'JAL': # imm[20|10:1|11|19:12]
            adjust_imm_operand(aOpr, True, 20, '31,19-12,20,30-21')
        elif aInstr.name.startswith('B'): #branch instrs imm[12|10:5]-...-imm[4:1|11]
            if '[12|10:5]' in aOpr.name:
                adjust_imm_operand(aOpr, True, 12, '31,7,30-25,11-8')
            else: #imm[4:1|11]
                aInstr.operands.remove(aOpr)
        else: #should be stores imm[11:5]-...-imm[4:0]
            if '[11:5]' in aOpr.name:
                adjust_imm_operand(aOpr, True, 12, '31-25,11-7')
            else: #imm[4:0]
                aInstr.operands.remove(aOpr)

    def adjust_shamt(self, aInstr, aOpr):
        if aInstr.name in ['SLLI', 'SRAI', 'SRLI']:
            aInstr.form = 'RV32I' if aOpr.bits == '24-20' else 'RV64I'

    def adjust_csr(self, aInstr, aOpr):
        adjust_csr_operand(aOpr)

    def adjust_uimm(self, aInstr, aOpr):
        adjust_imm_operand(aOpr, False, 4)

    def adjust_barrier(self, aInstr, aOpr):
        if aOpr.name in ['pred', 'succ']:
            aOpr.choices = 'Barrier option'
        else: #fm operand
            aOpr.choices = 'Fence mode'

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


class V_InstructionAdjustor(InstructionAdjustor):

    def __init__(self):
        super().__init__()
        self._VdrdUsesRdInstrs = ['VPOPC', 'VFIRST', 'VMV.X.S', 'VFMV.F.S']
        self._Vs2DifferVdInstrs = ['VFSLIDE1UP.VF', 'VRGATHER.VX', 'VSLIDE1UP.VX', 'VSLIDEUP.VX',
                                    'VMSBF.M', 'VMSIF.M', 'VMSOF.M', 'VIOTA.M', 'VRGATHER.VI',
                                    'VSLIDEUP.VI', 'VRGATHER.VV', 'VCOMPRESS.VM']
        self._Vs1DifferVdInstrs = ['VRGATHER.VV', 'VCOMPRESS.VM']
        self._mLsPattern = re.compile(r'^V(?P<access>[LS])')

    def adjust_instruction_by_format(self, aInstr):
        aInstr.name = escape(aInstr.name)
        aInstr.asm.format = escape(aInstr.asm.format)

        add_layout_operand(aInstr)
        adjust_register_layout(aInstr)

        for opr in aInstr.operands:
            if opr.name in ['vd/rd', 'vd', 'rd', 'vd$\\neq$0']:
                self.adjust_vdrd(aInstr, opr)
            if opr.name.startswith('vs'):
                self.adjust_vs(aInstr, opr)
            if opr.name.startswith('rs'):
                self.adjust_rs(aInstr, opr)
            if 'imm' in opr.name:
                self.adjust_imm(aInstr, opr)
            if opr.name == 'vm':
                adjust_vm_operand(aInstr, opr)

        gen_asm_operand(aInstr)

        if aInstr.iclass == 'VectorLoadStoreInstruction':
            addr_mode = self._MopBits(aInstr)
            ls_result = self._mLsPattern.match(aInstr.name)
            if addr_mode in ['11', '01']:
                add_vec_indexed_addr_operand(aInstr, 'rs1', 'vs2', ldst_access(ls_result.group('access')))
                if aInstr.find_operand('vs3', fail_not_found=False) is not None:
                    add_differ_attribute(aInstr, 'vs2', 'vs3')
                elif aInstr.find_operand('vd', fail_not_found=False) is not None:
                    add_differ_attribute(aInstr, 'vs2', 'vd')
            elif addr_mode in ['10']: 
                add_vec_strided_addr_operand(aInstr, 'rs1', 'rs2', ldst_access(ls_result.group('access')))
            elif aInstr.name.startswith('VL'):
                add_vec_bols_addr_operand(aInstr, 'rs1', 'Read')
            elif aInstr.name.startswith('VS'):
                add_vec_bols_addr_operand(aInstr, 'rs1', 'Write')

        if aInstr.iclass == 'VectorAMOInstructionRISCV':
            add_vec_indexed_addr_operand(aInstr, 'rs1', 'vs2', 'ReadWrite')


    def adjust_vdrd(self, aInstr, aOpr):
        if aOpr.name == 'rd':
            adjust_gpr_operand(aOpr, 'Write', 'GPR', 8)
        elif aOpr.name == 'vd':
            if aInstr.name.startswith('VAMO'):
                aOpr.oclass = 'VectorIndexedDataRegisterOperand'
                add_differ_attribute(aInstr, 'vs2', 'vd')
            if aInstr.iclass == 'VectorLoadStoreInstruction':
                if self._MopBits(aInstr) in ['01', '11']:
                    aOpr.oclass = 'VectorIndexedDataRegisterOperand'
                    add_differ_attribute(aInstr, 'vs2', 'vd')
                else:
                   aOpr.oclass = 'VectorDataRegisterOperand'

            adjust_vecreg_operand(aOpr, 'Write')
        elif aOpr.name == 'vd/rd':
            if aInstr.name in self._VdrdUsesRdInstrs:
                aOpr.name = 'rd'
                adjust_gpr_operand(aOpr, 'Write', self._VdrdRegtype(aInstr), 4)
            else:
                aOpr.name = 'vd'
                adjust_vecreg_operand(aOpr, 'Write')
        elif 'vd$\\neq$0' in aOpr.name:
            adjust_vecreg_operand(aOpr, 'Write', 'Nonzero vector registers')
            aOpr.name = 'vd'

    def adjust_rs(self, aInstr, aOpr):
        if aOpr.name == 'rs1':
            if aInstr.iclass == 'VectorLoadStoreInstruction' or aInstr.iclass == 'VectorAMOInstructionRISCV': 
                adjust_gpr_operand(aOpr, 'Read', 'GPR', 4, 'Nonzero GPRs')
            else:
                if '.F' in aInstr.name:
                    adjust_gpr_operand(aOpr, 'Read', 'FPR', 4)
                else:
                    adjust_gpr_operand(aOpr, 'Read', self._Rs1Regtype(aInstr), 4)

            if aInstr.name.startswith('VSETVL'): #set rs1 class for both vsetvl operations
                aOpr.oclass = 'VsetvlAvlRegisterOperand'
        elif aOpr.name == 'rs2':
            if aInstr.iclass == 'VectorLoadStoreInstruction':
                adjust_gpr_operand(aOpr, 'Read', 'GPR', 4, 'Nonzero GPRs')
            else:
                adjust_gpr_operand(aOpr, 'Read', 'GPR', 8)

            if aInstr.name == 'VSETVL': #set rs2 class for just vsetvl (replaced by zimm in vsetvli)
                aOpr.oclass = 'VsetvlVtypeRegisterOperand'
                aOpr.differ = 'rs1'

    def adjust_vs(self, aInstr, aOpr):
        adjust_vecreg_operand(aOpr, 'Read')
        if aOpr.name == 'vs1':
            if aInstr.name in self._Vs1DifferVdInstrs:
                aOpr.differ = 'vd'
        if aOpr.name == 'vs2':
            if aInstr.name in self._Vs2DifferVdInstrs:
                aOpr.differ = 'vd'
        if aOpr.name == 'vs3':
            if aInstr.name.startswith('VAMO'): 
                aOpr.oclass = 'VectorIndexedDataRegisterOperand'
                add_differ_attribute(aInstr, 'vs2', 'vs3') 
            elif aInstr.iclass == 'VectorLoadStoreInstruction':
                aOpr.oclass = self._Vs3OperandClass(aInstr)
                if aOpr.oclass == 'VectorIndexedDataRegisterOperand':
                    add_differ_attribute(aInstr, 'vs2', 'vs3')

    def adjust_imm(self, aInstr, aOpr):
        if aOpr.name.startswith('z'): #zimm11
            aOpr.oclass = 'VsetvlVtypeImmediateOperand'
        else: #simm5
            aOpr.oclass = 'SignedImmediateOperand'

    def _Funct3(self, aInstr):
        return aInstr.find_operand('const_bits').value[-10:-7]

    def _Opcode(self, aInstr):
        return aInstr.find_operand('const_bits').value[-7:]

    def _MopBits(self, aInstr):
        return aInstr.find_operand('const_bits').value[4:6]

    def _VdrdRegtype(self, aInstr):
        if self._Opcode(aInstr) ==  '1010111':
            if self._Funct3(aInstr) in ['001','101']: #OPFVV/OPFVF
               return 'FPR'

        return 'GPR'

    def _Rs1Regtype(self, aInstr):
        if self._Opcode(aInstr) == '1010111':
            if self._Funct3(aInstr) == '101': #OPFVF
                return 'FPR'

        return 'GPR'


    def _Vs3OperandClass(self, aInstr):
        if self._MopBits(aInstr) in ['01', '11']:
            return 'VectorIndexedDataRegisterOperand'

        return 'VectorDataRegisterOperand'
