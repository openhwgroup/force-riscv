#!/usr/bin/env bash
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
PAUSE='read -sn1 -p'

while getopts "y" opt; do
    case "$opt" in
    y)  PAUSE='read -t5 -sn1 -p'
        ;;
    esac
done

make clean
rm -rf standalone
git clone https://github.com/riscv/riscv-isa-sim standalone
cd standalone
git checkout 61f0dab33f7e529cc709908840311a8a7dcb23ce

echo "Preparing to remove DTC dependencies"

sed -i  '/^if test x"$DTC" == xno; then :/,/fi/ s/^/# /'  ./configure
cat ./configure | grep -B10 -A7 'if test x"$DTC" == xno; then :'
$PAUSE "Please review edit(s) then press Enter to continue"; echo

sed -i  '/^  if (dtb_pid == 0)/,/^  }/ s/^/\/\/ /'             ./riscv/dts.cc
sed -i  '/^  waitpid(dts_pid, &status, 0);/,/^  }/ s/^/\/\/ /' ./riscv/dts.cc
sed -i  '/^  waitpid(dtb_pid, &status, 0);/,/^  }/ s/^/\/\/ /' ./riscv/dts.cc
cat ./riscv/dts.cc | grep -B9 -A10 -e"if (dtb_pid == 0) {" -e"waitpid(dts_pid, &status, 0);" -e"waitpid(dtb_pid, &status, 0);"
$PAUSE "Please review edit(s) then press Enter to continue"; echo

./configure
make -j
cd ..

# NOTE: so_build/cosim/inc contains the handcar_cosim_wrapper.h file.  Don't remove it.
rm -rf src spike_mod so_build/cosim/src bin

mkdir src
mkdir -p spike_mod/insns
mkdir -p so_build/cosim/src
mkdir bin
./create_handcar_files.bash  
./filesurgeon.py
make -j8
cp bin/handcar_cosim.so ../utils/handcar
