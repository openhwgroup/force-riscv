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

# Introduction to the Register Builder script, with register modification script option.

    NOTICE: The modification script functionality has been merged into the register builder script.

    The register builder is used to create system register files. The starting point is the system registers 'starter' file (default is input/system_registers_starter.xml),
    which is maintained by manual editing.
    
    In addition to the starter file, a modification script may be used to edit in changes, to the system register files produced from processing the starter file.

Files:

    register_builder.py - Read the starter XML file, output necessary register files and choices files, apply modifications.
    
    The register builder script relies on the following files:

    	BootPriority.py - Contains a structure (containing an if statement) that selects proper boot priority (via name or type)
    	ModifyRegisterFile.py - Controls the command line input for modifying the register file
    	RiscVRegDef.py - Actual register definition file used to modify registers, register choices, and field choices

    	register_changes/ - Location for all register change modification files. At this time, all register modification scripts must reside in the register_changes sub-directory.

How to run modification scripts:

    A modification script may be specified as an command line option to the register_builder.py script. Use './register_builder -h' to view usage, including how to specify a
    modification script.


How to use modification scripts:

    In general, the modification scripts are just predefined Python lists of dictionaries of the things that need to be modified. Example implementations are listed below.

    1) To add new registers:
        new_registers = [{'target':'register_tree',     #Required: target tree; can be system, app, impl, or choices (for register choices)
                          'register':'register_name',   #Required: name of register
                          'size':64,                    #Required: size of register
                          'physical_register':'physical_register_name', #Required: name of associated physical register; adds physical register if it doesn't exist yet
                          'index':'0x123',              #Required: index of the register in hex
                          'fields':[{'field':'field_name_1', 'shift':0, 'size':16},     #Required: list of fields, at least one field is required
                                    {'field':'field_name_2', 'shift':16, 'size':48}],   #field: name of field, shift: offset, size: size of bit field
                          'choice':{'name':'register_name','value':'0x123','weight':10,'description':'description of register choice'}}] #Optional: register choice; adds corresponding register choice to the register choice tree

    2) To update existing registers:
        changed_registers = [{'target':'register_tree',     #Required: target tree; can be system, app, impl, or choices (for register choices)
                              'register':'register_name',   #Required: name of register
                              'size':64,                    #Optional: size of register
                              'physical_register':'physical_register_name', #Optional: name of associated physical register; adds physical register if it doesn't exist yet
                              'index':'0x123',              #Optional: index of the register in hex
                              'fields':[{'field':'field_name_1', 'shift':0, 'size':16},     #Optional: list of fields, at least one field is required
                                        {'field':'field_name_2', 'shift':16, 'size':48}],   #field: name of field, shift: offset, size: size of bit field

    3) To delete registers:
        delete_registers = [{'target':'register_tree',      #Required: target tree; can be system, app, impl, or choices (for register choices)
                             'register':'register_name'}]   #Required: name of register

    4) To add new physical registers:
        new_physical_registers = [{'target':'register_tree',    #Required: target tree; can be system, app, impl, or choices (for register choices)
                                   'physical_register':'physical_register_name',    #Required: name of physical register
                                   'register':'register_name',  #Optional: alias for name of physical register
                                   'size':64,                   #Required: size of register
                                   'index':'0x123',             #Required: index of register
                                   'reset':0,                   #Optional: register reset value
                                   'type':'register_type'}]     #Optional: type of register

    5) To delete register choices:
        delete_register_choices = [{'name':'register_choice_name'}] #Required: name of register choice

List of current issues:

    1) The following modification features are implemented but have not been thoroughly tested: copy registers, changing specific register attributes, changing specific physical register attributes, adding new field choices, and updating field choice weights.

    2) The current version of the implementation of boot priority is a giant if statement. There are commented out lines denoting how to change a boot priority inside BootPriority.py, but in the future, this implementation should be redesigned to just use a table or prior/default definition as both of those would be less of a mess to deal with.

    3) Register field choices don't have proper descriptions as of yet (they all say "P L A C E H O L D E R").

