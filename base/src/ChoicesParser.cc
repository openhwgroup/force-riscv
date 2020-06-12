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
#include <Choices.h>
#include <ChoicesParser.h>
#include <Architectures.h>
#include <UtilityFunctions.h>
#include <StringUtils.h>
#include <Config.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <XmlTreeWalker.h>
#include <Log.h>

#include <pugixml.h>
#include <string.h>
#include <iostream>

/*!
  \file ChoicesParser.cc
  \brief Parser code for all choices files.
*/

using namespace std;

namespace Force {

  /*!
    \class ChoicesFileParser
    \brief Private class for parsing choices files.

    ChoicesFileParser inherits pugi::xml_tree_walker.  This parser parse various choices files.
  */
  class ChoicesFileParser : public pugi::xml_tree_walker {
  public:
    /*!
      Constructor, pass in pointer to ChoicesParser object.
    */
    explicit ChoicesFileParser(ChoicesParser* choicesParser)
      : mpChoicesParser(choicesParser), mChoiceTreeChain(), mpChoicesSet(nullptr)
    {
    }

    ~ChoicesFileParser() //!< Destructor, check if there is dangling pointers.
    {
      if (mChoiceTreeChain.size()){
        LOG(fail) << "Unexpected dangling pointer at the end of choices file parsing." << endl;
        FAIL("unexpected-dangling-pointer-parsing-choices-file");
      }
    }
    ASSIGNMENT_OPERATOR_ABSENT(ChoicesFileParser);
    COPY_CONSTRUCTOR_DEFAULT(ChoicesFileParser);

    /*!
      Handles choices file elements.
     */
    virtual bool for_each(pugi::xml_node& node)
    {
      const char * node_name = node.name();
      if (strcmp(node_name, "choices_file") == 0) {
        // known node names. do nothing
      }
      else if (strcmp(node_name, "choices") == 0) {
        process_choices(node);
      }
      else if (strcmp(node_name, "choice") == 0) {
        process_choice(node);
      }
      else {
        LOG(fail) << "Unknown choices file node name \'" << node_name << "\'." << endl;
        FAIL("choices-unknown-node");
      }

      return true; // continue traversal
    }

    /*!
      Implement end function to add the last InstructionStructure object.
     */
    virtual bool end(pugi::xml_node& node)
    {
      commit_choices_tree();
      return true;
    }

    /*!
      Process attributes of \<choices\> element.
     */
    void process_choices(pugi::xml_node& node)
    {
      if (depth() == 1) {
        commit_choices_tree();
      }

      string name;
      string type;
      uint32 weight = 0;
      for (pugi::xml_attribute const& attr: node.attributes()) {
        const char* attr_name = attr.name();
        if (strcmp(attr_name, "name") == 0) name = attr.value();
        else if (strcmp(attr_name, "type") == 0) type = attr.value();
        else if (strcmp(attr_name, "weight") == 0) weight = parse_uint32(attr.value());
        else if (strcmp(attr_name, "description") == 0) continue;
        else {
          LOG(fail) << "Unknown choices attribute \'" << attr_name << endl;
          FAIL("unknown-choices-attribute");
        }
      }
      if (name.size() == 0) {
        LOG(fail) << "{process_choices} expected to have \"name\" attribute." << endl;
        FAIL("choices-no-name");
      }
      ChoiceTree* new_tree = new ChoiceTree(name, 0, weight);
      add_choices_tree(new_tree, type);
    }

    /*!
      Commit last ChoiceTree root object if there is any.
    */
    void commit_choices_tree()
    {

      if (mChoiceTreeChain.size()) {
        mpChoicesSet->AddChoiceTree(mChoiceTreeChain.front());
        mChoiceTreeChain.clear();
      }
    }

    /*!
      Add ChoiceTree to the mChoiceTreeChain vector.  Assign to parent tree if not the root tree.
    */
    void add_choices_tree(ChoiceTree* new_tree, const string& type)
    {
      int level = depth();
      if (level == 1) {
        if (mChoiceTreeChain.size()) {
          LOG(fail) << "{add_choice_tree} expecting mChoiceTreeChain to be empty." << endl;
          FAIL("choices-tree-chain-not-empty");
        }
        EChoicesType choices_type {EChoicesType(0)};
        if (type == "RegisterOperand" || type == "ImmediateOperand" || type == "ChoicesOperand") {
          choices_type = EChoicesType::OperandChoices;
        }
        else if (type == "RegisterFieldValue") {
          choices_type = EChoicesType::RegisterFieldValueChoices;
        }
        else if (type == "Paging") {
          choices_type = EChoicesType::PagingChoices;
        }
        else if (type == "General") {
          choices_type = EChoicesType::GeneralChoices;
        }
        else if (type == "Dependence") {
          choices_type = EChoicesType::DependenceChoices;
        }
        else {
          LOG(fail) << "Type of choices tree \"" << type << "\" not supported." << endl;
          FAIL("unsupported-choices-tree-type");
        }
        mpChoicesSet = mpChoicesParser->mChoicesSets[int(choices_type)];
        mChoiceTreeChain.push_back(new_tree);
        return;
      }

      if (level < 1) {
        LOG(fail) << "{add_choice_tree} unexpected depth: " << dec << level << endl;
        FAIL("unexpected-choices-depth");
      }

      // level > 1
      if (mChoiceTreeChain.size() < uint32(level - 1)) {
        LOG(fail) << "{add_choice_tree} expecting mChoiceTreeChain to at least have " << dec << (level - 1) << " items." << endl;
        FAIL("choices-tree-chain-too-few-items");
      }
      // clear out trees from previous sibling.
      while (mChoiceTreeChain.size() >= uint32(level)) {
        mChoiceTreeChain.pop_back();
      }

      mChoiceTreeChain.back()->AddChoice(new_tree);
      mChoiceTreeChain.push_back(new_tree);
    }

    /*!
      Process attributes of \<choice\> element.
    */
    void process_choice(pugi::xml_node& node)
    {
      string name, value_str, weight_str;

      for (pugi::xml_attribute const& attr: node.attributes()) {
        const char* attr_name = attr.name();
        if (strcmp(attr_name, "name") == 0) name = attr.value();
        else if (strcmp(attr_name, "value") == 0) value_str = attr.value();
        else if (strcmp(attr_name, "weight") == 0) weight_str = attr.value();
        else if (strcmp(attr_name, "description") == 0) continue;
        else {
          LOG(fail) << "Unknown choice attribute \'" << attr_name << endl;
          FAIL("unknown-choice-attribute");
        }
      }
      if (name.size() == 0) name = value_str;
      if (weight_str.size() == 0) {
        LOG(fail) << "{process_choice} expected to have \"weight\" attribute." << endl;
      }
      uint32 value = 0, weight = 0;
      value = parse_uint32(value_str);
      weight = parse_uint32(weight_str);
      Choice* new_choice = new Choice(name, value, weight);
      mChoiceTreeChain.back()->AddChoice(new_choice);
    }

  private:
    ChoicesParser* mpChoicesParser; //!< Pointer to ChoicesParser object.
    vector<ChoiceTree* > mChoiceTreeChain; //!< Pointer chain including root tree to the current ChoiceTree object being worked on.
    ChoicesSet* mpChoicesSet; //!< Pointer to the current ChoicesSet selected by the type of the Choices tree.
  };

  void ChoicesParser::Setup(const ArchInfo& archInfo)
  {
    check_enum_size(EChoicesTypeSize);

    if (mChoicesSets.size() > 0) {
      LOG(fail) << "{ChoicesParser::Setup} ChoicesSet vector is not empty at start." << endl;
      FAIL("vector-to-populate-not-empty");
    }

    for (EChoicesTypeBaseType i = 0; i < EChoicesTypeSize; ++ i) {
      EChoicesType ct = EChoicesType(i);
      mChoicesSets.push_back(new ChoicesSet(ct));
    }

    auto cfg_ptr = Config::Instance();
    const list<string>& choices_files = archInfo.ChoicesFiles();
    for (auto const& cfile_name : choices_files) {
      ChoicesFileParser choices_file_parser(this);
      string full_file_path = cfg_ptr->LookUpFile(cfile_name);
      parse_xml_file(full_file_path, "choices", choices_file_parser);
    }
  }

}
