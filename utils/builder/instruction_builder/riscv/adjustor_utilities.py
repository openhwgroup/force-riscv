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
from shared.instruction import Operand, add_addressing_operand
import re

def adjust_gpr_operand(aOpr, aAccess, aType, aSize, aChoices=None):
    aOpr.access = aAccess
    aOpr.type = aType
    if aChoices is not None:
        aOpr.choices = aChoices;
        if aChoices == 'Prime GPRs':
            aOpr.oclass = 'CompressedRegisterOperandRISCV'
    elif 'GPR' in aType:
        aOpr.choices = 'GPRs'
    elif 'FPR' in aType:
        set_fpr_choices(aOpr, aSize)

def adjust_imm_operand(aOpr, aSigned, aSize, aBits=None):
    if aSigned:
        aOpr.set_attribute('class', 'SignedImmediateOperand')
        aOpr.name = '{}{}'.format('simm', aSize)
    else:
        aOpr.name = '{}{}'.format('imm', aSize)

    if aBits is not None:
        aOpr.bits = aBits

def adjust_imm_operandNew(aOpr, aSigned=False, aClass=None, aExclude=None):
    if aSigned:
        aOpr.set_attribute('class', 'SignedImmediateOperand')

    if aClass is not None:
        aOpr.set_attribute('class', aClass)

    if aExclude is not None:
        aOpr.exlude = aExclude

def adjust_rm_operand(aOpr):
    aOpr.type = 'Choices'
    aOpr.choices = 'Rounding mode'

def adjust_csr_operand(aOpr):
    aOpr.type = 'SysReg'
    aOpr.access = 'ReadWrite'
    aOpr.choices = 'System registers'

def adjust_vecreg_operand(aOpr, aAccess, aChoices=None):
    aOpr.type = "VECREG"
    aOpr.access = aAccess
    if aChoices is not None:
        aOpr.choices = aChoices
    else:
        aOpr.choices = "Vector registers"

def adjust_vm_operand(aInstr, aOpr):
    aOpr.type = 'Choices'
    aOpr.choices = 'Vector mask'
    aOpr.oclass = 'VectorMaskOperand'
    if aInstr.find_operand('vd', fail_not_found=False):
        aOpr.differ = 'vd'

def set_fpr_choices(aOpr, aSize):
    if aSize == 2:
        aOpr.choices = '16-bit SIMD/FP registers'
    elif aSize == 4:
        aOpr.choices = '32-bit SIMD/FP registers'
    elif aSize == 8:
        aOpr.choices = '64-bit SIMD/FP registers'
    elif aSize == 16:
        aOpr.choices = '128-bit SIMD/FP registers'

def gen_asm_operand(aInstr):
    if aInstr.operands:
        for opr in aInstr.operands:
            if opr.type not in 'VectorLayout':
                aInstr.asm.format += ' %s,'
                aInstr.asm.ops.append(opr.name)

        aInstr.asm.format = aInstr.asm.format[:-1] #remove trailing comma

def fp_operand_size(aSize):
    if 'H' in aSize:
        return 2
    elif 'S' in aSize:
        return 4
    elif 'D' in aSize:
        return 8
    elif 'Q' in aSize:
        return 16

    return 0 #error case, should fail

def int_ldst_size(aSize):
    if 'B' in aSize:
        return 1
    elif 'H' in aSize:
        return 2
    elif 'W' in aSize:
        return 4
    elif 'D' in aSize:
        return 8

    return 0 #error case, should fail

def ldst_access(aAccess):
    if 'L' in aAccess:
        return 'Read'
    elif 'S' in aAccess:
        return 'Write'

    return '' #error case, should fail

#returns size of reg, and whether the char passed represents an int or gpr reg
def src_dst_size_regtype(aInstr, aSize):
    if 'H' in aSize:
        return (2, True)
    elif 'W' in aSize:
        if aInstr.name.startswith('FMV'):
            return (4, True)
        elif aInstr.name.startswith('FCVT'):
            return (4, False)
    elif 'S' in aSize:
        return (4, True)
    elif 'D' in aSize:
        return (8, True)
    elif 'Q' in aSize:
        return (16, True)
    elif aSize in ['X', 'L']:
        return (8, False)

    return (0, False) #error case, should fail

def add_bols_addr_operand(aInstr, aOprName, aOffsetName, aAccess, aSize, aScale):
    attr_dict = {'offset-scale': aScale, 'alignment': aSize, 'base': aOprName, 'data-size': aSize, 'element-size': aSize, 'mem-access': aAccess}
    subop_dict = {'base': aOprName, 'offset': aOffsetName}
    add_addressing_operand(aInstr, None, 'LoadStore', None, subop_dict, attr_dict)

def add_amo_addr_operand(aInstr, aOprName, aAccess, aSize, aOrder):
    attr_dict = {'alignment': aSize, 'base': aOprName, 'data-size': aSize, 'element-size': aSize, 'mem-access': aAccess}
    if 'Read' in aAccess:
        attr_dict['lorder'] = aOrder
    if 'Write' in aAccess:
        attr_dict['sorder'] = aOrder
    subop_dict = {'base': aOprName}
    add_addressing_operand(aInstr, None, 'LoadStore', None, subop_dict, attr_dict)

def add_pc_rel_branch_operand(aInstr, aOprName, aScale):
    attr_dict = {'offset-scale': aScale}
    subop_dict = {'offset': aOprName}
    add_addressing_operand(aInstr, None, 'Branch', 'PcRelativeBranchOperand', subop_dict, attr_dict)

def add_bo_branch_operand(aInstr, aOprName, aOffsetName, aScale):
    attr_dict = {'base': aOprName, 'offset-scale': aScale}
    subop_dict = {'base': aOprName, 'offset': aOffsetName}
    add_addressing_operand(aInstr, None, 'Branch', 'BaseOffsetBranchOperand', subop_dict, attr_dict)

def add_reg_branch_operand(aInstr, aOprName, aScale):
    attr_dict = {'base': aOprName}
    subop_dict = {'base': aOprName}
    add_addressing_operand(aInstr, None, 'Branch', 'RegisterBranchOperand', subop_dict, attr_dict)

def add_cond_branch_operand(aInstr, aOprName, aScale, aClass='ConditionalBranchOperandRISCV'):
    if aClass == 'CompressedConditionalBranchOperandRISCV':      # C.BEQZ or C.BNEZ
        theCondition = aInstr.name.replace(".","")               # remove the "." in the mnemonic
    else:
        theCondition = aInstr.name
    attr_dict = {'offset-scale': aScale, 'condition': theCondition}
    subop_dict = {'offset': aOprName}
    add_addressing_operand(aInstr, None, 'Branch', aClass, subop_dict, attr_dict)

def add_vec_bols_addr_operand(aInstr, aOprName, aAccess):
    width = get_element_size(aInstr.find_operand('const_bits'))
    attr_dict = {'alignment': width, 'base': aOprName, 'data-size': width, 'element-size': width, 'mem-access': aAccess}
    subop_dict = {'base': aOprName}
    add_addressing_operand(aInstr, None, 'LoadStore', 'VectorBaseOffsetLoadStoreOperand', subop_dict, attr_dict)

def add_vec_indexed_addr_operand(aInstr, aOprName, aIndexName, aAccess):
    attr_dict = {'base': aOprName, 'mem-access': aAccess}
    subop_dict = {'base': aOprName, 'index': aIndexName}
    add_addressing_operand(aInstr, None, 'LoadStore', 'VectorIndexedLoadStoreOperandRISCV', subop_dict, attr_dict)

def add_vec_strided_addr_operand(aInstr, aOprName, aIndexName, aAccess):
    reg_count = int(aInstr.find_operand('custom').regCount) if 'SEG' in aInstr.name else 1
    width = get_element_size(aInstr.find_operand('const_bits'))

    attr_dict = {'alignment': width, 'base': aOprName, 'data-size': width * reg_count, 'element-size': width, 'mem-access': aAccess}
    subop_dict = {'base': aOprName, 'index': aIndexName}
    add_addressing_operand(aInstr, None, 'LoadStore', 'VectorStridedLoadStoreOperandRISCV', subop_dict, attr_dict)

def add_ret_operand(aInstr):
    ret_opr = Operand()
    ret_opr.name = 'retoperand'
    ret_opr.type = 'Choices'
    ret_opr.choices = 'Ret choice'
    ret_opr.oclass = 'RetOperand'
    aInstr.add_operand(ret_opr)

def add_vtype_layout_operand(aInstr):
    layout_opr = Operand()
    layout_opr.name = "vtype"
    layout_opr.type = "VectorLayout"
    layout_opr.oclass = "VtypeLayoutOperand"
    aInstr.insert_operand(0, layout_opr)

def add_custom_layout_operand(aInstr, aRegCount, aElemWidth):
    layout_opr = Operand()
    layout_opr.name = "custom"
    layout_opr.type = "VectorLayout"
    layout_opr.oclass = "CustomLayoutOperand"
    layout_opr.regCount = aRegCount
    layout_opr.elemWidth = aElemWidth
    aInstr.insert_operand(0, layout_opr)

def add_whole_register_layout_operand(aInstr, aRegCount=1, aRegIndexAlignment=1):
    layout_opr = Operand()
    layout_opr.name = "whole"
    layout_opr.type = "VectorLayout"
    layout_opr.oclass = "WholeRegisterLayoutOperand"
    layout_opr.regCount = aRegCount
    layout_opr.regIndexAlignment = aRegIndexAlignment
    aInstr.insert_operand(0, layout_opr)


def add_layout_operand(aInstr):
    # TODO(Noah): Add additional load/store whole register instructions when they are supported by
    # Handcar.
    load_store_whole_register = ['VL1R.V', 'VS1R.V']

    if aInstr.name in load_store_whole_register:
        reg_count = int(aInstr.name[2])
        add_whole_register_layout_operand(aInstr, aRegCount=reg_count)
    elif aInstr.name in ('VMV1R.V', 'VMV2R.V', 'VMV4R.V', 'VMV8R.V'):
        reg_count = int(aInstr.name[3])
        add_whole_register_layout_operand(aInstr, aRegCount=reg_count, aRegIndexAlignment=reg_count)
    elif aInstr.name in ('VSETVL', 'VSETVLI'):
        pass  # No vector layout operand required
    elif aInstr.iclass == 'VectorLoadStoreInstruction' or aInstr.iclass == 'VectorAMOInstructionRISCV':
        reg_count = 1
        elem_width = None
        ints = re.findall('\d+', aInstr.name)
        if len(ints) > 1:
            reg_count = ints[0]
            elem_width = ints[1]
        else:
            elem_width = ints[0]

        add_custom_layout_operand(aInstr, aRegCount=reg_count, aElemWidth=elem_width)
    else:
        add_vtype_layout_operand(aInstr)

# Account for non-standard register layouts due to wide and narrow operands
def adjust_register_layout(aInstr):
    adjust_dest = False
    dest_layout_multiple = 2
    if aInstr.name.startswith('VW') or aInstr.name.startswith('VFW'):
        adjust_dest = True
    elif aInstr.name.startswith('VQMACC'):
        adjust_dest = True
        dest_layout_multiple = 4

    adjust_source = False
    source_layout_multiple = 2
    if '.W' in aInstr.name:
        adjust_source = True
    elif aInstr.name.startswith('VSEXT.VF') or aInstr.name.startswith('VZEXT.VF'):
        adjust_source = True
        layout_divisor = int(aInstr.name[-1])
        source_layout_multiple = 1 / layout_divisor

    if adjust_dest:
        adjust_dest_layout(aInstr, dest_layout_multiple)
        add_differ_attribute(aInstr, 'vs1', 'vd')

    if adjust_source:
        adjust_source_layout(aInstr, source_layout_multiple)

    if adjust_dest != adjust_source:
        add_differ_attribute(aInstr, 'vs2', 'vd')

def get_element_size(aConstBitsOpr):
    width = aConstBitsOpr.value[-10:-7]
    mew = aConstBitsOpr.value[3]
    if mew == '0':
        if width == '000':
            return 1
        elif width == '101':
            return 2
        elif width == '110':
            return 4
        elif width == '111':
            return 8
    elif mew == '1':
        if width == '000':
            return 16
        elif width == '101':
            return 32
        elif width == '110':
            return 64
        elif width == '111':
            return 128

def add_differ_attribute(aInstr, aOprName, aDifferName):
    opr = aInstr.find_operand(aOprName, fail_not_found=False)
    if opr:
        opr.differ = aDifferName

#TODO improve these - move to 1 seq if possible
def adjust_dest_layout(aInstr, aLayoutMultiple):
    dest_opr = aInstr.find_operand('vd', fail_not_found=False)
    if dest_opr is None:
        dest_opr = aInstr.find_operand('vd/rd')

    dest_opr.layoutMultiple = aLayoutMultiple

def adjust_source_layout(aInstr, aLayoutMultiple):
    vs2_opr = aInstr.find_operand('vs2')
    vs2_opr.layoutMultiple = aLayoutMultiple

def add_implied_register(aInstr, aOpName, aOpType, aAccessType, aInsertIndex):
    implied_opr = Operand()
    implied_opr.name = aOpName
    implied_opr.type = aOpType
    implied_opr.oclass = "ImpliedRegisterOperand"
    if aAccessType:
        implied_opr.access = aAccessType
    aInstr.insert_operand(aInsertIndex, implied_opr)

