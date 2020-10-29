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

import getopt, sys, os
sys.path.insert(0, '../..')

import xml.etree.ElementTree as ET
import xml.dom.minidom as DOM
import argparse
import RiscVRegDef as RISCV
from ModifyRegisterFile import Files 

def usage():
    usage_str = """
This register builder script is used to create system register files. The input is a 'starter' file,
in the form of an xml file, that contains system register definitions. The output can include a Force
system registers file, system register choices file, and (system) register field choices file.

To run the script, type: %s <options>, where <options> include:

  -i, --system_registers_starter <starter file>   Specify the system registers starter file (defaults to input/system_registers_starter.xml)

  -m, --modifications_script <mods script>        Specify the register/field/choices modifications script (has no default value)

                  
  -S, --system_registers                          Output the system registers file. (defaults to output/system_registers.xml)

  -R, --system_register_choices                   Output the system register choices file. (defaults to output/system_register_choices.xml)

  -F, --register_field_choices                    Output the register field choices file. (defaults to output/register_field_choices.xml)


  -s, --system_registers_file                     Specify the path to the system registers (output) file.

  -r, --system_register_choices_file              Specify the path to the system register choices (output) file.

  -f, --register_field_choices_file               Specify the path to the register field choices (output) file.


  -h, --help                                      Print this help message.

Examples:

  %s                       

          No options specified. use default starter file to produce all system register files.

  %s --system_registers

          Output only the system registers file.

  %s --system_registers_starter input/system_registers_starter_v10.xml -m register_changes/hypervisor_reg_updates.py

          A starter file (input/system_registers_starter_v10.xml) has been specified. Use same to produce all system register files.

          A modifications script (register_changes/hypervisor_reg_updates.py). Modifications from this script will be applied to
          system registers and/or system register choices and/or register field choices before creating output files.

""" % (sys.argv[0], sys.argv[0], sys.argv[0], sys.argv[0])
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

def output_system_registers(aTree, aOutputFile, aPrefixLicenseFile, aSize):
    #print("\t  Processing system registers...")
    
    registers = aTree.findall('.//register')

    output_root = ET.Element('registers')
    physical_registers = ET.Element('physical_registers')
    register_file = ET.Element('register_file')
    register_file.set('name', 'RISC-V Registers')

    for register in registers:
        generate_register(register, physical_registers, register_file, aSize)

    output_root.append(physical_registers)
    output_root.append(register_file)

    # check system register fields before writing out xml file...
    reg_file = RISCV.RegisterFile( { 'system_tree' : output_root } )

    reg_file.checkRegisterFieldSize()

    pretty_print_xml(aOutputFile, output_root, aPrefixLicenseFile)

    #print("\t  Done.")

def output_system_register_choices(aTree, aOutputFile, aPrefixLicenseFile):
    #print("\t  Processing system register choices...")

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
            choice.set('weight', register.get('choice_weight', '0'))

            if 'R' in register.get('privilege'):
                read.append(choice)
            if 'W' in register.get('privilege'):
                write.append(choice)

    output_root.append(read)
    output_root.append(write)

    # check system register fields before writing out xml file...
    reg_file = RISCV.RegisterFile( { 'system_tree' : output_root } )
    reg_file.checkRegisterFieldSize()

    pretty_print_xml(aOutputFile, output_root, aPrefixLicenseFile)

    #print("\t  Done.")
    
def output_register_field_choices(aTree, aOutputFile, aPrefixLicenseFile):
    #print("\t  Processing register field choices...")
    
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

    # check system register fields before writing out xml file...
    reg_file = RISCV.RegisterFile( { 'system_tree' : output_root } )
    reg_file.checkRegisterFieldSize()

    pretty_print_xml(aOutputFile, output_root, aPrefixLicenseFile)

    #print("\t  Done.")

    
def pretty_print_xml(aOutputFile, aOutputRoot, aPrefixLicenseFile):
    xml_str = DOM.parseString(ET.tostring(aOutputRoot)).toprettyxml(indent='  ')
    with open(aOutputFile, 'w') as f:
        if aPrefixLicenseFile:
            f.write(license_string)
        f.write(xml_str)

def generate_register(aRegister, aPhysicalRegisters, aRegisterFile, aSize):
    register = ET.Element('register')
    register.set('index', aRegister.get('index'))
    register.set('name', aRegister.get('name'))
    register.set('size', aSize if str(aRegister.get('size')) == '0' else aRegister.get('size'))
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
        physical_register.set('size', aSize if str(aRegister.get('size')) == '0' else aRegister.get('size'))
        physical_register.set('type', aRegister.get('type'))
        if aRegister.get('class'):
            physical_register.set('class', aRegister.get('class'))
        aPhysicalRegisters.append(physical_register)

    aRegisterFile.append(register)

# prefix the filename portion of a file path with a specified prefix.
# will ASSUME the filepath is correct

def insert_filename_prefix(aFilePath,aFilePrefix):
    head_tail = os.path.split(aFilePath)
    if len(head_tail) == 1:
        edited_filepath = "%s%s" % (aFilePrefix,head_tail[1])
    else:
        edited_filepath = "%s/%s%s" % (head_tail[0],aFilePrefix,head_tail[1])
    return (aFilePath,edited_filepath)


def build_registers(aXmlStarterFile='input/system_registers_starter_rv64.xml', aSystemRegistersFile='output/system_registers_rv64..xml',
                    aSystemRegisterChoicesFile='output/system_register_choices_rv64..xml', aRegisterFieldChoicesFile='output/register_field_choices_rv64..xml',
                    aSize = '64', aModificationsScript=None):

    xml_starter_file             = aXmlStarterFile
    system_registers_file        = aSystemRegistersFile
    system_register_choices_file = aSystemRegisterChoicesFile
    register_field_choices_file  = aRegisterFieldChoicesFile

    modifications_script         = aModificationsScript
    
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hSRFi:s:r:f:m:', [ 'help', 'system_registers', 'system_register_choices', 'register_field_choices',
                                                              'system_registers_starter=', 'system_registers_file=', 'system_register_choices_file=', 'register_field_choices_file=',
                                                              'modifications_script=' ] )
    except getopt.GetoptError as error:
        print(error)
        usage()
        sys.exit(1)

    output_all = True
    system_registers = False
    system_register_choices = False
    register_field_choices = False

    prefix_license_file = True
    do_mods = False
    
    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()
        if o in [ '-S', '--system_registers' ]:
            system_registers = True
            output_all = False
        elif o in [ '-m', '--modifications_script' ]:
            modifications_script = a
            print("\tRegister/field/choices modifications script: '%s'" % modifications_script)
            prefix_license_file = False
            do_mods = True
        elif o in [ '-R', '--system_register_choices' ]:
            system_register_choices = True
            output_all = False
        elif o in [ '-F', '--register_field_choices' ]:
            register_field_choices = True
            output_all = False
        elif o in [ '-i', '--system_registers_starter' ]:
            xml_starter_file = a
            print("\tUser-specified system registers starter file: '%s'" % xml_starter_file)
        elif o in [ '-s', '--system_registers_file' ]:
            system_registers_file = a
            print("\tUser-specified system registers file: '%s'" % system_registers_file)
        elif o in [ '-r', '--system_register_choices_file' ]:
            system_register_choices_file = a
            print("\tUser-specified system register choices file: '%s'" % system_register_choices_file)
        elif o in [ 'f', '--register_field_choices_file' ]:
            register_field_choices_file = a
            print("\tUser-specified register field choices file: '%s'" % register_field_choices_file)
        else:
            print('!!! Unsupported option: %s, Script aborted.' % o)
            usage()
            sys.exit(1)

    print("\tInput (starter) file: %s" % xml_starter_file)

    systemRegisterDefinitions = ET.parse(xml_starter_file)

    src_mod_files  = {}
    dest_mod_files = {}
    
    if do_mods:
        # prior to running the modifications process, set src/dest files for the mods...
        (post_mod_system_registers_file, system_registers_file)               = insert_filename_prefix(system_registers_file,'pre_edits_')
        (post_mod_system_register_choices_file, system_register_choices_file) = insert_filename_prefix(system_register_choices_file,'pre_edits_')
        (post_mod_register_field_choices_file, register_field_choices_file)   = insert_filename_prefix(register_field_choices_file,'pre_edits_')

    try:
        if output_all or system_registers:
            output_system_registers(systemRegisterDefinitions, system_registers_file, prefix_license_file, aSize)
            if do_mods:
                print("\tIntermediate system register definitions file: %s" % system_registers_file)
                src_mod_files['system']  = system_registers_file
                dest_mod_files['system'] = post_mod_system_registers_file
            else:
                print("\tSystem register definitions written to: %s\n" % system_registers_file)
        if output_all or system_register_choices:
            output_system_register_choices(systemRegisterDefinitions, system_register_choices_file, prefix_license_file)
            if do_mods:
                print("\tIntermediate system register choices written to: %s" % system_register_choices_file)
                src_mod_files['register_choices']  = system_register_choices_file
                dest_mod_files['register_choices'] = post_mod_system_register_choices_file
            else:
                print("\tSystem register choices written to: %s\n" % system_register_choices_file)
        if output_all or register_field_choices:
            output_register_field_choices(systemRegisterDefinitions, register_field_choices_file, prefix_license_file)
            if do_mods:
                print("\tIntermediate register field choices written to: %s" % register_field_choices_file)
                src_mod_files['field_choices']  = register_field_choices_file
                dest_mod_files['field_choices'] = post_mod_register_field_choices_file
            else:
                print("\tRegister field choices written to: %s\n" % register_field_choices_file)
    except ValueError as ex:
        print("\nERROR:",ex)
        print("\n---> Aborting register builder. Correct errors in starter file; try again.\n")
        sys.exit(1)


    if do_mods:
        # run the modifications script...
        print('')
        register_files = Files()
        register_files.addFiles(src_mod_files, modifications_script)
        register_files.modify()
        register_files.save(dest_mod_files, license_string)
        print("\n\tFinal output files: ",dest_mod_files)
        
if __name__ == '__main__':
    build_registers()
    build_registers(aXmlStarterFile='input/system_registers_starter_rv32.xml', aSystemRegistersFile='output/system_registers_rv32.xml',
                    aSystemRegisterChoicesFile='output/system_register_choices_rv32.xml', aRegisterFieldChoicesFile='output/register_field_choices_rv32.xml', 
                    aSize = '32', aModificationsScript=None)

