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
#include <cstdint>
#include <map>
#include <string>

namespace Force
{
class ParseGuide
{
    private:
        std::string _mName;
        int32_t * _mpInt32;
        int64_t * _mpInt64;
        uint32_t * _mpUint32;
        uint64_t * _mpUint64;
        std::string * _mpString;
 
    public:
        ParseGuide(const ParseGuide & other) : _mName(other._mName), _mpInt32(other._mpInt32), _mpInt64(other._mpInt64), _mpUint32(other._mpUint32), _mpUint64(other._mpUint64), _mpString(other._mpString) {};
 
        ParseGuide& operator=(const ParseGuide & other)
        {
            _mName = other._mName;
            _mpInt32 = other._mpInt32;
            _mpInt64 = other._mpInt64;
            _mpUint32 = other._mpUint32; 
            _mpUint64 = other._mpUint64;
            _mpString = other._mpString;
            return *this;
        }
 
        explicit ParseGuide(const std::string & arName, int32_t * apInt) : _mName(arName), _mpInt32(apInt), _mpInt64(nullptr), _mpUint32(nullptr), _mpUint64(nullptr), _mpString(nullptr){};
        explicit ParseGuide(const std::string & arName, int64_t * apInt) : _mName(arName), _mpInt32(nullptr), _mpInt64(apInt), _mpUint32(nullptr), _mpUint64(nullptr), _mpString(nullptr){};
        explicit ParseGuide(const std::string & arName, uint32_t * apInt) : _mName(arName), _mpInt32(nullptr), _mpInt64(nullptr), _mpUint32(apInt), _mpUint64(nullptr), _mpString(nullptr){};
        explicit ParseGuide(const std::string & arName, uint64_t * apInt) : _mName(arName), _mpInt32(nullptr), _mpInt64(nullptr), _mpUint32(nullptr), _mpUint64(apInt), _mpString(nullptr){};
        explicit ParseGuide(const std::string & arName, std::string * apString) : _mName(arName), _mpInt32(nullptr), _mpInt64(nullptr), _mpUint32(nullptr), _mpUint64(nullptr), _mpString(apString){};
 
        bool parse(std::map<std::string, std::string> & arOptionsMap);
};
};
