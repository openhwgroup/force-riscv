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

//------------------------------------------------
// include necessary header files here
//------------------------------------------------
#include <UtilityFunctions.h>
#include <string.h>

using text = std::string;

using namespace Force;
using namespace std;

const lest::test specification[] = {

CASE( "Test set 1 for Utility Functions" ) {

    SETUP( "basic number operation tests" )  {
        LOG(notice) << "Utility Functions Test Set 1..." << endl;

        // check enum size
        unsigned testEnumSize = 120;
        EXPECT_FAIL(check_enum_size (testEnumSize), "enum-class-too-large");
            
        // convert value to big endian data array
        uint8 dataArray[8];
        uint64 value64 = 0x1234567890ABCDEF;
        EXPECT_FAIL ( value_to_data_array_big_endian (value64, 10, dataArray), "unexpected-data-size" );
        value_to_data_array_big_endian (value64, 8, dataArray);
        EXPECT ( dataArray[0] == 0x12 );
        EXPECT ( dataArray[7] == 0xEF );

        // convert big endian data array to value
        uint8 big_endian_data_a[8] = {0x95, 0x9B, 0xB1, 0x9A, 0xDE, 0x89, 0xE6, 0xAC};
        EXPECT(data_array_to_value_big_endian(big_endian_data_a, 8) == 0x959BB19ADE89E6ACull);
        uint8 big_endian_data_b[5] = {0xA3, 0x95, 0xD3, 0x42, 0x4E};
        EXPECT(data_array_to_value_big_endian(big_endian_data_b, 5) == 0xA395D3424E000000ull);
        EXPECT_FAIL(data_array_to_value_big_endian(nullptr, 0), "unexpected-data-size");
        uint8 big_endian_data_c[9] = {0xAB, 0x0C, 0x97, 0x82, 0x39, 0x8E, 0x20, 0xE9, 0x70};
        EXPECT_FAIL(data_array_to_value_big_endian(big_endian_data_c, 9), "unexpected-data-size");

        // convert little endian data array to value
        uint8 little_endian_data_a[8] = {0x42, 0xDD, 0x60, 0x8D, 0x7E, 0xBD, 0x0A, 0x13};
        EXPECT(data_array_to_value_little_endian(little_endian_data_a, 8) == 0x130ABD7E8D60DD42ull);
        uint8 little_endian_data_b[3] = {0x1C, 0x30, 0x34};
        EXPECT(data_array_to_value_little_endian(little_endian_data_b, 3) == 0x34301Cull);
        EXPECT_FAIL(data_array_to_value_little_endian(nullptr, 0), "unexpected-data-size");
        uint8 little_endian_data_c[10] = {0x1F, 0x91, 0x37, 0x38, 0x97, 0x70, 0x6C, 0xC2, 0xAD, 0x4B};
        EXPECT_FAIL(data_array_to_value_little_endian(little_endian_data_c, 10), "unexpected-data-size");

        // convert element to big endian data array
        uint8 elementArray[4];
        value64 = 0x12345678;
        EXPECT_FAIL ( element_value_to_data_array_big_endian (value64, 5, elementArray), "unexpected-element-size" );
        element_value_to_data_array_big_endian (value64, 4, elementArray);
        EXPECT ( elementArray[0] == 0x12 );
        EXPECT ( elementArray[3] == 0x78 );
        
        // convert element to little endian data array
        value64 = 0x12345678;
        EXPECT_FAIL ( element_value_to_data_array_little_endian (value64, 5, elementArray), "unexpected-element-size" );
        element_value_to_data_array_little_endian (value64, 4, elementArray);
        EXPECT ( elementArray[0] == 0x78 );
        EXPECT ( elementArray[3] == 0x12 );
        
        // Convert big endian data array to element
        uint8 elementArray2[8] = {0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8};
        EXPECT_FAIL(data_array_to_element_value_big_endian(elementArray2, 0), "unexpected-data-size");
        EXPECT_FAIL(data_array_to_element_value_big_endian(elementArray2, 9), "unexpected-element-size");
        EXPECT_FAIL(data_array_to_element_value_big_endian(elementArray2, 3), "unexpected-element-size");
        EXPECT(data_array_to_element_value_big_endian(elementArray2, 8) == 0xf1f2f3f4f5f6f7f8ull);
        EXPECT(data_array_to_element_value_big_endian(elementArray2, 4) == 0xf1f2f3f4ull);

        // Convert little endian data array to element
        EXPECT_FAIL(data_array_to_element_value_little_endian(elementArray2, 0), "unexpected-data-size");
        EXPECT_FAIL(data_array_to_element_value_little_endian(elementArray2, 9), "unexpected-element-size");
        EXPECT_FAIL(data_array_to_element_value_little_endian(elementArray2, 3), "unexpected-element-size");
        EXPECT(data_array_to_element_value_little_endian(elementArray2, 8) == 0xf8f7f6f5f4f3f2f1ull);
        EXPECT(data_array_to_element_value_little_endian(elementArray2, 4) == 0xf4f3f2f1ull);

        // verify alignment
        uint64 alignmentTestVal = 1024;
        verify_alignment (alignmentTestVal);
        alignmentTestVal = 1011;
        EXPECT_FAIL ( verify_alignment (alignmentTestVal), "invalid-alignment" );
        uint32 shift = get_align_shift (1024);
        EXPECT ( shift == (uint32)10 );
        shift = get_align_shift (1048576);  // 1024 x 1024
        EXPECT ( shift == (uint32)20 );
        shift = get_align_shift (1073741824); // 1024 x 1024 x 1024
        EXPECT ( shift == (uint32)30 );

        // get aligned value
        EXPECT(get_aligned_value(0xD7BC8A07, 8) == 0xD7BC8A00ull);
        EXPECT(get_aligned_value(0xE395B563DFC6, 128) == 0xE395B563DF80ull);
        EXPECT_FAIL(get_aligned_value(0xC675, 165), "invalid-alignment");

        // sign extension
        uint64 value = 0x80000000;
        uint32 size = 32;
        uint64 signExtendedVal = sign_extend64 (value, size);
        uint64 expectedValue = 0xFFFFFFFF80000000;
        EXPECT ( signExtendedVal == expectedValue );

        // Get shift amount
        EXPECT(get_shift_amount("sh12") == 12u);
        EXPECT_FAIL(get_shift_amount("s12"), "unknown-shift-amount");
        EXPECT_FAIL(get_shift_amount("shift"), "unknown-shift-amount");

        // extend reg
        uint64 val64 = 0x40000000;
        uint64 ret_val = extend_regval (val64,  EExtendType::UXTW, 1);
        EXPECT ( ret_val == (uint64)0x80000000 );
        uint64 expectVal64 = 0x100000000;
        ret_val = extend_regval (val64, EExtendType::UXTX, 2);
        EXPECT ( ret_val == expectVal64 );
        val64 = 0x80000000;
        expectVal64 = 0xFFFFFFFF00000000;
        ret_val = extend_regval (val64, EExtendType::SXTW, 1);
        LOG(notice) << "Return value: " << hex << ret_val << endl;
        EXPECT ( ret_val == expectVal64 );
        val64 = 0x8000000000;
        expectVal64 = 0x10000000000;
        ret_val = extend_regval (val64, EExtendType::SXTX, 1);
        LOG(notice) << "Return value: " << hex << ret_val << endl;
        EXPECT ( ret_val == expectVal64 );

        // get offset
        bool offset_valid;
        ret_val = get_offset_field (expectVal64, 16, &offset_valid);
        EXPECT ( offset_valid == false );
        EXPECT ( ret_val == 0ull );
        ret_val = get_offset_field (expectVal64, 0xFF, &offset_valid);
        EXPECT ( offset_valid == true );
        ret_val = get_offset_field (0xB2C9, 20);
        EXPECT ( ret_val == 0xB2C9ull );
        ret_val = get_offset_field (0xFFFFFFFFFFF592B7, 23, &offset_valid);
        EXPECT ( offset_valid == true );
        EXPECT ( ret_val == 0x7592B7ull );
        EXPECT_FAIL(get_offset_field (0xB295, 12), "offset-out-of-range");
        ret_val = get_offset_field (0xFFFFFFFFFD95B8BD, 26, &offset_valid);
        EXPECT ( offset_valid == false );
        EXPECT ( ret_val == 0ull );

        // check to see if value is power of 2
        val64 = 1;
        bool valbool = is_power_of_2 (val64);
        EXPECT ( valbool == true );
        val64 = 3;
        valbool = is_power_of_2 (val64);
        EXPECT ( valbool == false );
        val64 = 16777216;
        valbool = is_power_of_2 (val64);
        EXPECT ( valbool == true );
        val64 = 16777217;
        valbool = is_power_of_2(val64);
        EXPECT ( valbool == false );

        // round up to power of 2
        val64 = 0x1ffffffff;
        ret_val = round_up_power2(val64);
        EXPECT (ret_val == (uint64)0x200000000);
        val64 = 0x8000000;
        EXPECT_FAIL (round_up_power2(val64),"invalid-round-up");
        val64 = 0xffffffffffffffff;
        EXPECT_FAIL (round_up_power2(val64),"round-up-overflow");

        // lowest bit set
        EXPECT(lowest_bit_set(0xED21B40) == 6ull);
        EXPECT(lowest_bit_set(0x12037F5D8D60000) == 17ull);
        EXPECT_FAIL(lowest_bit_set(0), "invalid-value");

        // highest bit set
        val64 = 0x1ffffffff;
        ret_val = highest_bit_set(val64);
        EXPECT(ret_val == (uint64)32);
        val64 = 0;
        EXPECT_FAIL(highest_bit_set(val64),"invalid-value");

        // test to see if generated mask is correct
        expectVal64 = get_mask64 ((uint32) 10);
        EXPECT ( expectVal64 == 0x3FFULL );
        expectVal64 = get_mask64 ((uint32) 10, (uint32) 10);
        EXPECT ( expectVal64 == (0x3FFULL << 10));
        expectVal64 = get_mask64 ((uint32) 0);
        EXPECT ( expectVal64 == 0ULL );
        expectVal64 = get_mask64 ((uint32) 1);
        EXPECT ( expectVal64 == 1ULL );
        expectVal64 = get_mask64 ((uint32) 32 );
        EXPECT ( expectVal64 == 0xFFFFFFFFULL );
        expectVal64 = get_mask64 ((uint32) 32, (uint32) 32);
        EXPECT ( expectVal64 == (0xFFFFFFFFULL << 32) );

        // test is complete
        LOG(notice) << "Complete Utility Functions Test Set 1." << endl;
    }
},

CASE( "Test set 2 for Utility Functions" ) {

    SETUP( "basic string operation tests" )  {
        LOG(notice) << "Utility Functions Test Set 2..." << endl;
        
        // get genmode strings from config bits
        EGenModeTypeBaseType baseVal = 0x0;
        string genModeString = get_gen_mode_string (baseVal);
        EXPECT ( genModeString == "" );
        baseVal = 0x1;
        genModeString = get_gen_mode_string (baseVal);
        EXPECT ( genModeString == "NoIss" );
        baseVal = 0x7;
        genModeString = get_gen_mode_string (baseVal);
        EXPECT ( genModeString == "NoIss-SimOff-NoEscape");
        baseVal = 0x7F;
        genModeString = get_gen_mode_string (baseVal);
        EXPECT ( genModeString == "NoIss-SimOff-NoEscape-NoJump-ReExe-Exception-NoSkip");
        baseVal = 0x6D;
        genModeString = get_gen_mode_string (baseVal);
        EXPECT ( genModeString == "NoIss-NoEscape-NoJump-Exception-NoSkip");

        // get genmode names from config bits
        baseVal = 0x0;
        genModeString = get_gen_mode_name(baseVal);
        EXPECT ( genModeString == "IssSim" );
        baseVal = 0x1;
        EXPECT_FAIL ( get_gen_mode_name(baseVal), "unsupported-gen-mode" );
        baseVal = 0x2;
        genModeString = get_gen_mode_name(baseVal);
        EXPECT ( genModeString == "SimOff" );
        baseVal = 14;
        genModeString = get_gen_mode_name (baseVal);
        EXPECT ( genModeString == "Linear" );

	// test is complete
        LOG(notice) << "Complete Utility Functions Test Set 2." << endl;
    }
},

};

int main( int argc, char * argv[] )
{
    Force::Logger::Initialize();
    int ret = lest::run( specification, argc, argv );
    Force::Logger::Destroy();
    return ret;
}
