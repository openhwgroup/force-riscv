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
                {"fname":"api_genVA_01_force.py",                      "options":{"max-instr":50000,}, "generator" : {"--max-instr" : 50000}},
                {"fname":"api_genVA_02_force.py",                      "options":{"max-instr":50000,}, "generator" : {"--max-instr" : 50000}},
                #fails#{"fname":"api_genVA_01_force.py",                      "options":{"max-instr":50000,}, "generator" : {"--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"api_genVA_02_force.py",                      "options":{"max-instr":50000,}, "generator" : {"--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"api_genVA_reuse_force.py",                   "options":{"max-instr":50000,}, "generator" : {"--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"api_genPA_01_force.py",                      "options":{"max-instr":50000,}, "generator" : {"--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"api_genVAforPA_01_force.py",                 "options":{"max-instr":50000,}, "generator" : {"--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"api_getPageInfo_01_force.py",                "options":{"max-instr":50000,}, "generator" : {"--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"api_verifyVirtualAddress_01_force.py",       "options":{"max-instr":50000,}, "generator" : {"--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"api_genFreePagesRange_01_force.py",          "options":{"max-instr":50000,}, "generator" : {"--max-instr" : 50000, "--options":"\"PrivilegeLevel=1\""}},
                {"fname":"skip_boot_force.py",                         "generator" : {"--options":"\"SkipBootCode=1\""}},
                {"fname":"Constraint_force.py",                        },
                {"fname":"LoadImmediate_force.py",                     },
                {"fname":"State_force.py",                             },
                {"fname":"WriteRegisterTest_force.py",                 },
                {"fname":"GenData_test_force.py",                      },
                {"fname":"GetRandomRegisterTest_force.py",             },
                {"fname":"InitializeRandomRandomlyTest_force.py",      },
                {"fname":"reg_dependence_force.py",                    },
                {"fname":"AccessReservedRegisterTest_force.py",        },
                {"fname":"BitstreamTest_force.py",                     },
                {"fname":"ReserveRegisterTest_force.py",               },
                {"fname":"RandomChoiceAPITest_force.py",               },
                {"fname":"QueryResourceEntropyTest_force.py",          },
                {"fname":"LoadRegisterPreambleTest_force.py",          },
                {"fname":"PickWeightedValue_test_force.py",            },
                {"fname":"InquiryAPITest_force.py",                    },
                {"fname":"ChoicesModificationTest_force.py",           },
                {"fname":"CustomEntryPointTest_force.py",              },
                {"fname":"LoopControlTest_force.py",                   },
                {"fname":"InitializeRegisterTest_force.py",            },
                {"fname":"SetMisaInitialValue_force.py",               },
                #{"fname":"*_force.py", "options":{"max-instr":50000,}, "generator":{"--max-instr":50000,} },
                ]
