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

make clean
git clone https://github.com/riscv/riscv-isa-sim standalone
cd standalone
git checkout 5d5ee23f574583145cd2093a1fdab677e313e1d2
./configure
make
cd ..
mkdir src
mkdir spike_mod
mkdir so_build/cosim/src
mkdir bin
./create_handcar_files.bash  
./filesurgeon.py
make -j8
cp bin/handcar_cosim.so ../utils/handcar
