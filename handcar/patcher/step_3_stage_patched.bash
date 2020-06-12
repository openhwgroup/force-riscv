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

cp patched/config.h ../spike_mod
cp patched/devices.h ../spike_mod
cp patched/disasm.h ../spike_mod
cp patched/mmu.cc ../spike_mod
cp patched/mret.h ../spike_mod      
cp patched/processor.cc ../spike_mod
cp patched/regnames.cc ../spike_mod
cp patched/simlib.h ../spike_mod  
cp patched/sret.h ../spike_mod
cp patched/decode.h ../spike_mod
cp patched/disasm.cc ../spike_mod
cp patched/execute.cc ../spike_mod
cp patched/mmu.h ../spike_mod 
cp patched/primitives.h ../spike_mod
cp patched/processor.h ../spike_mod
cp patched/simlib.cc ../spike_mod     
cp patched/simif.h ../spike_mod
cp patched/specialize.h ../spike_mod
cp patched/handcar* ../so_build/cosim/src
