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
#ifndef Force_UtilityFunctionsRISCV_H
#define Force_UtilityFunctionsRISCV_H

#include <Defines.h>
#include <Enums.h>
#include ARCH_ENUM_HEADER

namespace Force {

  void privilege_prefix(std::string& rStr, uint32 priv, bool upper=false);
  void privilege_prefix(std::string& rStr, EPrivilegeLevelType priv, bool upper=false);
  bool illegal_exception_return(uint32 targetPriv, uint32 currentPriv);
  uint32 table_level_to_addr_high_bit(uint32 level);
  EPagingExceptionType get_exception_type_from_access_type(bool isInstr, EMemAccessType memAccess);
}
#endif //Force_UtilityFunctionsRISCV_H
