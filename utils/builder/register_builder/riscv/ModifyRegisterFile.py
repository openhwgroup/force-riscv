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

#**************************************************************************************************
# ModifyRegisterFile.py
#
# This file reads existing register XML files and modifies the specified ones with data from the
# provided input file.
#**************************************************************************************************

import argparse
import RiscVRegDef as RISCV
import os

#**************************************************************************************************
# The files class handles assigning each of the parsed arguments to a variable designated for it.
#**************************************************************************************************
class Files:
    ## Constructor defines the various files and populates them with information from the arguments
    #  provided
    def __init__(self, aArguments):
        self.mInput = {}

        (self.mSystemRegisterFile, self.mSystemFlag) = self.setupFile(aArguments.system, 'system')
        (self.mAppRegisterFile, self.mAppFlag) = self.setupFile(aArguments.app, 'app')
        (self.mImplRegisterFile, self.mImplFlag) = self.setupFile(aArguments.impl, 'impl')
        (self.mRegisterChoicesFile, _) = self.setupFile(aArguments.register_choices,
            'register_choices')
        (self.mFieldChoicesFile, _) = self.setupFile(aArguments.field_choices, 'field_choices')

        self.mRegisterChangeFile = aArguments.data
        if self.mRegisterChangeFile:
            print('Using \'%s\' as register change file' % self.mRegisterChangeFile)

        self.mRegisterFile = RISCV.RegisterFile(self.mInput)

    ## The setup method takes in a file and input string and returns a tuple consisting of the
    #  associated file variable and flag or throws an error if the file provided is invalid
    def setupFile(self, aFile, aInput):
        flag = aFile is not None
        if flag and not os.path.isfile(aFile):
            raise Exception('Register file \'%s\' does not exist' % aFile)
        else:
            print('Modifying register file \'%s\'' % aFile)
        self.mInput[aInput] = aFile
        return (aFile, flag)

    ## Initializes the use of the register change file (by importing it as the provided name)
    def useModification(self):
        if '.py' in self.mRegisterChangeFile:
            self.mRegisterChangeFile = self.mRegisterChangeFile.replace('.py', '')

        exec('import register_changes.' + self.mRegisterChangeFile + ' as data', globals())

    ## Propagates (unsaved) modifications throughout the register files and validates register
    #  field size after propagation
    def modify(self):
        self.useModification()

        if hasattr(data, 'new_registers'):
            for register in data.new_registers:
                self.mRegisterFile.addRegisterFromFile(register)
        if hasattr(data, 'changed_registers'):
            for field in data.changed_registers:
                self.mRegisterFile.updateFieldsFromFile(field)
        if hasattr(data, 'delete_registers'):
            for register in data.delete_registers:
                self.mRegisterFile.deleteRegisterFromFile(register)
        if hasattr(data, 'new_physical_registers'):
            for physical_register in data.new_physical_registers:
                self.mRegisterFile.addPhysicalRegisterFromFile(physical_register)
        if hasattr(data, 'delete_register_choices'):
            for register_choice in data.delete_register_choices:
                self.mRegisterFile.deleteRegisterChoiceFromFile(register_choice)
        if hasattr(data, 'copy_registers'):
            for register in data.copy_registers:
                self.mRegisterFile.copyRegisterFromFile(register)
        if hasattr(data, 'changed_reg_attr'):
            for attribute in data.changed_reg_attr:
                self.mRegisterFile.updateRegisterAttributeFromFile(attribute)
        if hasattr(data, 'changed_physical_reg_attr'):
            for attribute in data.changed_physical_reg_attr:
                self.mRegisterFile.updatePhysicalRegisterAttributeFromFile(attribute)
        if hasattr(data, 'new_field_choices'):
            for choice in data.new_field_choices:
                self.mRegisterFile.updateFieldChoices(choice)
        if hasattr(data, 'choices_weight_changed'):
            for weight in data.choices_weight_changed:
                self.mRegisterFile.updateFieldWeight(weight)

        self.mRegisterFile.checkRegisterFieldSize()

    ## Saves current register trees to their files
    def save(self):
        if self.mSystemFlag:
            self.mRegisterFile.outputRiscVRegisterFileFromTree(self.mSystemRegisterFile)
        if self.mAppFlag:
            self.mRegisterFile.outputAppRegisterFileFromTree(self.mAppRegisterFile)
        if self.mImplFlag:
            self.mRegisterFile.outputImplRegisterFileFromTree(self.mImplRegisterFile)
        if self.mRegisterChoicesFile:
            self.mRegisterFile.outputRiscVRegisterChoicesFile(self.mRegisterChoicesFile)
        if self.mFieldChoicesFile:
            self.mRegisterFile.outputRiscVRegisterFieldChoicesFile(self.mFieldChoicesFile)

## Parsing arguments and modifying files according to the modification file
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Processes file location arguments')

    parser.add_argument('--system', help='Path to existing system register XML file')
    parser.add_argument('--app', help='Path to existing app register XML file')
    parser.add_argument('--impl', help='Path to existing implementation defined register XML file')
    parser.add_argument('--register_choices', help='Path to existing register choices XML file')
    parser.add_argument('--field_choices', help='Path to existing field choices XML file')
    parser.add_argument('--data', help='Path to modification file for already defined registers')

    arguments = parser.parse_args()

    register_files = Files(arguments)
    if register_files.mRegisterChangeFile:
        register_files.modify()
    register_files.save()

