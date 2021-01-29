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

control_items = [ {"fname":"test_muldiv_nan_boxing_force.py",},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=H,data_type=FP,size=16\"",}},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=H,data_type=INT,size=16\"",}},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=S,data_type=FP,size=32\"",}},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=S,data_type=INT,size=32\"",}},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=S,data_type=FP,size=16\"",}},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=S,data_type=INT,size=16\"",}},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=D,data_type=FP,size=64\"",}},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=D,data_type=INT,size=64\"",}},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=D,data_type=FP,size=32\"",}},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=D,data_type=INT,size=32\"",}},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=D,data_type=FP,size=16\"",}},
                  {"fname":"test_muldiv_data_constr_force.py", "generator":{"--options":"\"precision=D,data_type=INT,size=16\"",}},
                ]
