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

template_string = """
  ${raw_type} E${class_name}Size = ${enum_size};

  const string E${class_name}_to_string(E${class_name} in_enum)
  {
    switch (in_enum) {
${to_string_cases}
    default:
      unknown_enum_value("E${class_name}", (${raw_type})(in_enum));
    }
    return "";
  }

  E${class_name} string_to_E${class_name}(const string& in_str)
  {
    string enum_type_name = "E${class_name}";
${to_enum_function_body}
    return E${class_name}::${default_string_value};
  }

  E${class_name} try_string_to_E${class_name}(const string& in_str, bool& okay)
  {
${try_to_enum_function_body}
    return E${class_name}::${default_string_value};
  }

"""
