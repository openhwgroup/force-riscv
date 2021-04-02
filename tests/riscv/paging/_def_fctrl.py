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
  { "fname":"paging_force.py",                   
    "options": {'max-instr': 10000}, 
    "generator":{"--options":"\"PrivilegeLevel=1\"",}},
  { "fname":"paging_loadstore_force.py",         
    "options": {'max-instr': 10000}, 
    "generator":{"--options":"\"PrivilegeLevel=1\"",}},
  { "fname":"paging_force.py",                   
    "options": {'max-instr': 10000}, 
    "generator":{"--options":"\"PrivilegeLevel=1,FlatMap=1\"",}},
  { "fname":"paging_loadstore_force.py",         
    "options": {'max-instr': 10000}, 
    "generator":{"--options":"\"PrivilegeLevel=1,FlatMap=1\"",}},
  { "fname":"page_fault_on_load_store_force.py", 
    "options": {'max-instr': 10000}, 
   	"generator":{"--options":"\"PrivilegeLevel=1\"",}},
  { "fname":"page_fault_on_branch_force.py",     
    "options": {'max-instr': 10000}, 
   	"generator":{"--options":"\"PrivilegeLevel=1\"",}},
  { "fname":"page_fault_on_load_store_force.py", 
    "options": {'max-instr': 10000}, 
   	"generator":{"--options":"\"PrivilegeLevel=1,FlatMap=1\"",}},
  { "fname":"page_fault_on_branch_force.py",     
    "options": {'max-instr': 10000}, 
   	"generator":{"--options":"\"PrivilegeLevel=1,FlatMap=1\"",}},
  { "fname":"paging_force.py",                   
    "options": {'max-instr': 10000}, 
   	"generator":{"--options":"\"PrivilegeLevel=1,handlers_set=Fast\"",}},
  { "fname":"paging_loadstore_force.py",         
    "options": {'max-instr': 10000}, 
   	"generator":{"--options":"\"PrivilegeLevel=1,handlers_set=Fast\"",}},
  { "fname":"paging_force.py",                   
    "options": {'max-instr': 10000}, 
   	"generator":{"--options":"\"PrivilegeLevel=1,FlatMap=1,handlers_set=Fast\"",}},
  { "fname":"paging_loadstore_force.py",         
    "options": {'max-instr': 10000}, 
   	"generator":{"--options":"\"PrivilegeLevel=1,FlatMap=1,handlers_set=Fast\"",}},
  { "fname":"page_fault_on_load_store_force.py", 
    "options": {'max-instr': 10000}, 
   	"generator":{"--options":"\"PrivilegeLevel=1\"",}},
  { "fname":"page_fault_on_branch_force.py",     
    "options": {'max-instr': 10000}, 
	"generator":{"--options":"\"PrivilegeLevel=1,DelegateExceptions=0,handlers_set=Fast\"",}},
  { "fname":"page_fault_on_load_store_force.py", 
    "options": {'max-instr': 10000}, 
    "generator":{"--options":"\"PrivilegeLevel=1,DelegateExceptions=0,FlatMap=1,handlers_set=Fast\"",}},
  { "fname":"page_fault_on_branch_force.py",     
    "options": {'max-instr': 10000}, 
    "generator":{"--options":"\"PrivilegeLevel=1,DelegateExceptions=0,FlatMap=1,handlers_set=Fast\"",}},
  { "fname":"page_fault_fctrl.py",  }
      ]
