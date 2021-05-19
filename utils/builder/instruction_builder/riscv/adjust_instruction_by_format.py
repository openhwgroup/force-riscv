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
import re

from operand_adjustor import *
from shared.instruction import *

format_map = {}


def adjust_instruction_by_format(instr):

    instr_format = instr.get_format()

    if instr_format == "rs2-rs1-rd":
        return adjust_rs2_rs1_rd(instr)
    elif instr_format == "imm[11:0]-rs1-rd":
        return adjust_imm12_rs1_rd(instr)
    elif instr_format == "shamt-rs1-rd":
        return adjust_shamt_rs1_rd(instr)
    elif instr_format == "imm[31:12]-rd":
        return adjust_imm20_rd(instr)
    elif instr_format == "rs1-rd":
        return adjust_rs1_rd(instr)
    elif instr_format == "imm[20|10:1|11|19:12]-rd":
        return adjust_jal(instr)
    elif instr_format == "imm[12|10:5]-rs2-rs1-imm[4:1|11]":
        return adjust_branches(instr)
    elif instr_format == "imm[11:5]-rs2-rs1-imm[4:0]":
        return adjust_stores(instr)
    elif instr_format == "aq-rl-rs2-rs1-rd":
        return adjust_aq_rs2(instr)
    elif instr_format == "aq-rl-rs1-rd":
        return adjust_aq_rs1(instr)
    elif instr_format == "csr-rs1-rd":
        return adjust_csr_rs1(instr)
    elif instr_format == "csr-uimm-rd":
        return adjust_csr_imm(instr)
    elif instr_format == "rs3-rs2-rs1-rm-rd":
        return adjust_f_rs3(instr)
    elif instr_format == "rs2-rs1-rm-rd":
        return adjust_f_rs2(instr)
    elif instr_format == "rs1-rm-rd":
        return adjust_f_rs1(instr)
    elif instr_format == "fm-pred-succ-rs1-rd":
        return adjust_fence(instr)
    elif instr_format == "":
        return adjust_const_only(instr)
    else:
        record_instruction_format(instr_format)
        pass

    # dump_format_map()
    return False


def record_instruction_format(aInstrFormat):
    if aInstrFormat in format_map:
        format_map[aInstrFormat] += 1
    else:
        format_map[aInstrFormat] = 1


def dump_format_map():
    print("========================================")
    for key, value in sorted(format_map.items()):
        print("Format: %s, count: %d" % (key, value))


def adjust_rs2_rs1_rd(instr):
    opr_adjustor = OperandAdjustor(instr)

    # print("rs2-rs1-rd instr: {}".format(instr.name))

    if instr.name.startswith("F"):
        if (
            instr.name.startswith("FLE")
            or instr.name.startswith("FLT")
            or instr.name.startswith("FEQ")
        ):
            return adjust_fp_rs2_rs1_int_rd(instr)
        else:
            return adjust_fp_rs2_rs1_rd(instr)
    else:
        # integer instructions
        opr_adjustor.set_rd_int()
        opr_adjustor.set_rs1_int()
        opr_adjustor.set_rs2_int()
        return True


def adjust_fp_rs2_rs1_int_rd(instr):
    opr_adjustor = OperandAdjustor(instr)
    opr_adjustor.set_rd_int()
    instr.group = "Float"

    if ".H" in instr.name:
        opr_adjustor.set_rs2_hp()
        opr_adjustor.set_rs1_hp()
        return True
    if ".S" in instr.name:
        opr_adjustor.set_rs2_sp()
        opr_adjustor.set_rs1_sp()
        return True
    if ".D" in instr.name:
        opr_adjustor.set_rs2_dp()
        opr_adjustor.set_rs1_dp()
        return True
    if ".Q" in instr.name:
        opr_adjustor.set_rs2_qp()
        opr_adjustor.set_rs1_qp()
        return True

    return False


def adjust_fp_rs2_rs1_rd(instr):
    opr_adjustor = OperandAdjustor(instr)

    instr.group = "Float"

    if ".H" in instr.name:
        opr_adjustor.set_rs2_hp()
        opr_adjustor.set_rs1_hp()
        opr_adjustor.set_rd_hp()
        return True
    if ".S" in instr.name:
        opr_adjustor.set_rs2_sp()
        opr_adjustor.set_rs1_sp()
        opr_adjustor.set_rd_sp()
        return True
    if ".D" in instr.name:
        opr_adjustor.set_rs2_dp()
        opr_adjustor.set_rs1_dp()
        opr_adjustor.set_rd_dp()
        return True
    if ".Q" in instr.name:
        opr_adjustor.set_rs2_qp()
        opr_adjustor.set_rs1_qp()
        opr_adjustor.set_rd_qp()
        return True

    return False


# Lots integer instructions fall into this category.  Like ADDI, JALR, LB etc.
def adjust_imm12_rs1_rd(instr):
    opr_adjustor = OperandAdjustor(instr)
    instr_full_ID = instr.get_full_ID()

    if instr.name in ["ADDI", "ADDIW", "ANDI", "ORI", "SLTI", "SLTIU", "XORI"]:
        opr_adjustor.set_rd_int()
        opr_adjustor.set_rs1_int()
        opr_adjustor.set_imm("imm[11:0]", "simm12", True)
        return True
    elif instr.name == "JALR":
        return adjust_jalr(instr)
    elif instr.name in [
        "LB",
        "LBU",
        "LD",
        "LH",
        "LHU",
        "LW",
        "LWU",
    ]:  # integer load store instructions.
        return adjust_int_load(instr)
    elif instr_full_ID.startswith("FENCE"):
        instr.group = "System"
        opr_adjustor.set_imm("imm[11:0]", "imm12", False)
        opr_adjustor.set_rs1_int()
        opr_adjustor.set_rd_int()
        return True
    elif instr_full_ID.startswith("FL"):
        instr.group = "Float"
        instr.iclass = "LoadStoreInstruction"
        opr_adjustor.set_imm("imm[11:0]", "simm12", True)
        opr_adjustor.set_rs1_int_ls_base()

        if "H" in instr_full_ID:
            size = 2
            opr_adjustor.set_rd_hp()
        if "W" in instr_full_ID:
            size = 4
            opr_adjustor.set_rd_sp()
        elif "D" in instr_full_ID:
            size = 8
            opr_adjustor.set_rd_dp()
        elif "Q" in instr_full_ID:
            size = 16
            opr_adjustor.set_rd_qp()

        attr_dict = dict()  # dict for additional attribute
        subop_dict = dict()  # dict for sub operands
        subop_dict["base"] = "rs1"
        subop_dict["offset"] = "simm12"
        attr_dict["offset-scale"] = "0"
        attr_dict["alignment"] = size
        attr_dict["base"] = "rs1"
        attr_dict["data-size"] = size
        attr_dict["element-size"] = size
        attr_dict["mem-access"] = "Read"

        add_addressing_operand(instr, None, "LoadStore", None, subop_dict, attr_dict)
        return True

    # print ("instruction: %s" % instr.get_full_ID())
    return False


# Instructions like SLLI are handled here.
def adjust_shamt_rs1_rd(instr):
    opr_adjustor = OperandAdjustor(instr)
    opr_adjustor.set_rd_int()
    opr_adjustor.set_rs1_int()
    shamt_opr = instr.find_operand("shamt")
    opr_adjustor.add_asm_op(shamt_opr)

    if instr.name in ["SLLI", "SRAI", "SRLI"]:
        if shamt_opr.bits == "24-20":
            instr.form = "RV32I"
            return True
        if shamt_opr.bits == "25-20":
            instr.form = "RV64I"
            return True

    return True


# FCLASS/FMOV floating point instructions
def adjust_rs1_rd(instr):
    opr_adjustor = OperandAdjustor(instr)
    instr_full_ID = instr.get_full_ID()
    if instr_full_ID.startswith("FCLASS"):
        instr.group = "Float"
        if ".H" in instr_full_ID:
            opr_adjustor.set_rs1_hp()
            opr_adjustor.set_rd_int()
            return True
        if ".S" in instr_full_ID:
            opr_adjustor.set_rs1_sp()
            opr_adjustor.set_rd_int()
            return True
        elif ".D" in instr_full_ID:
            opr_adjustor.set_rs1_dp()
            opr_adjustor.set_rd_int()
            return True
        elif ".Q" in instr_full_ID:
            opr_adjustor.set_rs1_qp()
            opr_adjustor.set_rd_int()
            return True
    elif instr_full_ID.startswith("FMV"):
        instr.group = "Float"
        if ".W.X" in instr_full_ID:
            opr_adjustor.set_rs1_int()
            opr_adjustor.set_rd_sp()
            return True
        elif ".X.W" in instr_full_ID:
            opr_adjustor.set_rs1_sp()
            opr_adjustor.set_rd_int()
            return True
        elif ".H.X" in instr_full_ID:
            opr_adjustor.set_rs1_int()
            opr_adjustor.set_rd_hp()
            return True
        elif ".X.H" in instr_full_ID:
            opr_adjustor.set_rs1_hp()
            opr_adjustor.set_rd_int()
            return True
        elif ".D.X" in instr_full_ID:
            opr_adjustor.set_rs1_int()
            opr_adjustor.set_rd_dp()
            return True
        elif ".X.D" in instr_full_ID:
            opr_adjustor.set_rs1_dp()
            opr_adjustor.set_rd_int()
            return True

    return False


def adjust_imm20_rd(instr):
    opr_adjustor = OperandAdjustor(instr)
    opr_adjustor.set_rd_int()
    opr_adjustor.set_imm("imm[31:12]", "simm20", True)
    return True


# This function handles JAL specifically.
def adjust_jal(instr):
    opr_adjustor = OperandAdjustor(instr)
    opr_adjustor.set_rd_int()
    opr_adjustor.set_imm("imm[20|10:1|11|19:12]", "simm20", True)
    imm_opr = instr.find_operand("simm20")
    imm_opr.bits = "31,19-12,20,30-21"

    instr.iclass = "BranchInstruction"

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["offset"] = "simm20"
    attr_dict["offset-scale"] = "1"
    class_name = "PcRelativeBranchOperand"

    add_addressing_operand(instr, None, "Branch", class_name, subop_dict, attr_dict)
    return True


def adjust_jalr(instr):
    opr_adjustor = OperandAdjustor(instr)
    opr_adjustor.set_rd_int()
    opr_adjustor.set_rs1_int()
    opr_adjustor.set_imm("imm[11:0]", "simm12", True)
    imm_opr = instr.find_operand("simm12")
    imm_opr.bits = "31-20"

    instr.iclass = "BranchInstruction"

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["base"] = "rs1"
    subop_dict["offset"] = "simm12"
    attr_dict["offset-scale"] = "0"
    attr_dict["base"] = "rs1"
    class_name = "BaseOffsetBranchOperand"

    add_addressing_operand(instr, None, "Branch", class_name, subop_dict, attr_dict)
    return True


# Instructions like BEQ are handled here.
def adjust_branches(instr):
    opr_adjustor = OperandAdjustor(instr)
    opr_adjustor.set_rs1_int()
    opr_adjustor.set_rs2_int()
    opr_adjustor.set_imm("imm[4:1|11]", "simm12", True)
    imm_opr = instr.find_operand("simm12")
    imm_opr.bits = "31,7,30-25,11-8"

    remove_opr = instr.find_operand("imm[12|10:5]")
    instr.operands.remove(remove_opr)

    instr.iclass = "BranchInstruction"

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["offset"] = "simm12"
    attr_dict["offset-scale"] = "1"
    class_name = "ConditionalBranchOperandRISCV"

    if instr.name == "BEQ":
        attr_dict["condition"] = "BEQ"
    elif instr.name == "BGE":
        attr_dict["condition"] = "BGE"
    elif instr.name == "BGEU":
        attr_dict["condition"] = "BGEU"
    elif instr.name == "BLT":
        attr_dict["condition"] = "BLT"
    elif instr.name == "BLTU":
        attr_dict["condition"] = "BLTU"
    elif instr.name == "BNE":
        attr_dict["condition"] = "BNE"

    add_addressing_operand(instr, None, "Branch", class_name, subop_dict, attr_dict)
    return True


# Instructions including LB, LBU, LD, LH, LHU, LW, LWU are handled here.
def adjust_int_load(instr):
    opr_adjustor = OperandAdjustor(instr)
    opr_adjustor.set_rd_int()
    opr_adjustor.set_rs1_int_ls_base()
    opr_adjustor.set_imm("imm[11:0]", "simm12", True)

    instr.iclass = "LoadStoreInstruction"

    if instr.name in ["LB", "LBU"]:
        size = 1
    elif instr.name in ["LH", "LHU"]:
        size = 2
    elif instr.name in ["LW", "LWU"]:
        size = 4
    elif instr.name == "LD":
        size = 8

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["base"] = "rs1"
    subop_dict["offset"] = "simm12"
    attr_dict["offset-scale"] = "0"
    attr_dict["alignment"] = size
    attr_dict["base"] = "rs1"
    attr_dict["data-size"] = size
    attr_dict["element-size"] = size
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(instr, None, "LoadStore", None, subop_dict, attr_dict)
    return True


# Instructions including SD, FSD etc.
def adjust_stores(instr):
    if instr.name in ["SB", "SD", "SH", "SW"]:
        return adjust_int_stores(instr)

    if instr.name in ["FSW", "FSD", "FSQ", "FSH"]:
        return adjust_fp_stores(instr)

    return False


# Instructions including SB, SD, SH, SW.
def adjust_int_stores(instr):
    opr_adjustor = OperandAdjustor(instr)
    opr_adjustor.set_rs2_int()
    opr_adjustor.set_rs1_int_ls_base()
    opr_adjustor.set_imm("imm[11:5]", "simm12", True)

    imm_opr = instr.find_operand("simm12")
    imm_opr.bits = "31-25,11-7"

    remove_opr = instr.find_operand("imm[4:0]")
    instr.operands.remove(remove_opr)

    instr.iclass = "LoadStoreInstruction"

    if instr.name == "SB":
        size = 1
    elif instr.name == "SH":
        size = 2
    elif instr.name == "SW":
        size = 4
    elif instr.name == "SD":
        size = 8

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["base"] = "rs1"
    subop_dict["offset"] = "simm12"
    attr_dict["offset-scale"] = "0"
    attr_dict["alignment"] = size
    attr_dict["base"] = "rs1"
    attr_dict["data-size"] = size
    attr_dict["element-size"] = size
    attr_dict["mem-access"] = "Write"

    add_addressing_operand(instr, None, "LoadStore", None, subop_dict, attr_dict)
    return True


# FSW, FSD, FSQ, FSH - floating point store instructions
def adjust_fp_stores(instr):
    opr_adjustor = OperandAdjustor(instr)
    opr_adjustor.set_imm("imm[11:5]", "simm12", True)
    opr_adjustor.set_rs1_int_ls_base()

    imm_opr = instr.find_operand("simm12")
    imm_opr.bits = "31-25,11-7"

    remove_opr = instr.find_operand("imm[4:0]")
    instr.operands.remove(remove_opr)

    instr.iclass = "LoadStoreInstruction"
    instr.group = "Float"

    if "W" in instr.name:
        size = 4
        opr_adjustor.set_rs2_sp()
    if "H" in instr.name:
        size = 2
        opr_adjustor.set_rs2_hp()
    elif "D" in instr.name:
        size = 8
        opr_adjustor.set_rs2_dp()
    elif "Q" in instr.name:
        size = 16
        opr_adjustor.set_rs2_qp()

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["base"] = "rs1"
    subop_dict["offset"] = "simm12"
    attr_dict["offset-scale"] = "0"
    attr_dict["alignment"] = size
    attr_dict["base"] = "rs1"
    attr_dict["data-size"] = size
    attr_dict["element-size"] = size
    attr_dict["mem-access"] = "Write"

    add_addressing_operand(instr, None, "LoadStore", None, subop_dict, attr_dict)

    return True


# Instructions including SC.W, SC.D, AMO*.W, AMO*.D
def adjust_aq_rs2(instr):
    # aq, rl, rs2, rs1, rd
    opr_adjustor = OperandAdjustor(instr)
    opr_adjustor.set_rs2_int()
    opr_adjustor.set_rs1_int_ls_base()
    opr_adjustor.set_rd_int()

    instr_full_ID = instr.get_full_ID()
    if ".W" in instr_full_ID:
        size = 4
    elif ".D" in instr_full_ID:
        size = 8

    attr_dict = dict()
    subop_dict = dict()

    subop_dict["base"] = "rs1"

    attr_dict["alignment"] = size
    attr_dict["base"] = "rs1"
    attr_dict["data-size"] = size
    attr_dict["element-size"] = size

    if instr_full_ID.startswith("AMO"):
        instr.iclass = "LoadStoreInstruction"
        rs1_opr = instr.find_operand("rs1")
        rs1_opr.access = "ReadWrite"
        attr_dict["sorder"] = "AtomicRW"
        attr_dict["lorder"] = "AtomicRW"
        attr_dict["mem-access"] = "ReadWrite"
    elif instr_full_ID.startswith("SC"):
        instr.iclass = "UnpredictStoreInstruction"
        order = "Ordered"
        attr_dict["sorder"] = order
        attr_dict["mem-access"] = "Write"

    add_addressing_operand(instr, None, "LoadStore", None, subop_dict, attr_dict)
    return True


# Instructions including LR.W, LR.D
def adjust_aq_rs1(instr):
    # aq, rl, rs1, rd
    opr_adjustor = OperandAdjustor(instr)
    opr_adjustor.set_rs1_int_ls_base()
    opr_adjustor.set_rd_int()

    instr.iclass = "LoadStoreInstruction"

    instr_full_ID = instr.get_full_ID()

    if ".W" in instr_full_ID:
        size = 4
    elif ".D" in instr_full_ID:
        size = 8

    attr_dict = dict()
    subop_dict = dict()

    subop_dict["base"] = "rs1"

    attr_dict["alignment"] = size
    attr_dict["base"] = "rs1"
    attr_dict["data-size"] = size
    attr_dict["element-size"] = size
    attr_dict["mem-access"] = "Read"
    attr_dict["lorder"] = "Ordered"

    add_addressing_operand(instr, None, "LoadStore", None, subop_dict, attr_dict)
    return True


# CSR register Instructions
# <O name="systemreg" type="SysReg" bits="19,18-16,15-12,11-8,7-5"
# access="Write" choices="System registers"/>
# CSRRW - always writes
# if rd=x0, then the instruction shall not read the CSR and shall not
# cause any of the side effects that might occur on a CSR read
# CSRRS/C - always reads
# if rs1=x0 then the instruction will not write to the CSR at all, and so shall
# not cause any of the side effects that might otherwise occur on a CSR write
def adjust_csr_rs1(instr):
    opr_adjustor = OperandAdjustor(instr)
    instr.group = "System"
    instr.form = "register"

    csr_opr = instr.find_operand("csr")
    csr_opr.type = "SysReg"
    csr_opr.access = "ReadWrite"
    csr_opr.choices = "System registers"
    opr_adjustor.add_asm_op(csr_opr)

    opr_adjustor.set_rs1_int()
    opr_adjustor.set_rd_int()

    return True


# CSR Immediate Instructions
# the immediate forms use a 5-bit zero-extended immediate encoded in the rs1
# field
# CSRRWI - always writes
# if rd=x0, then the instruction shall not read the CSR and shall not cause
# any of the side effects that might occur on a CSR read
# CSRRSI/CSRRCI - alwas reads
# if rs1=x0 then the instruction will not write to the CSR at all, and so shall
# not cause any of the side effects that might otherwise occur on a CSR write
def adjust_csr_imm(instr):
    opr_adjustor = OperandAdjustor(instr)
    instr.group = "System"
    instr.form = "immediate"

    csr_opr = instr.find_operand("csr")
    csr_opr.type = "SysReg"
    csr_opr.access = "ReadWrite"
    csr_opr.choices = "System registers"
    opr_adjustor.add_asm_op(csr_opr)

    opr_adjustor.set_imm("uimm", "imm4", False)
    opr_adjustor.set_rd_int()

    return True


# Floating point Instructions w/ 3 register input operands
# adjust group/type to float
# fix rm type from register to immediate
def adjust_f_rs3(instr):
    opr_adjustor = OperandAdjustor(instr)
    instr_full_ID = instr.get_full_ID()

    instr.group = "Float"

    opr_adjustor.set_rm()

    if ".H" in instr_full_ID:
        opr_adjustor.set_rs1_hp()
        opr_adjustor.set_rs2_hp()
        opr_adjustor.set_rs3_hp()
        opr_adjustor.set_rd_hp()
        return True
    elif ".S" in instr_full_ID:
        opr_adjustor.set_rs1_sp()
        opr_adjustor.set_rs2_sp()
        opr_adjustor.set_rs3_sp()
        opr_adjustor.set_rd_sp()
        return True
    elif ".D" in instr_full_ID:
        opr_adjustor.set_rs1_dp()
        opr_adjustor.set_rs2_dp()
        opr_adjustor.set_rs3_dp()
        opr_adjustor.set_rd_dp()
        return True
    elif ".Q" in instr_full_ID:
        opr_adjustor.set_rs1_qp()
        opr_adjustor.set_rs2_qp()
        opr_adjustor.set_rs3_qp()
        opr_adjustor.set_rd_qp()
        return True

    return False


# Floating point Instructions w/ 2 register input operands
# adjust group/type to float
# fix rm type from register to immediate
def adjust_f_rs2(instr):
    opr_adjustor = OperandAdjustor(instr)
    instr_full_ID = instr.get_full_ID()

    instr.group = "Float"

    opr_adjustor.set_rm()

    if ".H" in instr_full_ID:
        opr_adjustor.set_rs1_hp()
        opr_adjustor.set_rs2_hp()
        opr_adjustor.set_rd_hp()
        return True
    elif ".S" in instr_full_ID:
        opr_adjustor.set_rs1_sp()
        opr_adjustor.set_rs2_sp()
        opr_adjustor.set_rd_sp()
        return True
    elif ".D" in instr_full_ID:
        opr_adjustor.set_rs1_dp()
        opr_adjustor.set_rs2_dp()
        opr_adjustor.set_rd_dp()
        return True
    elif ".Q" in instr_full_ID:
        opr_adjustor.set_rs1_qp()
        opr_adjustor.set_rs2_qp()
        opr_adjustor.set_rd_qp()
        return True

    return False


# Floating point Instructions w/ 1 register input operand
def adjust_f_rs1(instr):
    opr_adjustor = OperandAdjustor(instr)
    instr_full_ID = instr.get_full_ID()

    instr.group = "Float"

    opr_adjustor.set_rm()

    if instr_full_ID.startswith("FSQRT"):
        if ".H" in instr_full_ID:
            opr_adjustor.set_rs1_hp()
            opr_adjustor.set_rd_hp()
            return True
        elif ".S" in instr_full_ID:
            opr_adjustor.set_rs1_sp()
            opr_adjustor.set_rd_sp()
            return True
        elif ".D" in instr_full_ID:
            opr_adjustor.set_rs1_dp()
            opr_adjustor.set_rd_dp()
            return True
        elif ".Q" in instr_full_ID:
            opr_adjustor.set_rs1_qp()
            opr_adjustor.set_rd_qp()
            return True

    elif instr_full_ID.startswith("FCVT"):
        source_dest_pattern = re.compile(r"FCVT.(?P<dest>\w*).(?P<src>\w*)")
        source_dest_result = source_dest_pattern.match(instr_full_ID)

        src_str = source_dest_result.group("src")
        dest_str = source_dest_result.group("dest")

        if "S" in src_str:
            opr_adjustor.set_rs1_sp()
        elif "H" in src_str:
            opr_adjustor.set_rs1_hp()
        elif "D" in src_str:
            opr_adjustor.set_rs1_dp()
        elif "Q" in src_str:
            opr_adjustor.set_rs1_qp()
        elif ("W" in src_str) or ("L" in src_str):
            opr_adjustor.set_rs1_int()

        if "S" in dest_str:
            opr_adjustor.set_rd_sp()
        if "H" in dest_str:
            opr_adjustor.set_rd_hp()
        elif "D" in dest_str:
            opr_adjustor.set_rd_dp()
        elif "Q" in dest_str:
            opr_adjustor.set_rd_qp()
        elif ("W" in dest_str) or ("L" in dest_str):
            opr_adjustor.set_rd_int()

        return True

    return False


# Fence instruction
def adjust_fence(instr):
    opr_adjustor = OperandAdjustor(instr)
    instr_full_ID = instr.get_full_ID()

    if instr_full_ID.startswith("FENCE"):
        instr.group = "System"
        pred_opr = instr.find_operand("pred")
        pred_opr.choices = "Barrier option"
        succ_opr = instr.find_operand("succ")
        succ_opr.choices = "Barrier option"
        fm_opr = instr.find_operand("fm")
        fm_opr.choices = "Fence mode"
        opr_adjustor.set_rs1_int()
        opr_adjustor.set_rd_int()
        return True

    return False


# Const only operand instructions (ECALL, EBREAK)
def adjust_const_only(instr):
    instr_full_ID = instr.get_full_ID()

    if instr_full_ID.startswith("ECALL"):
        instr.iclass = "SystemCallInstruction"
        instr.group = "System"
        return True
    elif instr_full_ID.startswith("EBREAK"):
        instr.group = "System"
        return True

    return False
