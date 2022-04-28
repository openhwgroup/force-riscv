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
        "fname": "APIs/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "APIs/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "address_solving/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "loop/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "loop/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "branch/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "branch/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "exception_handlers/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/g_instructions/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/g_instructions/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/g_instructions_rv64/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/g_instructions_rv64/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/c_instructions/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/c_instructions/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/c_instructions_rv64/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/c_instructions_rv64/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/v_instructions/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/v_instructions/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/priv_instructions/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/priv_instructions/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/zfh_instructions/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "instructions/zfh_instructions_rv64/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "paging/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "paging/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "paging/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64_sv39.config"},
    },
    {
        "fname": "paging/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64_sv39.config"},
    },
    {
        "fname": "privilege_switch/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "privilege_switch/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "state_transition/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "vector/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "../../examples/riscv/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "../../examples/tutorial/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "bnt/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "register/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "register/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "multiprocessing/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "multiprocessing/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "thread_group/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "thread_group/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "rv64/_def_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
    {
        "fname": "rv64/_noiss_fctrl.py",
        "generator": {"--cfg": "config/riscv_rv64.config"},
    },
]
