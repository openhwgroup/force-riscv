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
# File: _def_fcfg.py
# Comment: default master_run configuration file
# Contributors: Howard Maler, Noah Sherrill, Amit Kumar, Jingliang(Leo) Wang

# need to source ./scripts/project.cshrc before running master run with rtl
# RTL command : "%s/verif/top/sim/%s/build/%s/%s" % (root, regr, cfg, exe)
master_config = {
    'rtl': {
        #                         , "regr": "logs"         # override the default regr setting, defaults to logs, optional, can be changed using ENV(RTL_REGR) as well
        #                         , "cfg" : "target_config"          # override the default configuration name, defaults to target_config, optional, can be changed using ENV(RTL_CFG) as well
        #                         , "exe": "uvm_simv_opt"          # override the default executable name, by default it is "uvm_simv_opt", can be changed using ENV(RTL_EXE) as well
        #                         , "meta_args":"" # override the default meta_args, by default set to "tc_mode=force railhouse=on prefetch_drop_int=10 no_commit_cycles=6000"
        "add_meta_args":"inject_rand_interrupt=on rand_int_type=1 int_deassert_high=100 int_deassert_low=50 int_assert_high=200 int_assert_low=100", # Add more meta args to default meta args
        "filter":True, 
    },
    'generate': {},
    'iss': {}
}

sequence_app_opts = [("force", {}),
        ("rtl", {}),
        ("fruntoctrl", {}),
        ]

single_run_app_opts = [("compile", {})]

