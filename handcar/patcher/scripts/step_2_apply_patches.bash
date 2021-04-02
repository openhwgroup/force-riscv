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

patch ./originals/config.h -i ./patches/config.h.patch -o ./patched/config.h
patch ./originals/devices.h -i ./patches/devices.h.patch -o ./patched/devices.h
patch ./originals/disasm.h -i ./patches/disasm.h.patch -o ./patched/disasm.h
patch ./originals/mmu.cc -i ./patches/mmu.cc.patch -o ./patched/mmu.cc
patch ./originals/mret.h -i ./patches/mret.h.patch -o ./patched/mret.h
patch ./originals/processor.cc -i ./patches/processor.cc.patch -o ./patched/processor.cc
patch ./originals/regnames.cc -i ./patches/regnames.cc.patch -o ./patched/regnames.cc
patch ./originals/sim.h -i ./patches/sim.h.patch -o ./patched/simlib.h
patch ./originals/sret.h -i ./patches/sret.h.patch -o ./patched/sret.h
patch ./originals/decode.h -i ./patches/decode.h.patch -o ./patched/decode.h
patch ./originals/disasm.cc -i ./patches/disasm.cc.patch -o ./patched/disasm.cc
patch ./originals/execute.cc -i ./patches/execute.cc.patch -o ./patched/execute.cc
patch ./originals/mmu.h -i ./patches/mmu.h.patch -o ./patched/mmu.h
patch ./originals/primitives.h -i ./patches/primitives.h.patch -o ./patched/primitives.h
patch ./originals/processor.h -i ./patches/processor.h.patch -o ./patched/processor.h
patch ./originals/sim.cc -i ./patches/sim.cc.patch -o ./patched/simlib.cc
patch ./originals/simif.h -i ./patches/simif.h.patch -o ./patched/simif.h
patch ./originals/specialize.h -i ./patches/specialize.h.patch -o ./patched/specialize.h
patch ./originals/spike.cc -i ./patches/spike.cc.patch -o ./patched/handcar_cosim_wrapper.cc

# instructions
patch ./originals/insns/vmsof_m.h -i ./patches/insns/vmsof_m.h.patch -o ./patched/insns/vmsof_m.h
patch ./originals/insns/vmadc_vim.h -i ./patches/insns/vmadc_vim.h.patch -o ./patched/insns/vmadc_vim.h
patch ./originals/insns/vfncvt_x_f_w.h -i ./patches/insns/vfncvt_x_f_w.h.patch -o ./patched/insns/vfncvt_x_f_w.h
patch ./originals/insns/vfwcvt_xu_f_v.h -i ./patches/insns/vfwcvt_xu_f_v.h.patch -o ./patched/insns/vfwcvt_xu_f_v.h
patch ./originals/insns/vrgather_vv.h -i ./patches/insns/vrgather_vv.h.patch -o ./patched/insns/vrgather_vv.h
patch ./originals/insns/vmsbc_vvm.h -i ./patches/insns/vmsbc_vvm.h.patch -o ./patched/insns/vmsbc_vvm.h
patch ./originals/insns/vsbc_vvm.h -i ./patches/insns/vsbc_vvm.h.patch -o ./patched/insns/vsbc_vvm.h
patch ./originals/insns/vrgather_vi.h -i ./patches/insns/vrgather_vi.h.patch -o ./patched/insns/vrgather_vi.h
patch ./originals/insns/vs1r_v.h -i ./patches/insns/vs1r_v.h.patch -o ./patched/insns/vs1r_v.h
patch ./originals/insns/vfcvt_f_xu_v.h -i ./patches/insns/vfcvt_f_xu_v.h.patch -o ./patched/insns/vfcvt_f_xu_v.h
patch ./originals/insns/vfwcvt_rtz_x_f_v.h -i ./patches/insns/vfwcvt_rtz_x_f_v.h.patch -o ./patched/insns/vfwcvt_rtz_x_f_v.h
patch ./originals/insns/vsbc_vxm.h -i ./patches/insns/vsbc_vxm.h.patch -o ./patched/insns/vsbc_vxm.h
patch ./originals/insns/vfmv_v_f.h -i ./patches/insns/vfmv_v_f.h.patch -o ./patched/insns/vfmv_v_f.h
patch ./originals/insns/vmv_s_x.h -i ./patches/insns/vmv_s_x.h.patch -o ./patched/insns/vmv_s_x.h
patch ./originals/insns/vadc_vvm.h -i ./patches/insns/vadc_vvm.h.patch -o ./patched/insns/vadc_vvm.h
patch ./originals/insns/vfmv_f_s.h -i ./patches/insns/vfmv_f_s.h.patch -o ./patched/insns/vfmv_f_s.h
patch ./originals/insns/vadc_vim.h -i ./patches/insns/vadc_vim.h.patch -o ./patched/insns/vadc_vim.h
patch ./originals/insns/vl1r_v.h -i ./patches/insns/vl1r_v.h.patch -o ./patched/insns/vl1r_v.h
patch ./originals/insns/vfcvt_rtz_xu_f_v.h -i ./patches/insns/vfcvt_rtz_xu_f_v.h.patch -o ./patched/insns/vfcvt_rtz_xu_f_v.h
patch ./originals/insns/vfslide1up_vf.h -i ./patches/insns/vfslide1up_vf.h.patch -o ./patched/insns/vfslide1up_vf.h
patch ./originals/insns/vfirst_m.h -i ./patches/insns/vfirst_m.h.patch -o ./patched/insns/vfirst_m.h
patch ./originals/insns/vfcvt_rtz_x_f_v.h -i ./patches/insns/vfcvt_rtz_x_f_v.h.patch -o ./patched/insns/vfcvt_rtz_x_f_v.h
patch ./originals/insns/vpopc_m.h -i ./patches/insns/vpopc_m.h.patch -o ./patched/insns/vpopc_m.h
patch ./originals/insns/vfcvt_f_x_v.h -i ./patches/insns/vfcvt_f_x_v.h.patch -o ./patched/insns/vfcvt_f_x_v.h
patch ./originals/insns/vfncvt_rtz_x_f_w.h -i ./patches/insns/vfncvt_rtz_x_f_w.h.patch -o ./patched/insns/vfncvt_rtz_x_f_w.h
patch ./originals/insns/vfncvt_f_x_w.h -i ./patches/insns/vfncvt_f_x_w.h.patch -o ./patched/insns/vfncvt_f_x_w.h
patch ./originals/insns/vmv_x_s.h -i ./patches/insns/vmv_x_s.h.patch -o ./patched/insns/vmv_x_s.h
patch ./originals/insns/vfncvt_f_f_w.h -i ./patches/insns/vfncvt_f_f_w.h.patch -o ./patched/insns/vfncvt_f_f_w.h
patch ./originals/insns/vfwcvt_f_xu_v.h -i ./patches/insns/vfwcvt_f_xu_v.h.patch -o ./patched/insns/vfwcvt_f_xu_v.h
patch ./originals/insns/vwmulsu_vv.h -i ./patches/insns/vwmulsu_vv.h.patch -o ./patched/insns/vwmulsu_vv.h
patch ./originals/insns/vfwcvt_x_f_v.h -i ./patches/insns/vfwcvt_x_f_v.h.patch -o ./patched/insns/vfwcvt_x_f_v.h
patch ./originals/insns/vmulhsu_vv.h -i ./patches/insns/vmulhsu_vv.h.patch -o ./patched/insns/vmulhsu_vv.h
patch ./originals/insns/vmvnfr_v.h  -i ./patches/insns/vmvnfr_v.h.patch -o ./patched/insns/vmvnfr_v.h
patch ./originals/insns/vmerge_vvm.h -i ./patches/insns/vmerge_vvm.h.patch -o ./patched/insns/vmerge_vvm.h
patch ./originals/insns/vfwcvt_f_x_v.h -i ./patches/insns/vfwcvt_f_x_v.h.patch -o ./patched/insns/vfwcvt_f_x_v.h
patch ./originals/insns/viota_m.h -i ./patches/insns/viota_m.h.patch -o ./patched/insns/viota_m.h
patch ./originals/insns/vwmulsu_vx.h -i ./patches/insns/vwmulsu_vx.h.patch -o ./patched/insns/vwmulsu_vx.h
patch ./originals/insns/vslide1up_vx.h -i ./patches/insns/vslide1up_vx.h.patch -o ./patched/insns/vslide1up_vx.h
patch ./originals/insns/vmadc_vxm.h -i ./patches/insns/vmadc_vxm.h.patch -o ./patched/insns/vmadc_vxm.h
patch ./originals/insns/vfslide1down_vf.h -i ./patches/insns/vfslide1down_vf.h.patch -o ./patched/insns/vfslide1down_vf.h
patch ./originals/insns/vmerge_vim.h -i ./patches/insns/vmerge_vim.h.patch -o ./patched/insns/vmerge_vim.h
patch ./originals/insns/vmadc_vvm.h -i ./patches/insns/vmadc_vvm.h.patch -o ./patched/insns/vmadc_vvm.h
patch ./originals/insns/vfcvt_xu_f_v.h -i ./patches/insns/vfcvt_xu_f_v.h.patch -o ./patched/insns/vfcvt_xu_f_v.h
patch ./originals/insns/vmerge_vxm.h -i ./patches/insns/vmerge_vxm.h.patch -o ./patched/insns/vmerge_vxm.h
patch ./originals/insns/vfwcvt_f_f_v.h -i ./patches/insns/vfwcvt_f_f_v.h.patch -o ./patched/insns/vfwcvt_f_f_v.h
patch ./originals/insns/vrgather_vx.h -i ./patches/insns/vrgather_vx.h.patch -o ./patched/insns/vrgather_vx.h
patch ./originals/insns/vfncvt_rod_f_f_w.h -i ./patches/insns/vfncvt_rod_f_f_w.h.patch -o ./patched/insns/vfncvt_rod_f_f_w.h
patch ./originals/insns/vfmerge_vfm.h -i ./patches/insns/vfmerge_vfm.h.patch -o ./patched/insns/vfmerge_vfm.h
patch ./originals/insns/vmsif_m.h -i ./patches/insns/vmsif_m.h.patch -o ./patched/insns/vmsif_m.h
patch ./originals/insns/vfncvt_xu_f_w.h -i ./patches/insns/vfncvt_xu_f_w.h.patch -o ./patched/insns/vfncvt_xu_f_w.h
patch ./originals/insns/vfmv_s_f.h  -i ./patches/insns/vfmv_s_f.h.patch -o ./patched/insns/vfmv_s_f.h
patch ./originals/insns/vid_v.h  -i ./patches/insns/vid_v.h.patch -o ./patched/insns/vid_v.h
patch ./originals/insns/vmulhsu_vx.h -i ./patches/insns/vmulhsu_vx.h.patch -o ./patched/insns/vmulhsu_vx.h
patch ./originals/insns/vslide1down_vx.h -i ./patches/insns/vslide1down_vx.h.patch -o ./patched/insns/vslide1down_vx.h
patch ./originals/insns/vfncvt_f_xu_w.h -i ./patches/insns/vfncvt_f_xu_w.h.patch -o ./patched/insns/vfncvt_f_xu_w.h
patch ./originals/insns/vcompress_vm.h -i ./patches/insns/vcompress_vm.h.patch -o ./patched/insns/vcompress_vm.h
patch ./originals/insns/vfcvt_x_f_v.h -i ./patches/insns/vfcvt_x_f_v.h.patch -o ./patched/insns/vfcvt_x_f_v.h
patch ./originals/insns/vfncvt_rtz_xu_f_w.h -i ./patches/insns/vfncvt_rtz_xu_f_w.h.patch -o ./patched/insns/vfncvt_rtz_xu_f_w.h
patch ./originals/insns/vfwcvt_rtz_xu_f_v.h -i ./patches/insns/vfwcvt_rtz_xu_f_v.h.patch -o ./patched/insns/vfwcvt_rtz_xu_f_v.h
patch ./originals/insns/vmsbc_vxm.h -i ./patches/insns/vmsbc_vxm.h.patch -o ./patched/insns/vmsbc_vxm.h
patch ./originals/insns/vmsbf_m.h -i ./patches/insns/vmsbf_m.h.patch -o ./patched/insns/vmsbf_m.h
patch ./originals/insns/vadc_vxm.h -i ./patches/insns/vadc_vxm.h.patch -o ./patched/insns/vadc_vxm.h
