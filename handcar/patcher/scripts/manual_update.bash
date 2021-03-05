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

mkdir updating
diff3 -m --label loc --label old --label new ../spike_mod/config.h originals/config.h upstream/config.h > updating/config.h
diff3 -m --label loc --label old --label new ../spike_mod/decode.h originals/decode.h upstream/decode.h > updating/decode.h
diff3 -m --label loc --label old --label new ../spike_mod/devices.h originals/devices.h upstream/devices.h > updating/devices.h
diff3 -m --label loc --label old --label new ../spike_mod/disasm.cc originals/disasm.cc upstream/disasm.cc > updating/disasm.cc
diff3 -m --label loc --label old --label new ../spike_mod/disasm.h originals/disasm.h upstream/disasm.h > updating/disasm.h
diff3 -m --label loc --label old --label new ../spike_mod/execute.cc originals/execute.cc upstream/execute.cc > updating/execute.cc
diff3 -m --label loc --label old --label new ../spike_mod/mmu.cc originals/mmu.cc upstream/mmu.cc > updating/mmu.cc
diff3 -m --label loc --label old --label new ../spike_mod/mmu.h originals/mmu.h upstream/mmu.h > updating/mmu.h
diff3 -m --label loc --label old --label new ../spike_mod/mret.h originals/mret.h upstream/mret.h > updating/mret.h
diff3 -m --label loc --label old --label new ../spike_mod/primitives.h originals/primitives.h upstream/primitives.h > updating/primitives.h
diff3 -m --label loc --label old --label new ../spike_mod/processor.cc originals/processor.cc upstream/processor.cc > updating/processor.cc
diff3 -m --label loc --label old --label new ../spike_mod/processor.h originals/processor.h upstream/processor.h > updating/processor.h
diff3 -m --label loc --label old --label new ../spike_mod/regnames.cc originals/regnames.cc upstream/regnames.cc > updating/regnames.cc
diff3 -m --label loc --label old --label new ../spike_mod/simif.h originals/simif.h upstream/simif.h > updating/simif.h
diff3 -m --label loc --label old --label new ../spike_mod/simlib.cc originals/sim.cc upstream/sim.cc > updating/simlib.cc
diff3 -m --label loc --label old --label new ../spike_mod/simlib.h originals/sim.h upstream/sim.h > updating/simlib.h
diff3 -m --label loc --label old --label new ../spike_mod/specialize.h originals/specialize.h upstream/specialize.h > updating/specialize.h
diff3 -m --label loc --label old --label new ../so_build/cosim/src/handcar_cosim_wrapper.cc originals/spike.cc upstream/spike.cc > updating/spike.cc
diff3 -m --label loc --label old --label new ../spike_mod/sret.h originals/sret.h upstream/sret.h > updating/sret.h
