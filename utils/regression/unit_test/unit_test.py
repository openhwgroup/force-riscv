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
# test_base.py

import sys
from common.msg_utils import Msg


# base unit test class
class UnitTest(object):
    def run(self):
        try:
            self.__enter__()
        finally:
            self.__exit__(sys.exc_info())

    def run_test(self):
        pass

    def process_result(self):
        pass

    def __enter__(self):
        self.run_test()

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.process_result()
        Msg.blank()
