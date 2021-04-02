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
# test_critical_section.py

from common.path_utils import PathUtils
from common.msg_utils import Msg

from unit_test import UnitTest
from common.kernel_objs import HiCriticalSection

from common.threads import HiThread


class UnitTest_HiCriticalSection(UnitTest):
    def run_test(self):
        Msg.info("HiCriticalSection: Start Unit Test ...")

    def process_result(self):
        Msg.info("HiCriticalSection: Process Test Result ...")
