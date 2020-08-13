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
control_items = [ {"fname":"APIs/_def_fctrl.py",     "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"APIs/_noiss_fctrl.py",   "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"address_solving/_def_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"loop/_def_fctrl.py",     "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"loop/_noiss_fctrl.py",   "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"branch/_def_fctrl.py",   "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"branch/_noiss_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"exception_handlers/_def_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"instructions/riscv_instructions/_def_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"instructions/riscv_instructions/_noiss_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  #{"fname":"instructions/c_instructions/_def_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"instructions/c_instructions/_noiss_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"instructions/v_instructions/_def_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"instructions/v_instructions/_noiss_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"paging/_def_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"paging/_noiss_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"privilege_switch/_def_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"privilege_switch/_noiss_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"state_transition/_def_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"vector/_def_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                  {"fname":"../../examples/riscv/_def_fctrl.py", "generator" : {"--cfg" : "config/riscv.config", }},
                ]
