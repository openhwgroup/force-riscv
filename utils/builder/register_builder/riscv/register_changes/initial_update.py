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
                  'register':'fcsr',
                  'size':64,
                  'physical_register':'fcsr',
                  'index':'0x3',
                  'fields':[{'field':'WPRI_VAR','shift':8,'size':56},
                            {'field':'FRM','shift':5,'size':3},
                            {'field':'NZ','shift':4,'size':1},
                            {'field':'DZ','shift':3,'size':1},
                            {'field':'OF','shift':2,'size':1},
                            {'field':'UF','shift':1,'size':1},
                            {'field':'NX','shift':0,'size':1}],
                  'choice':{'name':'fcsr','value':'0x3','weight':'0','description':'URW; Floating-point control and status register.'}}]

changed_registers = [{'target':'system',
                      'register':'fflags',
                      'size':64,
                      'physical_register':'fflags',
                      'index':'0x1',
                      'fields':[{'field':'WPRI_VAR','shift':5,'size':59},
                                {'field':'NZ','shift':4,'size':1},
                                {'field':'DZ','shift':3,'size':1},
                                {'field':'OF','shift':2,'size':1},
                                {'field':'UF','shift':1,'size':1},
                                {'field':'NX','shift':0,'size':1}]},

                     {'target':'system',
                      'register':'frm',
                      'size':64,
                      'physical_register':'frm',
                      'index':'0x2',
                      'fields':[{'field':'WPRI_VAR','shift':8,'size':56},
                                {'field':'WPRI_VAR','shift':0,'size':5},
                                {'field':'FRM','shift':5,'size':3}]},

                     {'target':'system',
                      'register':'mscratch',
                      'size':64,
                      'physical_register':'mscratch',
                      'index':'0x340',
                      'fields':[{'field':'MSCRATCH', 'shift':0, 'size':64}]}]

delete_register_choices = [{'name':'mstatus_hyp'}]
