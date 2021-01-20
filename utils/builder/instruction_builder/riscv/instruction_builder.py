#!/usr/bin/env python3
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

import getopt, sys
sys.path.insert(0, '../..')

from instruction_adjustor import G_InstructionAdjustor

def usage():
    usage_str = """%s
  --main     output the main instruction file.
  --c-ext    output the c-extension instruction file.
  --v-ext    output the v-extension instruction file.
  --priv     output the privileged instruction file.
  -h, --help print this help message
Example:
%s --main
""" % (sys.argv[0], sys.argv[0])
    print(usage_str)

license_string = """<!--
 Copyright (C) [2020] Futurewei Technologies, Inc.

 FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

 THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 FIT FOR A PARTICULAR PURPOSE.
 See the License for the specific language governing permissions and
 limitations under the License.
-->
"""
    
def process_instruction_file(aInputFile, aOutputFile, aSupportedFile, aAdjustor):
    from shared.instruction_file import InstructionFile
    from shared.instruction_file_parser import InstructionFileParser

    starter_file = aInputFile
    instr_file = InstructionFile()
    file_parser = InstructionFileParser(instr_file)
    file_parser.parse(starter_file)

    instr_file.adjust_instructions(aAdjustor)

    out_file_name = aOutputFile
    file_handle = open(out_file_name, "w")
    instr_file.write(file_handle, license_string)
    file_handle.close()

    supported_instr_file = aAdjustor._mSupportedInstrFile
    supported_file_handle = open(aSupportedFile, "w")
    supported_instr_file.write(supported_file_handle, license_string)
    supported_file_handle.close()

def build_instructions():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "h", ["help", "main", "c-ext", "v-ext", "priv"])
    except getopt.GetoptError as err:
        print (err)
        usage()
        sys.exit(1)

    output_all = True
    c_ext_only = False
    main_only = False
    v_ext_only = False
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
        elif o == "--priv":
            priv_only = True
            output_all = False
        elif o == "--main":
            main_only = True
            output_all = False
        else:
            print("unsupported option: %s" % o)
            sys.exit(1)

    #from adjust_instruction_by_format import adjust_instruction_by_format
    #from c_ext_adjust_instruction_by_format import c_ext_adjust_instruction_by_format
    #from v_ext_adjust_instruction_by_format import v_ext_adjust_instruction_by_format
    #from priv_adjust_instruction_by_format import priv_adjust_instruction_by_format

    if output_all or main_only:
        process_instruction_file("input/riscv_instructions_starter.xml", "output/riscv_instructions.xml", "output/supported_riscv_instructions.xml", G_InstructionAdjustor())

        #process_instruction_file("input/rv32i_instructions_starter.xml", "output/rv32i_instructions.xml", "output/rv32i_supported_instructions.xml", adjust_instruction_by_format)
        #process_instruction_file("input/rv64i_instructions_starter.xml", "output/rv64i_instructions.xml", "output/rv64i_supported_instructions.xml", adjust_instruction_by_format)
        #process_instruction_file("input/f_instructions_starter.xml", "output/f_instructions.xml", "output/f_supported_instructions.xml", adjust_instruction_by_format)
        #process_instruction_file("input/d_instructions_starter.xml", "output/d_instructions.xml", "output/d_supported_instructions.xml", adjust_instruction_by_format)
        #process_instruction_file("input/q_instructions_starter.xml", "output/q_instructions.xml", "output/q_supported_instructions.xml", adjust_instruction_by_format)
        

    #if output_all or c_ext_only:
        #process_instruction_file("input/c_instructions_starter.xml", "output/c_instructions.xml", "output/supported_c_instructions.xml", c_ext_adjust_instruction_by_format)
        #process_instruction_file("input/rv64only_c_instructions_starter.xml", "output/rv64only_c_instructions.xml", "output/supported_rv64only_c_instructions.xml", c_ext_adjust_instruction_by_format)
        #process_instruction_file("input/rv32c_instructions_starter.xml", "output/rv32c_instructions.xml", "output/rv32c_supported_instructions.xml", c_ext_adjust_instruction_by_format)
        #process_instruction_file("input/rv64c_instructions_starter.xml", "output/rv64c_instructions.xml", "output/rv64c_supported_instructions.xml", c_ext_adjust_instruction_by_format)

    #if output_all or v_ext_only:
        #process_instruction_file("input/v_instructions_starter.xml", "output/v_instructions.xml", "output/supported_v_instructions.xml", v_ext_adjust_instruction_by_format)

    #if output_all or priv_only:
        #process_instruction_file("input/priv_instructions_starter.xml", "output/priv_instructions.xml", "output/supported_priv_instructions.xml", priv_adjust_instruction_by_format)

#temp class/unit test for instruction parsing utility
if __name__ == "__main__":
    import force_path_resolver
    force_path_resolver.add_force_relative_path("utils/builder")
    build_instructions()
