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

from operand_adjustor import *
from shared.instruction import *

format_map = {}


def priv_adjust_instruction_by_format(instr):
    instr_format = instr.get_format()

    if instr_format == "rs2-rs1":
        return adjust_rs2_rs1(instr)
    elif instr_format == "rs1-rd":
        return adjust_rs1_rd(instr)
    elif instr_format == "":
        return adjust_const_only(instr)
    else:
        # print ("TODO instruction format: %s" % instr_format)
        record_instruction_format(instr_format)
        pass

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


def adjust_rs2_rs1(instr):
    if instr.name in ["SFENCE.VMA"]:
        instr.group = "System"
        instr.extension = "RV64Priv"
        opr_adjustor = OperandAdjustor(instr)
        opr_adjustor.set_rs2_int()
        opr_adjustor.set_rs1_int()
        return True

    return False


def adjust_rs1_rd(instr):
    return False


def adjust_const_only(instr):
    if instr.name in ["MRET", "SRET"]:
        instr.group = "System"
        instr.iclass = "RetInstruction"
        instr.extension = "RV64Priv"
        instr.constOp.bits = "31-0"
        ret_operand = Operand()
        ret_operand.name = "retoperand"
        ret_operand.type = "Choices"
        ret_operand.choices = "Ret choice"
        ret_operand.oclass = "RetOperand"
        instr.add_operand(ret_operand)
        return True

    return False
