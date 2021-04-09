#!/usr/bin/env python3
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

import getopt
import sys

sys.path.insert(0, "../..")

import instruction_adjustor


def usage():
    usage_str = """%s
  --g-ext    output the g-extension instruction file.
  --c-ext    output the c-extension instruction file.
  --v-ext    output the v-extension instruction file.
  --zfh-ext  output the zfh-extension instruction file.
  --priv     output the privileged instruction file.
  -h, --help print this help message
Example:
%s --g-ext
""" % (
        sys.argv[0],
        sys.argv[0],
    )
    print(usage_str)


license_string = """<!--
 Copyright (C) [2020] Futurewei Technologies, Inc.

 FORCE-RISCV is licensed under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

 THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
 OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 See the License for the specific language governing permissions and
 limitations under the License.
-->
"""


def process_instruction_file(
    aInputFile, aOutputFile, aSupportedFile, aAdjustByFormatFunc
):
    from shared.instruction_file import InstructionFile
    from shared.instruction_file_parser import InstructionFileParser

    starter_file = aInputFile
    instr_file = InstructionFile()
    file_parser = InstructionFileParser(instr_file)
    file_parser.parse(starter_file)

    instr_adjustor = instruction_adjustor.RiscV_InstructionAdjustor(
        aAdjustByFormatFunc
    )
    instr_file.adjust_instructions(instr_adjustor)

    out_file_name = aOutputFile
    file_handle = open(out_file_name, "w")
    instr_file.write(file_handle, license_string)
    file_handle.close()

    supported_instr_file = instr_adjustor.get_supported_instructions()
    supported_file_handle = open(aSupportedFile, "w")
    supported_instr_file.write(supported_file_handle, license_string)
    supported_file_handle.close()


def build_instructions():
    try:
        opts, args = getopt.getopt(
            sys.argv[1:],
            "h",
            ["help", "g-ext", "c-ext", "v-ext", "zfh-ext", "priv"],
        )
    except getopt.GetoptError as err:
        print(err)
        usage()
        sys.exit(1)

    output_all = True
    c_ext_only = False
    g_ext_only = False
    v_ext_only = False
    zfh_ext_only = False
    priv_only = False

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o == "--c-ext":
            c_ext_only = True
            output_all = False
        elif o == "--v-ext":
            v_ext_only = True
            output_all = False
        elif o == "--zfh-ext":
            zfh_ext_only = False
            output_all = False
        elif o == "--priv":
            priv_only = True
            output_all = False
        elif o == "--g-ext":
            g_ext_only = True
            output_all = False
        else:
            print("unsupported option: %s" % o)
            sys.exit(1)

    from adjust_instruction_by_format import adjust_instruction_by_format
    from c_ext_adjust_instruction_by_format import (
        c_ext_adjust_instruction_by_format,
    )
    from v_ext_adjust_instruction_by_format import (
        v_ext_adjust_instruction_by_format,
    )
    from priv_adjust_instruction_by_format import (
        priv_adjust_instruction_by_format,
    )

    if output_all or g_ext_only:
        process_instruction_file(
            "input/g_instructions_starter.xml",
            "output/g_instructions.xml",
            "output/supported_g_instructions.xml",
            adjust_instruction_by_format,
        )
        process_instruction_file(
            "input/g_instructions_rv64_starter.xml",
            "output/g_instructions_rv64.xml",
            "output/supported_g_instructions_rv64.xml",
            adjust_instruction_by_format,
        )

    if output_all or c_ext_only:
        process_instruction_file(
            "input/c_instructions_starter.xml",
            "output/c_instructions.xml",
            "output/supported_c_instructions.xml",
            c_ext_adjust_instruction_by_format,
        )
        process_instruction_file(
            "input/c_instructions_rv32_starter.xml",
            "output/c_instructions_rv32.xml",
            "output/supported_c_instructions_rv32.xml",
            c_ext_adjust_instruction_by_format,
        )
        process_instruction_file(
            "input/c_instructions_rv64_starter.xml",
            "output/c_instructions_rv64.xml",
            "output/supported_c_instructions_rv64.xml",
            c_ext_adjust_instruction_by_format,
        )

    if output_all or v_ext_only:
        process_instruction_file(
            "input/v_instructions_starter.xml",
            "output/v_instructions.xml",
            "output/supported_v_instructions.xml",
            v_ext_adjust_instruction_by_format,
        )

    if output_all or zfh_ext_only:
        process_instruction_file(
            "input/zfh_instructions_starter.xml",
            "output/zfh_instructions.xml",
            "output/supported_zfh_instructions.xml",
            adjust_instruction_by_format,
        )
        process_instruction_file(
            "input/zfh_instructions_rv64_starter.xml",
            "output/zfh_instructions_rv64.xml",
            "output/supported_zfh_instructions_rv64.xml",
            adjust_instruction_by_format,
        )

    if output_all or priv_only:
        process_instruction_file(
            "input/priv_instructions_starter.xml",
            "output/priv_instructions.xml",
            "output/supported_priv_instructions.xml",
            priv_adjust_instruction_by_format,
        )


# temp class/unit test for instruction parsing utility
if __name__ == "__main__":
    import force_path_resolver

    force_path_resolver.add_force_relative_path("utils/builder")
    build_instructions()
