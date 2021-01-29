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
from Constraint import ConstraintSet

from base.Sequence import Sequence
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV


# This test verifies that basic Constraint module functionality works as
# expected. No effort is made here to thoroughly test the ConstraintSet API, as
# this is done in the unit tests. The main purpose of this test is to verify
# that there is nothing obviously wrong with the Python wrapper.
class MainSequence(Sequence):
    def generate(self, **kargs):
        constr_a = ConstraintSet(0x23, 0x87)
        if constr_a.isEmpty():
            self.error("Constraint %s reported as empty." % constr_a)

        if constr_a.size() != 101:
            self.error(
                "Constraint %s reported incorrect size; Expected = 101, "
                "Actual = %d" % (constr_a, constr_a.size())
            )

        val = constr_a.chooseValue()
        if (val < 0x23) or (val > 0x87):
            self.error(
                "Chosen value %d lies outside of constraint %s"
                % (val, constr_a)
            )

        constr_a.addRange(0x35, 0x100)
        if str(constr_a) != "0x23-0x100":
            self.error("Constraint %s didn't add range correctly." % constr_a)

        constr_a.subRange(0x4F, 0x6F)
        if str(constr_a) != "0x23-0x4e,0x70-0x100":
            self.error(
                "Constraint %s didn't subtract range correctly." % constr_a
            )

        constr_b = ConstraintSet("0x18, 0x22-0x5C")
        constr_b.mergeConstraintSet(constr_a)
        if str(constr_b) != "0x18,0x22-0x5c,0x70-0x100":
            self.error(
                "Constraint %s didn't merge with constraint %s correctly."
                % (constr_b, constr_a)
            )

        constr_c = ConstraintSet(0x5B)
        constr_c.applyConstraintSet(constr_a)
        if not constr_c.isEmpty():
            self.error(
                "Constraint %s didn't apply constraint %s correctly."
                % (constr_c, constr_a)
            )

        constr_d = ConstraintSet(constr_a)
        if constr_d != constr_a:
            self.error(
                "Constraint %s didn't copy from constraint %s correctly."
                % (constr_d, constr_a)
            )


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
