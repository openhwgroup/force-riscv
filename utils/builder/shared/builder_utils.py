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
import os.path
import sys

from shared.builder_exception import BuilderException


def check_dir(dir_path):
    if not os.path.exists(dir_path):
        print('Nonexistent dir path "%s".' % dir_path)
        sys.exit(1)

    if not os.path.isdir(dir_path):
        print('Path "%s" is not a directory.' % dir_path)
        sys.exit(1)


def high_bits_width_to_str(hi_bit, width):
    if width == 0:
        print("Incorrect width => 0")
        sys.exit(1)
    elif width == 1:
        return "%d" % hi_bit
    else:
        if hi_bit < (width - 1):
            print("Incorrect hi bit %d and width %d combo" % (hi_bit, width))
        return "%d-%d" % (hi_bit, hi_bit - width + 1)


class BitValue(object):
    def __init__(self, hi, lo, val):
        self.high = hi
        self.low = lo
        self.value = val

    def to_string(self):
        ret_str = "(%d" % self.high
        if self.high != self.low:
            ret_str += "-%d" % self.low
        ret_str += "=%s)" % self.value
        return ret_str

    def get_bits_str(self):
        ret_str = "%d" % self.high
        if self.high != self.low:
            ret_str += "-%d" % self.low
        return ret_str

    def merge(self, other):
        if self.low == other.high + 1:
            self.low = other.low
            self.value += other.value
            return True
        return False


# Updating bits-value pair so that the bits are sorted and merged if they can,
# example: original bits-value "30,29,28-21,15-10,31", "00110100000000001"
#          sorted bits-value   "31-21,15-10",          "10011010000000000"


def update_bits_value(bits, value):
    bits_list = bits.split(",")
    if len(bits_list) == 1:
        return bits, value

    start_loc = 0
    bitval_list = list()
    # print("Updating bits value \"%s\", \"%s\"" % (bits, value))
    for bits_piece in bits_list:
        range_split = bits_piece.split("-")
        piece_sz = 1
        if len(range_split) == 1:
            bit_val = int(range_split[0])
            new_bitval_obj = BitValue(bit_val, bit_val, value[start_loc])
        else:
            bit_hi = int(range_split[0])
            bit_lo = int(range_split[1])
            piece_sz = bit_hi - bit_lo + 1
            new_bitval_obj = BitValue(bit_hi, bit_lo, value[start_loc : (start_loc + piece_sz)])

        bitval_list.append(new_bitval_obj)
        start_loc += piece_sz

    bitval_list.sort(key=lambda x: x.high, reverse=True)

    ret_bits = ""
    ret_value = ""
    cur_bitval_obj = bitval_list[0]
    for index in range(1, len(bitval_list)):
        bitval_obj = bitval_list[index]
        if not cur_bitval_obj.merge(bitval_obj):
            ret_bits += cur_bitval_obj.get_bits_str() + ","
            ret_value += cur_bitval_obj.value
            cur_bitval_obj = bitval_obj

    ret_bits += cur_bitval_obj.get_bits_str()
    ret_value += cur_bitval_obj.value
    # print ("Updated bits \"%s\", value \"%s\"" % (ret_bits, ret_value))
    if len(value) != len(ret_value):
        print(
            "ERROR updating bits-value pair, length before %d, after %d"
            % (len(value), len(ret_value))
        )
        sys.exit(1)
    return ret_bits, ret_value


def update_bits(bits):
    bits_list = bits.split(",")
    if len(bits_list) == 1:
        return bits

    bitval_list = list()
    # print("Updating bits value \"%s\", \"%s\"" % (bits, value))
    for bits_piece in bits_list:
        range_split = bits_piece.split("-")
        piece_sz = 1
        if len(range_split) == 1:
            bit_val = int(range_split[0])
            new_bitval_obj = BitValue(bit_val, bit_val, "")
        else:
            bit_hi = int(range_split[0])
            bit_lo = int(range_split[1])
            piece_sz = bit_hi - bit_lo + 1
            new_bitval_obj = BitValue(bit_hi, bit_lo, "")

        bitval_list.append(new_bitval_obj)

    bitval_list.sort(key=lambda x: x.high, reverse=True)

    ret_bits = ""
    cur_bitval_obj = bitval_list[0]
    for index in range(1, len(bitval_list)):
        bitval_obj = bitval_list[index]
        if not cur_bitval_obj.merge(bitval_obj):
            ret_bits += cur_bitval_obj.get_bits_str() + ","
            cur_bitval_obj = bitval_obj

    ret_bits += cur_bitval_obj.get_bits_str()
    return ret_bits


def merge_imm_value(value1, value2):
    if None is value1:
        return value2

    if len(value1) != len(value2):
        raise BuilderException(
            "Merging value but value length don't match: %d and %d." % (len(value1), len(value2))
        )

    merged_value = ""
    for i in range(len(value1)):
        if value1[i] == "x":
            merged_value += value2[i]
        else:
            merged_value += value1[i]
    return merged_value


def bit_string_to_list(bit_str):
    bit_list = list()
    range_list = bit_str.split(",")

    for range_i in range_list:
        range_split = range_i.split("-")
        if len(range_split) == 1:
            bit_list.append(range_split[0])
        else:
            range_start = int(range_split[0])
            range_end = int(range_split[1])
            for bit in range(range_start, range_end - 1, -1):
                bit_list.append(bit)

    return bit_list


def get_bit_or_range_string(range_start, range_end):
    if range_start == range_end:
        range_str = "%d" % range_start
    else:
        range_str = "%d-%d" % (range_start, range_end)
    return range_str


# This function assume bit_list to be already sorted in descending order
def bit_list_to_string(bit_list):
    range_start = int(bit_list[0])
    range_end = range_start
    bit_str = None

    for i in range(1, len(bit_list)):
        bit_value = int(bit_list[i])
        if bit_value == range_end - 1:
            range_end = bit_value
        else:
            range_str = get_bit_or_range_string(range_start, range_end)
            if bit_str:
                bit_str += "," + range_str
            else:
                bit_str = range_str
            range_start = bit_value
            range_end = bit_value

    range_str = get_bit_or_range_string(range_start, range_end)
    if bit_str:
        bit_str += "," + range_str
    else:
        bit_str = range_str

    return bit_str


def get_bits_size(bits_str):
    bits_list = bits_str.split(",")

    bits_size = 0
    # print("Updating bits value \"%s\", \"%s\"" % (bits, value))
    for bits_piece in bits_list:
        range_split = bits_piece.split("-")
        piece_sz = 1
        if len(range_split) == 2:
            bit_hi = int(range_split[0])
            bit_lo = int(range_split[1])
            piece_sz = bit_hi - bit_lo + 1

        bits_size += piece_sz

    return bits_size
