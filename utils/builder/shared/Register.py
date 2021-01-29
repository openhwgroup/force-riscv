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
#**************************************************************************************************
# RegisterDefinition.py
#
# This file defines the register and its associated register and bit fields and also handles
# reading from and writing to XML files.
#**************************************************************************************************

import sys
import os
import argparse
import re
import defusedxml.defusedxml.ElementTree as ET
#import boot_priority
import copy
#import registers_allow_writing as RW
#import registers_allow_reading as RR
#import update_register_class_reset as UR

#**************************************************************************************************
# A bit field has a size (# of bits) and a shift (# of bits to shift the bit-field left from the
# least significant bit).
#**************************************************************************************************
class BitField:
    ## Constructor takes size and shift
    def __init__(self, aSize, aShift):
        self.mSize = aSize
        self.mShift = aShift

    ## Prints the size and shift to stdout
    def print(self):
        print('\t  size:', self.mSize, 'shift:', self.mShift)

    ## Returns size
    def size(self):
        return self.mSize

    ## Returns shift
    def shift(self):
        return self.mShift

    ## Return a string that is a XML file segment
    def toXmlString(self, aIndent):
        xml_str = aIndent + "<bit_field shift=\"%d\" size=\"%d\"/>\n" % (self.mShift, self.mSize)
        return xml_str

#**************************************************************************************************
# A field value has a value and description of that value.
#**************************************************************************************************
class FieldValue:
    ## Constructor takes value and description
    def __init__(self, aValue, aDescription):
        self.mValue = aValue
        self.mDescription = aDescription

    ## Prints value and description to stdout
    def print(self):
        print('\t  fieldValue:', self.mValue, 'description:', self.mDescription)

    ## Returns value
    def value(self):
        return self.mValue

    ## Returns description
    def description(self):
        return self.mDescription

#**************************************************************************************************
# A register field has a name and is composed of one or more bit fields.
#**************************************************************************************************
class RegisterField:
    ## Constructor takes a name and an optional ElementTree argument
    def __init__(self, aName, aFieldEt = None):
        self.mName = aName
        self.mBitFields = []
        self.mFieldvalues = []
        self.mPhysicalRegister = None
        self.mShift = None
        if aFieldEt is not None:
            self.parse(aFieldEt)

    ## Sets register field shift
    def setShift(self, aShift):
        self.mShift = aShift

    ## Returns shift
    def shift(self):
        return self.mShift

    ## Returns name
    def name(self):
        return self.mName

    ## Sets physical register
    def setPhysicalRegister(self, aPhysicalRegister):
        self.mPhysicalRegister = aPhysicalRegister

    ## Returns physical register
    def physicalRegister(self):
        return self.mPhysicalRegister

    ## Parses ElementTree to populate register field
    def parse(self, aFieldEt):
        msb = None
        lsb = None
        for child in aFieldEt:
            if child.tag is 'name':
                self.mName = child.text
            elif child.tag is 'msb':
                msb = int(child.text)
            elif child.tag is 'lsb':
                lsb = int(child.text)
            elif child.tag is 'values':
                self.parseValues(child)
            else:
                print('Unrecognized tag in field:', child.tag)

        if self.mName is None:
            raise ValueError('Register definition missing field name')
        if msb is None or lsb is None:
            self.print()
            raise ValueError('Did not parse register field \'%s\' correctly' % self.mName)

        self.addBitField(msb, lsb)

    ## Parses values from ElementTree child
    def parseValues(self, aValues):
        for value_instance in aValues.findall('value_instance'):
            for child in value_instance:
                if child.tag is 'value':
                    value = child.text
                elif child.tag is 'description':
                    description = child.text

            field_value = FieldValue(value, description)
            self.mFieldValues.append(field_value)

    ## Adds bit field explicitly by specifying most and least significant bits
    def addBitField(self, aMsb, aLsb):
        size = aMsb - aLsb + 1
        bit_field = BitField(size, lsb)
        self.mBitFields.append(bit_field)
        return bit_field

    ## Returns bit fields as size, shift pairs in a list
    def bitFields(self):
        bit_fields = []
        for bit_field in self.mBitFields:
            bit_fields.append((bit_field.size(), bit_field.shift()))
        return bit_fields

    ## Merge existing register fields
    def merge(self, aRegisterField):
        for bit_field in aRegisterField.bit_fields:
            self.mBitFields.append(bit_field)

    ## Prints register fields and bit fields to stdout
    def print(self):
        print('\tname:', self.mName)
        for bit_field in self.mBitFields:
            bit_field.print()
        for field_value in self.mFieldValues:
            field_value.print()

    ## Returns size of register field in bits
    def size(self):
        size = 0
        for bit_field in self.mBitFields:
            size += bit_field.size()
        return size

    def toXmlString(self, aIndent):
        xml_str = aIndent + "<register_field name=\"%s\" physical_register=\"%s\" size=\"%d\">\n" % (self.name(), self.physicalRegister(), self.size())

        # Add bit field information here.
        for bit_fld in self.mBitFields:
            xml_str += bit_fld.toXmlString(aIndent + "  ")
        
        xml_str += aIndent + "</register_field>\n"

        return xml_str

#**************************************************************************************************
# A physical register has a name, size (# of bits), index, reset, physical register type, and
# physical register class.
#**************************************************************************************************
class PhysicalRegister:
    ## Constructor takes in a name and defaults the other variables
    def __init__(self, aName):
        self.mName = aName
        self.mSize = None
        self.mIndex = None
        self.mReset = None
        self.mSubIndex = None
        self.mPhysicalType = None
        self.mPhysicalClass = None
        
    ## Sets size, index, reset, type, and class from a logical register
    def setAttributesFromLogicalRegister(self, aLogicalRegister):
        self.mSize = aLogicalRegister.physicalLength()
        self.mReset = aLogicalRegister.reset()
        self.mIndex = aLogicalRegister.index()
        if self.mPhysicalType is None:
            self.mPhysicalType = aLogicalRegister.physicalType()
        if self.mPhysicalClass is None:
            self.mPhysicalClass = aLogicalRegister.physicalClass()

    ## Returns all physical register attributes as a dictionary
    def getAttributes(self):
        attributes = {'name':self.mName}
        if self.mSize is not None:
            attributes['size'] = self.mSize
        if self.mIndex is not None:
            attributes['index'] = self.mIndex
        if self.mReset is not None:
            attributes['reset'] = self.mReset
        if self.mSubIndex is not None:
            attributes['sub_index'] = self.mSubIndex
        if self.mPhysicalType is not None:
            attributes['type'] = self.mPhysicalType
        if self.mPhysicalClass is not None:
            attributes['class'] = self.mPhysicalClass
        return attributes

    ## Create a PhysicalRegister object with all attributes provided
    @classmethod
    def createPhysicalRegister(self, aName, aSize, aIndex, aType, aClass = None, aReset = None, aSubIndex = None):
        phys_reg = PhysicalRegister(aName)
        phys_reg.mSize = aSize
        phys_reg.mIndex = aIndex
        phys_reg.mPhysicalType = aType
        phys_reg.mPhysicalClass = aClass
        phys_reg.mReset = aReset
        phys_reg.mSubIndex = aSubIndex
        return phys_reg

    ## Write out the PhysicalRegister as a XML file segment.
    def toXmlString(self, aIndent):
        xml_str = aIndent + "<physical_register name=\"%s\" index=\"%d\" size=\"%d\" type=\"%s\"" % (self.mName, self.mIndex, self.mSize, self.mPhysicalType)

        if self.mPhysicalClass is not None:
            xml_str += " class=\"%s\"" % self.mPhysicalClass

        if self.mReset is not None:
            xml_str += " reset=\"%d\"" % self.mReset

        if self.mSubIndex is not None:
            xml_str += " sub_index=\"%d\"" % self.mSubIndex

        xml_str += "/>\n"
            
        return xml_str

#**************************************************************************************************
# A physical register file contains a dictionary of existing physical registers.
#**************************************************************************************************
class PhysicalRegisterFile:
    ## Constructor defines a(n empty) dictionary of physical registers
    def __init__(self):
        self.mPhysicalRegisters = { }
        self.mOutputOrder = []

    ## Returns a dictionary of physical registers
    def physicalRegisters(self):
        return self.mPhysicalRegisters

    ## Add a physical register
    def addPhysicalRegisterInOrder(self, aPhysRegister):
        self.mPhysicalRegisters[aPhysRegister.mName] = aPhysRegister
        self.mOutputOrder.append(aPhysRegister.mName)

    ## Returns a physical register corresponding to the name provided
    def getPhysicalRegister(self, aName):
        if aName in self.mPhysicalRegisters:
            return self.mPhysicalRegisters[aName]
        else:
            new_physical_register = PhysicalRegister(aName)
            self.mPhysicalRegisters[aName] = new_physical_register
            return new_physical_register

    ## Write out physical registers as a XML file segment
    def toXmlString(self, aIndent):
        xml_str = aIndent + "<physical_registers>\n"

        for phys_reg_name in self.mOutputOrder:
            phys_reg = self.mPhysicalRegisters[phys_reg_name]
            xml_str += phys_reg.toXmlString(aIndent + "  ")
        
        xml_str += aIndent + "</physical_registers>\n"
        return xml_str
        
#**************************************************************************************************
# A register has a name, length, index, boot priority, register type, register class, and a set of
# register fields. It also contains privilege and description fields along with other attributes
# that are defined via keyword arguments in the constructor like below:
#
#   register = Register(name='example', length=64, boot=1)
#**************************************************************************************************
class Register:
    ## Constructor defines an empty dictionary of register fields and takes in a variable amount of
    #  keyword arguments that are initialized if defined as attributes
    def __init__(self, **kwargs):
        self.mName = kwargs.get('name', '')
        #self.mPhysicalName = kwargs.get('physical_name', '')       #TODO: Do these physical fields
        self.mIndex = kwargs.get('index', None)                     # need to be defined for RISCV?
        self.mLength = kwargs.get('length', '')                     # If so, add the required
        #self.mPhysicalLength = kwargs.get('physical_length', None) # functionality for each.
        self.mBoot = kwargs.get('boot', 0)
        self.mType = kwargs.get('type', None)
        #self.mPhysicalType = kwargs.get('physical_type', None)
        self.mClass = kwargs.get('class', None)
        #self.mPhysicalClass = kwargs.get('physical_class', None)
        self.mPrivilege = kwargs.get('privilege', None)
        self.mDescription = kwargs.get('description', '')
        self.mRegisterFields = {}
        self.mOutputOrder = []

    ## Returns register name
    def name(self):
        return self.mName

    ## Sets register name
    def setName(self, aName):
        self.mName = aName

    ## Returns register index
    def index(self):
        return self.mIndex

    ## Returns register length
    def length(self):
        return self.mLength

    ## Sets register length
    def setLength(self, aLength):
        self.mLength = aLength

    ## Returns register boot priority
    def boot(self):
        return self.mBoot

    ## Sets register boot priority
    def setBoot(self, aBoot):
        self.mBoot = aBoot

    ## Returns register type
    def type(self):
        return self.mType

    ## Sets register type
    def setType(self, aType):
        self.mType = aType

    ## Returns register class
    def registerClass(self):
        return self.mClass

    ## Returns register privilege
    def privilege(self):
        return self.mPrivilege

    ## Sets register privilege
    def setPrivilege(self, aPrivilege):
        self.mPrivilege = aPrivilege

    ## Returns register description
    def description(self):
        return self.mDescription

    ## Sets register description
    def setDescription(self, aDescription):
        self.mDescription = aDescription

    ## Returns the dictionary of each register field for this register
    def registerFields(self):
        return self.mRegisterFields

    ## Returns total size of the register by adding together the sizes of all the register fields
    def size(self):
        size = 0
        for register_field in self.mRegisterFields:
            size += self.mRegisterFields[register_field].size()
        return size

    ## Returns a register field according to the provided name or raises an exception if name does
    #  not exist
    def getRegisterField(self, aRegisterFieldName):
        if aRegisterFieldName in self.mRegisterFields:
            return self.mRegisterFields[aRegisterFieldName]
        else:
            raise KeyError('Register.getRegisterField():Register field \'%s\' not found'
                % aRegisterFieldName)

    ## Add a register field in order, record the field name in the ordering list
    #
    def addRegisterFieldInOrder(self, aRegisterField):
        self.mRegisterFields[aRegisterField.mName] = aRegisterField
        self.mOutputOrder.append(aRegisterField.mName)
        
    ## Adds a new register field with the provided name and returns the new field (or the existing
    #  register field if the name already exists)
    def addRegisterField(self, aRegisterFieldName):
        if aRegisterFieldName not in self.mRegisterFields:
            self.mRegisterFields[aRegisterFieldName] = RegisterField(aRegisterFieldName)
        return self.mRegisterFields[aRegisterFieldName]

    ## Deletes a register field according to the provided name or raises an exception if name does
    #  not exist
    def deleteRegisterField(self, aRegisterFieldName):
        if aRegisterFieldName in self.mRegisterFields:
            del self.mRegisterFields[aRegisterFieldName]
        else:
            raise KeyError('Register.deleteRegisterField(): Register field \'%s\' not found'
                % aRegisterFieldName)

    ## Return register structural information as an XML segment string.
    def toXmlString(self, aIndent):
        xml_str = aIndent + "<register name=\"%s\" index=\"%d\" size=\"%d\" type=\"%s\" boot=\"0x%x\"" % (self.mName, self.mIndex, self.mLength, self.mType, self.mBoot)

        if self.mClass is not None:
            xml_str += " class=\"%s\"" % self.mClass
        xml_str += ">\n"

        # add register field information here.
        for key, reg_fld in sorted(self.mRegisterFields.items()):
            xml_str += reg_fld.toXmlString(aIndent + "  ")
        
        xml_str += aIndent + "</register>\n"
        return xml_str

#**************************************************************************************************
# A register file contains a dictionary of registers.
#**************************************************************************************************
class RegisterFile:
    ## Constructor defines a dictionary of registers and can take in an XML file with predefined
    #  registers (and parse them into the dictionary)
    def __init__(self, aXmlFile = None):
        self.mName = None
        self.mRegisters = {}
        self.mImplementations = {}
        self.mSystemTree = None
        self.mApplicationTree = None
        self.mImplementationTree = None
        self.mRegisterChoicesTree = None
        self.mFieldChoicesTree = None
        self.mOutputOrder = []

        if aXmlFile and len(aXmlFile) > 0:
            self.parseXmlFile(aXmlFile)

    ## Parses the XML file provided to the constructor
    def parseXmlFile(self, aXmlFile):
        for key in aXmlFile:
            if key is 'system':
                self.mSystemTree = ET.parse(aXmlFile[key])
            elif key is 'app':
                self.mApplicationTree = ET.parse(aXmlFile[key])
            elif key is 'impl':
                self.mImplementationTree = ET.parse(aXmlFile[key])
            elif key is 'register_choices':
                self.mRegisterChoicesTree = ET.parse(aXmlFile[key])
            elif key is 'field_choices':
                self.mFieldChoicesTree = ET.parse(aXmlFile[key])
            else:
                print('Unable to initiate RegisterFile with XML file type: \'%s\'' % key)

    ## Adds a register to registers dictionary and to implementation dictionary if implementation
    #  is defined
    def addRegister(self, aRegister):
        self.mRegisters[aRegister.name()] = aRegister
        if 'IMPLEMENTATION DEFINED' in aRegister.registerFields():
            self.mImplementations[aRegister.name()] = aRegister

    ## Add a register and record its name in output order list
    def addRegisterInOrder(self, aRegister):
        self.mRegisters[aRegister.name()] = aRegister
        self.mOutputOrder.append(aRegister.name())

    ## Returns a register according to the provided name or raises an exception if name does not
    #  exist
    def getRegister(self, aRegisterName):
        if aRegisterName in self.mRegisters:
            return self.mRegisters[aRegisterName]
        else:
            raise KeyError('RegisterFile.getRegister(): Register \'%s\' not found' % aRegisterName)

    ## Deletes a register (and associated register in the implementation dictionary) according to
    #  the provided name or raises an exception if the register does not exist
    def deleteRegister(self, aRegisterName):
        if aRegisterName in self.mImplementations:
            del self.mImplementations[aRegisterName]
        if aRegisterName in self.mRegisters:
            del self.mRegisters[aRegisterName]
        else:
            raise KeyError('RegisterFile.deleteRegister(): Register \'%s\' not found'
                % aRegisterName)

    ## Return a XML segment string containing structural information for all the registers.
    def toXmlString(self, aIndent):
        xml_str = aIndent + "<register_file name=\"%s\">\n" % self.mName

        for reg_name in self.mOutputOrder:
            reg_instance = self.getRegister(reg_name)
            xml_str += reg_instance.toXmlString(aIndent + "  ")
        
        xml_str += aIndent + "</register_file>\n"
        return xml_str

#****************************************************************************************************
# A register document container class that contains both the physical registers and the RegisterFile.
# This container can also write out the XML file.
#****************************************************************************************************     
class RegistersDocument(object):

    LICENSE_STRING = """<!--
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

    def __init__(self, aRegisterFileName):
        self.mPhysicalRegisters = PhysicalRegisterFile()
        self.mRegisterFile = RegisterFile()
        self.mRegisterFile.mName = aRegisterFileName

    ## Add a PhysicalRegister
    def addPhysicalRegister(self, aPhysRegister):
        self.mPhysicalRegisters.addPhysicalRegisterInOrder(aPhysRegister)

    ## Add a Register
    def addRegister(self, aRegister):
        self.mRegisterFile.addRegisterInOrder(aRegister)

    ## Write out the physical register and register contents into an XML file
    def writeXmlFile(self, aFileName):
        with open(aFileName, 'w') as f:
            f.write(self.LICENSE_STRING)
            f.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
            f.write("<registers>\n")

            f.write(self.mPhysicalRegisters.toXmlString("  "))

            f.write(self.mRegisterFile.toXmlString("  "))

            f.write("</registers>")
