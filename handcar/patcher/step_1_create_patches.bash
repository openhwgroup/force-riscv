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

mkdir -p originals/insns
mkdir -p modified/insns
mkdir -p patches/insns
mkdir -p patched/insns

cp ../standalone/config.h ./originals
cp ../standalone/riscv/devices.h ./originals
cp ../standalone/riscv/disasm.h ./originals
cp ../standalone/riscv/mmu.cc ./originals
cp ../standalone/riscv/insns/mret.h ./originals
cp ../standalone/riscv/processor.cc ./originals
cp ../standalone/riscv/regnames.cc ./originals
cp ../standalone/riscv/sim.h ./originals
cp ../standalone/riscv/insns/sret.h ./originals
cp ../standalone/riscv/decode.h ./originals
cp ../standalone/spike_main/disasm.cc ./originals
cp ../standalone/riscv/execute.cc ./originals
cp ../standalone/riscv/mmu.h ./originals
cp ../standalone/softfloat/primitives.h ./originals
cp ../standalone/riscv/processor.h ./originals
cp ../standalone/riscv/sim.cc ./originals
cp ../standalone/riscv/simif.h ./originals
cp ../standalone/softfloat/specialize.h ./originals
cp ../standalone/spike_main/spike.cc ./originals

# Instructions
cp ../standalone/riscv/insns/vmsof_m.h ./originals/insns/
cp ../standalone/riscv/insns/vmadc_vim.h ./originals/insns/
cp ../standalone/riscv/insns/vfncvt_x_f_w.h ./originals/insns/
cp ../standalone/riscv/insns/vfwcvt_xu_f_v.h ./originals/insns/
cp ../standalone/riscv/insns/vrgather_vv.h ./originals/insns/
cp ../standalone/riscv/insns/vmsbc_vvm.h ./originals/insns/
cp ../standalone/riscv/insns/vsbc_vvm.h ./originals/insns/
cp ../standalone/riscv/insns/vrgather_vi.h ./originals/insns/
cp ../standalone/riscv/insns/vs1r_v.h ./originals/insns/
cp ../standalone/riscv/insns/vfcvt_f_xu_v.h ./originals/insns/
cp ../standalone/riscv/insns/vfwcvt_rtz_x_f_v.h ./originals/insns/
cp ../standalone/riscv/insns/vsbc_vxm.h ./originals/insns/
cp ../standalone/riscv/insns/vfmv_v_f.h ./originals/insns/
cp ../standalone/riscv/insns/vmv_s_x.h ./originals/insns/
cp ../standalone/riscv/insns/vadc_vvm.h ./originals/insns/
cp ../standalone/riscv/insns/vfmv_f_s.h ./originals/insns/
cp ../standalone/riscv/insns/vadc_vim.h ./originals/insns/
cp ../standalone/riscv/insns/vl1r_v.h ./originals/insns/
cp ../standalone/riscv/insns/vfcvt_rtz_xu_f_v.h ./originals/insns/
cp ../standalone/riscv/insns/vfslide1up_vf.h ./originals/insns/
cp ../standalone/riscv/insns/vfirst_m.h ./originals/insns/
cp ../standalone/riscv/insns/vfcvt_rtz_x_f_v.h ./originals/insns/
cp ../standalone/riscv/insns/vpopc_m.h ./originals/insns/
cp ../standalone/riscv/insns/vfcvt_f_x_v.h ./originals/insns/
cp ../standalone/riscv/insns/vfncvt_rtz_x_f_w.h ./originals/insns/
cp ../standalone/riscv/insns/vfncvt_f_x_w.h ./originals/insns/
cp ../standalone/riscv/insns/vmv_x_s.h ./originals/insns/
cp ../standalone/riscv/insns/vfncvt_f_f_w.h ./originals/insns/
cp ../standalone/riscv/insns/vfwcvt_f_xu_v.h ./originals/insns/
cp ../standalone/riscv/insns/vwmulsu_vv.h ./originals/insns/
cp ../standalone/riscv/insns/vfwcvt_x_f_v.h ./originals/insns/
cp ../standalone/riscv/insns/vmulhsu_vv.h ./originals/insns/
cp ../standalone/riscv/insns/vmvnfr_v.h ./originals/insns/
cp ../standalone/riscv/insns/vmerge_vvm.h ./originals/insns/
cp ../standalone/riscv/insns/vfwcvt_f_x_v.h ./originals/insns/
cp ../standalone/riscv/insns/viota_m.h ./originals/insns/
cp ../standalone/riscv/insns/vwmulsu_vx.h ./originals/insns/
cp ../standalone/riscv/insns/vslide1up_vx.h ./originals/insns/
cp ../standalone/riscv/insns/vmadc_vxm.h ./originals/insns/
cp ../standalone/riscv/insns/vfslide1down_vf.h ./originals/insns/
cp ../standalone/riscv/insns/vmerge_vim.h ./originals/insns/
cp ../standalone/riscv/insns/vmadc_vvm.h ./originals/insns/
cp ../standalone/riscv/insns/vfcvt_xu_f_v.h ./originals/insns/
cp ../standalone/riscv/insns/vmerge_vxm.h ./originals/insns/
cp ../standalone/riscv/insns/vfwcvt_f_f_v.h ./originals/insns/
cp ../standalone/riscv/insns/vrgather_vx.h ./originals/insns/
cp ../standalone/riscv/insns/vfncvt_rod_f_f_w.h ./originals/insns/
cp ../standalone/riscv/insns/vfmerge_vfm.h ./originals/insns/
cp ../standalone/riscv/insns/vmsif_m.h ./originals/insns/
cp ../standalone/riscv/insns/vfncvt_xu_f_w.h ./originals/insns/
cp ../standalone/riscv/insns/vfmv_s_f.h ./originals/insns/
cp ../standalone/riscv/insns/vid_v.h ./originals/insns/
cp ../standalone/riscv/insns/vmulhsu_vx.h ./originals/insns/
cp ../standalone/riscv/insns/vslide1down_vx.h ./originals/insns/
cp ../standalone/riscv/insns/vfncvt_f_xu_w.h ./originals/insns/
cp ../standalone/riscv/insns/vcompress_vm.h ./originals/insns/
cp ../standalone/riscv/insns/vfcvt_x_f_v.h ./originals/insns/
cp ../standalone/riscv/insns/vfncvt_rtz_xu_f_w.h ./originals/insns/
cp ../standalone/riscv/insns/vfwcvt_rtz_xu_f_v.h ./originals/insns/
cp ../standalone/riscv/insns/vmsbc_vxm.h ./originals/insns/
cp ../standalone/riscv/insns/vmsbf_m.h ./originals/insns/
cp ../standalone/riscv/insns/vadc_vxm.h ./originals/insns/


#These lines were used when first creating the patches. There are not normally to be used unless updating the patches. DO NOT UNCOMMENT unless you know what you are doing.
###
#cp ../spike_mod/config.h ./modified
#cp ../spike_mod/devices.h ./modified
#cp ../spike_mod/disasm.h ./modified
#cp ../spike_mod/mmu.cc ./modified
#cp ../spike_mod/mret.h ./modified
#cp ../spike_mod/processor.cc ./modified
#cp ../spike_mod/regnames.cc ./modified
#cp ../spike_mod/simlib.h ./modified
#cp ../spike_mod/sret.h ./modified
#cp ../spike_mod/decode.h ./modified
#cp ../spike_mod/disasm.cc ./modified
#cp ../spike_mod/execute.cc ./modified
#cp ../spike_mod/mmu.h ./modified
#cp ../spike_mod/primitives.h ./modified
#cp ../spike_mod/processor.h ./modified
#cp ../spike_mod/simlib.cc ./modified
#cp ../spike_mod/simif.h ./modified
#cp ../spike_mod/specialize.h ./modified
#cp ../spike_mod/handcar* ./modified
#rm patches/config.h.patch  
#rm patches/devices.h.patch  
#rm patches/disasm.h.patch    
#rm patches/mmu.cc.patch  
#rm patches/mret.h.patch        
#rm patches/processor.cc.patch  
#rm patches/regnames.cc.patch  
#rm patches/sim.h.patch    
#rm patches/sret.h.patch
#rm patches/decode.h.patch  
#rm patches/disasm.cc.patch  
#rm patches/execute.cc.patch  
#rm patches/mmu.h.patch   
#rm patches/primitives.h.patch  
#rm patches/processor.h.patch   
#rm patches/sim.cc.patch       
#rm patches/simif.h.patch  
#rm patches/specialize.h.patch
#rm patches/spike.cc.patch

#cp ../spike_mod/insns/vmsof_m.h ./modified/insns/
#cp ../spike_mod/insns/vmadc_vim.h ./modified/insns/
#cp ../spike_mod/insns/vfncvt_x_f_w.h ./modified/insns/
#cp ../spike_mod/insns/vfwcvt_xu_f_v.h ./modified/insns/
#cp ../spike_mod/insns/vrgather_vv.h ./modified/insns/
#cp ../spike_mod/insns/vmsbc_vvm.h ./modified/insns/
#cp ../spike_mod/insns/vsbc_vvm.h ./modified/insns/
#cp ../spike_mod/insns/vrgather_vi.h ./modified/insns/
#cp ../spike_mod/insns/vs1r_v.h ./modified/insns/
#cp ../spike_mod/insns/vfcvt_f_xu_v.h ./modified/insns/
#cp ../spike_mod/insns/vfwcvt_rtz_x_f_v.h ./modified/insns/
#cp ../spike_mod/insns/vsbc_vxm.h ./modified/insns/
#cp ../spike_mod/insns/vfmv_v_f.h ./modified/insns/
#cp ../spike_mod/insns/vmv_s_x.h ./modified/insns/
#cp ../spike_mod/insns/vadc_vvm.h ./modified/insns/
#cp ../spike_mod/insns/vfmv_f_s.h ./modified/insns/
#cp ../spike_mod/insns/vadc_vim.h ./modified/insns/
#cp ../spike_mod/insns/vl1r_v.h ./modified/insns/
#cp ../spike_mod/insns/vfcvt_rtz_xu_f_v.h ./modified/insns/
#cp ../spike_mod/insns/vfslide1up_vf.h ./modified/insns/
#cp ../spike_mod/insns/vfirst_m.h ./modified/insns/
#cp ../spike_mod/insns/vfcvt_rtz_x_f_v.h ./modified/insns/
#cp ../spike_mod/insns/vpopc_m.h ./modified/insns/
#cp ../spike_mod/insns/vfcvt_f_x_v.h ./modified/insns/
#cp ../spike_mod/insns/vfncvt_rtz_x_f_w.h ./modified/insns/
#cp ../spike_mod/insns/vfncvt_f_x_w.h ./modified/insns/
#cp ../spike_mod/insns/vmv_x_s.h ./modified/insns/
#cp ../spike_mod/insns/vfncvt_f_f_w.h ./modified/insns/
#cp ../spike_mod/insns/vfwcvt_f_xu_v.h ./modified/insns/
#cp ../spike_mod/insns/vwmulsu_vv.h ./modified/insns/
#cp ../spike_mod/insns/vfwcvt_x_f_v.h ./modified/insns/
#cp ../spike_mod/insns/vmulhsu_vv.h ./modified/insns/
#cp ../spike_mod/insns/vmvnfr_v.h ./modified/insns/
#cp ../spike_mod/insns/vmerge_vvm.h ./modified/insns/
#cp ../spike_mod/insns/vfwcvt_f_x_v.h ./modified/insns/
#cp ../spike_mod/insns/viota_m.h ./modified/insns/
#cp ../spike_mod/insns/vwmulsu_vx.h ./modified/insns/
#cp ../spike_mod/insns/vslide1up_vx.h ./modified/insns/
#cp ../spike_mod/insns/vmadc_vxm.h ./modified/insns/
#cp ../spike_mod/insns/vfslide1down_vf.h ./modified/insns/
#cp ../spike_mod/insns/vmerge_vim.h ./modified/insns/
#cp ../spike_mod/insns/vmadc_vvm.h ./modified/insns/
#cp ../spike_mod/insns/vfcvt_xu_f_v.h ./modified/insns/
#cp ../spike_mod/insns/vmerge_vxm.h ./modified/insns/
#cp ../spike_mod/insns/vfwcvt_f_f_v.h ./modified/insns/
#cp ../spike_mod/insns/vrgather_vx.h ./modified/insns/
#cp ../spike_mod/insns/vfncvt_rod_f_f_w.h ./modified/insns/
#cp ../spike_mod/insns/vfmerge_vfm.h ./modified/insns/
#cp ../spike_mod/insns/vmsif_m.h ./modified/insns/
#cp ../spike_mod/insns/vfncvt_xu_f_w.h ./modified/insns/
#cp ../spike_mod/insns/vfmv_s_f.h ./modified/insns/
#cp ../spike_mod/insns/vid_v.h ./modified/insns/
#cp ../spike_mod/insns/vmulhsu_vx.h ./modified/insns/
#cp ../spike_mod/insns/vslide1down_vx.h ./modified/insns/
#cp ../spike_mod/insns/vfncvt_f_xu_w.h ./modified/insns/
#cp ../spike_mod/insns/vcompress_vm.h ./modified/insns/
#cp ../spike_mod/insns/vfcvt_x_f_v.h ./modified/insns/
#cp ../spike_mod/insns/vfncvt_rtz_xu_f_w.h ./modified/insns/
#cp ../spike_mod/insns/vfwcvt_rtz_xu_f_v.h ./modified/insns/
#cp ../spike_mod/insns/vmsbc_vxm.h ./modified/insns/
#cp ../spike_mod/insns/vmsbf_m.h ./modified/insns/
#cp ../spike_mod/insns/vadc_vxm.h ./modified/insns/
#rm patches/insns/vmsof_m.h.patch
#rm patches/insns/vmadc_vim.h.patch
#rm patches/insns/vfncvt_x_f_w.h.patch
#rm patches/insns/vfwcvt_xu_f_v.h.patch
#rm patches/insns/vrgather_vv.h.patch
#rm patches/insns/vmsbc_vvm.h.patch
#rm patches/insns/vsbc_vvm.h.patch
#rm patches/insns/vrgather_vi.h.patch
#rm patches/insns/vs1r_v.h.patch
#rm patches/insns/vfcvt_f_xu_v.h.patch
#rm patches/insns/vfwcvt_rtz_x_f_v.h.patch
#rm patches/insns/vsbc_vxm.h.patch
#rm patches/insns/vfmv_v_f.h.patch
#rm patches/insns/vmv_s_x.h.patch
#rm patches/insns/vadc_vvm.h.patch
#rm patches/insns/vfmv_f_s.h.patch
#rm patches/insns/vadc_vim.h.patch
#rm patches/insns/vl1r_v.h.patch
#rm patches/insns/vfcvt_rtz_xu_f_v.h.patch
#rm patches/insns/vfslide1up_vf.h.patch
#rm patches/insns/vfirst_m.h.patch
#rm patches/insns/vfcvt_rtz_x_f_v.h.patch
#rm patches/insns/vpopc_m.h.patch
#rm patches/insns/vfcvt_f_x_v.h.patch
#rm patches/insns/vfncvt_rtz_x_f_w.h.patch
#rm patches/insns/vfncvt_f_x_w.h.patch
#rm patches/insns/vmv_x_s.h.patch
#rm patches/insns/vfncvt_f_f_w.h.patch
#rm patches/insns/vfwcvt_f_xu_v.h.patch
#rm patches/insns/vwmulsu_vv.h.patch
#rm patches/insns/vfwcvt_x_f_v.h.patch
#rm patches/insns/vmulhsu_vv.h.patch
#rm patches/insns/vmvnfr_v.h.patch
#rm patches/insns/vmerge_vvm.h.patch
#rm patches/insns/vfwcvt_f_x_v.h.patch
#rm patches/insns/viota_m.h.patch
#rm patches/insns/vwmulsu_vx.h.patch
#rm patches/insns/vslide1up_vx.h.patch
#rm patches/insns/vmadc_vxm.h.patch
#rm patches/insns/vfslide1down_vf.h.patch
#rm patches/insns/vmerge_vim.h.patch
#rm patches/insns/vmadc_vvm.h.patch
#rm patches/insns/vfcvt_xu_f_v.h.patch
#rm patches/insns/vmerge_vxm.h.patch
#rm patches/insns/vfwcvt_f_f_v.h.patch
#rm patches/insns/vrgather_vx.h.patch
#rm patches/insns/vfncvt_rod_f_f_w.h.patch
#rm patches/insns/vfmerge_vfm.h.patch
#rm patches/insns/vmsif_m.h.patch
#rm patches/insns/vfncvt_xu_f_w.h.patch
#rm patches/insns/vfmv_s_f.h.patch
#rm patches/insns/vid_v.h.patch
#rm patches/insns/vmulhsu_vx.h.patch
#rm patches/insns/vslide1down_vx.h.patch
#rm patches/insns/vfncvt_f_xu_w.h.patch
#rm patches/insns/vcompress_vm.h.patch
#rm patches/insns/vfcvt_x_f_v.h.patch
#rm patches/insns/vfncvt_rtz_xu_f_w.h.patch
#rm patches/insns/vfwcvt_rtz_xu_f_v.h.patch
#rm patches/insns/vmsbc_vxm.h.patch
#rm patches/insns/vmsbf_m.h.patch
#rm patches/insns/vadc_vxm.h.patch
#
#echo "#" >> patches/config.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/config.h.patch
#echo "#" >> patches/config.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/config.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/config.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/config.h.patch
#echo "#" >> patches/config.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/config.h.patch
#echo "#" >> patches/config.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/config.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/config.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/config.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/config.h.patch
#echo "# limitations under the License." >> patches/config.h.patch
#echo "#" >> patches/config.h.patch
#diff originals/config.h modified/config.h >> patches/config.h.patch  
#
#echo "#" >> patches/devices.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/devices.h.patch
#echo "#" >> patches/devices.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/devices.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/devices.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/devices.h.patch
#echo "#" >> patches/devices.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/devices.h.patch
#echo "#" >> patches/devices.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/devices.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/devices.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/devices.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/devices.h.patch
#echo "# limitations under the License." >> patches/devices.h.patch
#echo "#" >> patches/devices.h.patch
#diff originals/devices.h modified/devices.h >> patches/devices.h.patch  
#
#echo "#" >> patches/disasm.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/disasm.h.patch
#echo "#" >> patches/disasm.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/disasm.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/disasm.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/disasm.h.patch
#echo "#" >> patches/disasm.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/disasm.h.patch
#echo "#" >> patches/disasm.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/disasm.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/disasm.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/disasm.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/disasm.h.patch
#echo "# limitations under the License." >> patches/disasm.h.patch
#echo "#" >> patches/disasm.h.patch
#diff originals/disasm.h modified/disasm.h >> patches/disasm.h.patch    
#
#echo "#" >> patches/mmu.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/mmu.cc.patch
#echo "#" >> patches/mmu.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/mmu.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/mmu.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/mmu.cc.patch
#echo "#" >> patches/mmu.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/mmu.cc.patch
#echo "#" >> patches/mmu.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/mmu.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/mmu.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/mmu.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/mmu.cc.patch
#echo "# limitations under the License." >> patches/mmu.cc.patch
#echo "#" >> patches/mmu.cc.patch
#diff originals/mmu.cc modified/mmu.cc >> patches/mmu.cc.patch  
#
#echo "#" >> patches/mret.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/mret.h.patch
#echo "#" >> patches/mret.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/mret.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/mret.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/mret.h.patch
#echo "#" >> patches/mret.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/mret.h.patch
#echo "#" >> patches/mret.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/mret.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/mret.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/mret.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/mret.h.patch
#echo "# limitations under the License." >> patches/mret.h.patch
#echo "#" >> patches/mret.h.patch
#diff originals/mret.h modified/mret.h >> patches/mret.h.patch        
#
#echo "#" >> patches/processor.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/processor.cc.patch
#echo "#" >> patches/processor.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/processor.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/processor.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/processor.cc.patch
#echo "#" >> patches/processor.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/processor.cc.patch
#echo "#" >> patches/processor.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/processor.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/processor.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/processor.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/processor.cc.patch
#echo "# limitations under the License." >> patches/processor.cc.patch
#echo "#" >> patches/processor.cc.patch
#diff originals/processor.cc modified/processor.cc >> patches/processor.cc.patch  
#
#echo "#" >> patches/regnames.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/regnames.cc.patch
#echo "#" >> patches/regnames.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/regnames.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/regnames.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/regnames.cc.patch
#echo "#" >> patches/regnames.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/regnames.cc.patch
#echo "#" >> patches/regnames.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/regnames.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/regnames.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/regnames.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/regnames.cc.patch
#echo "# limitations under the License." >> patches/regnames.cc.patch
#echo "#" >> patches/regnames.cc.patch
#diff originals/regnames.cc modified/regnames.cc >> patches/regnames.cc.patch  
#
#echo "#" >> patches/sim.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/sim.h.patch
#echo "#" >> patches/sim.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/sim.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/sim.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/sim.h.patch
#echo "#" >> patches/sim.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/sim.h.patch
#echo "#" >> patches/sim.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/sim.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/sim.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/sim.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/sim.h.patch
#echo "# limitations under the License." >> patches/sim.h.patch
#echo "#" >> patches/sim.h.patch
#diff originals/sim.h modified/simlib.h >> patches/sim.h.patch    
#
#echo "#" >> patches/sret.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/sret.h.patch
#echo "#" >> patches/sret.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/sret.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/sret.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/sret.h.patch
#echo "#" >> patches/sret.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/sret.h.patch
#echo "#" >> patches/sret.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/sret.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/sret.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/sret.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/sret.h.patch
#echo "# limitations under the License." >> patches/sret.h.patch
#echo "#" >> patches/sret.h.patch
#diff originals/sret.h modified/sret.h >> patches/sret.h.patch
#
#echo "#" >> patches/decode.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/decode.h.patch
#echo "#" >> patches/decode.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/decode.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/decode.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/decode.h.patch
#echo "#" >> patches/decode.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/decode.h.patch
#echo "#" >> patches/decode.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/decode.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/decode.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/decode.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/decode.h.patch
#echo "# limitations under the License." >> patches/decode.h.patch
#echo "#" >> patches/decode.h.patch
#diff originals/decode.h modified/decode.h >> patches/decode.h.patch  
#
#echo "#" >> patches/disasm.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/disasm.cc.patch
#echo "#" >> patches/disasm.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/disasm.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/disasm.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/disasm.cc.patch
#echo "#" >> patches/disasm.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/disasm.cc.patch
#echo "#" >> patches/disasm.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/disasm.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/disasm.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/disasm.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/disasm.cc.patch
#echo "# limitations under the License." >> patches/disasm.cc.patch
#echo "#" >> patches/disasm.cc.patch
#diff originals/disasm.cc modified/disasm.cc >> patches/disasm.cc.patch  
#
#echo "#" >> patches/execute.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/execute.cc.patch
#echo "#" >> patches/execute.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/execute.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/execute.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/execute.cc.patch
#echo "#" >> patches/execute.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/execute.cc.patch
#echo "#" >> patches/execute.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/execute.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/execute.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/execute.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/execute.cc.patch
#echo "# limitations under the License." >> patches/execute.cc.patch
#echo "#" >> patches/execute.cc.patch
#diff originals/execute.cc modified/execute.cc >> patches/execute.cc.patch  
#
#echo "#" >> patches/mmu.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/mmu.h.patch
#echo "#" >> patches/mmu.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/mmu.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/mmu.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/mmu.h.patch
#echo "#" >> patches/mmu.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/mmu.h.patch
#echo "#" >> patches/mmu.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/mmu.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/mmu.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/mmu.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/mmu.h.patch
#echo "# limitations under the License." >> patches/mmu.h.patch
#echo "#" >> patches/mmu.h.patch
#diff originals/mmu.h modified/mmu.h >> patches/mmu.h.patch   
#
#echo "#" >> patches/primitives.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/primitives.h.patch
#echo "#" >> patches/primitives.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/primitives.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/primitives.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/primitives.h.patch
#echo "#" >> patches/primitives.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/primitives.h.patch
#echo "#" >> patches/primitives.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/primitives.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/primitives.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/primitives.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/primitives.h.patch
#echo "# limitations under the License." >> patches/primitives.h.patch
#echo "#" >> patches/primitives.h.patch
#diff originals/primitives.h modified/primitives.h >> patches/primitives.h.patch  
#
#echo "#" >> patches/processor.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/processor.h.patch
#echo "#" >> patches/processor.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/processor.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/processor.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/processor.h.patch
#echo "#" >> patches/processor.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/processor.h.patch
#echo "#" >> patches/processor.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/processor.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/processor.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/processor.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/processor.h.patch
#echo "# limitations under the License." >> patches/processor.h.patch
#echo "#" >> patches/processor.h.patch
#diff originals/processor.h modified/processor.h >> patches/processor.h.patch   
#
#echo "#" >> patches/sim.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/sim.cc.patch
#echo "#" >> patches/sim.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/sim.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/sim.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/sim.cc.patch
#echo "#" >> patches/sim.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/sim.cc.patch
#echo "#" >> patches/sim.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/sim.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/sim.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/sim.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/sim.cc.patch
#echo "# limitations under the License." >> patches/sim.cc.patch
#echo "#" >> patches/sim.cc.patch
#diff originals/sim.cc modified/simlib.cc >> patches/sim.cc.patch       
#
#echo "#" >> patches/simif.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/simif.h.patch
#echo "#" >> patches/simif.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/simif.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/simif.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/simif.h.patch
#echo "#" >> patches/simif.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/simif.h.patch
#echo "#" >> patches/simif.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/simif.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/simif.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/simif.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/simif.h.patch
#echo "# limitations under the License." >> patches/simif.h.patch
#echo "#" >> patches/simif.h.patch
#diff originals/simif.h modified/simif.h >> patches/simif.h.patch  
#
#echo "#" >> patches/specialize.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/specialize.h.patch
#echo "#" >> patches/specialize.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/specialize.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/specialize.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/specialize.h.patch
#echo "#" >> patches/specialize.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/specialize.h.patch
#echo "#" >> patches/specialize.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/specialize.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/specialize.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/specialize.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/specialize.h.patch
#echo "# limitations under the License." >> patches/specialize.h.patch
#echo "#" >> patches/specialize.h.patch
#diff originals/specialize.h modified/specialize.h >> patches/specialize.h.patch
#
#echo "#" >> patches/spike.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/spike.cc.patch
#echo "#" >> patches/spike.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/spike.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/spike.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/spike.cc.patch
#echo "#" >> patches/spike.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/spike.cc.patch
#echo "#" >> patches/spike.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/spike.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/spike.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/spike.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/spike.cc.patch
#echo "# limitations under the License." >> patches/spike.cc.patch
#echo "#" >> patches/spike.cc.patch
#diff originals/spike.cc modified/handcar* >> patches/spike.cc.patch
#
###
#
### Instruction patches
#
#license="#\n# Copyright (C) [2020] Futurewei Technologies, Inc.\n#\n# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the \"License\");\n#  you may not use this file except in compliance with the License.\n#  You may obtain a copy of the License at\n#\n#  http://www.apache.org/licenses/LICENSE-2.0\n#\n# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER\n# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR\n# FIT FOR A PARTICULAR PURPOSE.\n# See the License for the specific language governing permissions and\n# limitations under the License.\n#"
#
#echo -e $license >> patches/insns/vmsof_m.h.patch
#echo -e $license >> patches/insns/vmadc_vim.h.patch
#echo -e $license >> patches/insns/vfncvt_x_f_w.h.patch
#echo -e $license >> patches/insns/vfwcvt_xu_f_v.h.patch
#echo -e $license >> patches/insns/vrgather_vv.h.patch
#echo -e $license >> patches/insns/vmsbc_vvm.h.patch
#echo -e $license >> patches/insns/vsbc_vvm.h.patch
#echo -e $license >> patches/insns/vrgather_vi.h.patch
#echo -e $license >> patches/insns/vs1r_v.h.patch
#echo -e $license >> patches/insns/vfcvt_f_xu_v.h.patch
#echo -e $license >> patches/insns/vfwcvt_rtz_x_f_v.h.patch
#echo -e $license >> patches/insns/vsbc_vxm.h.patch
#echo -e $license >> patches/insns/vfmv_v_f.h.patch
#echo -e $license >> patches/insns/vmv_s_x.h.patch
#echo -e $license >> patches/insns/vadc_vvm.h.patch
#echo -e $license >> patches/insns/vfmv_f_s.h.patch
#echo -e $license >> patches/insns/vadc_vim.h.patch
#echo -e $license >> patches/insns/vl1r_v.h.patch
#echo -e $license >> patches/insns/vfcvt_rtz_xu_f_v.h.patch
#echo -e $license >> patches/insns/vfslide1up_vf.h.patch
#echo -e $license >> patches/insns/vfirst_m.h.patch
#echo -e $license >> patches/insns/vfcvt_rtz_x_f_v.h.patch
#echo -e $license >> patches/insns/vpopc_m.h.patch
#echo -e $license >> patches/insns/vfcvt_f_x_v.h.patch
#echo -e $license >> patches/insns/vfncvt_rtz_x_f_w.h.patch
#echo -e $license >> patches/insns/vfncvt_f_x_w.h.patch
#echo -e $license >> patches/insns/vmv_x_s.h.patch
#echo -e $license >> patches/insns/vfncvt_f_f_w.h.patch
#echo -e $license >> patches/insns/vfwcvt_f_xu_v.h.patch
#echo -e $license >> patches/insns/vwmulsu_vv.h.patch
#echo -e $license >> patches/insns/vfwcvt_x_f_v.h.patch
#echo -e $license >> patches/insns/vmulhsu_vv.h.patch
#echo -e $license >> patches/insns/vmvnfr_v.h.patch
#echo -e $license >> patches/insns/vmerge_vvm.h.patch
#echo -e $license >> patches/insns/vfwcvt_f_x_v.h.patch
#echo -e $license >> patches/insns/viota_m.h.patch
#echo -e $license >> patches/insns/vwmulsu_vx.h.patch
#echo -e $license >> patches/insns/vslide1up_vx.h.patch
#echo -e $license >> patches/insns/vmadc_vxm.h.patch
#echo -e $license >> patches/insns/vfslide1down_vf.h.patch
#echo -e $license >> patches/insns/vmerge_vim.h.patch
#echo -e $license >> patches/insns/vmadc_vvm.h.patch
#echo -e $license >> patches/insns/vfcvt_xu_f_v.h.patch
#echo -e $license >> patches/insns/vmerge_vxm.h.patch
#echo -e $license >> patches/insns/vfwcvt_f_f_v.h.patch
#echo -e $license >> patches/insns/vrgather_vx.h.patch
#echo -e $license >> patches/insns/vfncvt_rod_f_f_w.h.patch
#echo -e $license >> patches/insns/vfmerge_vfm.h.patch
#echo -e $license >> patches/insns/vmsif_m.h.patch
#echo -e $license >> patches/insns/vfncvt_xu_f_w.h.patch
#echo -e $license >> patches/insns/vfmv_s_f.h.patch
#echo -e $license >> patches/insns/vid_v.h.patch
#echo -e $license >> patches/insns/vmulhsu_vx.h.patch
#echo -e $license >> patches/insns/vslide1down_vx.h.patch
#echo -e $license >> patches/insns/vfncvt_f_xu_w.h.patch
#echo -e $license >> patches/insns/vcompress_vm.h.patch
#echo -e $license >> patches/insns/vfcvt_x_f_v.h.patch
#echo -e $license >> patches/insns/vfncvt_rtz_xu_f_w.h.patch
#echo -e $license >> patches/insns/vfwcvt_rtz_xu_f_v.h.patch
#echo -e $license >> patches/insns/vmsbc_vxm.h.patch
#echo -e $license >> patches/insns/vmsbf_m.h.patch
#echo -e $license >> patches/insns/vadc_vxm.h.patch
#
#diff originals/insns/vmsof_m.h modified/insns/vmsof_m.h >> patches/insns/vmsof_m.h.patch
#diff originals/insns/vmadc_vim.h modified/insns/vmadc_vim.h >> patches/insns/vmadc_vim.h.patch
#diff originals/insns/vfncvt_x_f_w.h modified/insns/vfncvt_x_f_w.h >> patches/insns/vfncvt_x_f_w.h.patch
#diff originals/insns/vfwcvt_xu_f_v.h modified/insns/vfwcvt_xu_f_v.h >> patches/insns/vfwcvt_xu_f_v.h.patch
#diff originals/insns/vrgather_vv.h modified/insns/vrgather_vv.h >> patches/insns/vrgather_vv.h.patch
#diff originals/insns/vmsbc_vvm.h modified/insns/vmsbc_vvm.h >> patches/insns/vmsbc_vvm.h.patch
#diff originals/insns/vsbc_vvm.h modified/insns/vsbc_vvm.h >> patches/insns/vsbc_vvm.h.patch
#diff originals/insns/vrgather_vi.h modified/insns/vrgather_vi.h >> patches/insns/vrgather_vi.h.patch
#diff originals/insns/vs1r_v.h modified/insns/vs1r_v.h >> patches/insns/vs1r_v.h.patch
#diff originals/insns/vfcvt_f_xu_v.h modified/insns/vfcvt_f_xu_v.h >> patches/insns/vfcvt_f_xu_v.h.patch
#diff originals/insns/vfwcvt_rtz_x_f_v.h modified/insns/vfwcvt_rtz_x_f_v.h >> patches/insns/vfwcvt_rtz_x_f_v.h.patch
#diff originals/insns/vsbc_vxm.h modified/insns/vsbc_vxm.h >> patches/insns/vsbc_vxm.h.patch
#diff originals/insns/vfmv_v_f.h modified/insns/vfmv_v_f.h >> patches/insns/vfmv_v_f.h.patch
#diff originals/insns/vmv_s_x.h modified/insns/vmv_s_x.h >> patches/insns/vmv_s_x.h.patch
#diff originals/insns/vadc_vvm.h modified/insns/vadc_vvm.h >> patches/insns/vadc_vvm.h.patch
#diff originals/insns/vfmv_f_s.h modified/insns/vfmv_f_s.h >> patches/insns/vfmv_f_s.h.patch
#diff originals/insns/vadc_vim.h modified/insns/vadc_vim.h >> patches/insns/vadc_vim.h.patch
#diff originals/insns/vl1r_v.h modified/insns/vl1r_v.h >> patches/insns/vl1r_v.h.patch
#diff originals/insns/vfcvt_rtz_xu_f_v.h modified/insns/vfcvt_rtz_xu_f_v.h >> patches/insns/vfcvt_rtz_xu_f_v.h.patch
#diff originals/insns/vfslide1up_vf.h modified/insns/vfslide1up_vf.h >> patches/insns/vfslide1up_vf.h.patch
#diff originals/insns/vfirst_m.h modified/insns/vfirst_m.h >> patches/insns/vfirst_m.h.patch
#diff originals/insns/vfcvt_rtz_x_f_v.h modified/insns/vfcvt_rtz_x_f_v.h >> patches/insns/vfcvt_rtz_x_f_v.h.patch
#diff originals/insns/vpopc_m.h modified/insns/vpopc_m.h >> patches/insns/vpopc_m.h.patch
#diff originals/insns/vfcvt_f_x_v.h modified/insns/vfcvt_f_x_v.h >> patches/insns/vfcvt_f_x_v.h.patch
#diff originals/insns/vfncvt_rtz_x_f_w.h modified/insns/vfncvt_rtz_x_f_w.h >> patches/insns/vfncvt_rtz_x_f_w.h.patch
#diff originals/insns/vfncvt_f_x_w.h modified/insns/vfncvt_f_x_w.h >> patches/insns/vfncvt_f_x_w.h.patch
#diff originals/insns/vmv_x_s.h modified/insns/vmv_x_s.h >> patches/insns/vmv_x_s.h.patch
#diff originals/insns/vfncvt_f_f_w.h modified/insns/vfncvt_f_f_w.h >> patches/insns/vfncvt_f_f_w.h.patch
#diff originals/insns/vfwcvt_f_xu_v.h modified/insns/vfwcvt_f_xu_v.h >> patches/insns/vfwcvt_f_xu_v.h.patch
#diff originals/insns/vwmulsu_vv.h modified/insns/vwmulsu_vv.h >> patches/insns/vwmulsu_vv.h.patch
#diff originals/insns/vfwcvt_x_f_v.h modified/insns/vfwcvt_x_f_v.h >> patches/insns/vfwcvt_x_f_v.h.patch
#diff originals/insns/vmulhsu_vv.h modified/insns/vmulhsu_vv.h >> patches/insns/vmulhsu_vv.h.patch
#diff originals/insns/vmvnfr_v.h  modified/insns/vmvnfr_v.h  >> patches/insns/vmvnfr_v.h.patch
#diff originals/insns/vmerge_vvm.h modified/insns/vmerge_vvm.h >> patches/insns/vmerge_vvm.h.patch
#diff originals/insns/vfwcvt_f_x_v.h modified/insns/vfwcvt_f_x_v.h >> patches/insns/vfwcvt_f_x_v.h.patch
#diff originals/insns/viota_m.h  modified/insns/viota_m.h  >> patches/insns/viota_m.h.patch
#diff originals/insns/vwmulsu_vx.h modified/insns/vwmulsu_vx.h >> patches/insns/vwmulsu_vx.h.patch
#diff originals/insns/vslide1up_vx.h modified/insns/vslide1up_vx.h >> patches/insns/vslide1up_vx.h.patch
#diff originals/insns/vmadc_vxm.h  modified/insns/vmadc_vxm.h >> patches/insns/vmadc_vxm.h.patch
#diff originals/insns/vfslide1down_vf.h modified/insns/vfslide1down_vf.h >> patches/insns/vfslide1down_vf.h.patch
#diff originals/insns/vmerge_vim.h modified/insns/vmerge_vim.h >> patches/insns/vmerge_vim.h.patch
#diff originals/insns/vmadc_vvm.h  modified/insns/vmadc_vvm.h >> patches/insns/vmadc_vvm.h.patch
#diff originals/insns/vfcvt_xu_f_v.h modified/insns/vfcvt_xu_f_v.h >> patches/insns/vfcvt_xu_f_v.h.patch
#diff originals/insns/vmerge_vxm.h modified/insns/vmerge_vxm.h >> patches/insns/vmerge_vxm.h.patch
#diff originals/insns/vfwcvt_f_f_v.h modified/insns/vfwcvt_f_f_v.h >> patches/insns/vfwcvt_f_f_v.h.patch
#diff originals/insns/vrgather_vx.h modified/insns/vrgather_vx.h >> patches/insns/vrgather_vx.h.patch
#diff originals/insns/vfncvt_rod_f_f_w.h modified/insns/vfncvt_rod_f_f_w.h >> patches/insns/vfncvt_rod_f_f_w.h.patch
#diff originals/insns/vfmerge_vfm.h modified/insns/vfmerge_vfm.h >> patches/insns/vfmerge_vfm.h.patch
#diff originals/insns/vmsif_m.h  modified/insns/vmsif_m.h  >> patches/insns/vmsif_m.h.patch
#diff originals/insns/vfncvt_xu_f_w.h modified/insns/vfncvt_xu_f_w.h >> patches/insns/vfncvt_xu_f_w.h.patch
#diff originals/insns/vfmv_s_f.h  modified/insns/vfmv_s_f.h  >> patches/insns/vfmv_s_f.h.patch
#diff originals/insns/vid_v.h  modified/insns/vid_v.h  >> patches/insns/vid_v.h.patch
#diff originals/insns/vmulhsu_vx.h modified/insns/vmulhsu_vx.h >> patches/insns/vmulhsu_vx.h.patch
#diff originals/insns/vslide1down_vx.h modified/insns/vslide1down_vx.h >> patches/insns/vslide1down_vx.h.patch
#diff originals/insns/vfncvt_f_xu_w.h modified/insns/vfncvt_f_xu_w.h >> patches/insns/vfncvt_f_xu_w.h.patch
#diff originals/insns/vcompress_vm.h modified/insns/vcompress_vm.h >> patches/insns/vcompress_vm.h.patch
#diff originals/insns/vfcvt_x_f_v.h modified/insns/vfcvt_x_f_v.h >> patches/insns/vfcvt_x_f_v.h.patch
#diff originals/insns/vfncvt_rtz_xu_f_w.h modified/insns/vfncvt_rtz_xu_f_w.h >> patches/insns/vfncvt_rtz_xu_f_w.h.patch
#diff originals/insns/vfwcvt_rtz_xu_f_v.h modified/insns/vfwcvt_rtz_xu_f_v.h >> patches/insns/vfwcvt_rtz_xu_f_v.h.patch
#diff originals/insns/vmsbc_vxm.h modified/insns/vmsbc_vxm.h >> patches/insns/vmsbc_vxm.h.patch
#diff originals/insns/vmsbf_m.h modified/insns/vmsbf_m.h >> patches/insns/vmsbf_m.h.patch
#diff originals/insns/vadc_vxm.h modified/insns/vadc_vxm.h >> patches/insns/vadc_vxm.h.patch
#
