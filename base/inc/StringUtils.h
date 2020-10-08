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
#ifndef Force_StringUtils_H
#define Force_StringUtils_H

#include <Defines.h>
#include <string>
#include <vector>

namespace Force {

  uint32 parse_bin32(const std::string& rStr, bool* pErrorStatus=nullptr); //!< Parse binary format value string into a uint32 type.
  uint64 parse_bin64(const std::string& rStr, bool* pErrorStatus=nullptr); //!< Parse binary format value string into a uint64 type.
  bool parse_range32(const std::string& rStr, uint32& rRangeLow, uint32& rRangeHigh); //!< Parse a range/value string, return true if the string is indeed a range.
  bool parse_range64(const std::string& rStr, uint64& rRangeLow, uint64& rRangeHigh); //!< Parse a range/value string, return true if the string is indeed a range.
  uint32 parse_uint32(const std::string& rStr, bool* pErrorStatus=nullptr); //<! Parse value string into a uint32 type.
  uint64 parse_uint64(const std::string& rStr, bool* pErrorStatus=nullptr); //<! Parse value string into a uint64 type.
  float parse_float(const std::string& rStr, bool* pErrorStatus=nullptr); //<! Parse value string into a float type.
  bool parse_bool(const std::string& rStr); //<! Parse value string into a bool type
  bool parse_assignment(const std::string& rStr, std::string& rVar, std::string& rVal); //!< Parse an assignment string in the format of "a=b", where both a and b could be an expression.
  void strip_white_spaces(std::string& rStr); //!< Strip white spaces off the string.
  bool parse_brackets_strings(const std::string& rStr, std::vector<std::string>& rStrings);  //!< return strings on brackets

  /*!
    \class StringSplitter
    \brief A convenience class for splitting string.

    Split a given string into multiple sub-strings delimited by the given separator.
  */
  class StringSplitter {
  public:
    StringSplitter(const std::string& rStr, char sep, bool no_skip=false); //<! Constructor with string and separator given.

    bool EndOfString() const; //!< Check if already at end of string.
    const std::string NextSubString() const;
  private:
    const std::string& mString; //!< Constant reference to the string being worked on.
    char mSeparator; //!< Separator char.
    mutable std::string::size_type mStart; //!< Starting location of the current sub-string.
    bool mNoSkip; //!< Whether empty sub-string skip
  };

}

#endif
