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
#include "StringUtils.h"

#include <algorithm>
#include <sstream>

#include "Log.h"

using namespace std;

namespace Force {

  uint64 parse_uint64(const string& rStr, bool* pErrorStatus)
  {
    size_t pos = 0;
    uint64 ret_val = 0;

    try {
      ret_val = stoull(rStr, &pos, 0);
    }
    catch (invalid_argument& iaE) {
      if (pErrorStatus) {
        //LOG(error) << "{parse_uint64} Error parsing \'" << rStr << "\', invalid argument." << endl;
        *pErrorStatus = true;
      } else {
        LOG(fail) << "Failed parsing \'" << rStr << "\', invalid argument." << endl;
        FAIL("parse-error-uint64-invalid");
      }
    }
    catch (out_of_range& oorE) {
      if (pErrorStatus) {
        //LOG(error) << "{parse_uint64} Error parsing \'" << rStr << "\', invalid argument" << endl;
        *pErrorStatus = true;
      } else {
        LOG(fail) << "Failed parsing \'" << rStr << "\', out of range." << endl;
        FAIL("parse-error-uint64-range");
      }
    }
    return ret_val;
  }

  uint32 parse_uint32(const string& rStr, bool* pErrorStatus)
  {
    size_t pos = 0;
    uint32 ret_val = 0;

    try {
      ret_val = stoul(rStr, &pos, 0);
    }
    catch (invalid_argument& iaE) {
      if (pErrorStatus) {
        //LOG(error) << "{parse_uint32} Error parsing \'" << rStr << "\', invalid argument." << endl;
        *pErrorStatus = true;
      } else {
        LOG(fail) << "Failed parsing \'" << rStr << "\', invalid argument." << endl;
        FAIL("parse-error-uint32-invalid");
      }
    }
    catch (out_of_range& oorE) {
      if (pErrorStatus) {
        //LOG(error) << "{parse_uint32} Error parsing \'" << rStr << "\', invalid argument" << endl;
        *pErrorStatus = true;
      } else {
        LOG(fail) << "Failed parsing \'" << rStr << "\', out of range." << endl;
        FAIL("parse-error-uint32-range");
      }
    }
    return ret_val;
  }

  float parse_float(const string& rStr, bool* pErrorStatus)
  {
    float ret_val = 0;

    try {
      ret_val = stof(rStr);
    }
    catch (invalid_argument&) {
      if (pErrorStatus) {
        LOG(debug) << "{parse_float} Error parsing \'" << rStr << "\', invalid argument." << endl;
        *pErrorStatus = true;
      } else {
        LOG(fail) << "{parse_float} Failed parsing \'" << rStr << "\', invalid argument." << endl;
        FAIL("parse-error-float-invalid");
      }
    }
    catch (out_of_range&) {
      if (pErrorStatus) {
        LOG(debug) << "{parse_float} Error parsing \'" << rStr << "\', invalid argument" << endl;
        *pErrorStatus = true;
      } else {
        LOG(fail) << "{parse_float} Failed parsing \'" << rStr << "\', out of range." << endl;
        FAIL("parse-error-float-range");
      }
    }

    return ret_val;
  }

  bool parse_bool(const string& rStr)
  {
    if (rStr == "true" || rStr == "True")
      return true;
    else if (rStr == "false" || rStr == "False")
      return false;

    LOG(fail) << "Failed parsing \'" << rStr << "\', failed." << endl;
    FAIL("parse-error-bool");
    return false;
  }

  uint32 parse_bin32(const string& rStr, bool* pErrorStatus)
  {
    size_t pos = 0;
    uint32 ret_val = 0;

    try {
      ret_val = stoul(rStr, &pos, 2);
    }
    catch (invalid_argument& iaE) {
      if (pErrorStatus) {
        LOG(error) << "{parse_bin32} Error parsing \'" << rStr << "\', invalid argument." << endl;
        *pErrorStatus = true;
      } else {
        LOG(fail) << "Failed parsing \'" << rStr << "\', invalid argument." << endl;
        FAIL("parse-error-bin32-invalid");
      }
    }
    catch (out_of_range& oorE) {
      if (pErrorStatus) {
        LOG(error) << "{parse_bin32} Error parsing \'" << rStr << "\', out of range." << endl;
        *pErrorStatus = true;
      } else {
        LOG(fail) << "Failed parsing \'" << rStr << "\', out of range." << endl;
        FAIL("parse-error-bin32-range");
      }
    }

    return ret_val;
  }

  uint64 parse_bin64(const string& rStr, bool* pErrorStatus)
  {
    size_t pos = 0;
    uint64 ret_val = 0;

    try {
      ret_val = stoull(rStr, &pos, 2);
    }
    catch (invalid_argument& iaE) {
      if (pErrorStatus) {
        LOG(error) << "{parse_bin64} Error parsing \'" << rStr << "\', invalid argument." << endl;
        *pErrorStatus = true;
      } else {
        LOG(fail) << "Failed parsing \'" << rStr << "\', invalid argument." << endl;
        FAIL("parse-error-bin64-invalid");
      }
    }
    catch (out_of_range& oorE) {
      if (pErrorStatus) {
        LOG(error) << "{parse_bin64} Error parsing \'" << rStr << "\', out of range." << endl;
        *pErrorStatus = true;
      } else {
        LOG(fail) << "Failed parsing \'" << rStr << "\', out of range." << endl;
        FAIL("parse-error-bin64-range");
      }
    }

    return ret_val;
  }

  bool parse_range32(const string& rStr, uint32& rRangeLow, uint32& rRangeHigh)
  {
    for (string::size_type pos = 0; pos < rStr.size(); ++ pos) {
      if (rStr[pos] == '-') {
        string first_part = rStr.substr(0, pos);
        rRangeHigh = parse_uint32(first_part);
        string second_part = rStr.substr(pos + 1);
        rRangeLow = parse_uint32(second_part);
        if (rRangeHigh < rRangeLow) {
          swap(rRangeLow, rRangeHigh);
        }
        return (rRangeHigh > rRangeLow);
      }
    }
    rRangeLow = parse_uint32(rStr);
    rRangeHigh = rRangeLow;
    return false;
  }

  /*!
    The str coming into parse_range64 tend to be in lower-higher order, different from parse_range32
  */
  bool parse_range64(const string& rStr, uint64& rRangeLow, uint64& rRangeHigh)
  {
    for (string::size_type pos = 0; pos < rStr.size(); ++ pos) {
      if (rStr[pos] == '-') {
        string first_part = rStr.substr(0, pos);
        rRangeLow = parse_uint64(first_part);
        string second_part = rStr.substr(pos + 1);
        rRangeHigh = parse_uint64(second_part);
        if (rRangeHigh < rRangeLow) {
          swap(rRangeLow, rRangeHigh);
        }
        return (rRangeHigh > rRangeLow);
      }
    }
    rRangeLow = parse_uint64(rStr);
    rRangeHigh = rRangeLow;
    return false;
  }

  bool parse_assignment(const string& rStr, string& rVar, string& rVal)
  {
    for (string::size_type pos = 0; pos < rStr.size(); ++ pos) {
      if (rStr[pos] == '=') {
        rVar = rStr.substr(0, pos);
        strip_white_spaces(rVar);
        rVal = rStr.substr(pos + 1);
        strip_white_spaces(rVal);

        if (rVar.size() == 0) return false;
        if (rVal.size() == 0) return false;

        return true;
      }
    }

    return false;
  }

  bool parse_brackets_strings(const string& rStr, vector<string>& rStrings)
  {
    size_t prefix = rStr.find("(");
    if (prefix == string::npos)
      return true;
    size_t postfix = rStr.find(")");
    if (postfix == string::npos)
      return false;
    string eval = rStr.substr(prefix + 1, postfix - prefix - 1);
    rStrings.push_back(eval);
    string remains = rStr.substr(postfix + 1);
    return parse_brackets_strings(remains, rStrings);
  }

  void strip_white_spaces(string& rStr)
  {
    string::iterator end_pos = remove(rStr.begin(), rStr.end(), ' ');
    rStr.erase(end_pos, rStr.end());
  }

  StringSplitter::StringSplitter(const string& rStr, char sep, bool no_skip)
    : mString(rStr), mSeparator(sep), mStart(0), mNoSkip(no_skip)
  {
  }

  bool StringSplitter::EndOfString() const
  {
    return mStart >= mString.size();
  }

  const string StringSplitter::NextSubString() const
  {
    string::size_type cur_index = mStart;
    string::size_type str_size = mString.size();
    for (; cur_index < str_size; ++ cur_index) {
      if (mString[cur_index] == mSeparator) {
        if (cur_index > mStart or mNoSkip) {
          string sub_str = mString.substr(mStart, (cur_index - mStart));
          mStart = cur_index + 1;
          return sub_str;
        } else {
          // special case, empty sub-string, skip
          mStart = cur_index + 1;
        }
      }
    }

    if (cur_index > mStart or mNoSkip) {
      string sub_str = mString.substr(mStart, (cur_index - mStart));
      mStart = cur_index;
      return sub_str;
    } else {
      LOG(fail) << "String splitting out of range." << endl;
      FAIL("string-splitting-out-of-range");
      return "";
    }
  }

}
