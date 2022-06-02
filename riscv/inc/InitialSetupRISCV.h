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
#ifndef Force_InitialSetupRISCV_H
#define Force_InitialSetupRISCV_H

#include <map>

#include "Defines.h"
#include "Enums.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class Config;
  class Generator;
  class ChoiceTree;

  /*!
    \struct SystemOption
    \brief Handles system options.
  */
  struct SystemOption {
  public:
    ESystemOptionType mType; //!< System option type.
    uint32 mValue; //!< System option value.
    bool mValid; //!< System option specified.
  public:
    explicit SystemOption(ESystemOptionType sysOpType) //!< Constructor.
      : mType(sysOpType), mValue(0), mValid(false)
    {

    }

    void Process(const Config& rConfig); //!< Process the system option.
  };

  /*!
    \class InitialSetupRISCV
    \brief Handles initial setup for RISCV architecture.
  */
  class InitialSetupRISCV {
  public:
    explicit InitialSetupRISCV(Generator* pGen); //!< Constructor with pointer to Generator given.
    ~InitialSetupRISCV(); //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(InitialSetupRISCV);
    COPY_CONSTRUCTOR_ABSENT(InitialSetupRISCV);
    void Process(); //!< Process initial setup.
  protected:
    const Config* mpConfig; //!< Pointer to Config object.
    Generator* mpGenerator; //!< Pointer to generator.
    SystemOption* mpFlatMapOption; //!< Flat Map system option
    SystemOption* mpPrivilegeLevelOption; //!< Privilege level system option
    SystemOption* mpDisablePagingOption; //!< Disable paging system option
  };

}

#endif //Force_InitialSetupRISCV_H
