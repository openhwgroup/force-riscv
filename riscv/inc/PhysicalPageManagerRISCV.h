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
#ifndef Force_PhysicalPageManagerRISCV_H
#define Force_PhysicalPageManagerRISCV_H

#include "PhysicalPageManager.h"

namespace Force {

  /*!
    \class PhysicalPageManagerRISCV
    \brief Top level module for managing relationship between allocated physical and virtual pages.
  */
  class PhysicalPageManagerRISCV : public PhysicalPageManager {
  public:
    PhysicalPageManagerRISCV(EMemBankType bankType, MemoryTraitsManager* pMemTraitsManager); //!< Constructor
    COPY_CONSTRUCTOR_ABSENT(PhysicalPageManagerRISCV);
    SUBCLASS_DESTRUCTOR_DEFAULT(PhysicalPageManagerRISCV); //!< Destructor
    ASSIGNMENT_OPERATOR_ABSENT(PhysicalPageManagerRISCV);
  protected:
    const std::vector<EPteType>& GetPteTypes() const override { return mPteTypes; } //! Return vector of EPteTypes
  private:
    std::vector<EPteType> mPteTypes; //!< Vector of EPteTypes to maintain page aligned addresses
  };

}

#endif  // Force_PhysicalPageManagerRISCV_H
