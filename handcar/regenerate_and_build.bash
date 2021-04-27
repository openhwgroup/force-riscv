#!/usr/bin/env bash
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

# SCRIPT DEFAULTS
unset NO_GIT
pause() {
    echo
    echo "$1"
    echo
}

function usage {
    echo "Usage: $(basename "$0") [-in]"
    echo "    -i  interactive mode"
    echo "    -n  no-git mode"
    echo ""
    exit 1
}

while getopts ":in" opt; do
    case "$opt" in
    i)  # interactive requested, so redefine 'pause'
        echo "===== option 'i' detected"
        pause() {
            echo
            read -r -sn1 -p "$1 -- Press Enter to continue or Ctrl-C to quit"
            echo
        }
        ;;
    n)  # no git
        echo "===== option 'n' detected"
        NO_GIT=1
        ;;
    ?)  # usage
        echo "Invalid option: -${OPTARG}"
        echo
        usage
        ;;
    esac
done

make clean

if [ -z "${NO_GIT}" ]; then
    pause "Preparing to clone spike"
    rm -rf standalone
    git clone https://github.com/riscv/riscv-isa-sim standalone
    cd standalone || exit 3
    git checkout 61f0dab33f7e529cc709908840311a8a7dcb23ce
else
    echo
    echo ===== NO-GIT requested =====
    echo "You are responsible for ensuring that handcar/standalone contains"
    echo "a clone of https://github.com/riscv/riscv-isa-sim, and a checkout"
    echo "of hash 61f0dab33f7e529cc709908840311a8a7dcb23ce"
    echo
    cd standalone || exit 3
fi

echo "===== Preparing to remove DTC dependencies"

# shellcheck disable=SC2016
sed -i  '/^if test x"$DTC" == xno; then :/,/fi/ s/^/# /'  ./configure
echo
echo "vvvvv Begin auto-edit output vvvvv"

# shellcheck disable=SC2016,SC2002
cat ./configure | grep -B10 -A7 'if test x"$DTC" == xno; then :'
echo "^^^^^ End auto-edit output ^^^^^"
pause "===== Please review edit(s) above"

sed -i  '/^  if (dtb_pid == 0)/,/^  }/ s/^/\/\/ /'             ./riscv/dts.cc
sed -i  '/^  waitpid(dts_pid, &status, 0);/,/^  }/ s/^/\/\/ /' ./riscv/dts.cc
sed -i  '/^  waitpid(dtb_pid, &status, 0);/,/^  }/ s/^/\/\/ /' ./riscv/dts.cc
echo "vvvvv Begin auto-edit output vvvvv"
# shellcheck disable=SC2002
cat ./riscv/dts.cc | grep -B9 -A10 -e"if (dtb_pid == 0) {" -e"waitpid(dts_pid, &status, 0);" -e"waitpid(dtb_pid, &status, 0);"
echo "^^^^^ End auto-edit output ^^^^^"
pause "Please review edit(s) above"

pause "===== Preparing to configure"
./configure

# NOTE: Compile runs twice due to failure from this flag, so disabling it
echo "===== Preparing to edit makefile"
sed -i  '/^default-CFLAGS/ s/$/ -fno-var-tracking-assignments/'  Makefile
echo
echo "vvvvv Begin auto-edit output vvvvv"
# shellcheck disable=SC2002
cat Makefile | grep -B10 -A7 "^default-CFLAGS"
echo "^^^^^ End auto-edit output ^^^^^"
pause "===== Please review edit(s) above"

echo "===== Preparing to run makefile for standalone/"
make -j
cd ..

rm -rf  bin inc src so_build spike_mod

mkdir bin
mkdir inc
mkdir src
mkdir -p so_build/cosim/src
mkdir -p spike_mod/insns

cp -r ./headers/* .
cp ../3rd_party/inc/softfloat/softfloat.h ./inc/.

pause "===== Preparing to create handcar files"
cd ./patcher || exit 1
./patcher.py patch --clean
cd ..

pause "===== Preparing to run filesurgeon"
./filesurgeon.py

pause "===== Preparing to run handcar make"
make -j

pause "===== Preparing to copy handcar_cosim.so"
rsync -c ./bin/handcar_cosim.so ../utils/handcar/handcar_cosim.so
echo $?