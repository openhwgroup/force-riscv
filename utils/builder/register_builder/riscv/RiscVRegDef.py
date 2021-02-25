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
# RiscVRegDef.py
#
# This file defines the RISCV register and its associated register and bit
# fields and also handles reading from and writing to XML files.

import copy
import os

import defusedxml.defusedxml.ElementTree as ET
import defusedxml.defusedxml.minidom as DOM

from BootPriority import *


# A bit field has a size (# of bits) and a shift (# of bits to shift the
# bit-field left from the least significant bit).
class BitField:
    # Constructor takes size and shift
    def __init__(self, aSize, aShift):
        self.mSize = aSize
        self.mShift = aShift

    # Prints the size and shift to stdout
    def print(self):
        print("\t  size:", self.mSize, "shift:", self.mShift)

    # Returns size
    def size(self):
        return self.mSize

    # Returns shift
    def shift(self):
        return self.mShift


# A field value has a value and description of that value.
class FieldValue:
    # Constructor takes value and description
    def __init__(self, aValue, aDescription):
        self.mValue = aValue
        self.mDescription = aDescription

    # Prints value and description to stdout
    def print(self):
        print(
            "\t  fieldValue:", self.mValue, "description:", self.mDescription
        )

    # Returns value
    def value(self):
        return self.mValue

    # Returns description
    def description(self):
        return self.mDescription


# A register field has a name and is composed of one or more bit fields.
class RegisterField:
    # Constructor takes a name and an optional ElementTree argument
    def __init__(self, aName, aFieldEt=None):
        self.mName = aName
        self.mBitFields = []
        self.mFieldvalues = []
        self.mPhysicalRegister = None
        self.mShift = None
        if aFieldEt is not None:
            self.parse(aFieldEt)

    # Sets register field shift
    def setShift(self, aShift):
        self.mShift = aShift

    # Returns shift
    def shift(self):
        return self.mShift

    # Returns name
    def name(self):
        return self.mName

    # Sets physical register
    def setPhysicalRegister(self, aPhysicalRegister):
        self.mPhysicalRegister = aPhysicalRegister

    # Returns physical register
    def physicalRegister(self):
        return self.mPhysicalRegister

    # Parses ElementTree to populate register field
    def parse(self, aFieldEt):
        msb = None
        lsb = None
        for child in aFieldEt:
            if child.tag == "name":
                self.mName = child.text
            elif child.tag == "msb":
                msb = int(child.text)
            elif child.tag == "lsb":
                lsb = int(child.text)
            elif child.tag == "values":
                self.parseValues(child)
            else:
                print("Unrecognized tag in field:", child.tag)

        if self.mName is None:
            raise ValueError("Register definition missing field name")
        if msb is None or lsb is None:
            self.print()
            raise ValueError(
                "Did not parse register field '%s' correctly" % self.mName
            )

        self.addBitField(msb, lsb)

    # Parses values from ElementTree child
    def parseValues(self, aValues):
        for value_instance in aValues.findall("value_instance"):
            for child in value_instance:
                if child.tag == "value":
                    value = child.text
                elif child.tag == "description":
                    description = child.text

            field_value = FieldValue(value, description)
            self.mFieldValues.append(field_value)

    # Adds bit field explicitly by specifying most and least significant bits
    def addBitField(self, aMsb, aLsb):
        size = aMsb - aLsb + 1
        bit_field = BitField(size, lsb)
        self.mBitFields.append(bit_field)
        return bit_field

    # Returns bit fields as size, shift pairs in a list
    def bitFields(self):
        bit_fields = []
        for bit_field in self.mBitFields:
            bit_fields.append((bit_field.size(), bit_field.shift()))
        return bit_fields

    # Merge existing register fields
    def merge(self, aRegisterField):
        for bit_field in aRegisterField.bit_fields:
            self.mBitFields.append(bit_field)

    # Prints register fields and bit fields to stdout
    def print(self):
        print("\tname:", self.mName)
        for bit_field in self.mBitFields:
            bit_field.print()
        for field_value in self.mFieldValues:
            field_value.print()

    # Returns size of register field in bits
    def size(self):
        size = 0
        for bit_field in self.mBitFields:
            size += bit_field.size()
        return size


# A physical register has a name, size (# of bits), index, reset, physical
# register type, and physical register class.
class PhysicalRegister:
    # Constructor takes in a name and defaults the other variables
    def __init__(self, aName):
        self.mName = aName
        self.mSize = None
        self.mIndex = None
        self.mReset = None
        self.mPhysicalType = None
        self.mPhysicalClass = None

    # Sets size, index, reset, type, and class from a logical register
    def setAttributesFromLogicalRegister(self, aLogicalRegister):
        self.mSize = aLogicalRegister.physicalLength()
        self.mReset = aLogicalRegister.reset()
        self.mIndex = aLogicalRegister.index()
        if self.mPhysicalType is None:
            self.mPhysicalType = aLogicalRegister.physicalType()
        if self.mPhysicalClass is None:
            self.mPhysicalClass = aLogicalRegister.physicalClass()

    # Returns all physical register attributes as a dictionary
    def getAttributes(self):
        attributes = {"name": self.mName}
        if self.mSize is not None:
            attributes["size"] = self.mSize
        if self.mIndex is not None:
            attributes["index"] = self.mIndex
        if self.mReset is not None:
            attributes["reset"] = self.mReset
        if self.mPhysicalType is not None:
            attributes["type"] = self.mPhysicalType
        if self.mPhysicalClass is not None:
            attributes["class"] = self.mPhysicalClass
        return attributes


# A physical register file contains a dictionary of existing physical registers
class PhysicalRegisterFile:
    # Constructor defines a(n empty) dictionary of physical registers
    def __init__(self):
        self.mPhysicalRegisters = {}

    # Returns a dictionary of physical registers
    def physicalRegisters(self):
        return self.mPhysicalRegisters

    # Returns a physical register corresponding to the name provided
    def getPhysicalRegister(self, aName):
        if aName in self.mPhysicalRegisters:
            return self.mPhysicalRegisters[aName]
        else:
            new_physical_register = PhysicalRegister(aName)
            self.mPhysicalRegisters[aName] = new_physical_register
            return new_physical_register


# A register has a name, length, index, boot priority, register type, register
# class, and a set of register fields. It also contains privilege and
# description fields along with other attributes that are defined via keyword
# arguments in the constructor like below:
#
#   register = Register(name='example', length=64, boot=1)
class Register:
    # Constructor defines an empty dictionary of register fields and takes in
    # a variable amount of keyword arguments that are initialized if defined as
    # attributes
    def __init__(self, **kwargs):
        self.mName = kwargs.get("name", "")
        self.mSkipPhysical = kwargs.get("skip_physical", 0)

        self.mPhysicalName = kwargs.get("physical_name", "")
        self.mIndex = kwargs.get("index", None)
        self.mLength = kwargs.get("length", "")
        self.mPhysicalLength = kwargs.get("physical_length", None)
        self.mBoot = kwargs.get("boot", 0)
        self.mType = kwargs.get("type", None)
        self.mClass = kwargs.get("class", None)
        self.mPrivilege = kwargs.get("privilege", None)
        self.mDescription = kwargs.get("description", "")
        self.mRegisterFields = {}

    # Returns register name
    def name(self):
        return self.mName

    # Sets register name
    def setName(self, aName):
        self.mName = aName

    # Returns true if skip physical flag is set to anything other than 0,
    # false otherwise
    def skipPhysical(self):
        return self.mSkipPhysical == 1

    # Returns name of the physical register this register references
    def physicalName(self):
        return self.mPhysicalName

    # Sets name of the physical register this register references
    def setPhysicalName(self, aPhysicalName):
        self.mPhysicalName = aPhysicalName

    # Returns register index
    def index(self):
        return self.mIndex

    # Returns register length
    def length(self):
        return self.mLength

    # Sets register length
    def setLength(self, aLength):
        self.mLength = aLength

    # Returns length of the physical register this register references
    def physicalLength(self):
        if self.mPhysicalLength is None:
            return self.length()
        else:
            return self.mPhysicalLength

    # Sets length of the physical register this register references
    def setPhysicalLength(self, aPhysicalLength):
        self.mPhysicalLength = aPhysicalLength

    # Returns register boot priority
    def boot(self):
        return self.mBoot

    # Sets register boot priority
    def setBoot(self, aBoot):
        self.mBoot = aBoot

    # Returns register type
    def type(self):
        return self.mType

    # Sets register type
    def setType(self, aType):
        self.mType = aType

    # Returns register class
    def registerClass(self):
        return self.mClass

    # Returns register privilege
    def privilege(self):
        return self.mPrivilege

    # Sets register privilege
    def setPrivilege(self, aPrivilege):
        self.mPrivilege = aPrivilege

    # Returns register description
    def description(self):
        return self.mDescription

    # Sets register description
    def setDescription(self, aDescription):
        self.mDescription = aDescription

    # Returns the dictionary of each register field for this register
    def registerFields(self):
        return self.mRegisterFields

    # Returns total size of the register by adding together the sizes of all
    # the register fields
    def size(self):
        size = 0
        for register_field in self.mRegisterFields:
            size += self.mRegisterFields[register_field].size()
        return size

    # Returns a register field according to the provided name or raises an
    # exception if name does not exist
    def getRegisterField(self, aRegisterFieldName):
        if aRegisterFieldName in self.mRegisterFields:
            return self.mRegisterFields[aRegisterFieldName]
        else:
            raise KeyError(
                "Register.getRegisterField():"
                "Register field '%s' not found" % aRegisterFieldName
            )

    # Adds a new register field with the provided name and returns the new
    # field (or the existing register field if the name already exists)
    def addRegisterField(self, aRegisterFieldName):
        if aRegisterFieldName not in self.mRegisterFields:
            self.mRegisterFields[aRegisterFieldName] = RegisterField(
                aRegisterFieldName
            )
        return self.mRegisterFields[aRegisterFieldName]

    # Deletes a register field according to the provided name or raises an
    # exception if name does not exist
    def deleteRegisterField(self, aRegisterFieldName):
        if aRegisterFieldName in self.mRegisterFields:
            del self.mRegisterFields[aRegisterFieldName]
        else:
            raise KeyError(
                "Register.deleteRegisterField(): Register "
                "field '%s' not found" % aRegisterFieldName
            )


# A register file contains a dictionary of registers.
class RegisterFile:
    # Constructor defines a dictionary of registers and can take in an XML
    # file with predefined registers (and parse them into the dictionary)
    def __init__(self, aXmlFile=None):
        self.mRegisters = {}
        self.mImplementations = {}
        self.mSystemTree = None
        self.mApplicationTree = None
        self.mImplementationTree = None
        self.mRegisterChoicesTree = None
        self.mFieldChoicesTree = None

        self.mLicenseText = None

        if aXmlFile and len(aXmlFile) > 0:
            self.parseXmlFile(aXmlFile)

    # Parses the XML file provided to the constructor
    def parseXmlFile(self, aXmlFile):
        for key in aXmlFile:
            if aXmlFile[key] is not None:
                if key == "system":
                    self.mSystemTree = ET.parse(aXmlFile[key])
                elif key == "app":
                    self.mApplicationTree = ET.parse(aXmlFile[key])
                elif key == "impl":
                    self.mImplementationTree = ET.parse(aXmlFile[key])
                elif key == "register_choices":
                    self.mRegisterChoicesTree = ET.parse(aXmlFile[key])
                elif key == "field_choices":
                    self.mFieldChoicesTree = ET.parse(aXmlFile[key])
                elif key == "system_tree":
                    self.mSystemTree = aXmlFile[key]
                elif key == "app_tree":
                    self.mApplicationTree = aXmlFile[key]
                elif key == "impl_tree":
                    self.mImplementationTree = aXmlFile[key]
                elif key == "register_choices_tree":
                    self.mRegisterChoicesTree = aXmlFile[key]
                elif key == "field_choices_tree":
                    self.mFieldChoicesTree = aXmlFile[key]
                else:
                    print(
                        "Unable to initiate RegisterFile with XML file "
                        "type: '%s'" % key
                    )
            else:
                print("Key '%s' is not associated with a file" % key)

    # Adds a register to registers dictionary and to implementation dictionary
    # if implementation is defined
    def addRegister(self, aRegister):
        self.mRegisters[aRegister.name()] = aRegister
        if "IMPLEMENTATION DEFINED" in aRegister.registerFields():
            self.mImplementations[aRegister.name()] = aRegister

    # Returns a register according to the provided name or raises an exception
    # if name does not exist
    def getRegister(self, aRegisterName):
        if aRegisterName in self.mRegisters:
            return self.mRegisters[aRegisterName]
        else:
            raise KeyError(
                "RegisterFile.getRegister(): Register '%s' not "
                "found" % aRegisterName
            )

    # Deletes a register (and associated register in the implementation
    # dictionary) according to the provided name or raises an exception if the
    # register does not exist
    def deleteRegister(self, aRegisterName):
        if aRegisterName in self.mImplementations:
            del self.mImplementations[aRegisterName]
        if aRegisterName in self.mRegisters:
            del self.mRegisters[aRegisterName]
        else:
            raise KeyError(
                "RegisterFile.deleteRegister(): "
                "Register '%s' not found" % aRegisterName
            )

    # Validates a register's field sizes
    def checkRegisterFieldSize(self):
        system_tree = self.getTreeFromLabel("system")
        if system_tree is None:
            return
        self.checkSystemRegisterFieldSize(system_tree)

        impl_tree = self.getTreeFromLabel("impl")
        if impl_tree is None:
            return
        self.checkSystemRegisterFieldSize(impl_tree)

    # Validates an individual tree's sizes
    def checkSystemRegisterFieldSize(self, aTree):
        registers = aTree.findall(".//register")
        for register in registers:
            if register.attrib["boot"] != "0":
                self.checkPhysicalRegisterSize(register, aTree)
                size = register.attrib["size"]
                name = register.attrib["name"]

                sum_size = 0
                for field in register:
                    sum_size += int(field.attrib["size"])

                    sum_bit_size = 0
                    for bit in field:
                        sum_bit_size += int(bit.attrib["size"])

                    if sum_bit_size != int(field.attrib["size"]):
                        raise ValueError(
                            "Sum of bit sizes (%s bits) and field size "
                            "(%s bits) for field '%s' in register '%s' "
                            "are different"
                            % (
                                sum_bit_size,
                                field.attrib["size"],
                                field.attrib["name"],
                                register.attrib["name"],
                            )
                        )

                if sum_size != int(size):
                    raise ValueError(
                        "Sum of field sizes (%s) and register size (%s) for "
                        "register '%s' are different"
                        % (sum_size, size, register.attrib["name"])
                    )

    # Validates a register's size matches its physical size
    def checkPhysicalRegisterSize(self, aRegister, aTree):
        size = aRegister.attrib["size"]
        name = aRegister.attrib["name"]
        physical_registers = aTree.findall(".//physical_register")
        for physical_register in physical_registers:
            if name == physical_register.attrib["name"]:
                physical_register_size = physical_register.attrib["size"]
                if size != physical_register_size:
                    raise ValueError(
                        "Register '%s' size (%s bits) and physical size "
                        "(%s bits) are different"
                        % (name, size, physical_register_size)
                    )

    # Adds a register from a file
    def addRegisterFromFile(self, aRegister):
        tree = self.getTreeFromLabel(aRegister["target"])
        self.addRegisterToTree(tree, aRegister)

    # Returns tree from provided label
    def getTreeFromLabel(self, aLabel):
        if aLabel == "system":
            return self.mSystemTree
        elif aLabel == "app":
            return self.mApplicationTree
        elif aLabel == "impl":
            return self.mImplementationTree
        elif aLabel == "choices":
            return self.mRegisterChoicesTree

        print("RegisterFile.getTreeFromLabel(): Unknown label: %s" % aLabel)
        return None

    # Adds provided register to the specified tree
    def addRegisterToTree(self, aTree, aRegister):
        registers = aTree.findall(".//register")
        found_register = None
        for register in registers:
            if register.attrib["name"] == aRegister["register"]:
                found_register = register

        if found_register is not None:
            self.updateFieldsToTree(aTree, aRegister)
            if self.mRegisterChoicesTree is not None:
                self.mRegisterChoicesTree.findall("choices")
            else:
                choice_elements = None
            found_choice = None
            if choice_elements is not None:
                for choice in choice_elements[0].getchildren():
                    choice_name = choice.attrib["name"]
                    if choice_name == aRegister["register"]:
                        found_choice = choice
                        break

            if found_choice is None:
                self.addRegisterChoices(aRegister)
        else:
            self.addNewRegisterToTree(aTree, aRegister)
            if aRegister.get("choice") is not None:
                self.addRegisterChoices(aRegister)
            self.updateFieldsToTree(aTree, aRegister)

    # Updates or adds specified fields in the provided tree
    def updateFieldsToTree(self, aTree, aRegister):
        root = aTree.getroot()

        for child in root:
            if child.tag != "register_file":
                continue
            for register in child:
                if register.attrib["name"] != aRegister["register"]:
                    continue
                bit_fields_to_be_removed = []
                register_field_to_be_removed = []
                new_size = 0
                for register_field in register:
                    for bit_field in register_field:
                        new_mask = self.getMaskFromField(aRegister["fields"])
                        current_mask = self.getMaskFromBitField(bit_field)
                        if new_mask & current_mask != 0:
                            kept_mask = current_mask & ~new_mask
                            if kept_mask == 0:
                                bit_fields_to_be_removed.append(
                                    bit_field.attrib["shift"]
                                )
                            else:
                                new_ranges = self.getContinuousRanges(
                                    kept_mask
                                )
                                bit_field.attrib["shift"] = str(
                                    new_ranges[0][0]
                                )
                                bit_field.attrib["size"] = str(
                                    new_ranges[0][1]
                                )
                                register_field.attrib["size"] = str(
                                    new_ranges[0][1]
                                )
                                for i in range(1, len(new_ranges)):
                                    duplicate = copy.deepcopy(bit_field)
                                    duplicate.attrib["shift"] = str(
                                        new_ranges[i][0]
                                    )
                                    duplicate.attrib["size"] = str(
                                        new_ranges[i][1]
                                    )
                                    register_field.append(duplicate)

                    while len(bit_fields_to_be_removed) > 0:
                        for bit in register_field:
                            if bit.attrib["shift"] in bit_fields_to_be_removed:
                                register_field.remove(bit)
                                bit_fields_to_be_removed.remove(
                                    bit.attrib["shift"]
                                )
                                break

                    if len(register_field.getchildren()) is 0:
                        register_field_to_be_removed.append(register_field)
                    else:
                        register_field.attrib["size"] = str(
                            self.getFieldSizeFromBitsInRegisterField(
                                register_field
                            )
                        )

                while len(register_field_to_be_removed) > 0:
                    register_field = register_field_to_be_removed[0]
                    register.remove(register_field)
                    register_field_to_be_removed.remove(register_field)

                for register_field in aRegister["fields"]:
                    merge = False
                    for current_field in register:
                        shift = int(register_field["shift"])
                        size = int(register_field["size"])
                        shift_size = shift + size
                        current_shift = int(current_field[0].attrib["shift"])
                        current_size = int(current_field[0].attrib["size"])
                        current_shift_size = current_shift + current_size
                        if (
                            register_field["field"]
                            == current_field.attrib["name"]
                        ):
                            merge = True
                            if (
                                shift_size == current_shift
                                or current_shift_size == shift
                            ):
                                current_field[0].attrib["shift"] = str(
                                    min(current_shift, shift)
                                )
                                current_field[0].attrib["size"] = str(
                                    current_size + size
                                )
                                current_field.attrib["size"] = current_field[
                                    0
                                ].attrib["size"]
                            else:
                                original_bit_field = current_field[0]
                                new_bit_field = copy.deepcopy(
                                    original_bit_field
                                )
                                new_bit_field.attrib["shift"] = str(shift)
                                new_bit_field.attrib["size"] = str(size)
                                current_field.append(new_bit_field)

                        current_bit_size = 0
                        for bit_field in current_field:
                            current_bit_size += int(bit_field.attrib["size"])
                        current_field.attrib["size"] = str(current_bit_size)

                    new_size += register_field["size"]

                    if merge:
                        continue

                    physical_register = None
                    if len(register.getchildren()) > 0:
                        physical_register = register[0].attrib[
                            "physical_register"
                        ]
                    elif "physical_register" in aRegister:
                        physical_register = aRegister["physical_register"]
                    else:
                        physical_register = register.attrib["name"]

                    register_field_name = register_field["field"]
                    register_field_tag = register_field.get(
                        "field_type", "register_field"
                    )
                    register_field_attributes = {
                        "name": register_field_name,
                        "size": str(register_field["size"]),
                        "physical_register": physical_register,
                    }
                    if "reset" in register_field:
                        register_field_attributes["reset"] = str(
                            register_field["reset"]
                        )
                    if "class" in register_field:
                        register_field_attributes["class"] = register_field[
                            "class"
                        ]

                    register_field_element = ET.SubElement(
                        register, register_field_tag, register_field_attributes
                    )
                    register_field_element.tail = "\n"
                    bit_field_element = ET.SubElement(
                        register_field_element,
                        "bit_field",
                        {
                            "size": str(register_field["size"]),
                            "shift": str(register_field["shift"]),
                        },
                    )
                    bit_field_element.tail = "\n"

                register.attrib["size"] = str(new_size)
                register.attrib["boot"] = str(
                    BootPriority.getBootPriority(
                        register.attrib["name"], register.attrib.get("type"), 0
                    )
                )

                if "skip_physical" not in aRegister:
                    self.addPhysicalRegisterToTree(root, aRegister)

    # Returns the calculated size of a register field by adding up the total
    # sizes of all the bit fields
    def getFieldSizeFromBitsInRegisterField(self, aRegisterField):
        size = 0
        for bit_field in aRegisterField:
            size += int(bit.attrib["size"])
        return size

    # Returns a list of the provided mask's continuous ranges
    def getContinuousRanges(self, aMask):
        ranges = []
        start = False
        shift = 0
        size = 0
        for i in range(0, 64):
            if aMask & (1 << i):
                if not start:
                    start = True
                    shift = i
            else:
                if start:
                    start = False
                    size = i - shift
                    ranges.append([shift, size])
        if start:
            size = 64 - shift
            ranges.append([shift, size])
        return ranges

    # Returns mask from provided bit field
    def getMaskFromBitField(self, aBitField):
        return (
            1 << int(aBitField.attrib["shift"]) + int(aBitField.attrib["size"])
        ) - (1 << int(aBitField.attrib["shift"]))

    # Returns mask from provided fields
    def getMaskFromField(self, aFields):
        mask = 0
        for field in aFields:
            mask |= (1 << field["shift"] + field["size"]) - (
                1 << field["shift"]
            )
        return mask

    # Adds register choices
    def addRegisterChoices(self, aRegister):
        if "hasRead" in aRegister:
            self.addNewRegisterToRegisterChoices(aRegister, aWriteChoice=False)
        elif "hasWrite" in aRegister:
            self.addNewRegisterToRegisterChoices(aRegister, aReadChoice=False)
        else:
            self.addNewRegisterToRegisterChoices(aRegister)

    # Adds register choices based on read-only, write-only, or read-write
    def addNewRegisterToRegisterChoices(
        self, aRegister, aReadChoice=True, aWriteChoice=True
    ):
        register_type = aRegister.get("type", "SysReg")
        if register_type != "SysReg":
            print(
                "Not adding system register choices for register '%s' "
                "(type: '%s')" % (aRegister["register"], register_type)
            )
            return
        choices = self.mRegisterChoicesTree.findall("choices")
        if aReadChoice:
            ET.SubElement(choices[0], "choice", aRegister["choice"])
        if aWriteChoice:
            ET.SubElement(choices[1], "choice", aRegister["choice"])

    # Adds new register to the provided tree
    def addNewRegisterToTree(self, aTree, aRegister):
        root = aTree.getroot()
        name = aRegister["register"]
        size = aRegister["size"]
        index = aRegister["index"]
        register_class = aRegister.get("class", None)
        register_type = aRegister.get("type", "SysReg")
        if "physical_register" in aRegister:
            physical_register = aRegister["physical_register"]
        else:
            physical_register = aRegister["register"]

        skip = False
        if "skip_physical" in aRegister:
            skip = True
        if not skip:
            self.addPhysicalRegisterToTree(root, aRegister)

        register_file = root.find("register_file")
        if register_file is None:
            raise ValueError("There is no register file in the xml tree.")

        boot = 0
        if size != 0:
            boot = BootPriority.getBootPriority(name, register_type, 0)

        register_properties = {
            "index": index,
            "name": name,
            "size": "%d" % size,
            "boot": "%d" % boot,
            "type": register_type,
        }
        if register_class:
            register_properties["class"] = register_class
        if "init_policy" in aRegister:
            register_properties["init_policy"] = aRegister["init_policy"]

        ET.SubElement(register_file, "register", register_properties)

    # Deletes specified register
    def deleteRegisterFromFile(self, aRegister):
        tree = self.getTreeFromLabel(aRegister["target"])
        self.deleteRegisterFromTree(tree, aRegister)

    # Deletes specified register and associated register choices from
    # provided tree
    def deleteRegisterFromTree(self, aTree, aRegister):
        physical_registers = aTree.find("physical_registers")
        for physical_register in physical_registers.getchildren():
            if aRegister.get("register") == physical_register.attrib["name"]:
                physical_registers.remove(physical_register)

        register_file = aTree.find("register_file")
        for register in register_file.getchildren():
            if aRegister.get("register") == register.attrib["name"]:
                register_file.remove(register)

        choices = aTree.findall("choices")
        if choices:
            for choice in choices[0].getchildren():
                if aRegister.get("register") == choice.attrib["name"]:
                    choices[0].remove(choice)
            for choice in choices[1].getchildren():
                if aRegister.get("register") == choice.attrib["name"]:
                    choices[1].remove(choice)

    # Deletes specified register choice
    def deleteRegisterChoiceFromFile(self, aRegisterChoice):
        self.deleteRegisterChoice(aRegisterChoice)

    # Deletes specified register choice from the register choices tree
    def deleteRegisterChoice(self, aRegisterChoice):
        choices_trees = self.mRegisterChoicesTree.findall("choices")
        for choices in choices_trees:
            for choice in choices.getchildren():
                if aRegisterChoice.get("name") == choice.attrib["name"]:
                    choices.remove(choice)

    # Adds specified physical register from file
    def addPhysicalRegisterFromFile(self, aRegister):
        tree = self.getTreeFromLabel(aRegister["target"])
        self.addPhysicalRegisterToTree(tree, aRegister)

    # Adds specified physical register to the provided tree
    def addPhysicalRegisterToTree(self, aTree, aRegister):
        physical_registers = aTree.find("physical_registers")
        for physical_register in physical_registers.getchildren():
            name = physical_register.attrib["name"]
            if (
                aRegister.get("physical_register") == name
                or aRegister.get("register") == name
            ):
                return

        name = aRegister.get("physical_register")
        if name is None:
            name = aRegister.get("register")
        new_physical_register = ET.SubElement(
            physical_registers,
            "physical_register",
            {"name": str(name), "size": "%d" % aRegister["size"]},
        )
        new_physical_register.set("type", str(aRegister.get("type", "SysReg")))
        if "reset" in aRegister:
            new_physical_register.set("reset", "%d" % aRegister["reset"])
        if "index" in aRegister:
            new_physical_register.set("index", "%s" % aRegister["index"])

    # Copies specified register from file
    def copyRegisterFromFile(self, aRegister):
        tree = self.getTreeFromLabel(aRegister["target"])
        self.copyRegisterToTree(tree, aRegister)

    # Copies specified register into the provided tree
    def copyRegisterToTree(self, aTree, aRegister):
        register_file = tree.findall(".//register_file")
        found_register = None
        for register in register_file[0]:
            if register.attrib["name"] == aRegister["source"]:
                found_register = register
                break

        if found_register:
            copies = aRegister["copies"]
            for register_name in copies:
                copied_register = copy.deepcopy(found_register)
                copied_register.attrib["name"] = register_name
                register_file[0].append(copied_register)

    # Updates specified fields from file
    def updateFieldsFromFile(self, aFields):
        tree = self.getTreeFromLabel(aFields["target"])
        self.updateFieldsToTree(tree, aFields)

    # Updates register attributes from file
    def updateRegisterAttributeFromFile(self, aAttribute):
        tree = self.getTreeFromLabel(aAttribute["target"])
        self.updateRegisterAttributeToTree(tree, aAttribute)

    # Updates specified register attributes in the provided tree
    def updateRegisterAttributeToTree(self, aTree, aAttribute):
        root = aTree.getroot()

        for child in root:
            if child.tag == "register_file":
                for register in child:
                    if register.attrib["name"] == aAttribute["register"]:
                        for key, value in aAttribute.items():
                            if key == "physical_register":
                                for field in register.getchildren():
                                    field.attrib["physical_register"] = value
                            elif key != "register" and key != "target":
                                register.attrib[key] = value

    # Updates physical register attributes from file
    def updatePhysicalRegisterAttributeFromFile(self, aAttribute):
        tree = self.getTreeFromLabel(aAttribute["target"])
        self.updatePhysicalRegisterAttributeToTree(tree, aAttribute)

    # Updates specified physical register attributes in the provided tree
    def updatePhysicalRegisterAttributeToTree(self, aTree, aAttribute):
        root = aTree.getroot()

        for child in root:
            if child.tag == "physical_registers":
                for physical_register in child:
                    if (
                        physical_register.attrib["name"]
                        == aAttribute["register"]
                    ):
                        for key, value in aAttribute.items():
                            if key != "register" and key != "target":
                                physical_register.attrib[key] = value

    # Updates field choices
    def updateFieldChoices(self, aChoice):
        field_choices = self.mFieldChoicesTree.getroot()

        update_flag = False
        for field_choice in field_choices:
            if field_choice.attrib["name"] == aChoice["field_name"]:
                update_flag = True
                for choice in field_choice.getchildren():
                    field_choice.remove(choice)
                for choice in aChoice["choices"]:
                    weight = child.get("weight", 10)
                    ET.SubElement(
                        field_choice,
                        "choice",
                        {
                            "value": child["value"],
                            "description": child["description"],
                            "weight": weight,
                        },
                    )
                break

        if not update_flag:
            new_field_choice = ET.SubElement(
                field_choices,
                "choices",
                {"name": aChoice["field_name"], "type": "RegisterFieldValue"},
            )
            for choice in aChoice["choices"]:
                weight = choice.get("weight", 10)
            ET.SubElement(
                new_field_choice,
                "choice",
                {
                    "value": choice["value"],
                    "description": choice["description"],
                    "weight": weight,
                },
            )

    # Updates field weight
    def updateFieldWeight(self, aWeight):
        field_choices = self.mFieldChoicesTree.getroot()

        for field_choice in field_choices:
            if field_choice.attrib["name"] == aWeight["field_name"]:
                for choice in aWeight["choices"]:
                    for child in field_choice.getchildren():
                        if choice["value"] == child.attrib["value"]:
                            child.attrib["weight"] = choice["weight"]
                break

    # Writes register file from system register tree
    def outputRiscVRegisterFileFromTree(
        self, aSystemRegisterFile, aLicenseText=None
    ):
        self.mLicenseText = aLicenseText
        self.outputRiscVRegisterFile(aSystemRegisterFile, False, "system")

    # Generic RISCV register file writer
    def outputRiscVRegisterFile(self, aRegisterFile, aImpl=False, aLabel=None):
        if aLabel is not None:
            if aLabel == "system":
                self.prettyPrint(self.mSystemTree, "index", aRegisterFile)
            elif aLabel == "app":
                self.prettyPrint(self.mApplicationTree, "index", aRegisterFile)
            elif aLabel == "impl":
                self.prettyPrint(
                    self.mImplementationTree, "index", aRegisterFile
                )
            else:
                print(
                    "Label '%s' unrecognized while writing register "
                    "file" % aLabel
                )
        self.mLicenseText = None

    # Writes xml file with some internal formatting
    def prettyPrint(self, aTree, aAttribute, aOutput):
        self.sortTree(aTree.getroot(), aAttribute)
        xml_str = DOM.parseString(ET.tostring(aTree.getroot())).toprettyxml(
            indent="  "
        )
        # removing extra whitespace that gets added by toprettyxml()
        xml_str = os.linesep.join(
            [s for s in xml_str.splitlines() if s.strip()]
        )
        with open(aOutput, "w") as f:
            if self.mLicenseText is not None:
                f.write(self.mLicenseText)
            f.write(xml_str)

    # Sorts tree provided via root
    def sortTree(self, aRoot, aAttribute):
        if len(aRoot.getchildren()) > 0:
            self.sortChildrenByAttribute(aRoot, aAttribute)
            for child in aRoot:
                self.sortTree(child, aAttribute)

    # Sorts children of supplied parent node according to attribute
    def sortChildrenByAttribute(self, aParentNode, aAttribute):
        if (
            len(aParentNode.getchildren()) > 0
            and aAttribute in aParentNode[0].attrib
        ):
            aParentNode[:] = sorted(
                aParentNode,
                key=lambda child: self.getProperSortKey(child, aAttribute),
            )

    # Determines if the supplied attribute is a hex string or something else
    # (like a name)
    def getProperSortKey(self, aChild, aAttribute):
        try:
            return int(aChild.get(aAttribute), 16)
        except ValueError:
            return aChild.get(aAttribute)

    # Writes register file from application register tree
    def outputAppRegisterFileFromTree(
        self, aAppRegisterFile, aLicenseText=None
    ):
        self.mLicenseText = aLicenseText
        self.outputRiscVRegisterFile(aAppRegisterfile, False, "app")

    # Writes register file from implementation register tree
    def outputImplRegisterFileFromTree(
        self, aImplRegisterFile, aLicenseText=None
    ):
        self.mLicenseText = aLicenseText
        self.outputRiscVRegisterFile(aImplRegisterFile, True, "impl")

    # Writes register choices file
    def outputRiscVRegisterChoicesFile(
        self, aRegisterChoicesFile, aLicenseText=None
    ):
        if self.mRegisterChoicesTree is not None:
            self.mLicenseText = aLicenseText
            self.prettyPrint(
                self.mRegisterChoicesTree, "value", aRegisterChoicesFile
            )

    # Writes register field choices file
    def outputRiscVRegisterFieldChoicesFile(
        self, aRegisterFieldChoicesFile, aLicenseText=None
    ):
        if self.mFieldChoicesTree is not None:
            self.mLicenseText = aLicenseText
            self.prettyPrint(
                self.mFieldChoicesTree, "name", aRegisterFieldChoicesFile
            )
