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
#ifndef Force_PathUtils_H
#define Force_PathUtils_H

#include <string>

namespace Force {

  std::string get_parent_path(const std::string& in_path); //!< Return parent path of a path string.
  std::string get_file_stem(const std::string& in_path); //!< Return the filename stem of a path string
  std::string get_real_path(const char* in_path); //!< Obtain real path of the file name passed in through \param in_path.
  bool verify_file_path(const std::string& file_name, bool fail_not_valid=true); //<! Verify if a file path exist and readable.

}

#endif
