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
#include "ParseGuide.h"
#include <string>
#include <iostream>
#include <Log.h>

namespace Force
{
bool ParseGuide::parse(std::map<std::string, std::string> & arOptionsMap)
{
    std::string whats_being_found = _mName;
    try
    {
        auto it = arOptionsMap.find(whats_being_found);
        if(it != arOptionsMap.end())
        {
            if(_mpInt32 != nullptr)
            {
                (*_mpInt32) = std::stoi(it->second);
                return true;
            }
            else if(_mpInt64 != nullptr)
            {
                (*_mpInt64) = std::stoll(it->second);
                return true;
            }
            else if(_mpUint32 != nullptr)
            {
                (*_mpUint32) = std::stoul(it->second);
                return true;
            }
            else if(_mpUint64 != nullptr)
            {
                (*_mpUint64) = std::stoull(it->second);
                return true;
            }
            else if(_mpString != nullptr)
            {
                (*_mpString) = it->second;
                return true;
            }
        }
    }
    catch(std::invalid_argument& e)
    {
        LOG(error) << "#### Error: [RegDependsChecker::parsePluginsOptions] No conversion could be performed while finding: " << whats_being_found << ", exception: " << e.what() << std::endl;
        throw;
    }
    catch(std::out_of_range& e)
    {
        LOG(error) << "#### Error: [RegDependsChecker::parsePluginsOptions] Argument is out of range while finding: " << whats_being_found << ", exception: " << e.what() << std::endl;
        throw;
    }
    catch(...)
    {
        throw;
    }

    return false;
};
};

