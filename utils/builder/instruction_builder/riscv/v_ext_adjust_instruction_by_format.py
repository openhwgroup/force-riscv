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
from shared.instruction import add_addressing_operand
from vector_operand_adjustor import VectorOperandAdjustor
from xml.sax.saxutils import escape

import re

format_map = {}

def v_ext_adjust_instruction_by_format(aInstruction):
    # escaping < and > characters
    aInstruction.name = escape(aInstruction.name)
    aInstruction.asm.format = escape(aInstruction.asm.format)

    # Get the format prior to adding the layout operand, so that it's not necessary to strip off the
    # layout operand name.
    instruction_format = aInstruction.get_format()

    add_layout_operand(aInstruction)

    adjust_register_layout(aInstruction)

    if instruction_format == 'vd/rd-vs2-vs1-vm':
        return adjust_vdrd_vs2_vs1_vm(aInstruction)
    elif instruction_format == 'vd-vs2-vs1-vm':
        return adjust_vd_vs2_vs1_vm(aInstruction)
    elif instruction_format == 'vd-vs2-simm5-vm':
        return adjust_vd_vs2_simm5_vm(aInstruction)
    elif instruction_format == 'vd-vs2-rs1-vm':
        return adjust_vd_vs2_rs1_vm(aInstruction)
    elif instruction_format == 'vd/rd-vs2-rs1-vm':
        return adjust_vdrd_vs2_rs1_vm(aInstruction)
    # unary instruction formats
    elif instruction_format == 'vd/rd-vs2-vm':
        return adjust_vdrd_vs2_vm(aInstruction)
    elif instruction_format == 'vd-vs2-vm':
        return adjust_vd_vs2_vm(aInstruction)
    elif instruction_format == 'vd/rd-vs2':
        return adjust_vdrd_vs2(aInstruction)
    elif instruction_format == 'rd-vs2-vm':
        return adjust_rd_vs2_vm(aInstruction)
    elif instruction_format == 'rd-vs2':
        return adjust_rd_vs2(aInstruction)
    elif instruction_format == 'vd-rs1-vm':
        return adjust_vd_rs1_vm(aInstruction)
    elif instruction_format == 'vd/rd-rs1-vm':
        return adjust_vdrd_rs1_vm(aInstruction)
    elif instruction_format == 'vd-vm':
        return adjust_vd_vm(aInstruction)
    # vsetvl/i instructions
    elif instruction_format == 'rd-rs1-rs2':
        return adjust_rd_rs1_rs2(aInstruction)
    elif instruction_format == 'rd-rs1-zimm[10:0]':
        return adjust_rd_rs1_zimm_10_0(aInstruction)
    # vmerge and vmv instructions
    elif instruction_format == 'vd-vs2-simm5':
        return adjust_vd_vs2_simm5(aInstruction)
    elif instruction_format == 'vd$\\neq$0-vs2-simm5':
        return adjust_vd_nonzero_vs2_simm5(aInstruction)
    elif instruction_format == 'vd-vs2-vs1':
        return adjust_vd_vs2_vs1(aInstruction)
    elif instruction_format == 'vd$\\neq$0-vs2-vs1':
        return adjust_vd_nonzero_vs2_vs1(aInstruction)
    elif instruction_format == 'vd-vs2-rs1':
        return adjust_vd_vs2_rs1(aInstruction)
    elif instruction_format == 'vd$\\neq$0-vs2-rs1':
        return adjust_vd_nonzero_vs2_rs1(aInstruction)
    elif instruction_format == 'vd-simm5':
        return adjust_vd_simm5(aInstruction)
    elif instruction_format == 'vd-vs1':
        return adjust_vd_vs1(aInstruction)
    elif instruction_format == 'vd-vs2':
        return adjust_vd_vs2(aInstruction)
    # vl<nf>r/vs<nf>r instructions
    elif instruction_format == 'vd-rs1':
        return adjust_vd_rs1(aInstruction)
    elif instruction_format == 'vd-rs1-vs2-vm':
        return adjust_vd_rs1_vs2_vm(aInstruction)
    elif instruction_format == 'vd-rs1-rs2-vm':
        return adjust_vd_rs1_rs2_vm(aInstruction)
    elif instruction_format == 'vs3-rs1':
        return adjust_vs3_rs1(aInstruction)
    elif instruction_format == 'vs3-rs1-vm':
        return adjust_vs3_rs1_vm(aInstruction)
    elif instruction_format == 'vs3-rs1-vs2-vm':
        return adjust_vs3_rs1_vs2_vm(aInstruction)
    elif instruction_format == 'vs3-rs1-rs2-vm':
        return adjust_vs3_rs1_rs2_vm(aInstruction)
    # vamo instructions
    elif instruction_format == 'rs1-vs2-vs3-vm':
        return adjust_rs1_vs2_vs3_vm(aInstruction)
    elif instruction_format == 'rs1-vs2-vd-vm':
        return adjust_rs1_vs2_vd_vm(aInstruction)
    else:
        record_instruction_format(instruction_format)

    return False

def record_instruction_format(aInstructionFormat):
    if aInstructionFormat in format_map:
        format_map[aInstructionFormat] += 1
    else:
        format_map[aInstructionFormat] = 1

def add_layout_operand(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    load_store_whole_register = ['VL1R.V', 'VL2R.V', 'VL3R.V', 'VL4R.V', 'VL5R.V', 'VL6R.V', 'VL7R.V', 'VL8R.V', 'VS1R.V', 'VS2R.V', 'VS3R.V', 'VS4R.V', 'VS5R.V', 'VS6R.V', 'VS7R.V', 'VS8R.V']
    if aInstruction.name in load_store_whole_register:
        reg_count = int(aInstruction.name[2])
        operand_adjustor.add_whole_register_layout_operand(aRegCount=reg_count)
    elif aInstruction.name in ('VMV1R.V', 'VMV2R.V', 'VMV4R.V', 'VMV8R.V'):
        reg_count = int(aInstruction.name[3])
        operand_adjustor.add_whole_register_layout_operand(aRegCount=reg_count, aRegIndexAlignment=reg_count)
    elif aInstruction.name in ('VSETVL', 'VSETVLI'):
        pass  # No vector layout operand required
    elif aInstruction.iclass == 'VectorLoadStoreInstruction':
        reg_count = 1
        elem_width = None
        ints = re.findall('\d+', aInstruction.name)
        if len(ints) > 1:
            reg_count = ints[0]
            elem_width = ints[1]
        else:
            elem_width = ints[0]
        operand_adjustor.add_custom_layout_operand(aRegCount=reg_count, aElemWidth=elem_width)
    else:
        operand_adjustor.add_vtype_layout_operand()

# Account for non-standard register layouts due to widening and narrowing instructions
def adjust_register_layout(aInstruction):
    wide_dest = False
    dest_layout_multiple = 2
    if aInstruction.name.startswith('VW') or aInstruction.name.startswith('VFW'):
        wide_dest = True
    elif aInstruction.name.startswith('VQMACC'):
        wide_dest = True
        dest_layout_multiple = 4

    wide_source = False
    if '.W' in aInstruction.name:
        wide_source = True

    operand_adjustor = VectorOperandAdjustor(aInstruction)
    if wide_dest:
        operand_adjustor.set_wide_dest(aLayoutMultiple=dest_layout_multiple)
        operand_adjustor.set_vs1_differ_vd()

    if wide_source:
        operand_adjustor.set_wide_source()

    if wide_dest != wide_source:
        operand_adjustor.set_vs2_differ_vd()

def convert_width_to_size(aWidth):
    if aWidth == '000': #VxB
        return 1
    elif aWidth == '101': #VxH
        return 2
    elif aWidth == '110': #VxW
        return 4
    elif aWidth == '111': #VxE
        return 1 #TODO (Chris): SEW/8 - search vtype? <------- look into rewriting this for 0.9
    return 1 #TODO: default to 1...

def adjust_vd_rs1(aInstruction):
    if aInstruction.iclass == 'VectorLoadStoreInstruction':
        operand_adjustor = VectorOperandAdjustor(aInstruction)
        operand_adjustor.set_vd_ls_dest()
        operand_adjustor.set_rs1_int_ls_base()

        width = convert_width_to_size(aInstruction.find_operand('const_bits').value[-10:-7])
        attr_dict = dict()
        subop_dict = dict()
        subop_dict["base"] = "rs1"
        attr_dict["alignment"] = width
        attr_dict["base"] = "rs1"
        #attr_dict["data-size"] = 1 #TODO: depends on LMUL
        attr_dict["element-size"] = width
        attr_dict["mem-access"] = "Read"

        add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)
    else:
        operand_adjustor = VectorOperandAdjustor(aInstruction)
        operand_adjustor.set_vd()
        if '.F' in aInstruction.name:
            operand_adjustor.set_rs1_sp()
        else:
            operand_adjustor.set_rs1_int()

    return True

def adjust_vs3_rs1(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vs3_ls_source()
    operand_adjustor.set_rs1_int_ls_base()

    width = convert_width_to_size(aInstruction.find_operand('const_bits').value[-10:-7])
    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "rs1"
    attr_dict["alignment"] = width
    attr_dict["base"] = "rs1"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    return True

def adjust_vs3_rs1_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vs3_ls_source()
    operand_adjustor.set_rs1_int_ls_base()

    width = convert_width_to_size(aInstruction.find_operand('const_bits').value[-10:-7])
    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "rs1"
    attr_dict["alignment"] = width
    attr_dict["base"] = "rs1"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_vm()
    return True

def adjust_vs3_rs1_vs2_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vs3_ls_source()
    operand_adjustor.set_rs1_int_ls_base()

    width = convert_width_to_size(aInstruction.find_operand('const_bits').value[-10:-7])
    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "rs1"
    attr_dict["alignment"] = width
    attr_dict["base"] = "rs1"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_vs2()

    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "vs2"
    attr_dict["alignment"] = width
    attr_dict["base"] = "vs2"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_vm()
    return True

def adjust_vs3_rs1_rs2_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vs3_ls_source()
    operand_adjustor.set_rs1_int_ls_base()

    width = convert_width_to_size(aInstruction.find_operand('const_bits').value[-10:-7])
    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "rs1"
    attr_dict["alignment"] = width
    attr_dict["base"] = "rs1"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_rs2_int_ls_base()

    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "rs2"
    attr_dict["alignment"] = width
    attr_dict["base"] = "rs2"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_vm()
    return True

def adjust_rs1_vs2_vs3_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_rs1_int_ls_base()

    width = convert_width_to_size(aInstruction.find_operand('const_bits').value[-10:-7])
    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "rs1"
    attr_dict["alignment"] = width
    attr_dict["base"] = "rs1"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_vs2()

    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "vs2"
    attr_dict["alignment"] = width
    attr_dict["base"] = "vs2"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_vs3_ls_source()
    operand_adjustor.set_vm()
    return True

def adjust_rd_rs1_rs2(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_rd_int()
    operand_adjustor.set_rs1_int()
    operand_adjustor.set_rs2_int()
    return True

def adjust_rd_rs1_zimm_10_0(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_rd_int()
    operand_adjustor.set_rs1_int()
    operand_adjustor.set_imm('zimm[10:0]', 'zimm10', True)
    return True

def adjust_vdrd_rs1_vm(aInstruction):
    funct3 = aInstruction.find_operand('const_bits').value[11:14]
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    if funct3 == '101': #OPFVF
        operand_adjustor.set_vdrd_sp()
        operand_adjustor.set_rs1_sp()
    else:
        operand_adjustor.set_vdrd_int()
        operand_adjustor.set_rs1_int()
    operand_adjustor.set_vm()
    return True

def adjust_vd_rs1_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    if aInstruction.iclass == 'VectorLoadStoreInstruction':
        operand_adjustor.set_vd_ls_dest()
        operand_adjustor.set_rs1_int_ls_base()

        width = convert_width_to_size(aInstruction.find_operand('const_bits').value[-10:-7])
        attr_dict = dict()
        subop_dict = dict()
        subop_dict["base"] = "rs1"
        attr_dict["alignment"] = width
        attr_dict["base"] = "rs1"
        #attr_dict["data-size"] = 1 #TODO: depends on LMUL
        attr_dict["element-size"] = width
        attr_dict["mem-access"] = "Read"

        add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)
    else:
        funct3 = aInstruction.find_operand('const_bits').value[11:14]
        operand_adjustor.set_vd()
        if funct3 == '101': #OPFVF
            operand_adjustor.set_rs1_sp()
        else:
            operand_adjustor.set_rs1_int()
    operand_adjustor.set_vm()
    return True

def adjust_vd_rs1_vs2_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd_ls_dest()
    operand_adjustor.set_rs1_int_ls_base()

    width = convert_width_to_size(aInstruction.find_operand('const_bits').value[-10:-7])
    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "rs1"
    attr_dict["alignment"] = width
    attr_dict["base"] = "rs1"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_vs2()

    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "vs2"
    attr_dict["alignment"] = width
    attr_dict["base"] = "vs2"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_vm()
    return True

def adjust_vd_rs1_rs2_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd_ls_dest()
    operand_adjustor.set_rs1_int_ls_base()

    width = convert_width_to_size(aInstruction.find_operand('const_bits').value[-10:-7])
    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "rs1"
    attr_dict["alignment"] = width
    attr_dict["base"] = "rs1"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_rs2_int_ls_base()

    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "rs2"
    attr_dict["alignment"] = width
    attr_dict["base"] = "rs2"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_vm()
    return True

def adjust_rs1_vs2_vd_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_rs1_int_ls_base()

    width = convert_width_to_size(aInstruction.find_operand('const_bits').value[-10:-7])
    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "rs1"
    attr_dict["alignment"] = width
    attr_dict["base"] = "rs1"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_vs2()

    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "vs2"
    attr_dict["alignment"] = width
    attr_dict["base"] = "vs2"
    #attr_dict["data-size"] = 1 #TODO: depends on LMUL
    attr_dict["element-size"] = width
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

    operand_adjustor.set_vd_ls_dest()
    operand_adjustor.set_vm()
    return True

def adjust_vdrd_vs2_vm(aInstruction):
    funct3 = aInstruction.find_operand('const_bits').value[11:14]
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    if funct3 == '001': #OPFVV
        operand_adjustor.set_vdrd_sp()
    else:
        operand_adjustor.set_vdrd_int()
    operand_adjustor.set_vs2()
    operand_adjustor.set_vm()
    return True

def adjust_vd_vs2_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vs2()
    operand_adjustor.set_vm()
    return True

def adjust_vdrd_vs2(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    if '.F' in aInstruction.name:
        operand_adjustor.set_vdrd_sp()
    else:
        operand_adjustor.set_vdrd_int()
    operand_adjustor.set_vs2()
    return True

def adjust_vd_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vm()
    return True

def adjust_rd_vs2_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_rd_int()
    operand_adjustor.set_vs2()
    operand_adjustor.set_vm()
    return True

def adjust_rd_vs2(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_rd_int()
    operand_adjustor.set_vs2()
    return True

def adjust_vdrd_vs2_vs1_vm(aInstruction):
    funct3 = aInstruction.find_operand('const_bits').value[6:9]
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    if funct3 == '001': #OPFVV
        operand_adjustor.set_vdrd_sp()
    else:
        operand_adjustor.set_vdrd_int()
    operand_adjustor.set_vs2()
    operand_adjustor.set_vs1()
    operand_adjustor.set_vm()
    return True

def adjust_vd_vs1(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vs1()
    return True

def adjust_vd_vs2(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vs2()
    return True

def adjust_vd_vs2_vs1(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vs2()
    operand_adjustor.set_vs1()
    return True

def adjust_vd_nonzero_vs2_vs1(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd_nonzero()
    operand_adjustor.set_vs2()
    operand_adjustor.set_vs1()
    return True

def adjust_vd_vs2_vs1_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vs2()
    operand_adjustor.set_vs1()
    operand_adjustor.set_vm()
    return True

def adjust_vd_simm5(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_imm('simm5', 'simm5', True)
    return True

def adjust_vd_vs2_simm5(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vs2()
    operand_adjustor.set_imm('simm5', 'simm5', True)
    return True

def adjust_vd_nonzero_vs2_simm5(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd_nonzero()
    operand_adjustor.set_vs2()
    operand_adjustor.set_imm('simm5', 'simm5', True)
    return True

def adjust_vd_vs2_simm5_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vs2()
    operand_adjustor.set_imm('simm5', 'simm5', True)
    operand_adjustor.set_vm()
    return True

def adjust_vd_vs2_rs1(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vs2()
    operand_adjustor.set_rs1_int()
    return True

def adjust_vd_nonzero_vs2_rs1(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd_nonzero()
    operand_adjustor.set_vs2()
    operand_adjustor.set_rs1_int()
    return True

def adjust_vd_vs2_rs1_vm(aInstruction):
    funct3 = aInstruction.find_operand('const_bits').value[6:9]
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vs2()
    if funct3 == '101': #OPFVF
        operand_adjustor.set_rs1_sp()
    else:
        operand_adjustor.set_rs1_int()
    operand_adjustor.set_vm()
    return True

def adjust_vdrd_vs2_rs1_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vdrd_int()
    operand_adjustor.set_vs2()
    operand_adjustor.set_rs1_int()
    operand_adjustor.set_vm()
    return True

