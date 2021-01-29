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
from riscv.EnvRISCV import EnvRISCV
from riscv.GenThreadRISCV import GenThreadRISCV
from base.Sequence import Sequence
from Constraint import ConstraintSet

## This test verifies that multithreaded execution runs smoothly when APIs are called that use
# ThreadContextNoAdvance on the back end.
class MainSequence(Sequence):

    def generate(self, **kargs):
        self._testConstraintSetMethods()

        with self.threadLockingContext():
            self._testConstraintSetMethods()

    ## Call a variety of ConstraintSet methods, which should utilize ThreadContextNoAdvance.
    def _testConstraintSetMethods(self):
        for _ in range(5):
            constr_a = ConstraintSet(0xF9B3, 0x207CE)
            if constr_a.isEmpty():
                self.error('Constraint %s reported as empty.' % constr_a)

            if constr_a.size() != 69148:
                self.error('Constraint %s reported incorrect size; Expected = 69148, Actual = %d' % (constr_a, constr_a.size()))

            val = constr_a.chooseValue()
            if (val < 0xF9B3) or (val > 0x207CE):
                self.error('Chosen value %d lies outside of constraint %s' % (val, constr_a))

            constr_a.addRange(0xE700, 0xE800)
            if str(constr_a) != '0xe700-0xe800,0xf9b3-0x207ce':
                self.error('Constraint %s didn\'t add range correctly.' % constr_a)

            constr_a.subRange(0xE780, 0x1F390)
            if str(constr_a) != '0xe700-0xe77f,0x1f391-0x207ce':
                self.error('Constraint %s didn\'t subtract range correctly.' % constr_a)

            constr_b = ConstraintSet(0x20800)
            constr_b.mergeConstraintSet(constr_a)
            if str(constr_b) != '0xe700-0xe77f,0x1f391-0x207ce,0x20800':
                self.error('Constraint %s didn\'t merge with constraint %s correctly.' % (constr_b, constr_a))

            constr_c = ConstraintSet('0x20000-0x20600,0x20800')
            constr_c.applyConstraintSet(constr_a)
            if str(constr_c) != '0x20000-0x20600':
                self.error('Constraint %s didn\'t apply constraint %s correctly.' % (constr_c, constr_a))

            constr_d = ConstraintSet(constr_a)
            if constr_d != constr_a:
                self.error('Constraint %s didn\'t copy from constraint %s correctly.' % (constr_d, constr_a))


MainSequenceClass = MainSequence
GenThreadClass = GenThreadRISCV
EnvClass = EnvRISCV
