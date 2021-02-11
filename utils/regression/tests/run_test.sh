#!/bin/sh
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

echo .
echo .
cd utils/regression || exit
echo .
echo .
# run the directory tests
echo Start Default Regression Run
./master_run.py 
find ./output/regression -type d | grep -E "[0-9]{5}" | wc
echo Finished Regression Run
echo .
echo .
echo Start Default Performance Run
./master_run.py -m perf 
find ./output/performance -type d | grep -E "[0-9]{5}" | wc
echo Finished Performance Run

echo .
echo .

