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
#include <PagingInfo.h>
#include <PteStructure.h>
#include <Architectures.h>
#include <Config.h>
#include <XmlTreeWalker.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <StringUtils.h>
#include <GenException.h>
#include <Log.h>

#include <pugixml.h>

#include <string.h>

using namespace std;

/*!
  \file PagingInfo.cc
  \brief Code for PteStructure container.
*/

namespace Force {

  /*!
    \class PagingParser
    \brief Parser class for instruction files.

    PagingParser inherits pugi::xml_tree_walker.  This parser parse various instruction files.
  */
  class PagingParser : public pugi::xml_tree_walker {
  public:
    /*!
      Constructor, pass in pointer to PagingInfo object.
    */
    explicit PagingParser(PagingInfo* pPagingInfo)
      : mpPagingInfo(pPagingInfo), mpPteStructure(nullptr), mpPteAttributeStructure(nullptr)
    {
    }

    ~PagingParser() //!< Destructor, check if there is dangling pointers.
    {
      if ((nullptr != mpPteStructure) || (nullptr != mpPteAttributeStructure)) {
        LOG(fail) << "Unexpected dangling pointer, mpPteStructure is nullptr? " << (nullptr == mpPteStructure) << " mpPteAttributeStructure is nullptr? " << (nullptr == mpPteAttributeStructure) << endl;
        FAIL("unexpected-dangling-pointer");
      }
    }

    ASSIGNMENT_OPERATOR_ABSENT(PagingParser);
    COPY_CONSTRUCTOR_DEFAULT(PagingParser);
    /*!
      Handles paging file elements.
     */
    bool for_each(pugi::xml_node& node) override
    {
      const char * node_name = node.name();

      if (strcmp(node_name, "page_tables") == 0) {
        // known node names.
      }
      else if (strcmp(node_name, "paging_mode") == 0) {
        process_paging_mode_node(node);
      }
      else if (strcmp(node_name, "pte") == 0) {
        process_pte_node(node);
      }
      else if (strcmp(node_name, "pte_attribute") == 0) {
        process_pte_attribute_node(node);
      }
      else {
        LOG(fail) << "Unknown paging file node name \'" << node_name << "\'." << endl;
        FAIL("paging-unknown-node");
      }

      return true; // continue traversal
    }

    /*!
      Implement end function to add the last PteStructure object.
    */
    bool end(pugi::xml_node& node) override
    {
      commit_pte_structure();
      return true;
    }

    /*!
      Process attributes of \<paging_mode\> element.
     */
    void process_paging_mode_node(pugi::xml_node& node)
    {
      try {
        for (pugi::xml_attribute const& attr: node.attributes()) {
          const char* attr_name = attr.name();

          if (strcmp(attr.name(), "name") == 0) {
            mpPagingInfo->mPagingMode = string_to_EPagingMode(attr.value());
          }
          else {
            LOG(fail) << "Unknown paging_mode attribute \'" << attr_name << "\'" << endl;
            FAIL("unknown-paging-mode-attribute");
          }
        }
      }
      catch (const EnumTypeError& enum_error) {
        LOG(fail) << "Error interpeting enum: " << enum_error.what() << " when processing paging_mode node." << endl;
        FAIL("error-interpeting-paging-mode");
      }
    }

    /*!
      Process attributes of \<pte\> element.
     */
    void process_pte_node(pugi::xml_node& node)
    {
      commit_pte_structure();

      mpPteStructure = new PteStructure(msPteClass);

      try {
        for (pugi::xml_attribute const& attr: node.attributes()) {
          const char* attr_name = attr.name();
          if (strcmp(attr_name, "type") == 0) {
            string pte_type = "P";
            pte_type += attr.value();
            mpPteStructure->mType = string_to_EPteType(pte_type);
          }
          else if (strcmp(attr_name, "category") == 0) mpPteStructure->mCategory = string_to_EPteCategoryType(attr.value());
          else if (strcmp(attr_name, "granule") == 0) {
            string granule_type = "G";
            granule_type += attr.value();
            mpPteStructure->mGranule = string_to_EPageGranuleType(granule_type);
          }
          else if (strcmp(attr_name, "level") == 0) mpPteStructure->mLevel = parse_uint32(attr.value());
          else if (strcmp(attr_name, "stage") == 0) mpPteStructure->mStage = parse_uint32(attr.value());
          else if (strcmp(attr_name, "class") == 0) mpPteStructure->mClass = attr.value();
          else {
            LOG(fail) << "Unknown PTE attribute \'" << attr_name << "\'" << endl;
            FAIL("unknown-pte-attribute");
          }
        }
      }
      catch (const EnumTypeError& enum_error) {
        LOG(fail) << "Error interpeting enum: " << enum_error.what() << " when processing PTE node." << endl;
        FAIL("error-interpeting-pte-enum");
      }
    }

    /*!
      Commit last PteStructure object if there is any.
    */
    void commit_pte_structure()
    {
      if (nullptr != mpPteStructure) {
        commit_pte_attribute_structure();

        mpPagingInfo->AddPte(mpPteStructure);
        mpPteStructure = nullptr;
      }
    }

    /*!
      Process attributes of \<pte_attribute\> element.
    */
    void process_pte_attribute_node(pugi::xml_node& node)
    {
      commit_pte_attribute_structure();

      mpPteAttributeStructure = new PteAttributeStructure(msPteAttributeClass);

      try {
        for (pugi::xml_attribute const& attr: node.attributes()) {
          const char* attr_name = attr.name();
          if (strcmp(attr_name, "type") == 0) {
            mpPteAttributeStructure->mTypeText = attr.value();
            mpPteAttributeStructure->mType = string_to_EPteAttributeType(attr.value());
          }
          else if (strcmp(attr_name, "bits") == 0) mpPteAttributeStructure->SetBits(attr.value());
          else if (strcmp(attr_name, "class") == 0) mpPteAttributeStructure->mClass = attr.value();
          else if (strcmp(attr_name, "value") == 0) {
            uint32 attr_value = parse_bin32(attr.value());
            mpPteAttributeStructure->SetValue(attr_value);
          }
          else if (strcmp(attr_name, "factory") == 0) {
            mpPteAttributeStructure->mFactory = (parse_uint32(attr.value()) > 0);
          }
          else {
            LOG(fail) << "Unknown PteAttributeStructure attribute \'" << attr_name << "\'" << endl;
            FAIL("unknown-pte-attribute-structure-attribute");
          }
        }
      }
      catch (const EnumTypeError& enum_error) {
        LOG(fail) << "Error interpeting enum: " << enum_error.what() << " when processing PTE attribute node." << endl;
        FAIL("error-interpeting-pte-attribute-enum");
      }
    }

    /*!
      Commit last PteAttributeStructure object if there is any.
    */
    void commit_pte_attribute_structure()
    {
      if (nullptr != mpPteAttributeStructure) {
        mpPteStructure->AddAttribute(mpPteAttributeStructure);
        mpPteAttributeStructure = nullptr;
      }
    }

  private:
    PagingInfo* mpPagingInfo; //!< Pointer to the PagingInfo object.
    PteStructure* mpPteStructure; //!< Pointer to the current PteStructure object being worked on.
    PteAttributeStructure* mpPteAttributeStructure; //!< Pointer to the current PteAttributeStructure object being worked on.

    static std::string msPteClass; //!< Default PTE class.
    static std::string msPteAttributeClass; //!< Default PTE attribute class.
    friend class PagingInfo;
  };

  string PagingParser::msPteClass;
  string PagingParser::msPteAttributeClass;

  PagingInfo::~PagingInfo()
  {
    for (auto pte_item : mPteSet) {
      delete pte_item.second;
    }
  }

  void PagingInfo::Setup(const ArchInfo& archInfo)
  {
    PagingParser::msPteClass = archInfo.DefaultPteClass();
    PagingParser::msPteAttributeClass = archInfo.DefaultPteAttributeClass();

    auto cfg_ptr = Config::Instance();
    const list<string>& paging_files = archInfo.PagingFiles();
    for (auto const& pfile_name : paging_files) {
      PagingParser paging_parser(this);
      string full_file_path = cfg_ptr->LookUpFile(pfile_name);
      parse_xml_file(full_file_path, "paging", paging_parser);
    }
  }

  const PteStructure* PagingInfo::LookUpPte(const std::string& pteName) const
  {
    auto find_iter = mPteSet.find(pteName);
    if (find_iter == mPteSet.end()) {
      LOG(fail) << "PTE with ID \"" << pteName << "\" not found." << endl;
      FAIL("pte-look-up-by-id-fail");
    }

    return find_iter->second;
  }

  void  PagingInfo::AddPte(PteStructure* pteStruct)
  {
    string pfull_id = pteStruct->FullId();
    LOG(trace) << "Adding new PTE: " << pfull_id << " size=" << dec << pteStruct->Size() << endl;

    const auto find_iter = mPteSet.find(pfull_id);
    if (find_iter != mPteSet.end()) {
      LOG(fail) << "Duplicated PTE full name \'" << pfull_id << "\'." << endl;
      FAIL("duplicated-pte-full-name");
    } else {
      mPteSet[pfull_id] = pteStruct;
    }
  }

}
