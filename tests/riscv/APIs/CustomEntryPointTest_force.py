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

class testUtil(Sequence):

  def __init__(self, gen_thread):
    super(testUtil, self).__init__(gen_thread)
    self.entryFunction = self.load
    self.testValue = 0

  def load(self, arg1, arg2, **kargs):
    self.testValue = arg1 + arg2
    self.generate(**kargs)

  def generate(self, **kargs):
    if kargs is not None:
      for (key, value) in sorted(kargs.items()):
        self.notice("%s %s" % (key, value))
    self.notice("test value=%d" % self.testValue)

class MainSequence(Sequence):

  def generate(self, **kargs):
    test_util = testUtil(self.genThread)
    #Run with entry function specified as test_util.load
    test_util.run(arg1=1, arg2=4, arg3=423)
    test_util.entryFunction = None
    #Run with default control flow, in this case test_util.generate
    test_util.run(arg1=1, arg2=9, arg3=32)
    #Execute load function directly, without using run()
    test_util.load(2,4)

## Points to the MainSequence defined in this file
MainSequenceClass = MainSequence

## Using GenThreadRISCV by default, can be overriden with extended classes
GenThreadClass = GenThreadRISCV

## Using EnvRISCV by default, can be overriden with extended classes
EnvClass = EnvRISCV

