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
  /*!
    ${comments}
  */
  enum class E${class_name} : ${raw_type} {
${value_settings}
  };
  extern ${raw_type} E${class_name}Size;
  extern const std::string E${class_name}_to_string(E${class_name} in_enum); //!< Get string name for enum.
  extern E${class_name} string_to_E${class_name}(const std::string& in_str); //!< Get enum value for string name.
  extern E${class_name} try_string_to_E${class_name}(const std::string& in_str, bool& okay); //!< Try to get enum value for string name, set status to indicate if conversion successful. Return value is indeterminate on failure.
  typedef ${raw_type} E${class_name}BaseType; //!< Define a type name for the enum base data type.

"""
