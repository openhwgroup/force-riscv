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
from base.InstructionMap import InstructionMap

"""

------------------------------------------------------------------------------
------------------------------------------------------------------------------
Groupings from ISA instruction listings

References...
  The RISC-V Instruction Set Manual, Volume I: Unprivileged ISA
  > Chapter 24 - RV32/64G Instruction Set Listings
  > Chapter 16.8 - RVC Instruction Set Listings - Tables 16.5-16.7
------------------------------------------------------------------------------

Extension       Component extensions and/or instructions
------------------------------------------------------------------------------
RV32I             completed     
RV64I             completed
RV32F             completed
RV64F             completed
RV32D             completed
RV64D             completed
RV32M             completed
RV64M             completed
RV32A             completed
RV64A             completed
Zicsr             completed
Zifencei          completed
RV_A              completed     =   RV32A + RV64A
RV_C              completed         Does not contain instructions exclusive to
                                    RV32C or RV128C.
RV_V              pending       =   ALU_V + LDST_V
RV_G              completed     =   RV32I + RV64I + RV32F + RV64F + RV32D +
                                    RV64D + RV32M + RV64M + RV32A + RV64A +
                                    Zicsr + Zifencei
                                    (More succinctly, RV64I plus the standard
                                    extensions: I, M, A, F, D, Zicsr, Zifencei)
Trap_Return       pending       =   {URET, SRET, MRET, WFI}
Fence             pending       =   {SFENCE.VMA, HFENCE.BVMA, HFENCE.GVMA}

RV_ALL_NoVector   pending       =   RV_G + RV_C + Trap_Return + Fence
RV_ALL            pending       =   ALL_RISCV_NoVector + RV_V



Additional groups focused on functional units
----------------------------------------------------------

Regular loads and stores
  LD_Int            completed
  LD_Float          completed
  ST_Int            completed
  ST_Float          completed
  LDST_Int          completed       =   LD_Int + ST_Int
  LDST_Float        completed       =   LD_Float + ST_Float
  LDST_IntFloat     completed       =   LDST_Int + LDST_Float
  LDST_IFC          completed       =   LDST_IntFloat + LDST_C
  LDST_All          partially completed, missing V       
                                    =   LDST_IntFloat + LDST_C + (LDST_V)
  LDST_Byte         completed
  LDST_Half         completed
  LDST_Word         completed
  LDST_Double       completed

Regular ALU Int
  ALU_Int32         completed       =   RV32I - loads - stores - branches - 
                                        umps
  ALU_Int64         completed       =   RV64I - loads - stores
  ALU_M             completed       =   RV32M + RV64M
  ALU_Int_All       completed       =   ALU_Int32 + ALU_Int64 + ALU_M +
                                        ALU_Int_C
  BranchJump        completed

Regular ALU Float
  ALU_Float_Single  completed       =   RV32S + RV64S - loads - stores - FCVT
  ALU_Float_Double  completed       =   RV32D + RV64D - loads - stores - FCVT
  FCVT              completed
  ALU_Float_All     completed       =   ALU_Float_Single + ALU_Float_Double 

C extension
  LD_C              completed
  ST_C              completed
  LDST_C            completed       =   LD_C + ST_C
  BranchJump_C      completed
  ALU_Int_C         completed

V extension
  LD_V              pending
  ST_V              pending
  LDST_V            pending         =   LD_V + ST_V
  ALU_V             pending

==============================================================================
"""


# Utility function used to combine multiple dictionaries into one
def merge(*args):
    result = {}
    for dict1 in args:
        result.update(dict1)
    return result


RV32I_instructions = {
    "ADD##RISCV": 10,
    "ADDI##RISCV": 10,
    "AND##RISCV": 10,
    "ANDI##RISCV": 10,
    "AUIPC##RISCV": 10,
    "BEQ##RISCV": 10,
    "BGE##RISCV": 10,
    "BGEU##RISCV": 10,
    "BLT##RISCV": 10,
    "BLTU##RISCV": 10,
    "BNE##RISCV": 10,
    "EBREAK##RISCV": 0,  # disabled for now - set weight to 0
    "ECALL##RISCV": 0,  # disabled for now - set weight to 0
    "FENCE##RISCV": 10,
    "JAL##RISCV": 10,
    "JALR##RISCV": 10,
    "LB##RISCV": 10,
    "LBU##RISCV": 10,
    "LH##RISCV": 10,
    "LHU##RISCV": 10,
    "LUI##RISCV": 10,
    "LW##RISCV": 10,
    "OR##RISCV": 10,
    "ORI##RISCV": 10,
    "SB##RISCV": 10,
    "SH##RISCV": 10,
    "SLL##RISCV": 10,
    "SLLI#RV32I#RISCV": 10,
    "SLT##RISCV": 10,
    "SLTI##RISCV": 10,
    "SLTIU##RISCV": 10,
    "SLTU##RISCV": 10,
    "SRA##RISCV": 10,
    "SRAI#RV32I#RISCV": 10,
    "SRL##RISCV": 10,
    "SRLI#RV32I#RISCV": 10,
    "SUB##RISCV": 10,
    "SW##RISCV": 10,
    "XOR##RISCV": 10,
    "XORI##RISCV": 10,
}

RV32I_map = InstructionMap("RV32I_instructions", RV32I_instructions)

RV64I_instructions = {
    "ADDIW##RISCV": 10,
    "ADDW##RISCV": 10,
    "LD##RISCV": 10,
    "LWU##RISCV": 10,
    "SD##RISCV": 10,
    "SLLI#RV64I#RISCV": 10,
    "SLLIW##RISCV": 10,
    "SLLW##RISCV": 10,
    "SRAI#RV64I#RISCV": 10,
    "SRAIW##RISCV": 10,
    "SRAW##RISCV": 10,
    "SRLI#RV64I#RISCV": 10,
    "SRLIW##RISCV": 10,
    "SRLW##RISCV": 10,
    "SUBW##RISCV": 10,
}

RV64I_map = InstructionMap("RV64I_instructions", RV64I_instructions)

RV32F_instructions = {
    "FADD.S#Single-precision#RISCV": 10,
    "FCLASS.S##RISCV": 10,
    "FCVT.S.W##RISCV": 10,
    "FCVT.S.WU##RISCV": 10,
    "FCVT.W.S##RISCV": 10,
    "FCVT.WU.S##RISCV": 10,
    "FDIV.S#Single-precision#RISCV": 10,
    "FEQ.S##RISCV": 10,
    "FLE.S##RISCV": 10,
    "FLT.S##RISCV": 10,
    "FLW##RISCV": 10,
    "FMADD.S#Single-precision#RISCV": 10,
    "FMAX.S##RISCV": 10,
    "FMIN.S##RISCV": 10,
    "FMSUB.S#Single-precision#RISCV": 10,
    "FMUL.S#Single-precision#RISCV": 10,
    "FMV.W.X##RISCV": 10,
    "FMV.X.W##RISCV": 10,
    "FNMADD.S#Single-precision#RISCV": 10,
    "FNMSUB.S#Single-precision#RISCV": 10,
    "FSGNJ.S##RISCV": 10,
    "FSGNJN.S##RISCV": 10,
    "FSGNJX.S##RISCV": 10,
    "FSQRT.S##RISCV": 10,
    "FSUB.S#Single-precision#RISCV": 10,
    "FSW##RISCV": 10,
}

RV32F_map = InstructionMap("RV32F_instructions", RV32F_instructions)

RV64F_instructions = {
    "FCVT.L.S##RISCV": 10,
    "FCVT.LU.S##RISCV": 10,
    "FCVT.S.L##RISCV": 10,
    "FCVT.S.LU##RISCV": 10,
}

RV64F_map = InstructionMap("RV64F_instructions", RV64F_instructions)

RV32D_instructions = {
    "FADD.D#Double-precision#RISCV": 10,
    "FCLASS.D##RISCV": 10,
    "FCVT.D.S##RISCV": 10,
    "FCVT.D.W##RISCV": 10,
    "FCVT.D.WU##RISCV": 10,
    "FCVT.S.D##RISCV": 10,
    "FCVT.W.D##RISCV": 10,
    "FCVT.WU.D##RISCV": 10,
    "FDIV.D#Double-precision#RISCV": 10,
    "FEQ.D##RISCV": 10,
    "FLD##RISCV": 10,
    "FLE.D##RISCV": 10,
    "FLT.D##RISCV": 10,
    "FMADD.D#Double-precision#RISCV": 10,
    "FMAX.D##RISCV": 10,
    "FMIN.D##RISCV": 10,
    "FMSUB.D#Double-precision#RISCV": 10,
    "FMUL.D#Double-precision#RISCV": 10,
    "FNMADD.D#Double-precision#RISCV": 10,
    "FNMSUB.D#Double-precision#RISCV": 10,
    "FSD##RISCV": 10,
    "FSGNJ.D##RISCV": 10,
    "FSGNJN.D##RISCV": 10,
    "FSGNJX.D##RISCV": 10,
    "FSQRT.D##RISCV": 10,
    "FSUB.D#Double-precision#RISCV": 10,
}

RV32D_map = InstructionMap("RV32D_instructions", RV32D_instructions)

RV64D_instructions = {
    "FCVT.D.L##RISCV": 10,
    "FCVT.D.LU##RISCV": 10,
    "FCVT.L.D##RISCV": 10,
    "FCVT.LU.D##RISCV": 10,
    "FMV.D.X##RISCV": 10,
    "FMV.X.D##RISCV": 10,
}

RV64D_map = InstructionMap("RV64D_instructions", RV64D_instructions)

ZFH32_instructions = {
    "FLH##RISCV": 10,
    "FSH##RISCV": 10,
    "FADD.H#Half-precision#RISCV": 10,
    "FSUB.H#Half-precision#RISCV": 10,
    "FMUL.H#Half-precision#RISCV": 10,
    "FDIV.H#Half-precision#RISCV": 10,
    "FMIN.H##RISCV": 10,
    "FMAX.H##RISCV": 10,
    "FSQRT.H##RISCV": 10,
    "FMADD.H#Half-precision#RISCV": 10,
    "FNMADD.H#Half-precision#RISCV": 10,
    "FMSUB.H#Half-precision#RISCV": 10,
    "FNMSUB.H#Half-precision#RISCV": 10,
    "FCVT.W.H##RISCV": 10,
    "FCVT.H.W##RISCV": 10,
    "FCVT.WU.H##RISCV": 10,
    "FCVT.H.WU##RISCV": 10,
    "FCVT.S.H##RISCV": 10,
    "FCVT.H.S##RISCV": 10,
    "FCVT.D.H##RISCV": 10,
    "FCVT.H.D##RISCV": 10,
    "FCVT.Q.H##RISCV": 10,
    "FCVT.H.Q##RISCV": 10,
    "FCLASS.H##RISCV": 10,
    "FEQ.H##RISCV": 10,
    "FLE.H##RISCV": 10,
    "FLT.H##RISCV": 10,
    "FMV.H.X##RISCV": 10,
    "FMV.X.H##RISCV": 10,
    "FSGNJ.H##RISCV": 10,
    "FSGNJN.H##RISCV": 10,
    "FSGNJX.H##RISCV": 10,
}

ZFH32_map = InstructionMap("ZFH32_instructions", ZFH32_instructions)

ZFH64_instructions = {
    "FCVT.L.H##RISCV": 10,
    "FCVT.H.L##RISCV": 10,
    "FCVT.LU.H##RISCV": 10,
    "FCVT.H.LU##RISCV": 10,
}

ZFH64_map = InstructionMap("ZFH64_instructions", ZFH64_instructions)

ZFH_instructions = merge(ZFH32_instructions, ZFH64_instructions)
ZFH_map = InstructionMap("ZFH_instructions", ZFH_instructions)

RV32M_instructions = {
    "DIV##RISCV": 10,
    "DIVU##RISCV": 10,
    "MUL##RISCV": 10,
    "MULH##RISCV": 10,
    "MULHSU##RISCV": 10,
    "MULHU##RISCV": 10,
    "REM##RISCV": 10,
    "REMU##RISCV": 10,
}

RV32M_map = InstructionMap("RV32M_instructions", RV32M_instructions)

RV64M_instructions = {
    "DIVUW##RISCV": 10,
    "DIVW##RISCV": 10,
    "MULW##RISCV": 10,
    "REMUW##RISCV": 10,
    "REMW##RISCV": 10,
}

RV64M_map = InstructionMap("RV64M_instructions", RV64M_instructions)

ALU_M_instructions = merge(RV32M_instructions, RV64M_instructions)
ALU_M_map = InstructionMap("ALU_M_instructions", ALU_M_instructions)

RV32A_instructions = {
    "AMOADD.W##RISCV": 10,
    "AMOAND.W##RISCV": 10,
    "AMOMAX.W##RISCV": 10,
    "AMOMAXU.W##RISCV": 10,
    "AMOMIN.W##RISCV": 10,
    "AMOMINU.W##RISCV": 10,
    "AMOOR.W##RISCV": 10,
    "AMOSWAP.W##RISCV": 10,
    "AMOXOR.W##RISCV": 10,
    "LR.W##RISCV": 10,
    "SC.W##RISCV": 10,
}

RV32A_map = InstructionMap("RV32A_instructions", RV32A_instructions)

RV64A_instructions = {
    "AMOADD.D##RISCV": 10,
    "AMOAND.D##RISCV": 10,
    "AMOMAX.D##RISCV": 10,
    "AMOMAXU.D##RISCV": 10,
    "AMOMIN.D##RISCV": 10,
    "AMOMINU.D##RISCV": 10,
    "AMOOR.D##RISCV": 10,
    "AMOSWAP.D##RISCV": 10,
    "AMOXOR.D##RISCV": 10,
    "LR.D##RISCV": 10,
    "SC.D##RISCV": 10,
}

RV64A_map = InstructionMap("RV64A_instructions", RV64A_instructions)

RV_A_instructions = merge(RV32A_instructions, RV64A_instructions)
RV_A_map = InstructionMap("RV_A_instructions", RV_A_instructions)

Zicsr_instructions = {
    "CSRRC#register#RISCV": 10,
    "CSRRCI#immediate#RISCV": 10,
    "CSRRS#register#RISCV": 10,
    "CSRRSI#immediate#RISCV": 10,
    "CSRRW#register#RISCV": 10,
    "CSRRWI#immediate#RISCV": 10,
}

Zicsr_map = InstructionMap("Zicsr_instructions", Zicsr_instructions)

Zifencei_instructions = {"FENCE.I##RISCV": 10}

Zifencei_map = InstructionMap("Zifencei_instructions", Zifencei_instructions)

RV_G_instructions = merge(
    RV32I_instructions,
    RV64I_instructions,
    RV32F_instructions,
    RV64F_instructions,
    RV32D_instructions,
    RV64D_instructions,
    RV32M_instructions,
    RV64M_instructions,
    RV32A_instructions,
    RV64A_instructions,
    #   Zicsr_instructions,
    Zifencei_instructions,
)

RV_G_map = InstructionMap("RV_G_instructions", RV_G_instructions)

RV32_G_instructions = merge(
    RV32I_instructions,
    RV32F_instructions,
    RV32D_instructions,
    RV32M_instructions,
    RV32A_instructions,
    #   Zicsr_instructions,
    Zifencei_instructions,
)

RV32_G_map = InstructionMap("RV32_G_instructions", RV32_G_instructions)

LD_Int_instructions = {
    "LB##RISCV": 10,
    "LBU##RISCV": 10,
    "LH##RISCV": 10,
    "LHU##RISCV": 10,
    "LW##RISCV": 10,
    "LD##RISCV": 10,
    "LWU##RISCV": 10,
}

LD_Int_map = InstructionMap("LD_Int_instructions", LD_Int_instructions)

LD_Int32_instructions = {
    "LB##RISCV": 10,
    "LBU##RISCV": 10,
    "LH##RISCV": 10,
    "LHU##RISCV": 10,
    "LW##RISCV": 10,
}

LD_Int32_map = InstructionMap("LD_Int32_instructions", LD_Int32_instructions)

ST_Int_instructions = {
    "SB##RISCV": 10,
    "SH##RISCV": 10,
    "SW##RISCV": 10,
    "SD##RISCV": 10,
}

ST_Int_map = InstructionMap("ST_Int_instructions", ST_Int_instructions)

ST_Int32_instructions = {"SB##RISCV": 10, "SH##RISCV": 10, "SW##RISCV": 10}

ST_Int32_map = InstructionMap("ST_Int32_instructions", ST_Int32_instructions)

LD_Float_instructions = {"FLW##RISCV": 10, "FLD##RISCV": 10}

LD_Float_map = InstructionMap("LD_Float_instructions", LD_Float_instructions)

ST_Float_instructions = {"FSW##RISCV": 10, "FSD##RISCV": 10}

ST_Float_map = InstructionMap("ST_Float_instructions", ST_Float_instructions)

LDST_Byte_instructions = {"SB##RISCV": 10, "LB##RISCV": 10, "LBU##RISCV": 10}

LDST_Byte_map = InstructionMap(
    "LDST_Byte_instructions", LDST_Byte_instructions
)

LDST_Half_instructions = {"SH##RISCV": 10, "LH##RISCV": 10, "LHU##RISCV": 10}

LDST_Half_map = InstructionMap(
    "LDST_Half_instructions", LDST_Half_instructions
)

LDST_Word_instructions = {
    "FLW##RISCV": 10,
    "FSW##RISCV": 10,
    "SW##RISCV": 10,
    "LW##RISCV": 10,
    "LWU##RISCV": 10,
    "C.LW##RISCV": 10,
    "C.SW##RISCV": 10,
    "C.LWSP##RISCV": 10,
    "C.SWSP##RISCV": 10,
}

LDST_Word_map = InstructionMap(
    "LDST_Word_instructions", LDST_Word_instructions
)

LDST_Double_instructions = {
    "FLD##RISCV": 10,
    "FSD##RISCV": 10,
    "SD##RISCV": 10,
    "LD##RISCV": 10,
    "C.FLD##RISCV": 10,
    "C.FSD##RISCV": 10,
    "C.LD##RISCV": 10,
    "C.SD##RISCV": 10,
    "C.FLDSP##RISCV": 10,
    "C.FSDSP##RISCV": 10,
    "C.LDSP##RISCV": 10,
    "C.SDSP##RISCV": 10,
}

LDST_Double_map = InstructionMap(
    "LDST_Double_instructions", LDST_Double_instructions
)

# Combine groups
LDST_Int_instructions = merge(LD_Int_instructions, ST_Int_instructions)
LDST_Int_map = InstructionMap("LDST_Int_instructions", LDST_Int_instructions)

LDST_Int32_instructions = merge(LD_Int32_instructions, ST_Int32_instructions)
LDST_Int32_map = InstructionMap(
    "LDST_Int32_instructions", LDST_Int32_instructions
)

LDST_Float_instructions = merge(LD_Float_instructions, ST_Float_instructions)
LDST_Float_map = InstructionMap(
    "LDST_Float_instructions", LDST_Float_instructions
)

LDST_IntFloat_instructions = merge(
    LDST_Float_instructions, LDST_Int_instructions
)
LDST_IntFloat_map = InstructionMap(
    "LDST_IntFloat_instructions", LDST_IntFloat_instructions
)

LDST32_IntFloat_instructions = merge(
    LDST_Float_instructions, LDST_Int32_instructions
)
LDST32_IntFloat_map = InstructionMap(
    "LDST_IntFloat_instructions", LDST32_IntFloat_instructions
)

LD_C_instructions = {
    "C.FLD##RISCV": 10,
    "C.LD##RISCV": 10,
    "C.LW##RISCV": 10,
    "C.FLDSP##RISCV": 10,
    "C.LDSP##RISCV": 10,
    "C.LWSP##RISCV": 10,
}

LD_C_map = InstructionMap("LD_C_instructions", LD_C_instructions)

LD_C32_instructions = {
    "C.FLD##RISCV": 10,
    "C.LW##RISCV": 10,
    "C.FLDSP##RISCV": 10,
    "C.LWSP##RISCV": 10,
}

LD_C32_map = InstructionMap("LD_C32_instructions", LD_C32_instructions)

ST_C_instructions = {
    "C.FSD##RISCV": 10,
    "C.SD##RISCV": 10,
    "C.SW##RISCV": 10,
    "C.FSDSP##RISCV": 10,
    "C.SDSP##RISCV": 10,
    "C.SWSP##RISCV": 10,
}

ST_C_map = InstructionMap("ST_C_instructions", ST_C_instructions)

ST_C32_instructions = {
    "C.FSD##RISCV": 10,
    "C.SW##RISCV": 10,
    "C.FSDSP##RISCV": 10,
    "C.SWSP##RISCV": 10,
}

ST_C32_map = InstructionMap("ST_C32_instructions", ST_C32_instructions)

LDST_C_instructions = merge(LD_C_instructions, ST_C_instructions)
LDST_C_map = InstructionMap("LDST_C_instructions", LDST_C_instructions)

LDST_C32_instructions = merge(LD_C32_instructions, ST_C32_instructions)
LDST_C32_map = InstructionMap("LDST_C32_instructions", LDST_C32_instructions)

# All LDST except vector
LDST_IFC_instructions = merge(LDST_IntFloat_instructions, LDST_C_instructions)
LDST_IFC_map = InstructionMap("LDST_IFC_instructions", LDST_IFC_instructions)

LDST32_IFC_instructions = merge(
    LDST32_IntFloat_instructions, LDST_C32_instructions
)
LDST32_IFC_map = InstructionMap(
    "LDST32_IFC_instructions", LDST32_IFC_instructions
)

# All LDST -   >>>> Needs V added <<<<<<
LDST_All_instructions = merge(LDST_IFC_instructions)
LDST_All_map = InstructionMap("LDST_All_instructions", LDST_All_instructions)

LDST32_All_instructions = merge(LDST32_IFC_instructions)
LDST32_All_map = InstructionMap(
    "LDST32_All_instructions", LDST32_All_instructions
)

BranchJump_instructions = {
    "BEQ##RISCV": 10,
    "BGE##RISCV": 10,
    "BGEU##RISCV": 10,
    "BLT##RISCV": 10,
    "BLTU##RISCV": 10,
    "BNE##RISCV": 10,
    "JAL##RISCV": 10,
    "JALR##RISCV": 10,
}

BranchJump_map = InstructionMap(
    "BranchJump_instructions", BranchJump_instructions
)

ALU_Int32_instructions = {
    "ADD##RISCV": 10,
    "ADDI##RISCV": 10,
    "AND##RISCV": 10,
    "ANDI##RISCV": 10,
    "AUIPC##RISCV": 10,
    "LUI##RISCV": 10,
    "OR##RISCV": 10,
    "ORI##RISCV": 10,
    "SLL##RISCV": 10,
    "SLLI#RV32I#RISCV": 10,
    "SLT##RISCV": 10,
    "SLTI##RISCV": 10,
    "SLTIU##RISCV": 10,
    "SLTU##RISCV": 10,
    "SRA##RISCV": 10,
    "SRAI#RV32I#RISCV": 10,
    "SRL##RISCV": 10,
    "SRLI#RV32I#RISCV": 10,
    "SUB##RISCV": 10,
    "XOR##RISCV": 10,
    "XORI##RISCV": 10,
}

ALU_Int32_map = InstructionMap(
    "ALU_Int32_instructions", ALU_Int32_instructions
)

ALU_Int64_instructions = {
    "ADDIW##RISCV": 10,
    "ADDW##RISCV": 10,
    "SLLI#RV64I#RISCV": 10,
    "SLLIW##RISCV": 10,
    "SLLW##RISCV": 10,
    "SRAI#RV64I#RISCV": 10,
    "SRAIW##RISCV": 10,
    "SRAW##RISCV": 10,
    "SRLI#RV64I#RISCV": 10,
    "SRLIW##RISCV": 10,
    "SRLW##RISCV": 10,
    "SUBW##RISCV": 10,
}

ALU_Int64_map = InstructionMap(
    "ALU_Int64_instructions", ALU_Int64_instructions
)

ALU_Float_Single_instructions = {
    "FADD.S#Single-precision#RISCV": 10,
    "FCLASS.S##RISCV": 10,
    "FDIV.S#Single-precision#RISCV": 10,
    "FEQ.S##RISCV": 10,
    "FLE.S##RISCV": 10,
    "FLT.S##RISCV": 10,
    "FMADD.S#Single-precision#RISCV": 10,
    "FMAX.S##RISCV": 10,
    "FMIN.S##RISCV": 10,
    "FMSUB.S#Single-precision#RISCV": 10,
    "FMUL.S#Single-precision#RISCV": 10,
    "FNMADD.S#Single-precision#RISCV": 10,
    "FNMSUB.S#Single-precision#RISCV": 10,
    "FSGNJ.S##RISCV": 10,
    "FSGNJN.S##RISCV": 10,
    "FSGNJX.S##RISCV": 10,
    "FSQRT.S##RISCV": 10,
    "FSUB.S#Single-precision#RISCV": 10,
}

ALU_Float_Single_map = InstructionMap(
    "ALU_Float_Single_instructions", ALU_Float_Single_instructions
)

ALU_Float_Double_instructions = {
    "FADD.D#Double-precision#RISCV": 10,
    "FCLASS.D##RISCV": 10,
    "FDIV.D#Double-precision#RISCV": 10,
    "FEQ.D##RISCV": 10,
    "FLE.D##RISCV": 10,
    "FLT.D##RISCV": 10,
    "FMADD.D#Double-precision#RISCV": 10,
    "FMAX.D##RISCV": 10,
    "FMIN.D##RISCV": 10,
    "FMSUB.D#Double-precision#RISCV": 10,
    "FMUL.D#Double-precision#RISCV": 10,
    "FNMADD.D#Double-precision#RISCV": 10,
    "FNMSUB.D#Double-precision#RISCV": 10,
    "FSGNJ.D##RISCV": 10,
    "FSGNJN.D##RISCV": 10,
    "FSGNJX.D##RISCV": 10,
    "FSQRT.D##RISCV": 10,
    "FSUB.D#Double-precision#RISCV": 10,
}

ALU_Float_Double_map = InstructionMap(
    "ALU_Float_Double_instructions", ALU_Float_Double_instructions
)

FCVT_instructions = {
    "FMV.W.X##RISCV": 10,
    "FMV.X.W##RISCV": 10,
    "FCVT.S.W##RISCV": 10,
    "FCVT.S.WU##RISCV": 10,
    "FCVT.W.S##RISCV": 10,
    "FCVT.WU.S##RISCV": 10,
    "FCVT.L.S##RISCV": 10,
    "FCVT.LU.S##RISCV": 10,
    "FCVT.S.L##RISCV": 10,
    "FCVT.S.LU##RISCV": 10,
    "FCVT.D.S##RISCV": 10,
    "FCVT.D.W##RISCV": 10,
    "FCVT.D.WU##RISCV": 10,
    "FCVT.S.D##RISCV": 10,
    "FCVT.W.D##RISCV": 10,
    "FCVT.WU.D##RISCV": 10,
    "FCVT.D.L##RISCV": 10,
    "FCVT.D.LU##RISCV": 10,
    "FCVT.L.D##RISCV": 10,
    "FCVT.LU.D##RISCV": 10,
    "FMV.D.X##RISCV": 10,
    "FMV.X.D##RISCV": 10,
}

FCVT_map = InstructionMap("FCVT_instructions", FCVT_instructions)

ALU_Float_All_instructions = merge(
    ALU_Float_Single_instructions, ALU_Float_Double_instructions
)
ALU_Float_All_map = InstructionMap(
    "ALU_Float_All_instructions", ALU_Float_All_instructions
)

BranchJump_C_instructions = {
    "C.BEQZ##RISCV": 10,
    "C.BNEZ##RISCV": 10,
    "C.EBREAK##RISCV": 10,
    "C.J##RISCV": 10,
    "C.JR##RISCV": 10,
}

BranchJump_C_map = InstructionMap(
    "BranchJump_C_instructions", BranchJump_C_instructions
)

ALU_Int_C_instructions = {
    "C.ADD##RISCV": 10,
    "C.ADDI##RISCV": 10,
    "C.ADDI16SP##RISCV": 10,
    "C.ADDI4SPN##RISCV": 10,
    "C.ADDIW##RISCV": 10,
    "C.ADDW##RISCV": 10,
    "C.AND##RISCV": 10,
    "C.ANDI##RISCV": 10,
    "C.LI##RISCV": 10,
    "C.LUI##RISCV": 10,
    "C.MV##RISCV": 10,
    "C.NOP##RISCV": 10,
    "C.OR##RISCV": 10,
    "C.SLLI##RISCV": 10,
    "C.SLLI64##RISCV": 10,
    "C.SRAI64##RISCV": 10,
    "C.SRLI64##RISCV": 10,
    "C.SUB##RISCV": 10,
    "C.SUBW##RISCV": 10,
    "C.XOR##RISCV": 10,
}

ALU_Int32_C_instructions = {
    "C.ADD##RISCV": 10,
    "C.ADDI##RISCV": 10,
    "C.ADDI16SP##RISCV": 10,
    "C.ADDI4SPN##RISCV": 10,
    "C.AND##RISCV": 10,
    "C.ANDI##RISCV": 10,
    "C.LI##RISCV": 10,
    "C.LUI##RISCV": 10,
    "C.MV##RISCV": 10,
    "C.NOP##RISCV": 10,
    "C.OR##RISCV": 10,
    "C.SLLI##RISCV": 10,
    "C.SUB##RISCV": 10,
    "C.XOR##RISCV": 10,
}

ALU_Int_C_map = InstructionMap(
    "ALU_Int_C_instructions", ALU_Int_C_instructions
)

RV_C_instructions = merge(
    ALU_Int_C_instructions, BranchJump_C_instructions, LDST_C_instructions
)

RV_C_map = {ALU_Int_C_map: 10, BranchJump_C_map: 10, LDST_C_map: 10}

ALU_Int_All_instructions = merge(
    ALU_Int32_instructions,
    ALU_Int64_instructions,
    ALU_M_instructions,
    ALU_Int_C_instructions,
)
ALU_Int_All_map = InstructionMap(
    "ALU_Int_All_instructions", ALU_Int_All_instructions
)

ALU_Int32_All_instructions = merge(
    ALU_Int32_instructions, RV32M_instructions, ALU_Int32_C_instructions
)
ALU_Int32_All_map = InstructionMap(
    "ALU_Int32_All_instructions", ALU_Int32_All_instructions
)
