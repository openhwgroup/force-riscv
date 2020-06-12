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
#include <lest/lest.hpp>
#include <Log.h>
#include <PathUtils.h>
#include <Defines.h>
#include <climits>
#include <string>
#include <unistd.h>

using text = std::string;
using namespace Force;

const lest::test specification[] = {

// Some of the methods tested here return absolute paths, which will vary depending on the local
// environment. Tests for such methods settle for verifying the latter part of the path, which
// should not depend on the environment, is as expected.
CASE("Test PathUtils") {

  SETUP("Setup PathUtils")  {

    SECTION("Test getting file names from paths") {
      std::string test_string = "/home/user1/docs/src/test.txt";
      EXPECT(get_file_stem(test_string) == "test");
      test_string = "/execute.test";
      EXPECT(get_file_stem(test_string) == "execute");
      test_string = "main hello string/hello0.test me helloMsg/hello0.txt";
      EXPECT(get_file_stem(test_string) == "hello0");
      test_string = "";
      EXPECT(get_file_stem(test_string) == "");
    }

    SECTION("Test getting the parent directory name from paths") {
      std::string test_string = "../../../../base/src/Config.cc";
      std::string parent_path = get_parent_path(test_string);
      std::string expected_path_suffix = "/base/src";
      uint64 suffix_start = parent_path.size() - expected_path_suffix.size();
      EXPECT(parent_path.substr(suffix_start) == expected_path_suffix);

      test_string = "../../../../utils/handcar/Makefile";
      parent_path = get_parent_path(test_string);
      expected_path_suffix = "/utils/handcar";
      suffix_start = parent_path.size() - expected_path_suffix.size();
      EXPECT(parent_path.substr(suffix_start) == expected_path_suffix);
    }

    SECTION("Test getting real path names from paths") {
      const char* test_string = "/home/user1/docs/src/test.txt";
      EXPECT_FAIL(get_real_path(test_string), "get-real-path-fail");

      test_string = "../../../../base/inc/Generator.h";
      std::string real_path = get_real_path(test_string);
      std::string expected_path_suffix = "/base/inc/Generator.h";
      uint64 suffix_start = real_path.size() - expected_path_suffix.size();
      EXPECT(real_path.substr(suffix_start) == expected_path_suffix);
    }

    SECTION("Test verifing file names from paths") {
      std::string test_string = "../../../../utils/handcar/SimLoader.h";
      bool val = verify_file_path(test_string, true);
      EXPECT(val == true);
      test_string = "/home/user1/docs/src/test.txt";
      EXPECT_FAIL(verify_file_path(test_string, true), "fail-verify-file-path");
    }
  }
},

};

int main(int argc, char* argv[])
{
  Force::Logger::Initialize();
  int ret = lest::run(specification, argc, argv);
  Force::Logger::Destroy();
  return ret;
}
