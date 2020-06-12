//
// Copyright (C) [2020] Futurewei Technologies, Inc.
//
// FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
// FIT FOR A PARTICULAR PURPOSE.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef Force_XmlTreeWalker_H
#define Force_XmlTreeWalker_H

#include <string>

namespace pugi{
  class xml_tree_walker;
}

namespace Force {

  void parse_xml_file(const std::string& file_path, const std::string& file_type, pugi::xml_tree_walker& xml_parser); //!< Parse supporting data XML file.

}

#endif
