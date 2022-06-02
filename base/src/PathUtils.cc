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
#include "PathUtils.h"

#include <unistd.h>

#include <climits>
#include <cstring>

#include "Log.h"

using namespace std;

namespace Force {

  /*!
    Generally useful when given a file name with full path, then return the name of the directory containing the file
   */
  string get_parent_path(const std::string& in_path)
  {
    string real_path = get_real_path(in_path.c_str());
    string::size_type pos = real_path.find_last_of('/');
    if (pos == string::npos) {
      LOG(fail) << "Failed to find parent path of \"" << in_path << "\"." << endl;
      FAIL("get-parent-path-fail");
    }
    return real_path.substr(0, pos);
  }

  string get_file_stem(const string& in_path)
  {
    string file_name = in_path;
    string::size_type pos = in_path.find_last_of('/');
    if (pos != string::npos) {
      file_name = in_path.substr(pos + 1);
    }
    pos = file_name.find_last_of('.');
    if (pos != string::npos) {
      file_name = file_name.substr(0, pos);
    }
    return file_name;
  }

  /*!
    A thin wrapper around C realpath function.
   */
  string get_real_path(const char* in_path)
  {
    char actual_path[PATH_MAX];

    if (!realpath(in_path, actual_path)) {
      LOG(fail) << "Failed to find real path for \"" << in_path << "\"." << endl;
      FAIL("get-real-path-fail");
    }

    return string(actual_path);
  }

  /*!
    fail_not_valid default to true.  When fail_not_valid is true, will fail out if the file_name doesn't exist or cannot be read.  If fail_not_valid is false, will just return false.
   */
  bool verify_file_path(const std::string& file_name, bool fail_not_valid)
  {
    if (::access(file_name.c_str(), F_OK | R_OK)) {
      if (fail_not_valid) {
        LOG(fail) << "Failed verifying file \'" << file_name << "\' due to : " << strerror(errno) << endl;
        FAIL("fail-verify-file-path");
      } else {
        return false;
      }
    }
    return true;
  }

}
