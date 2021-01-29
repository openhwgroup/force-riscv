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
# This template tests the API genData()
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence


class MainSequence(Sequence):
    def generate(self, **kargs):
        # generate INT32 has forced value.
        int32 = self.genData("INT32(0x55555)")
        if int32 != 0x55555:
            self.error(
                'Failed to generate data pattern "INT32(0x55555)" data : 0x%x'
                % (int32)
            )

        # generate INT64 in specified range.
        int64 = self.genData("INT64(0x8010-0x9000)")
        if int64 < 0x8010 or int64 > 0x9000:
            self.error(
                'Failed to generate data pattern "INT64(0x8010-0x9000)" '
                "data : 0x%x" % (int64)
            )

        # generate Floating-point data.
        fp64_pattern = "FP64(sign=1)(exp=0x100-0x200)(frac=0x300)"
        fp64 = self.genData(
            fp64_pattern
        )  # self.genData("FP64(sign=1)(exp=0x100-0x200)(frac=0x300)")
        fraction = fp64 & 0xFFFFFFFFFFFFF
        if fraction != 0x300:
            self.error(
                "Failed to generate data pattern %s frac : 0x%x"
                % (fp64_pattern, fraction)
            )
        exponent = (fp64 & 0x7FF0000000000000) >> 52
        if exponent < 0x100 or exponent > 0x200:
            self.error(
                "Failed to generate data pattern %s exp  : 0x%x"
                % (fp64_pattern, exponent)
            )
        sign = (fp64 & 0x8000000000000000) >> 63
        if sign != 0x1:
            self.error(
                "Failed to generate data pattern %s sign : 0x%x"
                % (fp64_pattern, sign)
            )

        # generate Vector 64 bits width data.
        vec64_pattern = "[0]INT32(0x1)[1]INT32(0x2)"
        vec64 = self.genData(
            vec64_pattern
        )  # self.genData("[0]INT32(0x1)[1]INT32(0x2)")
        elem0 = vec64 & 0xFFFFFFFF
        if elem0 != 0x1:
            self.error(
                "Failed to generate data pattern %s elem0 : 0x%x"
                % (vec64_pattern, elem0)
            )
        elem1 = (vec64 & 0xFFFFFFFF00000000) >> 32
        if elem1 != 0x2:
            self.error(
                "Failed to generate data pattern %s elem1 : 0x%x"
                % (vec64_pattern, elem1)
            )

        # generate Vector 128 bits width data.
        vec128_pattern = "[0]INT32(0x1)[1]INT32(0x2)[2]INT64(0x3)"
        (vec64_1, vec64_0) = self.genData(
            vec128_pattern
        )  # self.genData("[0]INT32(0x1)[1]INT32(0x2)[2]INT64(0x3)")
        if vec64_0 != 0x200000001:
            self.error(
                "Failed to generate data pattern %s vec64_0 : 0x%x"
                % (vec128_pattern, vec64_0)
            )
        if vec64_1 != 0x3:
            self.error(
                "Failed to generate data pattern %s vec64_0 : 0x%x"
                % (vec128_pattern, vec64_1)
            )

        # generate Vector 128 bits width data.
        vec128_pattern = "[0,1,3]INT32(0x1)[2]INT32(0x2)"
        (vec64_1, vec64_0) = self.genData(
            vec128_pattern
        )  # self.genData("[0,1,3]INT32(0x1)[2]INT32(0x2)")
        if vec64_0 != 0x100000001:
            self.error(
                "Failed to generate data pattern %s vec64_0 : 0x%x"
                % (vec128_pattern, vec64_0)
            )
        if vec64_1 != 0x100000002:
            self.error(
                "Failed to generate data pattern %s vec64_1 : 0x%x"
                % (vec128_pattern, vec64_1)
            )

        # generate Vector 128 bits width data.
        vec128_pattern = (
            "[0,1,2]FP32(exp=0x10)(sign=0)(frac=0x200-0x300)[3]INT32(0xffff)"
        )
        # self.genData("[0,1,2]FP32(exp=0x100)(sign=0)(frac=0x200-0x300)
        # [3]INT32(0xffff)")
        (vec64_1, vec64_0) = self.genData(vec128_pattern)
        fp32_0 = vec64_0 & 0xFFFFFFFF
        fraction = fp32_0 & 0x7FFFFF
        if fraction < 0x200 or fraction > 0x300:
            self.error(
                "Failed to generate data pattern %s fp32_0 fraction : 0x%x"
                % (vec128_pattern, fraction)
            )
        exponent = (fp32_0 & 0x7F800000) >> 23
        if exponent != 0x10:
            self.error(
                "Failed to generate data pattern %s fp32_0 exponent : 0x%x"
                % (vec128_pattern, exponent)
            )
        sign = (fp32_0 & 0x80000000) >> 31
        if sign != 0:
            self.error(
                "Failed to generate data pattern %s fp32_0 sign : 0x%x"
                % (vec128_pattern, sign)
            )
        int32_3 = (vec64_1 & 0xFFFFFFFF00000000) >> 32
        if int32_3 != 0xFFFF:
            self.error(
                "Failed to generate data pattern %s int32_3 : 0x%x"
                % (vec128_pattern, int32_3)
            )

        self.notice("All generate are passed")


# Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

# Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

# Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV
