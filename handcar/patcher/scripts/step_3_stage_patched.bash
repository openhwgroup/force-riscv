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

cp ./patched/config.h ../spike_mod
cp ./patched/devices.h ../spike_mod
cp ./patched/disasm.h ../spike_mod
cp ./patched/mmu.cc ../spike_mod
cp ./patched/mret.h ../spike_mod      
cp ./patched/processor.cc ../spike_mod
cp ./patched/regnames.cc ../spike_mod
cp ./patched/simlib.h ../spike_mod  
cp ./patched/sret.h ../spike_mod
cp ./patched/decode.h ../spike_mod
cp ./patched/disasm.cc ../spike_mod
cp ./patched/execute.cc ../spike_mod
cp ./patched/mmu.h ../spike_mod 
cp ./patched/primitives.h ../spike_mod
cp ./patched/processor.h ../spike_mod
cp ./patched/simlib.cc ../spike_mod     
cp ./patched/simif.h ../spike_mod
cp ./patched/specialize.h ../spike_mod
cp ./patched/handcar* ../so_build/cosim/src

cp ./patched/insns/vmsof_m.h ../spike_mod/insns/
cp ./patched/insns/vmadc_vim.h ../spike_mod/insns/
cp ./patched/insns/vfncvt_x_f_w.h ../spike_mod/insns/
cp ./patched/insns/vfwcvt_xu_f_v.h ../spike_mod/insns/
cp ./patched/insns/vrgather_vv.h ../spike_mod/insns/
cp ./patched/insns/vmsbc_vvm.h ../spike_mod/insns/
cp ./patched/insns/vsbc_vvm.h ../spike_mod/insns/
cp ./patched/insns/vrgather_vi.h ../spike_mod/insns/
cp ./patched/insns/vs1r_v.h ../spike_mod/insns/
cp ./patched/insns/vfcvt_f_xu_v.h ../spike_mod/insns/
cp ./patched/insns/vfwcvt_rtz_x_f_v.h ../spike_mod/insns/
cp ./patched/insns/vsbc_vxm.h ../spike_mod/insns/
cp ./patched/insns/vfmv_v_f.h ../spike_mod/insns/
cp ./patched/insns/vmv_s_x.h ../spike_mod/insns/
cp ./patched/insns/vadc_vvm.h ../spike_mod/insns/
cp ./patched/insns/vfmv_f_s.h ../spike_mod/insns/
cp ./patched/insns/vadc_vim.h ../spike_mod/insns/
cp ./patched/insns/vl1r_v.h ../spike_mod/insns/
cp ./patched/insns/vfcvt_rtz_xu_f_v.h ../spike_mod/insns/
cp ./patched/insns/vfslide1up_vf.h ../spike_mod/insns/
cp ./patched/insns/vfirst_m.h ../spike_mod/insns/
cp ./patched/insns/vfcvt_rtz_x_f_v.h ../spike_mod/insns/
cp ./patched/insns/vpopc_m.h ../spike_mod/insns/
cp ./patched/insns/vfcvt_f_x_v.h ../spike_mod/insns/
cp ./patched/insns/vfncvt_rtz_x_f_w.h ../spike_mod/insns/
cp ./patched/insns/vfncvt_f_x_w.h ../spike_mod/insns/
cp ./patched/insns/vmv_x_s.h ../spike_mod/insns/
cp ./patched/insns/vfncvt_f_f_w.h ../spike_mod/insns/
cp ./patched/insns/vfwcvt_f_xu_v.h ../spike_mod/insns/
cp ./patched/insns/vwmulsu_vv.h ../spike_mod/insns/
cp ./patched/insns/vfwcvt_x_f_v.h ../spike_mod/insns/
cp ./patched/insns/vmulhsu_vv.h ../spike_mod/insns/
cp ./patched/insns/vmvnfr_v.h ../spike_mod/insns/
cp ./patched/insns/vmerge_vvm.h ../spike_mod/insns/
cp ./patched/insns/vfwcvt_f_x_v.h ../spike_mod/insns/
cp ./patched/insns/viota_m.h ../spike_mod/insns/
cp ./patched/insns/vwmulsu_vx.h ../spike_mod/insns/
cp ./patched/insns/vslide1up_vx.h ../spike_mod/insns/
cp ./patched/insns/vmadc_vxm.h ../spike_mod/insns/
cp ./patched/insns/vfslide1down_vf.h ../spike_mod/insns/
cp ./patched/insns/vmerge_vim.h ../spike_mod/insns/
cp ./patched/insns/vmadc_vvm.h ../spike_mod/insns/
cp ./patched/insns/vfcvt_xu_f_v.h ../spike_mod/insns/
cp ./patched/insns/vmerge_vxm.h ../spike_mod/insns/
cp ./patched/insns/vfwcvt_f_f_v.h ../spike_mod/insns/
cp ./patched/insns/vrgather_vx.h ../spike_mod/insns/
cp ./patched/insns/vfncvt_rod_f_f_w.h ../spike_mod/insns/
cp ./patched/insns/vfmerge_vfm.h ../spike_mod/insns/
cp ./patched/insns/vmsif_m.h ../spike_mod/insns/
cp ./patched/insns/vfncvt_xu_f_w.h ../spike_mod/insns/
cp ./patched/insns/vfmv_s_f.h ../spike_mod/insns/
cp ./patched/insns/vid_v.h ../spike_mod/insns/
cp ./patched/insns/vmulhsu_vx.h ../spike_mod/insns/
cp ./patched/insns/vslide1down_vx.h ../spike_mod/insns/
cp ./patched/insns/vfncvt_f_xu_w.h ../spike_mod/insns/
cp ./patched/insns/vcompress_vm.h ../spike_mod/insns/
cp ./patched/insns/vfcvt_x_f_v.h ../spike_mod/insns/
cp ./patched/insns/vfncvt_rtz_xu_f_w.h ../spike_mod/insns/
cp ./patched/insns/vfwcvt_rtz_xu_f_v.h ../spike_mod/insns/
cp ./patched/insns/vmsbc_vxm.h ../spike_mod/insns/
cp ./patched/insns/vmsbf_m.h ../spike_mod/insns/
cp ./patched/insns/vadc_vxm.h ../spike_mod/insns/

