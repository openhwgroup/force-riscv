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
new_registers = [{'target':'system',
                  'register':'vtype',
                  'size':64,
                  'physical_register':'vtype',
                  'index':'0xc21',
                  'fields':[{'field':'VILL','shift':63,'size':1},
                            {'field':'RESERVED (WRITE 0)','shift':8,'size':55},
                            {'field':'VMA','shift':7,'size':1},
                            {'field':'VTA','shift':6,'size':1},
                            {'field':'VLMUL','shift':5,'size':1},
                            {'field':'VSEW','shift':2,'size':3},
                            {'field':'VLMUL','shift':0,'size':2}],
                  'choice':{'name':'vtype','value':'0xc21','weight':'0','description':'URO; Vector data type register.'}},
                 {'target':'system',
                  'register':'vstart',
                  'size':64,
                  'physical_register':'vstart',
                  'index':'0x8',
                  'fields':[{'field':'VSTART','shift':0,'size':64}],
                  'choice':{'name':'vstart','value':'0x8','weight':'0','description':'URW; Vector start position.'}},
                 {'target':'system',
                  'register':'vxsat',
                  'size':64,
                  'physical_register':'vxsat',
                  'index':'0x9',
                  'fields':[{'field':'RESERVED','shift':1,'size':63},
                            {'field':'VXSAT','shift':0,'size':1}],
                  'choice':{'name':'vxsat','value':'0x9','weight':'0','description':'URW; Fixed-point Saturate Flag.'}},
                 {'target':'system',
                  'register':'vxrm',
                  'size':64,
                  'physical_register':'vxrm',
                  'index':'0xa',
                  'fields':[{'field':'RESERVED (WRITE 0)','shift':2,'size':62},
                            {'field':'VXRM','shift':0,'size':2}],
                  'choice':{'name':'vxrm','value':'0xa','weight':'0','description':'URW; Fixed-point Rounding Mode.'}},
                 {'target':'system',
                  'register':'vcsr',
                  'size':64,
                  'physical_register':'vcsr',
                  'index':'0xf',
                  'fields':[{'field':'RESERVED','shift':3,'size':61},
                            {'field':'VXRM','shift':1,'size':2},
                            {'field':'VXSAT','shift':0,'size':1}],
                  'choice':{'name':'vcsr','value':'0xf','weight':'0','description':'URW; Vector control and status register.'}},
                 {'target':'system',
                  'register':'vl',
                  'size':64,
                  'physical_register':'vl',
                  'index':'0xc20',
                  'fields':[{'field':'VL','shift':0,'size':64}],
                  'choice':{'name':'vl','value':'0xc20','weight':'0','description':'URO; Vector length.'}},
                 {'target':'system',
                  'register':'vlenb',
                  'size':64,
                  'physical_register':'vlenb',
                  'index':'0xc22',
                  # there should probably be 2 fields here instead of just 1 (vlenb with shift 0, size 61 and a reserved field with shift 61, size 3), but the spec doesn't explicitly specify that
                  'fields':[{'field':'VLENB','shift':0,'size':64}],
                  'choice':{'name':'vlenb','value':'0xc22','weight':'0','description':'URO; VLEN/8 (vector register length in bytes).'}}]
