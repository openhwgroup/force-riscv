//
// Copyright (C) [2020] Futurewei Technologies, Inc.
//
// FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
// FIT FOR A PARTICULAR PURPOSE.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "StringUtils.h"

#include "lest/lest.hpp"

#include "Log.h"

using text = std::string;
using namespace Force;

const lest::test specification[] = {

CASE("Test string conversions and manipulations") {

  SETUP("Create a test string")  {
    std::string test_string;

    SECTION("Test converting a binary string to uint32") {
      test_string = "10101001011011110000011110000101";
      bool error_status = false;
      uint32 val = parse_bin32(test_string, &error_status);
      EXPECT(val == uint32(2842625925));
      EXPECT_NOT(error_status);

      test_string = "hello";
      error_status = false;
      parse_bin32(test_string, &error_status);
      EXPECT(error_status);

      EXPECT_FAIL(parse_bin32(test_string, nullptr), "parse-error-bin32-invalid");

/* Doesn't work because the stoul functions doesn't detect 32-bit overflow like it should.
      test_string = "100000000000000000000000000000000";
//      test_string = "11111111111111111111111111111111111111111111111111111111111111111";
      error_status = false;
      val = parse_bin32(test_string, &error_status);
      EXPECT(error_status);
*/
    }

    SECTION("Test converting a binary string to uint64") {
      test_string = "0101010010110111100000111100001011010100101101111000001111000011";
      bool error_status = false;
      uint64 val = parse_bin64(test_string, &error_status);
      EXPECT(val == uint64(0x54b783c2d4b783c3));    // == 6104492692739687363
      EXPECT_NOT(error_status);

      test_string = "hello";
      error_status = false;
      parse_bin64(test_string, &error_status);
      EXPECT(error_status);

      EXPECT_FAIL(parse_bin64(test_string, nullptr), "parse-error-bin64-invalid");

      test_string = "10000000000000000000000000000000000000000000000000000000000000000";
      error_status = false;
      val = parse_bin64(test_string, &error_status);
      EXPECT(error_status);
    }

    SECTION("Test converting a string to uint32 range") {
      test_string = "1234-5678";
      uint32 low = 0;
      uint32 high = 0;
      bool success = parse_range32(test_string, low, high);
      EXPECT(low == uint32(1234));
      EXPECT(high == uint32(5678));
      EXPECT(success);

      test_string = "34567";
      success = parse_range32(test_string, low, high);
      EXPECT_NOT(success);

      test_string = "1234-1234";
      success = parse_range32(test_string, low, high);
      EXPECT_NOT(success);
    }

    SECTION("Test converting a string to uint64 range") {
      test_string = "1234567890-567890";
      uint64 low = 0;
      uint64 high = 0;
      bool success = parse_range64(test_string, low, high);
      EXPECT(low == uint64(567890));
      EXPECT(high == uint64(1234567890));
      EXPECT(success);

      test_string = "34567";
      success = parse_range64(test_string, low, high);
      EXPECT(low == uint64(34567));
      EXPECT_NOT(success);

      test_string = "1234567890-1234567890";
      success = parse_range64(test_string, low, high);
      EXPECT(low == uint64(1234567890));
      EXPECT_NOT(success);

      test_string = "0x4000000000000-0x400000000ffff";
      success = parse_range64(test_string, low, high);
      EXPECT(low == uint64(0x4000000000000));
      EXPECT(high == uint64(0x400000000ffff));
      EXPECT(success);
    }

    SECTION("Test converting a string to uint32") {
      test_string = "12345";
      bool error_status = false;
      uint32 val = parse_uint32(test_string, &error_status);
      EXPECT(val == uint32(12345));
      EXPECT_NOT(error_status);

      test_string = "hello";
      error_status = false;
      parse_uint32(test_string, &error_status);
      EXPECT(error_status);

/* Doesn't work because the stoul functions doesn't detect 32-bit overflow like it should.
      test_string = "0x1ffffffff";
      error_status = false;
      parse_uint32(test_string, &error_status);
      EXPECT(error_status);
*/
    }

    SECTION("Test converting a string to uint64") {
      test_string = "1234567";
      bool error_status = false;
      uint64 val = parse_uint64(test_string, &error_status);
      EXPECT(val == uint64(1234567));
      EXPECT_NOT(error_status);

      test_string = "hello";
      error_status = false;
      val = parse_uint64(test_string, &error_status);
      EXPECT(error_status);

      test_string = "0xffffffffffffffffffffff";
      error_status = false;
      val = parse_uint64(test_string, &error_status);
      EXPECT(error_status);
    }

    SECTION("Test converting a string to float") {
      test_string = "0.25";
      bool error_status = false;
      float val = parse_float(test_string, &error_status);
      EXPECT(val == 0.25f);
      EXPECT_NOT(error_status);

      test_string = "20.23";
      val = parse_float(test_string, &error_status);
      EXPECT(val == 20.23f);
      EXPECT_NOT(error_status);

      test_string = "hello";
      parse_float(test_string, &error_status);
      EXPECT(error_status);
      error_status = false;
      EXPECT_FAIL(parse_float(test_string), "parse-error-float-invalid");

      test_string = "1.5e100";
      parse_float(test_string, &error_status);
      EXPECT(error_status);
      error_status = false;
      EXPECT_FAIL(parse_float(test_string), "parse-error-float-range");
    }

    SECTION("Test converting a string to bool") {
      test_string = "true";
      bool val = parse_bool(test_string);
      EXPECT(val == true);

      test_string = "True";
      val = parse_bool(test_string);
      EXPECT(val == true);

      test_string = "false";
      val = parse_bool(test_string);
      EXPECT(val == false);

      test_string = "False";
      val = parse_bool(test_string);
      EXPECT(val == false);

      test_string = "hello";
      EXPECT_FAIL(parse_bool(test_string), "parse-error-bool");
    }

    SECTION("Test parsing assignment expressions") {
      test_string = "valid         name =                  valid value             ";
      std::string name;
      std::string value;
      bool success = parse_assignment(test_string, name, value);
      EXPECT(name == "validname");
      EXPECT(value == "validvalue");
      EXPECT(success);

      test_string = "name";
      success = parse_assignment(test_string, name, value);
      EXPECT_NOT(success);

      test_string = "=dummy value";
      success = parse_assignment(test_string, name, value);
      EXPECT_NOT(success);

      test_string = "dummy name=";
      success = parse_assignment(test_string, name, value);
      EXPECT_NOT(success);
    }

    SECTION("Test parsing brackets strings") {
      std::vector<std::string> v1;
      test_string = "(One (two (three))  (four))";
      std::vector<std::string> v2 = {"One (two (three", "four))", "four"};
      bool success = parse_brackets_strings(test_string, v1);
      EXPECT(success);
      EXPECT(v1 == v2);

      test_string = "(One (two (three))  (four";
      success = parse_brackets_strings(test_string, v1);
      EXPECT_NOT(success);
    }

    SECTION("Test removing white space") {
      test_string = "  Test String With White Spaces  ";
      strip_white_spaces(test_string);
      EXPECT(test_string == "TestStringWithWhiteSpaces");
    }
  }
},

CASE("Test string splitting") {

  SETUP("Setup StringSplitter")  {
    std::string test_string("This,Is,A,Test,String");
    StringSplitter string_splitter(test_string, ',');

    SECTION("Test getting the next substring") {
      EXPECT(string_splitter.NextSubString() == "This");
      EXPECT(string_splitter.NextSubString() == "Is");
      EXPECT(string_splitter.NextSubString() == "A");
      EXPECT(string_splitter.NextSubString() == "Test");
      EXPECT(string_splitter.NextSubString() == "String");
      EXPECT_FAIL(string_splitter.NextSubString(), "string-splitting-out-of-range");
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
