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

import copy

from shared.builder_utils import bit_string_to_list
from shared.instruction_file import *
from shared.instruction_file_parser import *


def change_to_index(bits_str_lst):
    res_lst = list()
    for str in bits_str_lst:
        num_lst = str.split("-")
        num_lst.sort()
        if len(num_lst) > 1:
            idx_range = range(int(num_lst[0]), int(num_lst[-1]) + 1)
            res_lst.extend(idx_range)
        elif len(num_lst) == 1:
            res_lst.append(int(num_lst[0]))
    res_lst.sort()
    return res_lst


def set_bit_lst_val(bit_lst, bit_idxs, bit_vals):
    for key in bit_idxs:
        vals = bit_vals[key]
        idxs_lst = bit_idxs[key].split(",")
        vals_lst = list()
        for char in vals:
            if char == ",":
                continue
            else:
                vals_lst.append(char)
        i = 0
        for idx in idxs_lst:
            val = vals_lst[i]
            bit_lst[int(idx)] = int(val)
            i = i + 1


def merge_instr_const_operand(instr, opr_data):
    opr = instr.get_const_operand()
    constx_bits = instr.constxBits
    if opr is None:
        return
    bits = opr.bits
    value = opr.value
    if bits is None:
        return
    bit_lst = [-1 for n in range(129)]

    set_bit_lst_val(bit_lst, constx_bits, opr_data)

    bits_str_lst = bits.split(",")
    bits_idx_list = change_to_index(bits_str_lst)
    if len(bits_idx_list) != len(value):
        raise Exception("const operand bits error")
    idx = 0
    for char in value[::-1]:
        bit_idx = bits_idx_list[idx]
        if char == "0":
            bit_lst[bit_idx] = 0
        elif char == "1":
            bit_lst[bit_idx] = 1
        idx = idx + 1

    idx = len(bit_lst) - 1
    new_bit_lst = list()
    new_value_lst = list()
    for var in bit_lst[::-1]:
        if var != -1:
            new_bit_lst.append("%d" % idx)
            new_value_lst.append(var)
        else:
            if len(new_bit_lst) and new_bit_lst[-1] != ",":
                new_bit_lst.append(",")
        idx = idx - 1

    new_bit = ""
    new_value = ""

    str_tmp = " ".join(new_bit_lst)
    str_tmp = str_tmp.strip(",")
    bits_section = str_tmp.split(",")
    for str in bits_section:
        num_str = str.strip().split(" ")
        if len(num_str) > 1:
            if len(new_bit):
                new_bit = new_bit + "," + num_str[0] + "-" + num_str[-1]
            else:
                new_bit = num_str[0] + "-" + num_str[-1]
        else:
            if len(new_bit):
                new_bit = new_bit + "," + num_str[0]
            else:
                new_bit = num_str[0]

    for var in new_value_lst:
        new_value = new_value + "%s" % var

    opr.bits = new_bit
    opr.value = new_value


def set_opr_attr(opr, opr_data):
    if opr_data is not None:
        replace_attribute_value(opr.attributes, opr_data)
    for attr_key in opr.attributes.keys():
        name = attr_key
        value = opr.attributes[attr_key]
        opr.set_attribute(name, value)


def instruction_operand_iterator(instr):
    for opr in instr.operands:
        if isinstance(opr, GroupOperand) and len(opr.operands):
            for opr_sub in opr.operands:
                yield opr_sub
        yield opr


def replace_attribute_value(attr, data):
    for key in attr.keys():
        value = attr[key]
        if key == "ops":
            idx = 0
            for opr_x in value:
                value_x = opr_x
                for data_key in data.keys():
                    data_val = data[data_key]
                    key_str = "%(" + data_key + ")s"
                    value_x = value_x.replace(key_str, data_val)
                    value[idx] = value_x
                idx = idx + 1
        else:
            for data_key in data.keys():
                data_val = data[data_key]
                key_str = "%(" + data_key + ")s"
                value = value.replace(key_str, data_val)
                attr[key] = value


def merge_instr_operand(instr, opr_data=None):
    for opr in instruction_operand_iterator(instr):
        set_opr_attr(opr, opr_data)
    if instr.constOp and instr.constOp.bits is None:
        set_opr_attr(instr.constOp, opr_data)

    if opr_data is not None:
        replace_attribute_value(instr.asm.attributes, opr_data)
    for key in instr.asm.attributes.keys():
        value = instr.asm.attributes[key]
        if key == "format":
            setattr(instr.asm, key, value)
        elif key == "ops":
            for opr_x in value:
                instr.asm.ops.append(opr_x)


def merge_instr_attribute(instr, data=None):
    if data is not None:
        replace_attribute_value(instr.attributes, data)

    for key in instr.attributes.keys():
        value = instr.attributes[key]
        setattr(instr, key, value)


def split_names_values(inst_info):
    names_lst = inst_info[::2]
    values_lst = inst_info[1::2]
    return names_lst, values_lst


def arrange_operand_values(names, values):
    const_opr_name_val_lst = list()
    opr_name_val_lst = list()
    for const_val in values.keys():
        opr_val = values[const_val]
        if isinstance(opr_val, str):
            opr_val = [values[const_val]]

        const_opr_name = None
        opr_names = None
        for key, value in names.items():
            const_opr_name = [key]
            if isinstance(value, str):
                opr_names = [value]
            else:
                opr_names = value

        const_opr_val = [const_val]

        # tmp_dict = dict()
        # tmp_dict[const_opr_name] = const_opr_val
        const_opr_name_val = dict(zip(const_opr_name, const_opr_val))
        const_opr_name_val_lst.append(const_opr_name_val)

        opr_name_val = dict(zip(opr_names, opr_val))
        opr_name_val_lst.append(opr_name_val)
    return const_opr_name_val_lst, opr_name_val_lst


def arrange_item(val_lst1, val_lst2):
    res_lst = list()
    for val in val_lst1:

        for val2 in val_lst2:

            arrange_val = dict()
            for key in val:
                arrange_val[key] = val[key]
            for key2 in val2:
                arrange_val[key2] = val2[key2]
            res_lst.append(arrange_val)
    return res_lst


# Arranges the elements in inst_info
def differentiate_attr_info(inst_info):
    names_lst, values_lst = split_names_values(inst_info)
    index = 0

    res_const_lst = list()
    res_name_val_lst = list()

    for names in names_lst:
        values = values_lst[index]
        const_opr_name_val_lst, opr_name_val_lst = arrange_operand_values(
            names, values
        )
        if len(res_const_lst):
            last_const_opr_name_item = res_const_lst[-1]
            last_opr_name_val_item = res_name_val_lst[-1]
            arranged_const_opr_lst = arrange_item(
                const_opr_name_val_lst, last_const_opr_name_item
            )
            arranged_opr_lst = arrange_item(
                opr_name_val_lst, last_opr_name_val_item
            )

            res_const_lst.append(arranged_const_opr_lst)
            res_name_val_lst.append(arranged_opr_lst)
        else:
            res_const_lst.append(const_opr_name_val_lst)
            res_name_val_lst.append(opr_name_val_lst)

        index = index + 1
    return res_const_lst[-1], res_name_val_lst[-1]


# Reset the instruction's attribute, operands and asm
def merge_instr(instr, instr_info):
    const_attr_lst, opr_attr_lst = differentiate_attr_info(instr_info)
    instr_lst = list()
    for attr_var in zip(const_attr_lst, opr_attr_lst):
        instr_x = copy.deepcopy(instr)
        const_attr = attr_var[0]
        opr_attr = attr_var[1]
        merge_instr_const_operand(instr_x, const_attr)
        merge_instr_operand(instr_x, opr_attr)
        merge_instr_attribute(instr_x, opr_attr)
        if instr_x.name == "TLBI":
            print(const_attr, opr_attr)
            if instr_x.form == "outer shareable":

                op1_opr = instr_x.find_operand("tlbi_os")
                op1_opr.width = len(bit_string_to_list(op1_opr.bits))
                instr_x.set_operand_attribute(
                    "tlbi_os", "width", op1_opr.width
                )

                build_tlbi_os_instructions(instr_x, op1_opr, instr_lst)
            elif instr_x.form == "range":
                op1_opr = instr_x.find_operand("tlbi_rg")
                op1_opr.width = len(bit_string_to_list(op1_opr.bits))
                instr_x.set_operand_attribute(
                    "tlbi_rg", "width", op1_opr.width
                )

                build_tlbi_rg_instructions(instr_x, op1_opr, instr_lst)
            else:
                raise Exception(
                    "Didn't know the choices operand of TLBI instruction."
                )

        instr_lst.append(instr_x)

    return instr_lst


def build_tlbi_os_instructions(instr, op1_opr, extra_instrs):
    instr.form = "VMALLE1OS"
    instr.asm.ops.remove("tlbi_os")

    extra_ops = {
        "VAE1OS": "0000001001",
        "ASIDE1OS": "0000001010",
        "VAAE1OS": "0000001011",
        "VALE1OS": "0000001101",
        "VAALE1OS": "0000001111",
        "IPAS2E1OS": "1000100000",
        "IPAS2LE1OS": "1000100100",
        "VAE2OS": "1000001001",
        "VALE2OS": "1000001101",
        "VMALLS12E1OS": "1000001110",
        "VAE3OS": "1100001001",
        "VALE3OS": "1100001101",
        "ALLE2OS": "1000001000",
        "ALLE1OS": "1000001100",
        "ALLE3OS": "1100001000",
    }
    for (key, value) in extra_ops.items():
        extra_instr = copy.deepcopy(instr)
        extra_instr.form = key
        extra_opr = copy.deepcopy(op1_opr)
        extra_opr.value = value
        # extra_opr.width = len(value)
        extra_instr.change_operand(extra_opr, True)  # merge with the const
        extra_instr.asm.format = "TLBI %s" % key + " %s"
        extra_instrs.append(extra_instr)

    op1_opr.value = "0000001000"
    instr.change_operand(op1_opr, True)  # merge with the const
    instr.asm.format = "TLBI VMALLE1OS %s"


def build_tlbi_rg_instructions(instr, op1_opr, extra_instrs):
    instr.form = "RVAE1"
    instr.asm.ops.remove("tlbi_rg")

    extra_ops = {
        "RVAAE1": "0000110011",
        "RVALE1": "0000110101",
        "RVAALE1": "0000110111",
        "RVAE1IS": "0000010001",
        "RVAAE1IS": "0000010011",
        "RVALE1IS": "0000010101",
        "RVAALE1IS": "0000010111",
        "RVAE1OS": "0000101001",
        "RVAAE1OS": "0000101011",
        "RVALE1OS": "0000101101",
        "RVAALE1OS": "0000101111",
        "RIPAS2E1IS": "1000000010",
        "RIPAS2LE1IS": "1000000110",
        "RIPAS2E1": "1000100010",
        "RIPAS2LE1": "1000100110",
        "RIPAS2E1OS": "1000100011",
        "RIPAS2LE1OS": "1000100111",
        "RVAE2": "1000110001",
        "RVALE2": "1000110101",
        "RVAE2IS": "1000010001",
        "RVALE2IS": "1000010101",
        "RVAE2OS": "1000101001",
        "RVALE2OS": "1000101101",
        "RVAE3": "1100110001",
        "RVALE3": "1100110101",
        "RVAE3IS": "1100010001",
        "RVALE3IS": "1100010101",
        "RVAE3OS": "1100101001",
        "RVALE3OS": "1100101101",
    }
    for (key, value) in extra_ops.items():
        extra_instr = copy.deepcopy(instr)
        extra_instr.form = key
        extra_opr = copy.deepcopy(op1_opr)
        extra_opr.value = value
        extra_instr.change_operand(extra_opr, True)  # merge with the const
        extra_instr.asm.format = "TLBI %s" % key + " %s"
        extra_instrs.append(extra_instr)

    op1_opr.value = "0000110001"
    instr.change_operand(op1_opr, True)  # merge with the const
    instr.asm.format = "TLBI RVAE1 %s"


# Arranges the elements in instr_info_val and sets them into the corresponding
# instruction base, and finalylly returns the set of instructions.
def differentiate_instruction(instr_val, instr_info_val=None):
    instr_lst = []
    if instr_info_val:
        if isinstance(instr_val, list):
            idx = 0
            for instr in instr_val:
                instr_info = instr_info_val[idx]
                instr_res = merge_instr(instr, instr_info)
                instr_lst.extend(instr_res)
                idx = idx + 1
        else:
            instr_res = merge_instr(instr_val, instr_info_val)
            instr_lst.extend(instr_res)
    else:
        merge_instr_operand(instr_val)
        merge_instr_attribute(instr_val)
        instr_lst.append(instr_val)
    return instr_lst


# Traverse the instruction file to find the instruction which has the same
# instruction name, form, isa as instr_name_form_isa.
# For example: instruction_instance = get_referenced_instruction(
# instruction_file_instance, "ADD##RISCV")
def get_referenced_instruction(instr_file, instr_name_form_isa):
    attr_lst = instr_name_form_isa.split("#")
    instr_form = None
    instr_isa = None
    if len(attr_lst) == 0:
        raise Exception(
            "Get referenced instruction error.Input parameter "
            "'instr_name_form_isa' format error."
        )
        return
    if len(attr_lst) == 3:
        isa_str = attr_lst[2].strip()
        if len(isa_str):
            instr_isa = attr_lst[2]
    if len(attr_lst) >= 2:
        form_str = attr_lst[1].strip()
        if len(form_str):
            instr_form = attr_lst[1]
    instr_name = attr_lst[0]

    for instr in instr_file.instruction_iterator():
        if instr_name == instr.name:
            if instr_form and instr.form.lower() == instr_form.lower():
                if (
                        instr_isa and instr.isa.lower() == instr_isa.lower()
                ) or instr_isa is None:
                    return instr
            elif (
                    instr_form is None
                    and instr_isa
                    and instr.isa.lower() == instr_isa.lower()
            ):
                return instr
    raise Exception("The instruction not found:", instr_name_form_isa)
    return None


# parse a xml instruction file and return the instruction file instance
def parse_instruction_file(instr_xml_file_path):
    instr_file = InstructionFile()
    file_parser = InstructionFileParser(instr_file)
    file_parser.parse(instr_xml_file_path)
    return instr_file


def write_file(instr_file, out_file_name):
    file_handle = open(out_file_name, "w")
    instr_file.write(file_handle)
    file_handle.close()


# set instruction attribute
def set_instr_attribute(instr, instr_attr_val):
    for name in instr_attr_val.keys():
        val = instr_attr_val[name]
        setattr(instr, name, val)


# set operand's attribute of instruction
def set_instr_opr(instr, opr_name, opr_val):
    opr_attr_name = None
    opr_attr_val = None
    for key in opr_val.keys():
        name = key
        val = opr_val[key]

        if name == "name":
            opr_attr_name = name
            opr_attr_val = val
        else:
            instr.set_operand_attribute(opr_name, name, val)
    if opr_attr_val is not None:
        instr.set_operand_attribute(opr_name, opr_attr_name, opr_attr_val)


# Set asm attribute of instruction
def set_instr_asm(instr, asm_val):
    asm_op_idx_lst = list()

    for key in asm_val.keys():
        name = key
        val = asm_val[key]

        if name == "format":
            instr.rename_asm_attribute(name, val)
            continue
        else:
            asm_op_idx_lst.append(name)

    asm_op_idx_lst.sort()
    for op_name in asm_op_idx_lst:
        val = asm_val[op_name]
        res = instr.rename_asm_attribute(op_name, val)
        if res is False:
            instr.append_asm_op(val)


# According to the instruction_name#form#isa to find instruction in
# referenced_xml_file_path and reset the instruction's attribute according to
# instr_info. Return a instruction file instance.
def differentiate_referenced_instruction(instr_info, referenced_xml_file_path):
    instrs_file_referenced = parse_instruction_file(referenced_xml_file_path)
    instr_file_res = InstructionFile()

    for key in instr_info.keys():

        referenced_instr_name = key
        var_item = instr_info[key]
        instr_attr_val = None
        instr_opr_val = None
        if len(var_item) == 2:
            instr_attr_val = var_item[0]
            instr_opr_val = var_item[1]
        else:
            instr_opr_val = var_item[0]

        instr = copy.deepcopy(
            get_referenced_instruction(
                instrs_file_referenced, referenced_instr_name
            )
        )
        print("referenced instrution:\n")
        print(instr.to_string())

        if instr_attr_val:
            set_instr_attribute(instr, instr_attr_val)

        for key in instr_opr_val.keys():
            opr_name = key
            opr_val = instr_opr_val[key]
            if opr_name == "asm":
                set_instr_asm(instr, opr_val)
            else:
                set_instr_opr(instr, opr_name, opr_val)
        instr_file_res.add_instruction(instr)
        print("changed instrution:\n")
        print(instr.to_string())
    return instr_file_res
