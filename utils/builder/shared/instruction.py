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
from shared.builder_exception import BuilderException
from shared.builder_utils import (
    update_bits_value,
    merge_imm_value,
    bit_string_to_list,
    bit_list_to_string,
    update_bits,
)
import copy
import types


class Asm(object):
    def __init__(self):
        self.format = None
        self.ops = list()
        self.attributes = dict()

    def to_string(self, indent):
        ret_str = indent + '<asm format="%s"' % self.format

        op_index = 1
        for op in self.ops:
            ret_str += ' op%d="%s"' % (op_index, op)
            op_index += 1

        ret_str += "/>\n"
        return ret_str

    def find_op(self, op_name):
        for i in range(len(self.ops)):
            if self.ops[i] == op_name:
                return True

    def rename_op(self, old_name, new_name, multiple=False):
        found_any = False
        for i in range(len(self.ops)):
            if self.ops[i] == old_name:
                self.ops[i] = new_name
                if not multiple:
                    break
                else:
                    found_any = True
        else:
            if not found_any:
                raise BuilderException(
                    'Asm.rename_op: old op "%s" not found, ops are: %s.'
                    % (old_name, self.show_ops())
                )

    def show_ops(self):
        return "-".join(self.ops)


class Operand(object):
    def __init__(self):
        super(Operand, self).__init__()
        self.name = None
        self.ext = None
        self.bits = None
        self.value = None
        self.reserved = None
        self.type = None
        self.oclass = None
        self.access = None
        self.choices = None
        self.choices2 = None
        self.choices3 = None
        self.exclude = None
        self.differ = None
        self.slave = None
        self.layoutMultiple = None
        self.regCount = None
        self.regIndexAlignment = None
        self.elemWidth = None
        self.uop_param_type = None
        self.width = 0
        self.sizeType = None

    def update_type(self):
        if self.value:
            if (
                (len(self.value) == self.width)
                and (self.value.find("x") == -1)
                and (self.value.find("N") == -1)
            ):
                self.type = "Constant"
            elif self.value.find("(1)") != -1:
                self.name = "RES1" + self.name
                self.type = "Immediate"
                if self.value != "(1)(1)(1)(1)(1)":
                    print("Unhandled RES1 %s" % self.value)
                    sys.exit(1)
                self.value = None

    def element_close(self):
        return "/>\n"

    def to_string(self, indent):
        ret_str = "<O"
        if self.name:
            ret_str += ' name="%s"' % self.name
        if self.type:
            ret_str += ' type="%s"' % self.type
        if self.bits:
            ret_str += ' bits="%s"' % self.bits
        if self.value:
            ret_str += ' value="%s"' % self.value
        if self.reserved:
            ret_str += ' reserved="%s"' % self.reserved
        if self.access:
            ret_str += ' access="%s"' % self.access
        if self.choices:
            ret_str += ' choices="%s"' % self.choices
        if self.choices2:
            ret_str += ' choices2="%s"' % self.choices2
        if self.choices3:
            ret_str += ' choices3="%s"' % self.choices3
        if self.differ:
            ret_str += ' differ="%s"' % self.differ
        if self.oclass:
            ret_str += ' class="%s"' % self.oclass
        if self.slave:
            ret_str += ' slave="%s"' % self.slave
        if self.layoutMultiple:
            ret_str += ' layout-multiple="%s"' % self.layoutMultiple
        if self.regCount:
            ret_str += ' reg-count="%s"' % self.regCount
        if self.regIndexAlignment:
            ret_str += ' reg-index-alignment="%s"' % self.regIndexAlignment
        if self.elemWidth:
            ret_str += ' elem-width="%s"' % self.elemWidth
        if self.uop_param_type:
            ret_str += ' uop-param-type="%s"' % self.uop_param_type
        if self.exclude:
            ret_str += ' exclude="%s"' % self.exclude
        if self.sizeType:
            ret_str += ' sizeType="%s"' % self.sizeType
        if ret_str != "<O":
            ret_str = indent + ret_str + self.element_close()
        else:
            ret_str = ""
        return ret_str

    def add_value(self, val):
        if self.value:
            self.value += val
        else:
            self.value = val

    def merge_operand(self, opr, update_bits=False):
        if self.type != opr.type:
            raise BuilderException(
                'Merging different types of operand "%s"=>"%s" and "%s"=>"%s".'
                % (self.name, self.type, opr.name, opr.type)
            )

        self.bits += "," + opr.bits
        if self.value:
            self.value += opr.value
        else:
            self.value = opr.value

        self.width += opr.width
        if update_bits:
            self.bits = update_bits(self.bits)

    def update_bits_value(self):
        self.bits, self.value = update_bits_value(self.bits, self.value)

    def merge_value(self, opr):
        if self.width != opr.width:
            raise BuilderException(
                'Changing operand "%s" but width don\'t match: %d and %d.'
                % (opr.name, self.width, opr.width)
            )
        if self.type != opr.type:
            raise BuilderException(
                'Changing operand "%s" but type don\'t match: %s and %s.'
                % (opr.name, self.type, opr.type)
            )

        self.value = merge_imm_value(self.value, opr.value)

    def split_operand(self):
        split_const = copy.deepcopy(self)
        bit_list = bit_string_to_list(self.bits)
        if len(bit_list) != self.width:
            raise BuilderException(
                "Expecting bit list to be the same size as operand width."
            )

        const_bit_list = list()
        const_value = ""
        variable_bit_list = list()
        for i in range(self.width):
            if self.value[i] == "x":
                variable_bit_list.append(bit_list[i])
            else:
                const_bit_list.append(bit_list[i])
                const_value += self.value[i]

        self.bits = bit_list_to_string(variable_bit_list)
        self.width = len(variable_bit_list)
        self.value = None
        split_const.bits = bit_list_to_string(const_bit_list)
        split_const.width = len(const_bit_list)
        split_const.value = const_value
        split_const.update_type()
        return split_const

    def set_constatnt_bit(self, bit_loc, bit_value):
        if (self.value is None) or (len(self.value) == 0):
            self.value = "x" * self.width
        if bit_loc >= self.width:
            raise BuilderException(
                "set_constatnt_bit: bit location %d larger than operand "
                "width %d" % (bit_loc, self.width)
            )
        new_value = ""
        str_loc = self.width - bit_loc - 1
        for i in range(len(self.value)):
            if i == str_loc:
                new_value += bit_value
            else:
                new_value += self.value[i]
        self.value = new_value

    def set_extra_attribute(self, name, value):
        print(
            'WARNING: not handled operand attribute: %s="%s".' % (name, value)
        )
        raise Error

    def set_attribute(self, name, value):
        if name in [
            "name",
            "type",
            "bits",
            "value",
            "reserved",
            "access",
            "choices",
            "choices2",
            "choices3",
            "exclude",
            "differ",
            "ext",
            "slave",
            "layoutMultiple",
            "regCount",
            "regIndexAlignment",
        ]:
            setattr(self, name, value)
        elif name == "uop-param-type":
            self.uop_param_type = value
        elif name == "class":
            self.oclass = value
        elif name == "width":
            self.width = value
        elif name == "sizeType":
            self.sizeType = value
        else:
            self.set_extra_attribute(name, value)


class GroupOperand(Operand):
    def __init__(self):
        super(GroupOperand, self).__init__()

        self.operands = list()
        self.extra_attributes = dict()

    def add_operand(self, opr):
        self.operands.append(opr)

    def to_string(self, indent):
        ret_str = super(GroupOperand, self).to_string(indent)

        for key, value in sorted(self.extra_attributes.items()):
            ret_str += ' %s="%s"' % (key, value)

        ret_str += ">\n"

        for opr in self.operands:
            ret_str += opr.to_string(indent + "  ")

        ret_str += indent + "</O>\n"

        return ret_str

    def element_close(self):
        return ""

    def set_extra_attribute(self, attr_name, attr_value):
        self.extra_attributes[attr_name] = attr_value

    def find_operand(self, name, fail_not_found=True):
        for opr in self.operands:
            if opr.name == name:
                return opr

        if fail_not_found:
            raise BuilderException(
                'Operand "%s" not found in instruction "%s".'
                % (name, self.get_full_ID())
            )
        else:
            return None


class Instruction(object):
    def __init__(self):
        super(Instruction, self).__init__()

        self.name = None
        self.aliasing = None
        self.form = None
        self.isa = None
        self.constOp = None
        self.operands = list()
        self.iclass = None
        self.group = None
        self.asm = None
        self.extension = None
        self.constxBits = dict()
        self.attributes = dict()

    def get_full_ID(self):
        ret_str = self.name + "#"
        if self.form:
            ret_str += self.form
        ret_str += "#" + self.isa
        return ret_str

    def add_operand(self, opr):
        if opr.type == "Constant":
            if self.constOp:
                self.constOp.merge_operand(opr)
            else:
                self.constOp = opr
                self.constOp.name = "const_bits"
        else:
            self.operands.append(opr)

    def insert_operand(self, index, opr):
        self.operands.insert(index, opr)

    def find_operand(self, name, fail_not_found=True):
        for opr in self.operands:
            if opr.name == name:
                return opr
            if (
                name == "const_bits"
                and self.constOp
                and self.constOp.name == name
            ):
                return self.constOp

        if fail_not_found:
            raise BuilderException(
                'Operand "%s" not found in instruction "%s".'
                % (name, self.get_full_ID())
            )
        else:
            return None

    # Move an operand up in the operand list
    def move_up_operand(self, opr, num):
        op_index = self.operands.index(opr)
        new_index = op_index - num
        self.operands.remove(opr)
        self.operands.insert(new_index, opr)

    # Move an operand to the bottom in the operand list
    def move_bottom_operand(self, opr):
        self.operands.remove(opr)
        self.operands.append(opr)

    def remove_operand(self, aOprName):
        opr = self.find_operand(aOprName)
        self.operands.remove(opr)

    def change_operand(self, opr, update_const=False):
        if opr.type == "Constant":
            existing_opr = self.find_operand(opr.name)
            if existing_opr.width != opr.width:
                raise BuilderException(
                    'Changing operand "%s" but width don\'t match: %d and %d.'
                    % (opr.name, existing_opr.width, opr.width)
                )
            self.operands.remove(existing_opr)
            self.constOp.merge_operand(opr)
        elif opr.type in ["Immediate", "Choices"]:
            existing_opr = self.find_operand(opr.name)
            if existing_opr.value and existing_opr.value.find("!=") == 0:
                print(
                    "WARNING: need special handling of this instruction with "
                    'constraint "%s" on operand "%s".'
                    % (existing_opr.value, opr.name)
                )
                return
            existing_opr.merge_value(opr)
            existing_opr.update_type()
            if existing_opr.type != "Constant":
                # print ("Spliting operand \"%s\"" % opr.name)
                split_const = existing_opr.split_operand()
                self.constOp.merge_operand(split_const)
            else:
                self.operands.remove(existing_opr)
                self.constOp.merge_operand(existing_opr)
        elif opr.type == "Register":
            if opr.value == ("N" * opr.width):
                # the value field is a series of "N"s
                pass
            else:
                raise BuilderException(
                    "Unexpected change_operand on Register type with "
                    'value="%s".' % (opr.value)
                )
        else:
            raise BuilderException(
                'Unexpected change_operand type "%s".' % (opr.type)
            )

        if update_const:
            self.update_constant()

    def update_constant(self):
        self.constOp.update_bits_value()

    def write(self, file_handle):
        file_handle.write(self.to_string())

    def to_string(self):
        ret_str = "  <I"
        if self.name:
            ret_str += ' name="%s"' % self.name
        if self.form:
            ret_str += ' form="%s"' % self.form
        if self.isa:
            ret_str += ' isa="%s"' % self.isa
        if self.aliasing:
            ret_str += ' aliasing="%s"' % self.aliasing
        if self.iclass:
            ret_str += ' class="%s"' % self.iclass
        if self.group:
            ret_str += ' group="%s"' % self.group.capitalize()
        if self.extension:
            ret_str += ' extension="%s"' % self.extension
        ret_str += ">\n"

        if self.constOp:
            ret_str += self.constOp.to_string("    ")

        for opr in self.operands:
            ret_str += opr.to_string("    ")

        ret_str += self.asm.to_string("    ")

        ret_str += "  </I>\n"
        return ret_str

    def get_format(self):
        opr_names = list()
        for opr in self.operands:
            if opr.name != "const_bits":
                opr_names.append(opr.name)

        return "-".join(opr_names)

    def merge_remove_operand(self, name_a, name_b):
        # merge operand name_b with the operand name_a and remove operand
        # name_b
        name_a_opr = self.find_operand(name_a)
        name_b_opr = self.find_operand(name_b)
        name_a_opr.merge_operand(name_b_opr)
        self.operands.remove(name_b_opr)

    def rename_operand(self, name_a, name_b):
        # rename the operand name_a to name_b
        name_a_opr = self.find_operand(name_a)
        name_a_opr.name = name_b

    def add_const_operand(self, bits, value):
        opr = Operand()
        opr.set_attribute("name", "const_bits")
        opr.set_attribute("type", "Constant")
        opr.set_attribute("bits", bits)
        opr.set_attribute("value", value)
        self.add_operand(opr)

    def add_constx_operand(self, name, value):
        self.constxBits[name] = value

    def get_const_operand(self):
        if self.constOp:
            return self.constOp
        else:
            return None

    def search_operand(self, name, fail_not_found=True):
        for opr in self.operands:
            if opr.name == name:
                return opr
            if (
                name == "const_bits"
                and self.constOp
                and self.constOp.name == name
            ):
                return self.constOp
            if isinstance(opr, GroupOperand):
                opr_tmp = opr.find_operand(name, False)
                if opr_tmp is None:
                    continue
                else:
                    return opr_tmp

        if fail_not_found:
            raise BuilderException(
                'Operand "%s" not found in instruction "%s".'
                % (name, self.get_full_ID())
            )
        else:
            return None

    def set_operand_attribute(self, opr_name, name, value):
        opr = self.search_operand(opr_name, value)
        if opr is not None:
            opr.set_attribute(name, value)

    def append_asm_op(self, value):
        self.asm.ops.append(value)

    def rename_asm_attribute(self, name, value):
        if name == "format":
            self.asm.format = value
            return True
        elif name.startswith("op"):
            idx_str = name[2:]
            idx = int(idx_str) - 1
            if len(self.asm.ops) > idx:
                if value != "":
                    self.asm.ops[idx] = value
                else:
                    self.asm.ops.pop(idx)
                return True
            else:
                return False
        # else:
        #     for op in self.asm.ops:
        #         if op == name:
        #             self.asm.rename_op(name, value)
        #             return True
        return False


# create an addressing operand
# Parameters:
# instr      => instruction object to operand on
# opr_name   => name of the operand, can be None and created out of sub
#               operand names
# opr_type   => operand type, Branch or LoadStore
# class_name => operand class name
# subop_dict => a dict of sub operand types and names will be moved from under
#               the instruction into under the addressing operand
# attr_dict  => additional addressing operand attributes


def add_addressing_operand(
    instr, opr_name, opr_type, class_name, subop_dict, attr_dict=None
):
    addr_opr = GroupOperand()
    addr_opr.type = opr_type
    if class_name is not None:
        addr_opr.oclass = class_name
    instr.add_operand(addr_opr)

    subop_names = list()
    for subop_type, subop_name in sorted(subop_dict.items()):
        subop_names.append(subop_name)
        sub_opr = instr.find_operand(subop_name)
        instr.operands.remove(sub_opr)
        addr_opr.add_operand(sub_opr)
        addr_opr.set_extra_attribute(subop_type, subop_name)

    if opr_name:
        addr_opr.name = opr_name
    else:
        addr_opr.name = opr_type + "-" + "-".join(subop_names)

    if attr_dict:
        for attr_name, attr_value in attr_dict.items():
            addr_opr.set_extra_attribute(attr_name, attr_value)
