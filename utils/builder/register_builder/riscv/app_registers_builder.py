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

import sys

from Register import *


def create_app_register(aName, aSize, aIndex, aType, aClass, aBoot, aAltPhysReg=None):
    new_reg = Register(
        **{
            "name": aName,
            "length": aSize,
            "index": aIndex,
            "type": aType,
            "class": aClass,
            "boot": aBoot,
        }
    )
    new_field = RegisterField(aName)

    if aAltPhysReg is not None:
        new_field.mPhysicalRegister = aAltPhysReg
    else:
        new_field.mPhysicalRegister = aName

    new_bit_field = BitField(aSize, 0)
    new_field.mBitFields.append(new_bit_field)
    new_reg.addRegisterFieldInOrder(new_field)
    return new_reg


def create_double_precision_app_register(aIndex, aRegSize):

    d_reg = Register(
        **{
            "name": "D%d" % aIndex,
            "length": 64,
            "index": aIndex,
            "type": "FPR",
            "class": None,
            "boot": 0x3000,
        }
    )
    numberOfReg = 64 // aRegSize
    for i in range(0, numberOfReg):
        if numberOfReg > 1:
            d_field = RegisterField("D%d_%d" % (aIndex, i))
        else:
            d_field = RegisterField("D%d" % aIndex)
        d_field.mPhysicalRegister = "f%d_%d" % (aIndex, i)
        d_bfield = BitField(aRegSize, 0)
        d_field.mBitFields.append(d_bfield)
        d_reg.addRegisterFieldInOrder(d_field)

    return d_reg


def create_quad_precision_app_register(aIndex, aRegSize):
    q_reg = Register(
        **{
            "name": "Q%d" % aIndex,
            "length": 128,
            "index": aIndex,
            "type": "FPR",
            "class": "LargeRegister",
            "boot": 0,
        }
    )
    numberOfReg = 128 // aRegSize
    for i in range(0, numberOfReg):
        q_field = RegisterField("Q%d_%d" % (aIndex, i))
        q_field.mPhysicalRegister = "f%d_%d" % (aIndex, i)
        q_bfield = BitField(aRegSize, 0)
        q_field.mBitFields.append(q_bfield)
        q_reg.addRegisterFieldInOrder(q_field)

    return q_reg


def create_vector_app_register(aIndex, aRegSize):
    v_reg = Register(
        **{
            "name": "v%d" % aIndex,
            "length": 512,
            "index": aIndex,
            "type": "VECREG",
            "class": "LargeRegister",
            "boot": 0x3000,
        }
    )
    numberOfReg = 512 // aRegSize
    for i in range(0, numberOfReg):
        v_field = RegisterField("v%d_%d" % (aIndex, i))
        v_field.mPhysicalRegister = "v%d_%d" % (aIndex, i)
        v_bfield = BitField(aRegSize, 0)
        v_field.mBitFields.append(v_bfield)
        v_reg.addRegisterFieldInOrder(v_field)

    return v_reg


def build_app_registers(a_app_registers_file, a_xlen):

    for size in (32, 64):
        app_register_doc = RegistersDocument("RISC-V %s-bit Registers" % size)

        # Add x1-x31 registers
        for i in range(1, 32):
            app_register_doc.addPhysicalRegister(
                PhysicalRegister.createPhysicalRegister("x%d" % i, size, i, "GPR")
            )
            app_register_doc.addRegister(
                create_app_register("x%d" % i, size, i, "GPR", None, 0x1000)
            )

        # Add zero register
        app_register_doc.addPhysicalRegister(
            PhysicalRegister.createPhysicalRegister(
                "x0", size, 0, "GPR", "PhysicalRegisterRazwi", 0
            )
        )
        app_register_doc.addRegister(
            create_app_register("x0", size, 0, "ZR", "ReadOnlyZeroRegister", 0, "x0")
        )

        # Add PC register
        app_register_doc.addPhysicalRegister(
            PhysicalRegister.createPhysicalRegister("PC", size, 32, "PC")
        )
        app_register_doc.addRegister(create_app_register("PC", size, 32, "PC", None, 0))

        # number of sub-registers needed for quad FP 128-bit registers
        numberOfFpSubReg = 2

        # Add f0-f31 and S0-S31
        for i in range(0, 32):
            for subIndex in range(numberOfFpSubReg):
                app_register_doc.addPhysicalRegister(
                    PhysicalRegister.createPhysicalRegister(
                        "f%d_%d" % (i, subIndex),
                        64,
                        i,
                        "FPR",
                        aSubIndex=subIndex,
                    )
                )

            app_register_doc.addRegister(
                create_app_register("S%d" % i, 32, i, "FPR", None, 0, "f%d_0" % i)
            )

        # Add H0-H31
        for i in range(0, 32):
            app_register_doc.addRegister(
                create_app_register("H%d" % i, 16, i, "FPR", None, 0, "f%d_0" % i)
            )

        # Add D0-D31
        for i in range(0, 32):
            app_register_doc.addRegister(create_double_precision_app_register(i, 64))

        # Add Q0-Q31
        for i in range(0, 32):
            app_register_doc.addRegister(create_quad_precision_app_register(i, 64))

        # number of sub-registers needed for vector 512-bit registers
        numberOfVecSubReg = 8

        # add vector registers V0-V31
        for i in range(0, 32):
            for subIndex in range(numberOfVecSubReg):
                app_register_doc.addPhysicalRegister(
                    PhysicalRegister.createPhysicalRegister(
                        "v%d_%d" % (i, subIndex),
                        64,
                        i,
                        "VECREG",
                        aSubIndex=subIndex,
                    )
                )

            app_register_doc.addRegister(create_vector_app_register(i, 64))

        app_register_doc.writeXmlFile("output/app_registers_rv%s.xml" % size)


if __name__ == "__main__":
    app_registers_file = "output/app_registers.xml"
    xlen = 64

    if len(sys.argv) > 1:
        if sys.argv[1] == "--xlen":
            try:
                xlen = int(sys.argv[2])
            except BaseException:
                print("ERROR: 'xlen' option value should be integer")
                sys.exit(1)
            if xlen == 32:
                app_registers_file = "output/app_registers_rv32.xml"
            elif xlen != 64:
                print("ERROR: Only valid 'xlen' values are 32 or 64")
                sys.exit(1)
        else:
            print("Only valid option is '--xlen N', where N is " "either 32 or 64")
            sys.exit(1)

    print("\tApp registers file: %s, xlen: %d" % (app_registers_file, xlen))
    build_app_registers(app_registers_file, xlen)
