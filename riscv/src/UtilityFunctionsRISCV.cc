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
#include <Log.h>
#include <UtilityFunctionsRISCV.h>

#include <string>

using namespace std;

namespace Force {

  void privilege_prefix(std::string& rStr, uint32 priv, bool upper)
  {
    privilege_prefix(rStr, EPrivilegeLevelType(priv), upper);
  }

  void privilege_prefix(std::string& rStr, EPrivilegeLevelType priv, bool upper)
  {
    switch (priv)
    {
      case EPrivilegeLevelType::M:
        rStr = (upper) ? "M" : "m";
        break;
      case EPrivilegeLevelType::H:
        rStr = (upper) ? "H" : "h";
        break;
      case EPrivilegeLevelType::S:
        rStr = (upper) ? "S" : "s";
        break;
      case EPrivilegeLevelType::U:
        rStr = (upper) ? "U" : "u";
        break;
      default:
        LOG(warn) << "{UtilityFunctionsRISCV::privilege_register_prefix} unknown priv level type=" << EPrivilegeLevelType_to_string(priv) << endl;
        break;
    }
  }

  bool illegal_exception_return(uint32 targetPriv, uint32 currentPriv)
  {
    if (targetPriv > currentPriv) return true;

    return false;
  }

  uint32 table_level_to_addr_high_bit(uint32 level)
  {
    switch(level)
    {
      case 3:
        return 47;
      case 2:
        return 38;
      case 1:
        return 29;
      case 0:
        return 20;
      default:
        return 0;
    }

    return 0;
  }

  EPagingExceptionType get_exception_type_from_access_type(bool isInstr, EMemAccessType memAccess)
  {
    if (isInstr)
    {
      return EPagingExceptionType::InstructionPageFault;
    }

    if (memAccess == EMemAccessType::ReadWrite or memAccess == EMemAccessType::Write)
    {
      return EPagingExceptionType::StoreAmoPageFault;
    }

    return EPagingExceptionType::LoadPageFault; //TODO need to handle unknown memaccess potentially
  }
}
