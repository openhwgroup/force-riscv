#!/bin/bash
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

#export run_dir= pwd
#export base_dir=$(echo $PWD | sed 's/\(.*force-riscv\).*/\1\//')
#
#echo $run_dir
#echo $base_dir
#

../utils/regression/master_run.py -f ../examples/riscv/um_examples_fctrl.py 
sleep 5
../utils/regression/master_run.py -f ../examples/riscv/um_examples_fctrl.py --msg-lev=all
sleep 5
###FAILS##../utils/regression/master_run.py -f ../examples/riscv/um_examples_fctrl.py --client-lev=all
###sleep 5
../utils/regression/master_run.py -f ../examples/riscv/um_examples_fctrl.py -t 60 --max-instr=2000 --min-instr=50 --max-fails=5 --num-cores=1 --num-threads=1 --num-chips=1 --keep all
sleep 5
../utils/regression/master_run.py -f ../examples/riscv/um_examples_fctrl.py -t 60 --no-sim --max-instr=2000 --min-instr=50 --no-sim --max-fails=5 --num-cores=1 --num-threads=1 --num-chips=1 --keep all
sleep 5
# Set options that can be read in the test template with self.getOption() -- currently debugging
###fails##../utils/regression/master_run.py -f ../tests/riscv/masterRun/_def2_fctrl.py --generator={"--options":"\"loopCount=50\""
../utils/regression/master_run.py -f ../tests/riscv/masterRun/debug_fctrl.py -k all


