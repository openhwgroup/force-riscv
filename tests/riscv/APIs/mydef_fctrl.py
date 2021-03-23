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
    {
        "fname": "api_queryExceptionVectorBaseAddress_force.py",
        "options": {"max-instr": 50000},
        "generator": {"--cfg": "config/riscv.config", "--max-instr": 50000},
    },
    {
        "fname": "api_genVA_02_force.py",
        "options": {"max-instr": 50000},
        "generator": {"--cfg": "config/riscv.config", "--max-instr": 50000},
    },
    {
        "fname": "api_genVA_02_force.py",
        "options": {"max-instr": 50000},
        "generator": {
            "--cfg": "config/riscv.config",
            "--max-instr": 50000,
            "--options": '"PrivilegeLevel=1"',
        },
    },
    {
        "fname": "api_genPA_01_force.py",
        "options": {"max-instr": 50000},
        "generator": {
            "--cfg": "config/riscv.config",
            "--max-instr": 50000,
            "--options": '"PrivilegeLevel=1"',
        },
    },
    {
        "fname": "api_genVAforPA_01_force.py",
        "options": {"max-instr": 50000},
        "generator": {
            "--cfg": "config/riscv.config",
            "--max-instr": 50000,
            "--options": '"PrivilegeLevel=1"',
        },
    },
    {
        "fname": "api_getPageInfo_01_force.py",
        "options": {"max-instr": 50000},
        "generator": {
            "--cfg": "config/riscv.config",
            "--max-instr": 50000,
            "--options": '"PrivilegeLevel=1"',
        },
    },
    {
        "fname": "api_verifyVirtualAddress_01_force.py",
        "options": {"max-instr": 50000},
        "generator": {
            "--cfg": "config/riscv.config",
            "--max-instr": 50000,
            "--options": '"PrivilegeLevel=1"',
        },
    },
    {
        "fname": "api_genFreePagesRange_01_force.py",
        "options": {"max-instr": 50000},
        "generator": {
            "--cfg": "config/riscv.config",
            "--max-instr": 50000,
            "--options": '"PrivilegeLevel=1"',
        },
    },
    {
        "fname": "Constraint_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "LoadImmediate_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {"fname": "State_force.py", "generator": {"--cfg": "config/riscv.config"}},
    {
        "fname": "WriteRegisterTest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "GenData_test_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "GetRandomRegisterTest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "InitializeRandomRandomlyTest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "reg_dependence_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "AccessReservedRegisterTest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "BitstreamTest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "ReserveRegisterTest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "RandomChoiceAPITest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "QueryResourceEntropyTest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "LoadRegisterPreambleTest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "PickWeightedValue_test_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "InquiryAPITest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "ChoicesModificationTest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "CustomEntryPointTest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
    {
        "fname": "LoopControlTest_force.py",
        "generator": {"--cfg": "config/riscv.config"},
    },
]
