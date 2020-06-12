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
#ifndef Force_PageInfoRecord_H
#define Force_PageInfoRecord_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER
#include <Object.h>
#include <vector>

#include <map>

namespace Force {

  /*!
    \struct PageTableInfoRec
    \brief store page table information
  */
  struct PageTableInfoRec {
    PageTableInfoRec() : level(0), descr_addr(0), descr_value(0), descr_details() { }
    uint32 level;
    uint64 descr_addr;
    uint64 descr_value;
    std::map<std::string, std::string> descr_details;
  };

  /*!
    \struct PageInfoRec
    \brief store page information
  */
  struct PageInfoRec {
    std::string memoryType;
    std::string memoryAttr;
    uint64 lower;
    uint64 upper;
    uint64 physical_lower;
    uint64 physical_upper;
    uint64 phys_id;
    uint64 page_size;
    uint64 mem_attr_index;
    uint64 descr_value;
    std::map<std::string, std::string> descr_details;
    PageInfoRec() : memoryType(""), memoryAttr(""), lower(0), upper(0), physical_lower(0), physical_upper(0), phys_id(0), page_size(0), mem_attr_index(0), descr_value(0), descr_details()
    { }
  };

  /*!
    \class PageInformation
    \brief set and retrieve page and parent table(s) information
  */
  class PageInformation {
  public:
    PageInformation() : mPageTableRecs(), mPageInfoRec() {}
    ~PageInformation() {}

    void SetPageTableRecord (const PageTableInfoRec& page_table) const
    {
      mPageTableRecs.push_back(page_table);
    }
    const std::vector<PageTableInfoRec>& GetPageTableRecord() const { return mPageTableRecs; }

    void SetPageInfoRecord (const PageInfoRec& page_info) const
    {
      mPageInfoRec = page_info;
    }
    const PageInfoRec& GetPageInfo() const { return mPageInfoRec; }

    void Clear()
    {
      mPageTableRecs.clear();
    }

    void Copy (const PageInformation& pageInformation) const
    {
      for (auto item : pageInformation.GetPageTableRecord())
      {
        SetPageTableRecord (item);
      }
      SetPageInfoRecord (pageInformation.GetPageInfo());
    }
  private:
    mutable std::vector<PageTableInfoRec> mPageTableRecs;
    mutable PageInfoRec mPageInfoRec;
  };

}

#endif
