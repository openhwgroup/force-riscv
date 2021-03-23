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

template_string = """
CASE( "tests for E${class_name}" ) {

  SETUP ( "setup E${class_name}" )  {

    SECTION( "test enum to string conversion" ) {
${to_string_tests}
    }

    SECTION( "test string to enum conversion" ) {
${to_enum_tests}
    }

    SECTION( "test string to enum conversion with non-matching string" ) {
${to_enum_fail_tests}
    }

    SECTION( "test non-throwing string to enum conversion" ) {
      bool okay = false;
${try_to_enum_tests}
    }

    SECTION(
    "test non-throwing string-to-enum conversion with unmatched string" ) {
      bool okay = false;
${try_to_enum_fail_tests}
    }
  }
},

"""
