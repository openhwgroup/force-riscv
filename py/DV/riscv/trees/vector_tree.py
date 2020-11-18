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
from base.InstructionMap import InstructionMap


# Utility function used to combine multiple dictionaries into one single dictionary
def Merge(*args):
    result = {}
    for dict1 in args:
        result.update(dict1)
    return result


vinteger_instructions = {
    'VADD.VI##RISCV':10,
    'VADD.VV##RISCV':10,
    'VADD.VX##RISCV':10,
    'VSUB.VV##RISCV':10,
    'VSUB.VX##RISCV':10,
    'VRSUB.VI##RISCV':10,
    'VRSUB.VX##RISCV':10,
    'VWADD.VV##RISCV':10,
    'VWADD.VX##RISCV':10,
    'VWADD.W.VV##RISCV':10,                 # ISA has vwadd.wv
    'VWADD.W.VX##RISCV':10,                 # ISA has vwadd.wx
    'VWADDU.VV##RISCV':10,
    'VWADDU.VX##RISCV':10,
    'VWADDU.W.VV##RISCV':10,                # ISA has vwaddu.wv
    'VWADDU.W.VX##RISCV':10,                # ISA has vwaddu.wx
    'VWSUB.VV##RISCV':10,
    'VWSUB.VX##RISCV':10,
    'VWSUB.W.VV##RISCV':10,                 # ISA has vwsub.wv
    'VWSUB.W.VX##RISCV':10,                 # ISA has vwsub.wx
    'VWSUBU.VV##RISCV':10,
    'VWSUBU.VX##RISCV':10,
    'VWSUBU.W.VV##RISCV':10,                # ISA has vwsubu.wv
    'VWSUBU.W.VX##RISCV':10,                # ISA has vwsubu.wx
    'VADC.VI##RISCV':10,
    'VADC.VV##RISCV':10,
    'VADC.VX##RISCV':10,
    'VMADC.VI##RISCV':10,
    'VMADC.VV##RISCV':10,
    'VMADC.VX##RISCV':10,
    'VSBC.VV##RISCV':10,
    'VSBC.VX##RISCV':10,
    'VMSBC.VV##RISCV':10,
    'VMSBC.VX##RISCV':10,
    'VAND.VI##RISCV':10,
    'VAND.VV##RISCV':10,
    'VAND.VX##RISCV':10,
    'VOR.VI##RISCV':10,
    'VOR.VV##RISCV':10,
    'VOR.VX##RISCV':10,
    'VXOR.VI##RISCV':10,
    'VXOR.VV##RISCV':10,
    'VXOR.VX##RISCV':10,
    'VSLL.VI##RISCV':10,
    'VSLL.VV##RISCV':10,
    'VSLL.VX##RISCV':10,
    'VSRA.VI##RISCV':10,
    'VSRA.VV##RISCV':10,
    'VSRA.VX##RISCV':10,
    'VSRL.VI##RISCV':10,
    'VSRL.VV##RISCV':10,
    'VSRL.VX##RISCV':10,
    'VNSRA.WI##RISCV':10,
    'VNSRA.WV##RISCV':10,
    'VNSRA.WX##RISCV':10,
    'VNSRL.WI##RISCV':10,
    'VNSRL.WV##RISCV':10,
    'VNSRL.WX##RISCV':10,
    'VMSEQ.VI##RISCV':10,
    'VMSEQ.VV##RISCV':10,
    'VMSEQ.VX##RISCV':10,
    'VMSGT.VI##RISCV':10,
    'VMSGT.VX##RISCV':10,
    'VMSGTU.VI##RISCV':10,
    'VMSGTU.VX##RISCV':10,
    'VMSLE.VI##RISCV':10,
    'VMSLE.VV##RISCV':10,
    'VMSLE.VX##RISCV':10,
    'VMSLEU.VI##RISCV':10,
    'VMSLEU.VV##RISCV':10,
    'VMSLEU.VX##RISCV':10,
    'VMSLT.VV##RISCV':10,
    'VMSLT.VX##RISCV':10,
    'VMSLTU.VV##RISCV':10,
    'VMSLTU.VX##RISCV':10,
    'VMSNE.VI##RISCV':10,
    'VMSNE.VV##RISCV':10,
    'VMSNE.VX##RISCV':10,
    'VMAX.VV##RISCV':10,
    'VMAX.VX##RISCV':10,
    'VMAXU.VV##RISCV':10,
    'VMAXU.VX##RISCV':10,
    'VMIN.VV##RISCV':10,
    'VMIN.VX##RISCV':10,
    'VMINU.VV##RISCV':10,
    'VMINU.VX##RISCV':10,
    'VMUL.VV##RISCV':10,
    'VMUL.VX##RISCV':10,
    'VMULH.VV##RISCV':10,
    'VMULH.VX##RISCV':10,
    'VMULHSU.VV##RISCV':10,
    'VMULHSU.VX##RISCV':10,
    'VMULHU.VV##RISCV':10,
    'VMULHU.VX##RISCV':10,
    'VDIV.VV##RISCV':10,
    'VDIV.VX##RISCV':10,
    'VDIVU.VV##RISCV':10,
    'VDIVU.VX##RISCV':10,
    'VREM.VV##RISCV':10,
    'VREM.VX##RISCV':10,
    'VREMU.VV##RISCV':10,
    'VREMU.VX##RISCV':10,
    'VWMUL.VV##RISCV':10,
    'VWMUL.VX##RISCV':10,
    'VWMULSU.VV##RISCV':10,
    'VWMULSU.VX##RISCV':10,
    'VWMULU.VV##RISCV':10,
    'VWMULU.VX##RISCV':10,
    'VMACC.VV##RISCV':10,
    'VMACC.VX##RISCV':10,
    'VMADD.VV##RISCV':10,
    'VMADD.VX##RISCV':10,
    'VNMSAC.VV##RISCV':10,
    'VNMSAC.VX##RISCV':10,
    'VNMSUB.VV##RISCV':10,
    'VNMSUB.VX##RISCV':10,
    'VWMACC.VV##RISCV':10,
    'VWMACC.VX##RISCV':10,
    'VWMACCSU.VV##RISCV':10,
    'VWMACCSU.VX##RISCV':10,
    'VWMACCU.VV##RISCV':10,
    'VWMACCU.VX##RISCV':10,
    'VWMACCUS.VX##RISCV':10,
    'VQMACC.VV##RISCV':10,
    'VQMACC.VX##RISCV':10,
    'VQMACCSU.VV##RISCV':10,
    'VQMACCSU.VX##RISCV':10,
    'VQMACCU.VV##RISCV':10,
    'VQMACCU.VX##RISCV':10,
    'VQMACCUS.VX##RISCV':10,
    'VMERGE/VMV.VI##RISCV':10,                  # ISA has vmerge.vim
    'VMERGE/VMV.VV##RISCV':10,                  # ISA has vmerge.vvm
    'VMERGE/VMV.VX##RISCV':10,                  # ISA has vmerge.vxm
    # These move instructions are missing from v_instructions.xml.  Appears generation was corrupted.
    # vmv.v.v
    # vmv.v.x
    # vmv.v.i
    }

vinteger_map = InstructionMap('vinteger_instructions', vinteger_instructions)



vpermutation_instructions = {
    'VMV.S.X##RISCV':10,
    'VMV.X.S##RISCV':10,
    'VFMV.F.S##RISCV':10,
    'VFMV.S.F##RISCV':10,
    'VSLIDEUP.VI##RISCV':10,
    'VSLIDEUP.VX##RISCV':10,
    'VSLIDEDOWN.VI##RISCV':10,
    'VSLIDEDOWN.VX##RISCV':10,
    'VSLIDE1DOWN.VX##RISCV':10,
    'VSLIDE1UP.VX##RISCV':10,
    'VRGATHER.VI##RISCV':10,
    'VRGATHER.VV##RISCV':10,
    'VRGATHER.VX##RISCV':10,
    'VCOMPRESS.VV##RISCV':10,
    }

vpermutation_map = ('vpermutation_instructions', vpermutation_instructions)



vmask_instructions = {
    'VMAND.VV##RISCV':10,                   # ISA has .mm instead of .vv
    'VMANDNOT.VV##RISCV':10,                # ISA has .mm instead of .vv
    'VMNAND.VV##RISCV':10,                  # ISA has .mm instead of .vv
    'VMNOR.VV##RISCV':10,                   # ISA has .mm instead of .vv
    'VMOR.VV##RISCV':10,                    # ISA has .mm instead of .vv
    'VMORNOT.VV##RISCV':10,                 # ISA has .mm instead of .vv
    'VMXNOR.VV##RISCV':10,                  # ISA has .mm instead of .vv
    'VMXOR.VV##RISCV':10,                   # ISA has .mm instead of .vv
    'VPOPC##RISCV':10,                      # ISA has vpopc.m
    'VFIRST##RISCV':10,                     # ISA has vfirst.m
    'VMSBF##RISCV':10,                      # ISA has vmsbf.m
    'VMSIF##RISCV':10,                      # ISA has vmsif.m
    'VMSOF##RISCV':10,                      # ISA has vmsof.m
    'VIOTA##RISCV':10,                      # ISA has viota.m
    'VID##RISCV':10,                        # ISA has vid.v
    }

vmask_map = InstructionMap('vmask_instructions', vmask_instructions)



vreduction_instructions = {
    'VREDAND.VV##RISCV':10,                 # ISA has .vs instead of .vv
    'VREDMAX.VV##RISCV':10,                 # ISA has .vs instead of .vv
    'VREDMAXU.VV##RISCV':10,                # ISA has .vs instead of .vv
    'VREDMIN.VV##RISCV':10,                 # ISA has .vs instead of .vv
    'VREDMINU.VV##RISCV':10,                # ISA has .vs instead of .vv
    'VREDOR.VV##RISCV':10,                  # ISA has .vs instead of .vv
    'VREDSUM.VV##RISCV':10,                 # ISA has .vs instead of .vv
    'VREDXOR.VV##RISCV':10,                 # ISA has .vs instead of .vv
    'VWREDSUM.VV##RISCV':10,                # ISA has .vs instead of .vv
    'VWREDSUMU.VV##RISCV':10,               # ISA has .vs instead of .vv
    'VFREDMAX.VV##RISCV':10,                # ISA has .vs instead of .vv
    'VFREDMIN.VV##RISCV':10,                # ISA has .vs instead of .vv
    'VFREDOSUM.VV##RISCV':10,               # ISA has .vs instead of .vv
    'VFREDSUM.VV##RISCV':10,                # ISA has .vs instead of .vv
    'VFWREDOSUM.VV##RISCV':10,              # ISA has .vs instead of .vv
    'VFWREDSUM.VV##RISCV':10,               # ISA has .vs instead of .vv
    }

vreduction_map = InstructionMap('vreduction_instructions', vreduction_instructions)



vfloating_point_instructions = {
    'VFADD.VF##RISCV':10,
    'VFADD.VV##RISCV':10,
    'VFSUB.VF##RISCV':10,
    'VFSUB.VV##RISCV':10,
    'VFRSUB.VF##RISCV':10,
    'VFWADD.VF##RISCV':10,
    'VFWADD.VV##RISCV':10,
    'VFWADD.W.VF##RISCV':10,            # ISA has vfwadd.wf
    'VFWADD.W.VV##RISCV':10,            # ISA has vfwadd.wv
    'VFWSUB.VF##RISCV':10,
    'VFWSUB.VV##RISCV':10,
    'VFWSUB.W.VF##RISCV':10,            # ISA has vfwsub.wf
    'VFWSUB.W.VV##RISCV':10,            # ISA has vfwsub.wv
    'VFMUL.VF##RISCV':10,
    'VFMUL.VV##RISCV':10,
    'VFDIV.VF##RISCV':10,
    'VFDIV.VV##RISCV':10,
    'VFRDIV.VF##RISCV':10,
    'VFWMUL.VF##RISCV':10,
    'VFWMUL.VV##RISCV':10,
    'VFMACC.VF##RISCV':10,
    'VFMACC.VV##RISCV':10,
    'VFMADD.VF##RISCV':10,
    'VFMADD.VV##RISCV':10,
    'VFNMACC.VF##RISCV':10,
    'VFNMACC.VV##RISCV':10,
    'VFNMADD.VF##RISCV':10,
    'VFNMADD.VV##RISCV':10,
    'VFNMSAC.VF##RISCV':10,
    'VFNMSAC.VV##RISCV':10,
    'VFNMSUB.VF##RISCV':10,
    'VFNMSUB.VV##RISCV':10,
    'VFMSAC.VF##RISCV':10,
    'VFMSAC.VV##RISCV':10,
    'VFMSUB.VF##RISCV':10,
    'VFMSUB.VV##RISCV':10,
    'VFWMACC.VF##RISCV':10,
    'VFWMACC.VV##RISCV':10,
    'VFWMSAC.VF##RISCV':10,
    'VFWMSAC.VV##RISCV':10,
    'VFWNMACC.VF##RISCV':10,
    'VFWNMACC.VV##RISCV':10,
    'VFWNMSAC.VF##RISCV':10,
    'VFWNMSAC.VV##RISCV':10,
    'VFSQRT.V##RISCV':10,
    'VFMAX.VF##RISCV':10,
    'VFMAX.VV##RISCV':10,
    'VFMIN.VF##RISCV':10,
    'VFMIN.VV##RISCV':10,
    'VFSGNJ.VF##RISCV':10,
    'VFSGNJ.VV##RISCV':10,
    'VFSGNJN.VF##RISCV':10,
    'VFSGNJN.VV##RISCV':10,
    'VFSGNJX.VF##RISCV':10,
    'VFSGNJX.VV##RISCV':10,
    'VMFEQ.VF##RISCV':10,
    'VMFEQ.VV##RISCV':10,
    'VMFGE.VF##RISCV':10,
    'VMFGT.VF##RISCV':10,
    'VMFLE.VF##RISCV':10,
    'VMFLE.VV##RISCV':10,
    'VMFLT.VF##RISCV':10,
    'VMFLT.VV##RISCV':10,
    'VMFNE.VF##RISCV':10,
    'VMFNE.VV##RISCV':10,
    'VFCLASS.V##RISCV':10,
    'VFMERGE.VF/VFMV.VF##RISCV':10,             # ISA has vfmerge.vfm
    # ISA has vfmv.v.f which appears to have been corrupted in the generation

    # ISA has 6 VFxCVT type instructions - the "rtz" flavor that are
    # missing from the v_instructions.xml.  These were added in Version 0.9 of V spec
    #   vfcvt.rtz.xu.f.v
    #   vfcvt.rtz.x.f.v
    #   vfwcvt.rtz.xu.f.v
    #   vfwcvt.rtz.x.f.v
    #   vfncvt.rtz.xu.f.w
    #   vfncvt.rtz.x.f.w

    'VFCVT.F.X.V##RISCV':10,
    'VFCVT.F.XU.V##RISCV':10, 
    'VFCVT.X.F.V##RISCV':10,
    'VFCVT.XU.F.V##RISCV':10,
    'VFWCVT.F.F.V##RISCV':10,
    'VFWCVT.F.X.V##RISCV':10,
    'VFWCVT.F.XU.V##RISCV':10,
    'VFWCVT.X.F.V##RISCV':10,
    'VFWCVT.XU.F.V##RISCV':10,
    'VFNCVT.F.F.W##RISCV':10,
    'VFNCVT.F.X.W##RISCV':10,
    'VFNCVT.F.XU.W##RISCV':10,
    'VFNCVT.ROD.F.F.W##RISCV':10,
    'VFNCVT.X.F.W##RISCV':10,
    'VFNCVT.XU.F.W##RISCV':10,
    }

vfloating_point_map = InstructionMap('vfloating_point_instructions', vfloating_point_instructions)




vconfig_instructions = {
    'VSETVL##RISCV':10,
    'VSETVLI##RISCV':10,
    }

vconfig_map = InstructionMap('vconfig_instructions', vconfig_instructions)



vfixed_point_instructions = {
    'VSADD.VI##RISCV':10,
    'VSADD.VV##RISCV':10,
    'VSADD.VX##RISCV':10,
    'VSADDU.VI##RISCV':10,
    'VSADDU.VV##RISCV':10,
    'VSADDU.VX##RISCV':10,
    'VSSUB.VV##RISCV':10,
    'VSSUB.VX##RISCV':10,
    'VSSUBU.VV##RISCV':10,
    'VSSUBU.VX##RISCV':10,
    'VAADD.VV##RISCV':10,
    'VAADD.VX##RISCV':10,
    'VAADDU.VV##RISCV':10,
    'VAADDU.VX##RISCV':10,
    'VASUB.VV##RISCV':10,
    'VASUB.VX##RISCV':10,
    'VASUBU.VV##RISCV':10,
    'VASUBU.VX##RISCV':10,
    'VSMUL.VV##RISCV':10,
    'VSMUL.VX##RISCV':10,
    'VSSRA.VI##RISCV':10,
    'VSSRA.VV##RISCV':10,
    'VSSRA.VX##RISCV':10,
    'VSSRL.VI##RISCV':10,
    'VSSRL.VV##RISCV':10,
    'VSSRL.VX##RISCV':10,
    'VNCLIP.VI##RISCV':10,              # ISA has vnclip.wi
    'VNCLIP.VV##RISCV':10,              # ISA has vnclip.wv
    'VNCLIP.VX##RISCV':10,              # ISA has vnclip.wx
    'VNCLIPU.VI##RISCV':10,              # ISA has vnclipu.wi
    'VNCLIPU.VV##RISCV':10,              # ISA has vnclipu.wv
    'VNCLIPU.VX##RISCV':10,              # ISA has vnclipu.wx
    }

vfixed_point_map = InstructionMap('vfixed_point_instructions', vfixed_point_instructions)
    

Zvediv_instructions = {
    'VDOT.VV##RISCV':10,
    'VDOTU.VV##RISCV':10,
    'VFDOT.VV##RISCV':10,
    }

Zvediv_map = InstructionMap('Zvediv_instructions', Zvediv_instructions)



