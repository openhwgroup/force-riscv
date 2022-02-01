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
# PYTHON3 UP# /software/public/python/3.4.1/bin/python3

# insert parent dir to access base test builder class

from base_test_builder import BaseTestBuilder


class RiscVTestBuilder(BaseTestBuilder):
    def __init__(self):
        super().__init__("RISCV", True, 10, "", "", "debug_output.txt")
        self.mXmlFiles = [
            "g_instructions.xml",
            "g_instructions_rv64.xml",
            "c_instructions.xml",
            "c_instructions_rv64.xml",
            "c_instructions_rv32.xml",
            "v_instructions.xml",
            "priv_instructions.xml",
            "zfh_instructions.xml",
            "zfh_instructions_rv64.xml",
        ]
        self.mTxtFiles = ["genonly.txt", "unsupported.txt"]


if __name__ == "__main__":
    riscv_test_builder = RiscVTestBuilder()
    riscv_test_builder.run()
