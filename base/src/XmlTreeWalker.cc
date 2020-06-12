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
#include <XmlTreeWalker.h>
#include <Log.h>

#include <pugixml.h>

using namespace std;

namespace Force {

  /*!
    Parse XML data file.  Read XML file into a pugi::xml_document, then traverse and handle elementes using passed in pugi::xml_tree_walker class instance to populate XML file properties.
  */
  void parse_xml_file(const string& file_path, const string& file_type, pugi::xml_tree_walker& xml_parser)
  {
    LOG(notice) << "Loading " << file_type << " file: " << file_path << " ..." << endl;

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(file_path.c_str());

    if (!result) {
      LOG(fail) << "Faled parsing " << file_type << " file \"" << file_path << "\".  Error description: " << result.description() << endl;
      string fail_str = string("fail-parse-xml-") + file_type + "-file";
      FAIL(fail_str.c_str());
    }

    doc.traverse(xml_parser);
  }

}
