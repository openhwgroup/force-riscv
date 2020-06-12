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

rm ./patched/*

patch originals/config.h -i patches/config.h.patch -o patched/config.h 
patch originals/devices.h -i patches/devices.h.patch -o patched/devices.h 
patch originals/disasm.h -i patches/disasm.h.patch -o patched/disasm.h 
patch originals/mmu.cc -i patches/mmu.cc.patch -o patched/mmu.cc 
patch originals/mret.h -i patches/mret.h.patch -o patched/mret.h       
patch originals/processor.cc -i patches/processor.cc.patch -o patched/processor.cc 
patch originals/regnames.cc -i patches/regnames.cc.patch -o patched/regnames.cc 
patch originals/sim.h -i patches/sim.h.patch -o patched/simlib.h   
patch originals/sret.h -i patches/sret.h.patch -o patched/sret.h 
patch originals/decode.h -i patches/decode.h.patch -o patched/decode.h 
patch originals/disasm.cc -i patches/disasm.cc.patch -o patched/disasm.cc 
patch originals/execute.cc -i patches/execute.cc.patch -o patched/execute.cc 
patch originals/mmu.h -i patches/mmu.h.patch -o patched/mmu.h  
patch originals/primitives.h -i patches/primitives.h.patch -o patched/primitives.h 
patch originals/processor.h -i patches/processor.h.patch -o patched/processor.h 
patch originals/sim.cc -i patches/sim.cc.patch -o patched/simlib.cc      
patch originals/simif.h -i patches/simif.h.patch -o patched/simif.h 
patch originals/specialize.h -i patches/specialize.h.patch -o patched/specialize.h 
patch originals/spike.cc -i patches/spike.cc.patch -o patched/handcar_cosim_wrapper.cc
