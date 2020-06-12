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
#ifndef Force_PagingInfo_H
#define Force_PagingInfo_H

#include <string>
#include <map>

namespace Force {

  class PteStructure;
  class ArchInfo;

  /*!
    \class PagingInfo
    \brief Info class for the paging system.

    PagingInfo handles parsing of all paging files and retain necessary information for the paging system.
  */

  class PagingInfo {
  public:
    PagingInfo(): mPteSet() { } //!< Constructor.
    virtual ~PagingInfo(); //!< Destructor.

    void Setup(const ArchInfo& archInfo); //!< Setup PagingInfo container, load paging files.
    const PteStructure* LookUpPte(const std::string& pteName) const; //!< Look up PteStructure object by PTE ID.
  protected:
    void AddPte(PteStructure* pteStruct); //!< Add a new instance of PteStructure.

  private:
    std::map<std::string, PteStructure* > mPteSet; //!< Map containing all PteStructure objects.
    friend class PagingParser;
  };

}

#endif
