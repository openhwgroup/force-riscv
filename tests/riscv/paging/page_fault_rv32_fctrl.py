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
control_items = [
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_DA,PageFaultLevel=0"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_DA,PageFaultLevel=1"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_DA"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_U,PageFaultLevel=0"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_U,PageFaultLevel=1"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_U"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_X,PageFaultLevel=0"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_X,PageFaultLevel=1"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_X"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_WR,PageFaultLevel=0"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_WR,PageFaultLevel=1"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_WR"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_V,PageFaultLevel=0"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 20000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_V,PageFaultLevel=1"',
        },
    },
    {
        "fname": "page_fault_on_load_store_03_force.py",
        "options": {"max-instr": 50000},
        "generator": {
            "--options": '"PrivilegeLevel=1,IterCount=50,PageFaultType=Invalid_V"',
        },
    },
]
