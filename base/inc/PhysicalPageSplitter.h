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
#ifndef Force_PhysicalPageSplitter_H
#define Force_PhysicalPageSplitter_H

#include <Defines.h>

namespace Force {

  struct PhysicalPageSplit {
  public:
    uint64 mPa1; // Starting physical address for range intersecting with first page.
    uint32 mSize1; // Size of range intersecting with first page.
    uint32 mBank1; // Memory bank for first page.
    uint64 mPa2; // Starting physical address for range intersecting with second page.
    uint32 mSize2; // Size of range intersecting with second page.
    uint32 mBank2; // Memory bank for second page.
  };

  class VmMapper;
  class TranslationRange;

  /*!
    \class PhysicalPageSplitter
    \brief Class to split a VA range into one or more PA ranges, according to page boundaries.
  */
  class PhysicalPageSplitter {
  public:
    explicit PhysicalPageSplitter(const VmMapper* pVmMapper); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(PhysicalPageSplitter);
    DESTRUCTOR_DEFAULT(PhysicalPageSplitter); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(PhysicalPageSplitter);

    PhysicalPageSplit GetPhysicalPageSplit(cuint64 va, cuint32 size) const; //!< Get PA ranges corresponding to VA range.
  private:
    void GetTranslationRange(cuint64 va, TranslationRange& rTransRange) const; //!< Get translation range for the specified VA.
  private:
    const VmMapper* mpVmMapper;
  };

}

#endif  // Force_PhysicalPageSplitter_H
