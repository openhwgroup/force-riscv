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
# Basic test for speculative BNT sequence

from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from base.BntSequence import BntSequence
from base.ChoicesModifier import *


class MyBntSequence(BntSequence):
    def __init__(self, gen_thread, instr_num=10):
        super(MyBntSequence, self).__init__(gen_thread, instr_num)

    def BntSequence_IntegerArithmetic(self):
        self.notice("Speculative Generate Bnt Sequence for Arithmetic instructions")

        if self.getGlobalState("AppRegisterWidth") == 32:
            math_instr_dict = {
                "ADD##RISCV": 10,
                "ADDI##RISCV": 10,
                "SUB##RISCV": 10,
                "MUL##RISCV": 10,
                "MULH##RISCV": 10,
                "MULHSU##RISCV": 10,
                "MULHU##RISCV": 10,
                "DIV##RISCV": 10,
                "DIVU##RISCV": 10,
            }
        else:
            math_instr_dict = {
                "ADD##RISCV": 10,
                "ADDW##RISCV": 10,
                "ADDI##RISCV": 10,
                "ADDIW##RISCV": 10,
                "SUB##RISCV": 10,
                "SUBW##RISCV": 10,
                "MUL##RISCV": 10,
                "MULH##RISCV": 10,
                "MULHSU##RISCV": 10,
                "MULHU##RISCV": 10,
                "MULW##RISCV": 10,
                "DIV##RISCV": 10,
                "DIVU##RISCV": 10,
                "DIVUW##RISCV": 10,
                "DIVW##RISCV": 10,
            }

        for i in range(self.instr_num):
            instr_chosen = self.pickWeighted(math_instr_dict)
            self.genInstruction(instr_chosen)

        # set choices modifier shared by both taken and not-taken path
        choice_mod = ChoicesModifier(self.genThread)
        choice_mod.modifyOperandChoices(
            "GPRs", {"x1": 10000, "x2": 0}
        )  # X2 should be stack pointer
        mod_id = choice_mod.commitSet()

        self.setBntHook(
            **{"Func": "BntSequence_IntegerArithmetic"}
        )  # set the bnt sequence for the following branch instructions
        self.genInstruction(
            "JALR##RISCV",
            {
                "rs1": 1,
                "BRarget": "0xc0000000-0xc80000000",
                "SpeculativeBnt": True,
            },
        )
        self.genMetaInstruction("ADDI##RISCV", {"simm12": 0x200})  # instructions on taken path
        self.revertBntHook()  # revert the bnt hook to the last setting

        choice_mod.revert(mod_id)

    def BntSequence_L(self):
        self.notice("Speculative Generate Bnt Sequence for L")

        if self.getGlobalState("AppRegisterWidth") == 32:
            ls_dict = {
                "LR.W##RISCV": 10,
                "LUI##RISCV": 10,
                "LW##RISCV": 10,
                "LB##RISCV": 10,
                "LBU##RISCV": 10,
                "LH##RISCV": 10,
                "LHU##RISCV": 10,
                "SC.W##RISCV": 10,
                "SW##RISCV": 10,
                "SH##RISCV": 10,
                "SB##RISCV": 10,
            }
        else:
            ls_dict = {
                "LR.D##RISCV": 10,
                "LR.W##RISCV": 10,
                "LUI##RISCV": 10,
                "LW##RISCV": 10,
                "LWU##RISCV": 10,
                "LB##RISCV": 10,
                "LBU##RISCV": 10,
                "LH##RISCV": 10,
                "LHU##RISCV": 10,
                "LD##RISCV": 10,
                "SC.D##RISCV": 10,
                "SC.W##RISCV": 10,
                "SW##RISCV": 10,
                "SD##RISCV": 10,
                "SH##RISCV": 10,
                "SB##RISCV": 10,
            }

        for i in range(self.instr_num):
            instr_chosen = self.pickWeighted(ls_dict)
            self.genInstruction(instr_chosen, {"LSTarget": "0xd0000000-0xe0000000"})
        cur_pc = self.getPEstate("PC")
        self.genInstruction("JAL##RISCV", {"BRTarget": cur_pc})  # branch to itself


class MainSequence(Sequence):
    def generate(self, **kargs):

        # set choices modifier shared by both taken and not-taken paths
        choice_mod = ChoicesModifier(self.genThread)
        choice_mod.modifyOperandChoices(
            "GPRs", {"x0": 10000, "x2": 0}
        )  # X2 should be stack pointer
        mod_id = choice_mod.commitSet()

        id_arithmetic = self.setBntHook(
            Seq="MyBntSequence", Func="BntSequence_IntegerArithmetic"
        )  # set the bnt function for the following branch instructions
        self.genInstruction(
            "BGE##RISCV", {"SpeculativeBnt": True}
        )  # some conditional branch instruction
        self.genMetaInstruction("ADDI##RISCV", {"simm12": 0x100})  # instructions on taken path

        # revert the modifier
        choice_mod.revert(mod_id)

        id_l = self.setBntHook(
            Func="BntSequence_L"
        )  # set the bnt function for the following branch instructions
        self.genInstruction("JALR##RISCV", {"SpeculativeBnt": True})
        self.genMetaInstruction("ADDI##RISCV")  # instructions on the taken path

        self.revertBntHook(id_l)  # revert to the last setting: L-->Arithmetic


# Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

# Using the GenThreadRISCV by default, can be overwritten
GenThreadClass = GenThreadRISCV

# Using EnvRISCV by default
EnvClass = EnvRISCV
