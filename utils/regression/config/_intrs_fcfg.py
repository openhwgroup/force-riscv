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
# File: _def_fcfg.py
# Comment: default master_run configuration file
# Contributors: Howard Maler, Noah Sherrill, Amit Kumar, Jingliang(Leo) Wang

# need to source ./scripts/project.cshrc before running master run with rtl
# RTL command : "%s/verif/top/sim/%s/build/%s/%s" % (root, regr, cfg, exe)
master_config = {
    "rtl": {
        # These are meta_args for interrupt generations:
        "add_meta_args": "inject_rand_interrupt=on rand_int_type=1000 "
        "int_deassert_high=100 int_deassert_low=50 "
        "int_assert_high=200 int_assert_low=100",
        "filter": True,
    },
    "generate": {},
    "iss": {},
}

sequence_app_opts = [
    ("force", {}),
    ("rtl", {}),
    ("fruntoctrl", {}),
]

single_run_app_opts = [("compile", {})]
