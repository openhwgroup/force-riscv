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
#include "VariableParser.h"

#include <cstring>
#include <iostream>

#include "pugixml.h"

#include "Architectures.h"
#include "Config.h"
#include "Enums.h"
#include "Log.h"
#include "UtilityFunctions.h"
#include "Variable.h"
#include "XmlTreeWalker.h"
#include ARCH_ENUM_HEADER

/*!
  \file VariableParser.cc
  \brief Parser code for all variable files.
*/

using namespace std;

namespace Force {

  /*!
    \class VariableFileParser
    \brief Private class for parsing variable files.

    VariableFileParser inherits pugi::xml_tree_walker.  This parser parse various variable files.
  */
  class VariableFileParser : public pugi::xml_tree_walker {
  public:
    /*!
      Constructor, pass in pointer to VariableParser object.
    */
    explicit VariableFileParser(VariableParser* variableParser) :
     mpVariableParser(variableParser), mpVariableSet(nullptr)
    {
    }

    ~VariableFileParser() //!< Destructor
    {

    }

    ASSIGNMENT_OPERATOR_ABSENT(VariableFileParser);
    COPY_CONSTRUCTOR_DEFAULT(VariableFileParser);
    /*!
      Handles variable file elements.
     */
    virtual bool for_each(pugi::xml_node& node)
    {
      const char * node_name = node.name();
      if (strcmp(node_name, "variable_file") == 0) {
        // known node names. do nothing
      }
      else if (strcmp(node_name, "variable") == 0) {
        process_variable(node);
      }
      else {
        LOG(fail) << "Unknown variable file node name \'" << node_name << "\'." << endl;
        FAIL("variable-unknown-node");
      }

      return true; // continue traversal
    }

    /*!
      Process attributes of \<variable\> element.
     */
    void process_variable(pugi::xml_node& node)
    {
      string name;
      string value;
      string type;
      for (pugi::xml_attribute const& attr: node.attributes()) {
        const char* attr_name = attr.name();
        if (strcmp(attr_name, "name") == 0) name = attr.value();
        else if (strcmp(attr_name, "type") == 0) type = attr.value();
        else if (strcmp(attr_name, "value") == 0) value = attr.value();
        else {
          LOG(fail) << "Unknown variable attribute \'" << attr_name << endl;
          FAIL("unknown-variable-attribute");
        }
      }
      add_variable(name, value, type);
    }

    /*!
      Add variable to its variable set
    */
    void add_variable(const string& name, const string& value,  const string& type)
    {
      EVariableType variable_type =  string_to_EVariableType(type);

      mpVariableSet = mpVariableParser->mVariableSets[int(variable_type)];
      mpVariableSet->AddVariable(name, value);
      return;
    }

  private:
    VariableParser* mpVariableParser; //!< Pointer to VariableParser object.
    VariableSet* mpVariableSet; //!< Pointer to the current VariableSet selected by the type of the variable.
  };

  void VariableParser::Setup(const ArchInfo& archInfo)
  {
    check_enum_size(EVariableTypeSize);

    if (mVariableSets.size() > 0) {
      LOG(fail) << "{VariableParser::Setup} VariableSet vector is not empty at start." << endl;
      FAIL("vector-to-populate-not-empty");
    }

    for (EVariableTypeBaseType i = 0; i < EVariableTypeSize; ++ i) {
      EVariableType vt = EVariableType(i);
      mVariableSets.push_back(new VariableSet(vt));
    }

    auto cfg_ptr = Config::Instance();
    const list<string>& variable_files = archInfo.VariableFiles();
    for (auto const& vfile_name : variable_files) {
      VariableFileParser variable_file_parser(this);
      string full_file_path = cfg_ptr->LookUpFile(vfile_name);
      parse_xml_file(full_file_path, "variable",variable_file_parser);
    }
  }

}
