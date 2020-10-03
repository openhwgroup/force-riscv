from lxml import etree as ET

xml_header = '<?xml version="1.0" encoding="UTF-8"?>'
license_string = '''
<!--
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
'''

startRootTag = "<instruction_file>\n"

endRootTag = "</instruction_file>"

# src_instr_file is a string that holds the name of the file to be split
# dst_dict is a dictionary with destination file name as the key and extensions list as value (e.g. ["riscv_instructions_float.xml": [RV32F, RV64F])

def split_file(src_instr_file, dst_dict):

    src_instr_file_parsed = ET.parse(src_instr_file)

    instrs = src_instr_file_parsed.getroot()
        
    for dst in dst_dict:
        dst_file = open(dst, 'wb+')
        extensions = dst_dict[dst]
        dst_file.write(xml_header.encode())
        dst_file.write(license_string.encode())
        dst_file.write(startRootTag.encode())

        for instr in instrs:
            if instr.attrib['extension'] in extensions:
                dst_file.write(ET.tostring(instr, pretty_print=True))

        dst_file.write(endRootTag.encode())
        dst_file.close()

def split_files():

    dst_dict = dict()
    dst_dict['riscv_instructions_int32_starter.xml'] = ['RV32I', 'RV32M']
    dst_dict['riscv_instructions_int64_starter.xml'] = ['RV64I', 'RV64M']
    dst_dict['riscv_instructions_float_starter.xml'] = ['RV32F', 'RV64F']
    dst_dict['riscv_instructions_double_starter.xml'] = ['RV32D', 'RV64D']
    dst_dict['riscv_instructions_quad_starter.xml'] = ['RV32Q', 'RV64Q'] 
    split_file('../riscv_instructions_starter.xml', dst_dict)

    dst_dict = dict()
    dst_dict['riscv_instructions_compressed_rv32_starter.xml'] = ['RV32C']
    dst_dict['riscv_instructions_compressed_rv64_starter.xml'] = ['RV64C']
    split_file('../c_instructions_starter.xml', dst_dict)

split_files()
