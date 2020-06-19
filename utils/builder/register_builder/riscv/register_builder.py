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

import xml.etree.ElementTree as ET
import xml.dom.minidom as DOM

def usage():
    usage_str = """%s
  --system_registers    output the system registers file.
  --system_register_choices    output the system register choices file.
  --register_field_choices    output the register field choices file.
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

def output_system_registers(aTree, aOutputFile):
    registers = aTree.findall('.//register')

    output_root = ET.Element('registers')
    physical_registers = ET.Element('physical_registers')
    register_file = ET.Element('register_file')
    register_file.set('name', 'RISC-V Registers')

    for register in registers:
        generate_register(register, physical_registers, register_file)

    output_root.append(physical_registers)
    output_root.append(register_file)

    pretty_print_xml(aOutputFile, output_root)

def output_system_register_choices(aTree, aOutputFile):
    registers = aTree.findall('.//register')

    output_root = ET.Element('choices_file')
    read = ET.Element('choices')
    read.set('description', 'RISC-V Read system registers')
    read.set('name', 'Read system registers')
    read.set('type', 'RegisterOperand')
    write = ET.Element('choices')
    write.set('description', 'RISC-V Write system registers')
    write.set('name', 'Write system registers')
    write.set('type', 'RegisterOperand')

    for register in registers:
        if register.get('choice', 'true') == 'true':
            choice = ET.Element('choice')
            choice.set('description', '%s; %s' % (register.get('privilege'), register.get('description')))
            choice.set('name', register.get('name'))
            choice.set('value', register.get('index'))
            choice.set('weight', register.get('weight', '0'))

            if 'R' in register.get('privilege'):
                read.append(choice)
            if 'W' in register.get('privilege'):
                write.append(choice)

    output_root.append(read)
    output_root.append(write)

    pretty_print_xml(aOutputFile, output_root)

def output_register_field_choices(aTree, aOutputFile):
    registers = aTree.findall('.//register')

    output_root = ET.Element('choices_file')

    for register in registers:
        if register.get('choice', 'true') == 'true':
            fields = register.findall('.//register_field')
            for field in fields:
                choices = field.findall('.//choice')
                if choices:
                    choices_element = ET.Element('choices')
                    choices_element.set('name', '%s.%s' % (register.get('name'), field.get('name')))
                    choices_element.set('type', 'RegisterFieldValue')

                    for choice in choices:
                        choice_element = ET.Element('choice')
                        choice_element.set('description', choice.get('description'))
                        choice_element.set('value', choice.get('value'))
                        choice_element.set('weight', choice.get('weight', '10'))
                        choices_element.append(choice_element)

                    output_root.append(choices_element)

    pretty_print_xml(aOutputFile, output_root)

def pretty_print_xml(aOutputFile, aOutputRoot):
    xml_str = DOM.parseString(ET.tostring(aOutputRoot)).toprettyxml(indent='  ')
    with open(aOutputFile, 'w') as f:
        f.write(license_string)
        f.write(xml_str)

def generate_register(aRegister, aPhysicalRegisters, aRegisterFile):
    register = ET.Element('register')
    register.set('index', aRegister.get('index'))
    register.set('name', aRegister.get('name'))
    register.set('size', '64' if str(aRegister.get('size')) == '0' else aRegister.get('size'))
    register.set('type', aRegister.get('type'))
    if aRegister.get('size') != '0':
        register.set('boot', aRegister.get('boot', '1'))
    else:
        register.set('boot', '0')

    fields = aRegister.findall('.//register_field')
    for field in fields:
        register_field = ET.Element('register_field')
        register_field.set('name', field.get('name'))
        register_field.set('physical_register', aRegister.get('physical_register'))
        register_field.set('size', field.get('size'))
        if field.get('init_policy'):
            register_field.set('init_policy', field.get('init_policy'))

        bits = field.findall('.//bit_field')
        for bit in bits:
            bit_field = ET.Element('bit_field')
            bit_field.set('shift', bit.get('shift'))
            bit_field.set('size', bit.get('size'))
            register_field.append(bit_field)

        register.append(register_field)

    if aRegister.get('physical_register') and aRegister.get('physical_register') == aRegister.get('name') or aRegister.get('physical_register') is None:
        physical_register = ET.Element('physical_register')
        physical_register.set('index', aRegister.get('index'))
        physical_register.set('name', aRegister.get('physical_register', aRegister.get('name')))
        physical_register.set('size', '64' if str(aRegister.get('size')) == '0' else aRegister.get('size'))
        physical_register.set('type', aRegister.get('type'))
        if aRegister.get('class'):
            physical_register.set('class', aRegister.get('class'))
        aPhysicalRegisters.append(physical_register)

    aRegisterFile.append(register)

def build_registers(aXmlFile):
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'h', ['help', 'system_registers', 'system_register_choices', 'register_field_choices'])
    except getopt.GetoptError as error:
        print(error)
        usage()
        sys.exit(1)

    output_all = True
    system_registers = False
    system_register_choices = False
    register_field_choices = False

    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()
        elif o in ['--system_registers', '--system_register_choices', '--register_field_choices']:
            output_all = False
            if o == '--system_registers':
                system_registers = True
            if o == '--system_register_choices':
                system_register_choices = True
            if o == '--register_field_choices':
                register_field_choices = True
        else:
            print('unsupported option: %s' % o)
            sys.exit(1)

    if output_all or system_registers:
        output_system_registers(aXmlFile, 'output/system_registers.xml')
    if output_all or system_register_choices:
        output_system_register_choices(aXmlFile, 'output/system_register_choices.xml')
    if output_all or register_field_choices:
        output_register_field_choices(aXmlFile, 'output/register_field_choices.xml')

if __name__ == '__main__':
    starter = ET.parse('input/system_registers_starter.xml')
    build_registers(starter)

