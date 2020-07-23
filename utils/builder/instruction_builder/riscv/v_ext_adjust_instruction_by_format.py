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
    elif instruction_format == 'vd-rs1-vm':
        return adjust_vd_rs1_vm(aInstruction)
    elif instruction_format == 'vd/rd-rs1-vm':
        return adjust_vdrd_rs1_vm(aInstruction)
    # vsetvl/i instructions
    elif instruction_format == 'rd-rs1-rs2':
        return adjust_rd_rs1_rs2(aInstruction)
    elif instruction_format == 'rd-rs1-zimm[10:0]':
        return adjust_rd_rs1_zimm_10_0(aInstruction)
    # vmerge and vmv instructions
    elif instruction_format == 'vd-vs2-simm5':
        return adjust_vd_vs2_simm5(aInstruction)
    elif instruction_format == 'vd-vs2-vs1':
        return adjust_vd_vs2_vs1(aInstruction)
    elif instruction_format == 'vd-vs2-rs1':
        return adjust_vd_vs2_rs1(aInstruction)
    elif instruction_format == 'vd-simm5':
        return adjust_vd_simm5(aInstruction)
    elif instruction_format == 'vd-vs1':
        return adjust_vd_vs1(aInstruction)
    # vl<nf>r/vs<nf>r instructions
    elif instruction_format == 'vd-rs1':
        return adjust_vd_rs1(aInstruction)
    elif instruction_format == 'vs3-rs1':
        return adjust_vs3_rs1(aInstruction)
    else:
        record_instruction_format(instruction_format)

    return False

def record_instruction_format(aInstructionFormat):
    if aInstructionFormat in format_map:
        format_map[aInstructionFormat] += 1
    else:
        format_map[aInstructionFormat] = 1

def add_layout_operand(aInstruction):
    if aInstruction.name not in ('VSETVL', 'VSETVLI', 'VL1R.V', 'VS1R.V'):
        operand_adjustor = VectorOperandAdjustor(aInstruction)
        operand_adjustor.add_vtype_layout_operand()
    elif aInstruction.name in ('VL1R.V', 'VS1R.V'):
        operand_adjustor = VectorOperandAdjustor(aInstruction)
        operand_adjustor.add_whole_register_layout_operand()

# Account for non-standard register layouts due to widening and narrowing instructions
def adjust_register_layout(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    if aInstruction.name.startswith('VW') or aInstruction.name.startswith('VFW'):
        operand_adjustor.set_wide_dest()

    if '.W' in aInstruction.name:
        operand_adjustor.set_wide_source()

def adjust_vd_rs1(aInstruction):
    if aInstruction.iclass == 'VectorLoadStoreInstruction':
        operand_adjustor = VectorOperandAdjustor(aInstruction)
        operand_adjustor.set_vd_ls_dest()
        operand_adjustor.set_rs1_int_ls_base()

        attr_dict = dict()
        subop_dict = dict()
        subop_dict["base"] = "rs1"
        attr_dict["alignment"] = 1
        attr_dict["base"] = "rs1"
        attr_dict["data-size"] = 1
        attr_dict["element-size"] = 1
        attr_dict["mem-access"] = "Read"

        add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)
    else:
        operand_adjustor = VectorOperandAdjustor(aInstruction)
        operand_adjustor.set_vd()
        operand_adjustor.set_rs1_int()

    return True

def adjust_vs3_rs1(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vs3_ls_source()
    operand_adjustor.set_rs1_int_ls_base()

    attr_dict = dict()
    subop_dict = dict()
    subop_dict["base"] = "rs1"
    attr_dict["alignment"] = 1
    attr_dict["base"] = "rs1"
    attr_dict["data-size"] = 1
    attr_dict["element-size"] = 1
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(aInstruction, None, "LoadStore", "VectorLoadStoreOperand", subop_dict, attr_dict)

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
    funct3 = aInstruction.find_operand('const_bits').value[11:14]
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    if funct3 == '101': #OPFVF
        operand_adjustor.set_rs1_sp()
    else:
        operand_adjustor.set_rs1_int()
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

def adjust_vd_vs2_vs1(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
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

