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
#include <StringUtils.h>
#include <Log.h>
#include <algorithm>
#include <sstream>

using namespace std;

namespace Force {

  uint64 parse_uint64(const string& str, bool* error_status)
  {
    size_t pos = 0;
    uint64 ret_val = 0;

    try {
      ret_val = stoull(str, &pos, 0);
    }
    catch (invalid_argument &iaE) {
      if (error_status) {
        //LOG(error) << "{parse_uint64} Error parsing \'" << str << "\', invalid argument." << endl;
        *error_status = true;
      } else {
        LOG(fail) << "Failed parsing \'" << str << "\', invalid argument." << endl;
        FAIL("parse-error-uint64-invalid");
      }
    }
    catch (out_of_range &oorE) {
      if (error_status) {
        //LOG(error) << "{parse_uint64} Error parsing \'" << str << "\', invalid argument" << endl;
        *error_status = true;
      } else {
        LOG(fail) << "Failed parsing \'" << str << "\', out of range." << endl;
        FAIL("parse-error-uint64-range");
      }
    }
    return ret_val;
  }

  uint32 parse_uint32(const string& str, bool* error_status)
  {
    size_t pos = 0;
    uint32 ret_val = 0;

    try {
      ret_val = stoul(str, &pos, 0);
    }
    catch (invalid_argument &iaE) {
      if (error_status) {
        //LOG(error) << "{parse_uint32} Error parsing \'" << str << "\', invalid argument." << endl;
        *error_status = true;
      } else {
        LOG(fail) << "Failed parsing \'" << str << "\', invalid argument." << endl;
        FAIL("parse-error-uint32-invalid");
      }
    }
    catch (out_of_range &oorE) {
      if (error_status) {
        //LOG(error) << "{parse_uint32} Error parsing \'" << str << "\', invalid argument" << endl;
        *error_status = true;
      } else {
        LOG(fail) << "Failed parsing \'" << str << "\', out of range." << endl;
        FAIL("parse-error-uint32-range");
      }
    }
    return ret_val;
  }

  bool parse_bool(const string& str)
  {
    if (str == "true" || str == "True")
      return true;
    else if (str == "false" || str == "False")
      return false;

    LOG(fail) << "Failed parsing \'" << str << "\', failed." << endl;
    FAIL("parse-error-bool");
    return false;
  }

  uint32 parse_bin32(const string& str, bool* error_status)
  {
    size_t pos = 0;
    uint32 ret_val = 0;

    try {
      ret_val = stoul(str, &pos, 2);
    }
    catch (invalid_argument &iaE) {
      if (error_status) {
        LOG(error) << "{parse_bin32} Error parsing \'" << str << "\', invalid argument." << endl;
        *error_status = true;
      } else {
        LOG(fail) << "Failed parsing \'" << str << "\', invalid argument." << endl;
        FAIL("parse-error-bin32-invalid");
      }
    }
    catch (out_of_range &oorE) {
      if (error_status) {
        LOG(error) << "{parse_bin32} Error parsing \'" << str << "\', out of range." << endl;
        *error_status = true;
      } else {
        LOG(fail) << "Failed parsing \'" << str << "\', out of range." << endl;
        FAIL("parse-error-bin32-range");
      }
    }

    return ret_val;
  }

  uint64 parse_bin64(const string& str, bool* error_status)
  {
    size_t pos = 0;
    uint64 ret_val = 0;

    try {
      ret_val = stoull(str, &pos, 2);
    }
    catch (invalid_argument &iaE) {
      if (error_status) {
        LOG(error) << "{parse_bin64} Error parsing \'" << str << "\', invalid argument." << endl;
        *error_status = true;
      } else {
        LOG(fail) << "Failed parsing \'" << str << "\', invalid argument." << endl;
        FAIL("parse-error-bin64-invalid");
      }
    }
    catch (out_of_range &oorE) {
      if (error_status) {
        LOG(error) << "{parse_bin64} Error parsing \'" << str << "\', out of range." << endl;
        *error_status = true;
      } else {
        LOG(fail) << "Failed parsing \'" << str << "\', out of range." << endl;
        FAIL("parse-error-bin64-range");
      }
    }

    return ret_val;
  }

  bool parse_range32(const string& str, uint32& range_low, uint32& range_high)
  {
    for (string::size_type pos = 0; pos < str.size(); ++ pos) {
      if (str[pos] == '-') {
        string first_part = str.substr(0, pos);
        range_high = parse_uint32(first_part);
        string second_part = str.substr(pos + 1);
        range_low = parse_uint32(second_part);
        if (range_high < range_low) {
          uint32 temp = range_high;
          range_high = range_low;
          range_low = temp;
        }
        return (range_high > range_low);
      }
    }
    range_low = parse_uint32(str);
    range_high = range_low;
    return false;
  }

  /*!
    The str coming into parse_range64 tend to be in lower-higher order, different from parse_range32
  */
  bool parse_range64(const string& str, uint64& range_low, uint64& range_high)
  {
    for (string::size_type pos = 0; pos < str.size(); ++ pos) {
      if (str[pos] == '-') {
        string first_part = str.substr(0, pos);
        range_low = parse_uint64(first_part);
        string second_part = str.substr(pos + 1);
        range_high = parse_uint64(second_part);
        if (range_high < range_low) {
          uint64 temp = range_high;
          range_high = range_low;
          range_low = temp;
        }
        return (range_high > range_low);
      }
    }
    range_low = parse_uint64(str);
    range_high = range_low;
    return false;
  }

  bool parse_assignment(const string& str, string& var, string& val)
  {
    for (string::size_type pos = 0; pos < str.size(); ++ pos) {
      if (str[pos] == '=') {
        var = str.substr(0, pos);
        strip_white_spaces(var);
        val = str.substr(pos + 1);
        strip_white_spaces(val);

        if (var.size() == 0) return false;
        if (val.size() == 0) return false;

        return true;
      }
    }

    return false;
  }

  bool parse_brackets_strings(const string& str, vector<string>& strings)
  {
    size_t prefix = str.find("(");
    if (prefix == string::npos)
      return true;
    size_t postfix = str.find(")");
    if (postfix == string::npos)
      return false;
    string eval = str.substr(prefix + 1, postfix - prefix - 1);
    strings.push_back(eval);
    string remains = str.substr(postfix + 1);
    return parse_brackets_strings(remains, strings);
  }

  void strip_white_spaces(string& in_str)
  {
    string::iterator end_pos = remove(in_str.begin(), in_str.end(), ' ');
    in_str.erase(end_pos, in_str.end());
  }

  StringSplitter::StringSplitter(const string& in_str, char sep, bool no_skip)
    : mString(in_str), mSeparator(sep), mStart(0), mNoSkip(no_skip)
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
