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
control_items = [
                {"fname":"um_itree_01_force.py",                  "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_itree_02_force.py",                  "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_itree_03_force.py",                  "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_itree_04_force.py",                  "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_pageCrossing_01_force.py",           "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_pageCrossing_02_force.py",           "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_sequences_01_force.py",              "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_seqLibrary_01_force.py",             "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_regDependency_01_force.py",          "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_regDependency_02_force.py",          "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_choiceMod_01_force.py",              "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_choiceMod_02_force.py",              "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_EntryPoint_01_force.py",             "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_randomChoice_01_force.py",           "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_randomChoice_02_force.py",           "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"um_LoopControl_01_force.py",            "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"RVC_misaligned_force.py",               "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                ##FAILS#Needs updated system_register.xml file#{"fname":"um_SystemRegs_01_force.py",             "options":{"max-instr":50000,}, "generator" : {"--cfg" : "config/riscv.config", "--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                ]
