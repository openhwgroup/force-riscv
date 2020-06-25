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
from vector_operand_adjustor import VectorOperandAdjustor
from xml.sax.saxutils import escape

format_map = {}

def v_ext_adjust_instruction_by_format(aInstruction):
    # escaping < and > characters
    aInstruction.name = escape(aInstruction.name)
    aInstruction.asm.format = escape(aInstruction.asm.format)

    instruction_format = get_format(aInstruction)

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
    elif instruction_format == 'rs2-rs1-rd':
        return adjust_rs2_rs1_rd(aInstruction)
    elif instruction_format == 'zimm[10:0]-rs1-rd':
        return adjust_zimm_10_0_rs1_rd(aInstruction)
    else:
        record_instruction_format(instruction_format)

    return False

def get_format(aInstruction):
    opr_names = list()
    for opr in aInstruction.operands:
        if opr.name != "const_bits" and opr.name != "vtype":
            opr_names.append(opr.name)

    return "-".join(opr_names)

def record_instruction_format(aInstructionFormat):
    if aInstructionFormat in format_map:
        format_map[aInstructionFormat] += 1
    else:
        format_map[aInstructionFormat] = 1

def adjust_rs2_rs1_rd(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_rs2_int()
    operand_adjustor.set_rs1_int()
    operand_adjustor.set_rd_int()
    return True

def adjust_zimm_10_0_rs1_rd(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_imm('zimm[10:0]', 'zimm10', True)
    operand_adjustor.set_rs1_int()
    operand_adjustor.set_rd_int()
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

def adjust_vd_vs2_vs1_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vs2()
    operand_adjustor.set_vs1()
    operand_adjustor.set_vm()
    return True

def adjust_vd_vs2_simm5_vm(aInstruction):
    operand_adjustor = VectorOperandAdjustor(aInstruction)
    operand_adjustor.set_vd()
    operand_adjustor.set_vs2()
    operand_adjustor.set_imm('simm5', 'simm5', True)
    operand_adjustor.set_vm()
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

