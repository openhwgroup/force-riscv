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


def c_ext_adjust_instruction_by_format(aInstr):
    instr_format = aInstr.get_format()
    ret_val = False

    if instr_format == "rs1'/rd'-rs2'":
        ret_val = adjust_rs1p_or_rdp_rs2p(aInstr)
    elif instr_format == "rs1'/rd'":
        ret_val = adjust_rs1p_or_rdp(aInstr)
    elif instr_format == "imm[11|4|9:8|10|6|7|3:1|5]":
        ret_val = adjust_imm_branch(aInstr)
    elif instr_format == "imm[5]-rs1/rd$\\neq$0-imm[4:0]":
        ret_val = adjust_addiw(aInstr)
    elif instr_format == "imm[5]-rd$\\neq$0-imm[4:0]":
        ret_val = adjust_li(aInstr)
    elif instr_format == "imm[5]-rs1'/rd'-imm[4:0]":
        ret_val = adjust_andi(aInstr)
    elif instr_format == "imm[8|4:3]-rs1'-imm[7:6|2:1|5]":
        ret_val = adjust_cond_branch(aInstr)
    elif instr_format == "nzimm[17]-rd$\\neq$$\\{0,2\\}$-nzimm[16:12]":
        ret_val = adjust_lui(aInstr)
    elif instr_format == "nzimm[5]-nzimm[4:0]":
        ret_val = adjust_nop(aInstr)
    elif instr_format == "nzimm[5]-rs1/rd$\\neq$0-nzimm[4:0]":
        ret_val = adjust_addi(aInstr)
    elif instr_format == "nzimm[9]-2-nzimm[4|6|8:7|5]":
        ret_val = adjust_addi16sp(aInstr)
    elif instr_format == "nzuimm[5:4|9:6|2|3]-rd'":
        ret_val = adjust_addi4spn(aInstr)
    elif instr_format == "nzuimm[5]-rs1'/rd'-nzuimm[4:0]":
        ret_val = adjust_srai_srli(aInstr)
    elif instr_format == "nzuimm[5]-rs1/rd$\\neq$0-nzuimm[4:0]":
        ret_val = adjust_slli(aInstr)
    elif instr_format == "rd$\\neq$0-rs2$\\neq$0":
        ret_val = adjust_mv(aInstr)
    elif instr_format == "rs1$\\neq$0":
        ret_val = adjust_jalr_jr(aInstr)
    elif instr_format == "rs1/rd$\\neq$0":
        ret_val = adjust_slli64(aInstr)
    elif instr_format == "rs1/rd$\\neq$0-rs2$\\neq$0":
        ret_val = adjust_add(aInstr)
    elif instr_format == "uimm[5:2|7:6]-rs2":
        # SWSP FSWSP
        #      12-9  8-7
        # uimm[5:2 | 7:6]
        ret_val = adjust_store_sp(
            aInstr, "uimm[5:2|7:6]", "8-7,12-9", 4, 2
        )  # with imm name, size, and scale parameters
    elif instr_format == "uimm[5:3]-rs1'-uimm[2|6]-rd'":
        # LW FLW
        #      12-10  6   5
        # uimm[5:3  | 2 | 6]
        ret_val = adjust_load(
            aInstr, "uimm[5:3]", "uimm[2|6]", "5,12-10,6", 4, 2
        )  # imm names, size, and scale parameters
    elif instr_format == "uimm[5:3]-rs1'-uimm[2|6]-rs2'":
        # SW FSW
        ret_val = adjust_store(
            aInstr, "uimm[5:3]", "uimm[2|6]", "5,12-10,6", 4, 2
        )  # imm names, size, and scale parameters
    elif instr_format == "uimm[5:3]-rs1'-uimm[7:6]-rd'":
        # LD FLD
        #      12-10  6-5
        # uimm[5:3  | 7:6]
        ret_val = adjust_load(
            aInstr, "uimm[5:3]", "uimm[7:6]", "6-5,12-10", 8, 3
        )  # imm names, size, and scale parameters
    elif instr_format == "uimm[5:3]-rs1'-uimm[7:6]-rs2'":
        # SD FSD
        #      12-10  6-5
        # uimm[5:3  | 7:6]
        ret_val = adjust_store(
            aInstr, "uimm[5:3]", "uimm[7:6]", "6-5,12-10", 8, 3
        )  # imm names, size, and scale parameters
    elif instr_format == "uimm[5:3|8:6]-rs2":
        # SDSP FSDSP
        #      12-10  9-7
        # uimm[5:3  | 8:6]
        ret_val = adjust_store_sp(
            aInstr, "uimm[5:3|8:6]", "9-7,12-10", 8, 3
        )  # with imm name, size, and scale parameters
    elif instr_format == "uimm[5:4|8]-rs1'-uimm[7:6]-rd'":
        # LQ
        #      12  11  10  6-5
        # uimm[5 | 4 | 8 | 7:6]
        ret_val = adjust_load(
            aInstr, "uimm[5:4|8]", "uimm[7:6]", "10,6-5,12-11", 16, 4
        )  # imm names, size, and scale parameters
    elif instr_format == "uimm[5:4|8]-rs1'-uimm[7:6]-rs2'":
        # SQ
        #      12  11  10  6-5
        # uimm[5 | 4 | 8 | 7:6]
        ret_val = adjust_store(
            aInstr, "uimm[5:4|8]", "uimm[7:6]", "10,6-5,12-11", 16, 4
        )  # imm names, size, and scale parameters
    elif instr_format == "uimm[5:4|9:6]-rs2":
        # SQSP
        #      12-11  10-7
        # uimm[5:4  | 9:6]
        ret_val = adjust_store_sp(
            aInstr, "uimm[5:4|9:6]", "10-7,12-11", 16, 4
        )  # with imm name, size, and scale parameters
    elif (instr_format == "uimm[5]-rd$\\neq$0-uimm[4:2|7:6]") or (
        instr_format == "uimm[5]-rd-uimm[4:2|7:6]"
    ):
        # C.LWSP or C.FLWSP
        #      12  6-4   3-2
        # uimm[5 | 4:2 | 7:6]
        ret_val = adjust_load_sp(
            aInstr, "uimm[5]", "uimm[4:2|7:6]", "3-2,12,6-4", 4, 2
        )  # with imm name, size, and scale parameters
    elif (instr_format == "uimm[5]-rd$\\neq$0-uimm[4:3|8:6]") or (
        instr_format == "uimm[5]-rd-uimm[4:3|8:6]"
    ):
        # C.LDSP or C.FLDSP
        #      12  6-5   4-2
        # uimm[5 | 4:3 | 8:6]
        ret_val = adjust_load_sp(
            aInstr, "uimm[5]", "uimm[4:3|8:6]", "4-2,12,6-5", 8, 3
        )  # with imm name, size, and scale parameters
    elif instr_format == "uimm[5]-rd$\\neq$0-uimm[4|9:6]":
        # C.LQSP
        #      12  6   5-2
        # uimm[5 | 4 | 9:6]
        ret_val = adjust_load_sp(
            aInstr, "uimm[5]", "uimm[4|9:6]", "5-2,12,6", 16, 4
        )  # with imm name, size, and scale parameters
    elif aInstr.name == "C.EBREAK":
        ret_val = True
    else:
        record_instruction_format(instr_format)

    return ret_val


def record_instruction_format(aInstrFormat):
    if aInstrFormat in format_map:
        format_map[aInstrFormat] += 1
    else:
        format_map[aInstrFormat] = 1


def adjust_rs1p_or_rdp_rs2p(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.set_rs1p_or_rdp_int()
    opr_adjustor.set_rs2p_int()
    return True


# C.SRAI64, C.SRLI64
def adjust_rs1p_or_rdp(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.set_rs1p_or_rdp_int()
    return True


# C.J C.JAL
def adjust_imm_branch(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.set_imm("imm[11|4|9:8|10|6|7|3:1|5]", "simm11", True)

    #    12   11  10-9  8    7   6   5-3   2
    # imm[11 | 4 | 9:8 | 10 | 6 | 7 | 3:1 | 5]
    imm_opr = aInstr.find_operand("simm11")
    imm_opr.bits = "12,8,10-9,6,7,2,11,5-3"

    aInstr.iclass = "BranchInstruction"

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["offset"] = "simm11"
    attr_dict["offset-scale"] = "1"
    class_name = "PcRelativeBranchOperand"

    add_addressing_operand(
        aInstr, None, "Branch", class_name, subop_dict, attr_dict
    )

    # C.JAL is RV32 only instruction whose opcode overlap with C.ADDIW
    if aInstr.name == "C.JAL":
        # insert after the addressing mode operand
        opr_adjustor.add_implied_register("x1", "GPR", "Write", 1)

    return True


# C.BEQZ C.BNEZ
def adjust_cond_branch(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.set_imm("imm[7:6|2:1|5]", "simm8", True)
    aInstr.remove_operand("imm[8|4:3]")

    opr_adjustor.set_rs1p_int()
    opr_adjustor.add_implied_register(
        "x0", "GPR", "Read", 1
    )  # insert after the rs1' operand

    #     12  11-10  6-5   4-3   2
    # imm[8 | 4:3  | 7:6 | 2:1 | 5]
    imm_opr = aInstr.find_operand("simm8")
    imm_opr.bits = "12,6-5,2,11-10,4-3"

    aInstr.iclass = "BranchInstruction"

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["offset"] = "simm8"
    attr_dict["offset-scale"] = "1"
    class_name = "CompressedConditionalBranchOperandRISCV"

    if aInstr.name == "C.BEQZ":
        attr_dict["condition"] = "CBEQZ"
    elif aInstr.name == "C.BNEZ":
        attr_dict["condition"] = "CBNEZ"

    add_addressing_operand(
        aInstr, None, "Branch", class_name, subop_dict, attr_dict
    )
    return True


# C.ADDIW
def adjust_addiw(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.set_rd_rs1_nonzero_int()
    opr_adjustor.merge_imm_5_4_0()
    return True


# C.LI
def adjust_li(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.set_rd_nonzero_int()
    opr_adjustor.merge_imm_5_4_0()
    return True


# C.ANDI
def adjust_andi(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.set_rs1p_or_rdp_int()
    opr_adjustor.merge_imm_5_4_0()
    return True


# C.LUI
def adjust_lui(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.set_rd_not02_int()
    opr_adjustor.merge_nzimm_17_16_12()
    return True


# C.NOP
def adjust_nop(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    aInstr.remove_operand("nzimm[5]")
    imm_opr = aInstr.find_operand("nzimm[4:0]")
    imm_opr.bits = "12,6-2"
    imm_opr.value = "000000"
    imm_opr.type = "Constant"
    opr_adjustor.merge_into_const_bits(imm_opr)
    return True


# C.ADDI
def adjust_addi(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.set_rd_rs1_nonzero_int()
    opr_adjustor.merge_nzimm_5_4_0()
    return True


# C.ADDI16SP
def adjust_addi16sp(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.add_implied_register("x2", "GPR", "ReadWrite", 1)
    const_2 = aInstr.find_operand("2")
    const_2.value = "00010"
    const_2.type = "Constant"
    opr_adjustor.merge_into_const_bits(const_2)

    aInstr.remove_operand("nzimm[9]")
    imm_opr = aInstr.find_operand("nzimm[4|6|8:7|5]")
    #        12  6   5   4-3   2
    # nzimm[ 9 | 4 | 6 | 8:7 | 5 ]
    imm_opr.bits = "12,4-3,5,2,6"
    imm_opr.name = "imm6"
    opr_adjustor.set_nzimm(imm_opr)
    return True


# C.ADDI4SPN
def adjust_addi4spn(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.add_implied_register("x2", "GPR", "Read", 1)
    opr_adjustor.set_rdp_int()

    nzuimm_opr = aInstr.find_operand("nzuimm[5:4|9:6|2|3]")
    #         12-11  10-7   6   5
    # nzuimm[ 5:4   | 9:6 | 2 | 3 ]
    nzuimm_opr.bits = "10-7,12-11,5,6"
    nzuimm_opr.name = "imm8"
    opr_adjustor.set_nzimm(nzuimm_opr)
    return True


# C.SRAI C.SRLI
def adjust_srai_srli(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.set_rs1p_or_rdp_int()
    opr_adjustor.merge_nzuimm_5_4_0("shamt")
    return True


# C.SLLI
def adjust_slli(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)

    opr_adjustor.set_rd_rs1_nonzero_int()
    opr_adjustor.merge_nzuimm_5_4_0("shamt")
    return True


# C.MV
def adjust_mv(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)
    opr_adjustor.set_rd_nonzero_int()
    opr_adjustor.set_rs2_nonzero_int()
    return True


# C.JALR C.JR
def adjust_jalr_jr(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)
    opr_adjustor.set_rs1_nonzero_int()

    aInstr.iclass = "BranchInstruction"

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["base"] = "rs1"
    attr_dict["base"] = "rs1"
    class_name = "RegisterBranchOperand"

    add_addressing_operand(
        aInstr, None, "Branch", class_name, subop_dict, attr_dict
    )

    if aInstr.name == "C.JALR":
        # insert after the addressing mode operand
        opr_adjustor.add_implied_register("x1", "GPR", "Write", 1)
    return True


# C.SLLI64
def adjust_slli64(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)
    opr_adjustor.set_rd_rs1_nonzero_int()
    return True


# C.ADD
def adjust_add(aInstr):
    opr_adjustor = OperandAdjustor(aInstr)
    opr_adjustor.set_rd_rs1_nonzero_int()
    opr_adjustor.set_rs2_nonzero_int()
    return True


# C.SWSP C.FSWSP C.SDSP C.SQSP C.FSDSP
def adjust_store_sp(aInstr, aImmName, aImmBits, aSize, aScale):
    opr_adjustor = OperandAdjustor(aInstr)
    opr_adjustor.add_implied_register("x2", "GPR", "Read", 1)

    imm_opr = aInstr.find_operand(aImmName)
    imm_opr.bits = aImmBits
    imm_opr.name = "imm6"
    is_int = False
    is_sp = False

    if aInstr.name in ["C.SWSP", "C.SDSP"]:
        is_int = True

    # C.SQSP is RV128 only instruction whose opcode overlap with C.FSDSP
    elif aInstr.name == "C.SQSP":
        is_int = True
        return False

    # C.FSWSP is RV32 only instruction whose opcode overlap with C.SDSP
    elif aInstr.name == "C.FSWSP":
        is_sp = True

    elif aInstr.name != "C.FSDSP":
        return False

    if is_int:
        opr_adjustor.set_rs2_int()
    elif is_sp:
        opr_adjustor.set_rs2_sp()
    else:
        opr_adjustor.set_rs2_dp()

    aInstr.iclass = "LoadStoreInstruction"

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["base"] = "x2"
    subop_dict["offset"] = "imm6"
    attr_dict["offset-scale"] = "%d" % aScale
    attr_dict["alignment"] = aSize
    attr_dict["base"] = "x2"
    attr_dict["data-size"] = aSize
    attr_dict["element-size"] = aSize
    attr_dict["mem-access"] = "Write"

    add_addressing_operand(
        aInstr, None, "LoadStore", None, subop_dict, attr_dict
    )
    return True


# C.LWSP C.FLWSP C.LDSP C.LQSP C.FLDSP
def adjust_load_sp(aInstr, aImm1, aImm2, aImmBits, aSize, aScale):
    opr_adjustor = OperandAdjustor(aInstr)
    opr_adjustor.add_implied_register("x2", "GPR", "Read", 1)
    aInstr.remove_operand(aImm1)

    imm_opr = aInstr.find_operand(aImm2)
    imm_opr.bits = aImmBits
    imm_opr.name = "imm6"
    is_int = False
    is_sp = False

    if aInstr.name in ["C.LWSP", "C.LDSP"]:
        is_int = True

    # C.LQSP is RV128 only instruction whose opcode overlap with C.FLDSP
    elif aInstr.name == "C.LQSP":
        is_int = True
        return False

    # C.FLWSP is RV32 only instruction whose opcode overlap with C.LDSP
    elif aInstr.name == "C.FLWSP":
        is_sp = True

    elif aInstr.name != "C.FLDSP":
        return False

    if is_int:
        opr_adjustor.set_rd_nonzero_int()
    elif is_sp:
        opr_adjustor.set_rd_sp()
    else:
        opr_adjustor.set_rd_dp()

    aInstr.iclass = "LoadStoreInstruction"

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["base"] = "x2"
    subop_dict["offset"] = "imm6"
    attr_dict["offset-scale"] = "%d" % aScale
    attr_dict["alignment"] = aSize
    attr_dict["base"] = "x2"
    attr_dict["data-size"] = aSize
    attr_dict["element-size"] = aSize
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(
        aInstr, None, "LoadStore", None, subop_dict, attr_dict
    )
    return True


# C.LW C.FLW C.LD C.FLD C.LQ
def adjust_load(aInstr, aImm1, aImm2, aImmBits, aSize, aScale):
    opr_adjustor = OperandAdjustor(aInstr)

    aInstr.remove_operand(aImm1)
    imm_opr = aInstr.find_operand(aImm2)
    imm_opr.bits = aImmBits
    imm_opr.name = "imm5"
    is_int = False
    is_sp = False

    if aInstr.name in ["C.LW", "C.LD"]:
        is_int = True

    # C.LQ is RV128 only instruction whose opcode overlap with C.FLD
    elif aInstr.name == "C.LQ":
        is_int = True
        return False

    # C.FLW is RV32 only instruction whose opcode overlap with C.LD
    elif aInstr.name == "C.FLW":
        is_sp = True

    elif aInstr.name != "C.FLD":
        return False

    opr_adjustor.set_rs1p_int()
    if is_int:
        opr_adjustor.set_rdp_int()
    elif is_sp:
        opr_adjustor.set_rdp_sp()
    else:
        opr_adjustor.set_rdp_dp()

    aInstr.iclass = "LoadStoreInstruction"

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["base"] = "rs1'"
    subop_dict["offset"] = "imm5"
    attr_dict["offset-scale"] = "%d" % aScale
    attr_dict["alignment"] = aSize
    attr_dict["base"] = "rs1'"
    attr_dict["data-size"] = aSize
    attr_dict["element-size"] = aSize
    attr_dict["mem-access"] = "Read"

    add_addressing_operand(
        aInstr, None, "LoadStore", None, subop_dict, attr_dict
    )
    return True


# C.SW C.FSW C.SD C.FSD C.SQ
def adjust_store(aInstr, aImm1, aImm2, aImmBits, aSize, aScale):
    opr_adjustor = OperandAdjustor(aInstr)

    aInstr.remove_operand(aImm1)
    imm_opr = aInstr.find_operand(aImm2)
    imm_opr.bits = aImmBits
    imm_opr.name = "imm5"
    is_int = False
    is_sp = False

    if aInstr.name in ["C.SW", "C.SD"]:
        is_int = True

    # C.SQ is RV128 only instruction whose opcode overlap with C.FSD
    elif aInstr.name == "C.SQ":
        is_int = True
        return False

    # C.FSW is RV32 only instruction whose opcode overlap with C.SD
    elif aInstr.name == "C.FSW":
        is_sp = True

    elif aInstr.name != "C.FSD":
        return False

    opr_adjustor.set_rs1p_int()
    if is_int:
        opr_adjustor.set_rs2p_int()
    elif is_sp:
        opr_adjustor.set_rs2p_sp()
    else:
        opr_adjustor.set_rs2p_dp()

    aInstr.iclass = "LoadStoreInstruction"

    attr_dict = dict()  # dict for additional attribute
    subop_dict = dict()  # dict for sub operands
    subop_dict["base"] = "rs1'"
    subop_dict["offset"] = "imm5"
    attr_dict["offset-scale"] = "%d" % aScale
    attr_dict["alignment"] = aSize
    attr_dict["base"] = "rs1'"
    attr_dict["data-size"] = aSize
    attr_dict["element-size"] = aSize
    attr_dict["mem-access"] = "Write"

    add_addressing_operand(
        aInstr, None, "LoadStore", None, subop_dict, attr_dict
    )
    return True
