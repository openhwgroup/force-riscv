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
#include "Data.h"

#include "lest/lest.hpp"

#include "Defines.h"
#include "Enums.h"
#include "Log.h"
#include "Random.h"

using text = std::string;

const lest::test specification[] = {

CASE( "Basic Test Data" ) {
   SETUP( "setup and test Data class" ) {
     using namespace Force;
     auto const data1 = DataFactory::Instance()->BuildData("INT32(0x300)");
     auto const data2 = DataFactory::Instance()->BuildData("INT8(0x0-0xff)");
     SECTION ("test choose data") {
       EXPECT(data1->ChooseData() == 0x300u);
       uint64 data = data2->ChooseData();
       EXPECT(data >= 0x00u);
       EXPECT(data <= 0xffu);
     }
     SECTION ("test data width") {
       EXPECT(data1->GetDataWidth() == 4u);
       EXPECT(data2->GetDataWidth() == 1u);
     }
     SECTION ("test To String") {
       EXPECT(data1->ToString() == "INT32(0x300)");
       EXPECT(data2->ToString() == "INT8(0x0-0xff)");
     }
   }
},

CASE( "Basic Test FP Data" ) {
   SETUP( "setup and test FP Data class" ) {
     using namespace Force;
     auto const fpData1 = DataFactory::Instance()->BuildData("FP16(exp=0x11)(sign=0)(frac=0x0)");
     auto const fpData2 = DataFactory::Instance()->BuildData("FP64(sign=1)(exp=1)(frac=0)");
     auto const fpData4 = DataFactory::Instance()->BuildData("INT16(0x1000-0x2000)");
     SECTION ("test choose data") {
       EXPECT(fpData1->ChooseData() == 0x4400u);
       EXPECT(fpData2->ChooseData() == 0x8010000000000000ull);
       EXPECT(fpData4->ChooseData() >= 0x1000u);
       EXPECT(fpData4->ChooseData() <= 0x2000u);
     }
     SECTION ("test data width") {
       EXPECT(fpData1->GetDataWidth() == 2u);
       EXPECT(fpData2->GetDataWidth() == 8u);
       EXPECT(fpData4->GetDataWidth() == 2u);
     }
     SECTION ("test To String") {
       EXPECT(fpData1->ToString() == "FP16(exp=0x11)(sign=0x0)(frac=0x0)");
       EXPECT(fpData2->ToString() == "FP64(exp=0x1)(sign=0x1)(frac=0x0)");
       EXPECT(fpData4->ToString() == "INT16(0x1000-0x2000)");
     }
     
   }
},

CASE( "Basic Test SIMD Data" ) {
   SETUP( "setup and test SIMD Data class" ) {
     using namespace Force;
     auto const simdData1 = DataFactory::Instance()->BuildData("[0,1]FP16(exp=0x11)(sign=0)(frac=0x0)[2,3]INT16(0x300-0x300)");
     auto const simdData2 = DataFactory::Instance()->BuildData("[0,1]FP32(exp=0x1)(sign=1)(frac=0x0)[2,3]INT32(0x400-0x400)");
     auto simdData3 = DataFactory::Instance()->BuildData("[0,1,2,3]INT64()");
     SECTION ("test choose data") {
       std::vector<uint64> datas;
       EXPECT(simdData1->ChooseData() == 0x0300030044004400ull);
       dynamic_cast<const SIMDData*>(simdData2)->ChooseLargeData(datas);
       EXPECT(datas[0] == 0x8080000080800000ull);
       EXPECT(datas[1] == 0x0000040000000400ull);
     }
     SECTION ("test data width") {
       EXPECT(simdData1->GetDataWidth() == 8u);
       EXPECT(simdData2->GetDataWidth() == 16u);
     }
     SECTION ("test To String") {
       EXPECT(simdData1->ToString() == "[0]FP16(exp=0x11)(sign=0x0)(frac=0x0)[1]FP16(exp=0x11)(sign=0x0)(frac=0x0)[2]INT16(0x300)[3]INT16(0x300)");
       EXPECT(simdData2->ToString() == "[0]FP32(exp=0x1)(sign=0x1)(frac=0x0)[1]FP32(exp=0x1)(sign=0x1)(frac=0x0)[2]INT32(0x400)[3]INT32(0x400)");
     }

     SECTION("test large data 128-bit") {
       std::vector<uint64> datas;
       simdData2->ChooseLargeData(datas);
       EXPECT(datas.size() == 2u);
       EXPECT(datas[0] != datas[1]);
     }
     
     SECTION("test large data 256-bit") {
       std::vector<uint64> datas;
       simdData3->ChooseLargeData(datas);
       EXPECT(datas.size() == 4u);
       EXPECT(datas[0] != datas[2]);
     }
   }
},

CASE( "IntDataPattern Test" ) {
   SETUP( "setup and test IntDataPattern class" ) {
     using namespace Force;
     SECTION ("test choose all") {
        for(enum EDataType dataType = EDataType::INT8; dataType < EDataType::FP8; dataType = (EDataType)((int)dataType + 1)) {
            if(dataType == (EDataType)((int)EDataType::INT64 + 1)) {
                dataType = EDataType::FIX8;
            }
             IntDataPattern *patternData = new IntDataPattern(dataType);
             int dataLength = (patternData->GetDataWidth())*8;
             if(dataLength < 64) {
                 EXPECT(patternData->ChooseAllOne() == ((1ull<<dataLength) -1));
                 EXPECT(patternData->ChooseAllZero() == 0ull);
             }
             else if(dataLength == 64) {
                 EXPECT(patternData->ChooseAllOne() == 0xFFFFFFFFFFFFFFFF);
                 EXPECT(patternData->ChooseAllZero() == 0ull);
             }
             delete patternData;
         }
     }
     SECTION ("test low high bytes") {
        for(enum EDataType dataType = EDataType::INT8; dataType < EDataType::FP8; dataType = (EDataType)((int)dataType + 1)) {
            if(dataType == (EDataType)((int)EDataType::INT64 + 1)) {
                dataType = EDataType::FIX8;
            }
             IntDataPattern *patternData = new IntDataPattern(dataType);
             int dataLength = (patternData->GetDataWidth())*8;
             if(dataLength <= 64) {
                EXPECT((patternData->ChooseLSBZero()&0xFF) == 0ull);
                EXPECT((patternData->ChooseLSBOne()<<56>>56) == ((1ull<<8) - 1));
                EXPECT((patternData->ChooseMSBZero()>>(dataLength - 8)) == 0ull);
                EXPECT((patternData->ChooseMSBOne()>>(dataLength - 8)) == ((1<<8) -1ull));
             }
             delete patternData;
         }
     }
     SECTION ("test saturate") {
        for(enum EDataType dataType = EDataType::INT8; dataType < EDataType::FP8; dataType = (EDataType)((int)dataType + 1)) {
            if(dataType == (EDataType)((int)EDataType::INT64 + 1)) {
                dataType = EDataType::FIX8;
            }
             IntDataPattern *patternData = new IntDataPattern(dataType);
             int dataLength = (patternData->GetDataWidth())*8;
             // will not handle 128bits data
             if(dataLength <= 64) {
                uint64 signedAlmostSat = patternData->ChooseSignedAlmostSat();
                uint64 unsignedAlmostSat = patternData->ChooseUnsignedAlmostSat();
                // if unsignedAlmostSat in range 0~100,judgment is not necessary 
                if(unsignedAlmostSat >= patternData->GetAlmostSatValue()) {
                    EXPECT(((unsignedAlmostSat + patternData->GetAlmostSatValue())>>dataLength) != 0ull );
                }
                // 64bits data have no saturate data
                if (dataLength < 64) {
                    EXPECT((patternData->ChooseUnsignedSaturate()>>dataLength) != 0ull );
                    EXPECT((patternData->ChooseSignedSaturate()>>dataLength) != 0ull); 
                    EXPECT(((patternData->ChooseSignedSatPlugOne() - 1)>>dataLength) != 0ull);
                    EXPECT(((patternData->ChooseUnsignedSatPlugOne() - 1)>>dataLength) != 0ull);
                }
                EXPECT(signedAlmostSat < ((1<<(dataLength-1)) + patternData->GetAlmostSatValue()));
                EXPECT(signedAlmostSat > ((patternData->ChooseAllOne()>>1) - patternData->GetAlmostSatValue()));             
             }
             delete patternData;
         }
     }
   }
},

CASE( "Test IntDataPattern APIs" ) {
   SETUP( "setup IntDataPattern class" ) {
     using namespace Force;
     auto pDataPattern64 = new IntDataPattern(64);
     auto pDataPattern128 = new IntDataPattern(128);
     auto pDataPattern256 = new IntDataPattern(256);

     SECTION ("test choose large data 64-bit") {
       std::vector<uint64> datas;
       pDataPattern64->ChooseLargeData(datas);
       EXPECT(datas.size() == 1u);
     }
     
     SECTION ("test choose large data 128-bit") {
       std::vector<uint64> datas;
       pDataPattern128->ChooseLargeData(datas);
       EXPECT(datas.size() == 2u);
       EXPECT(datas[0] != datas[1]);
     }
     
     SECTION ("test choose large data 256-bit") {
       std::vector<uint64> datas;
       pDataPattern256->ChooseLargeData(datas);
       EXPECT(datas.size() == 4u);
       EXPECT(datas[0] != datas[3]);
     }
   }
},

CASE( "FpDataPattern Test" ) {
   SETUP( "setup and test FpDataPattern class" ) {
     using namespace Force;
     SECTION ("test API") {
        for(enum EDataType dataType = EDataType::FP16; dataType <= EDataType::FP64; dataType = (EDataType)((int)dataType + 1)) {
             FpDataPattern *patternData = new FpDataPattern(dataType);
             uint64 dataLength = (patternData->GetDataWidth())*8;
             uint64 nonZero = patternData->ChooseNonZero();
             uint64 positiveZero = patternData->ChoosePositiveZero();
             uint64 negativeZero = patternData->ChooseNegativeZero();
             uint64 positiveInfinity = patternData->ChoosePositiveInfinity();
             uint64 negativeInfinity = patternData->ChooseNegativeInfinity();
             uint64 quietNan = patternData->ChooseQuietNan();
             uint64 signalNan = patternData->ChooseSignalNan();

             EXPECT(nonZero != (0ull<<(dataLength-1)));
             EXPECT(nonZero != (1ull<<(dataLength-1)));
             EXPECT(positiveZero == (0ull<<(dataLength-1)));
             EXPECT(negativeZero == (1ull<<(dataLength-1)));
             if (16 == dataLength) {
                EXPECT(positiveInfinity == 0x1Full<<10);
                EXPECT(negativeInfinity == 0x3Full<<10);
                EXPECT((signalNan & 0x3Eull<<9) == (0x3Eull<<9));
                EXPECT((signalNan & 0x1FFull) != 0ull);
                EXPECT((quietNan & 0x3Full<<9) == (0x3Full<<9));
                EXPECT((quietNan & 0x1FFull) != 0ull);
             }
             else if ( 32 == dataLength) {
                EXPECT(positiveInfinity == 0xFFull<<23);
                EXPECT(negativeInfinity == 0x1FFull<<23);
                EXPECT((signalNan & 0x1FEull<<22) == (0x1FEull<<22));
                EXPECT((signalNan & 0x3FFFFFull) != 0ull);
                EXPECT((quietNan & 0x1FFull<<22) == (0x1FFull<<22));
                EXPECT((quietNan & 0x3FFFFFull) != 0ull);
             }
             else if (64 == dataLength) {
                EXPECT(positiveInfinity == 0x7FFull<<52);
                EXPECT(negativeInfinity == 0xFFFull<<52);
                EXPECT((signalNan & 0xFFEull<<51) == (0xFFEull<<51));
                EXPECT((signalNan & 0x7FFFFFFFFFFFFull) != 0ull);
                EXPECT((quietNan & 0xFFFull<<51) == (0xFFFull<<51));
                EXPECT((quietNan & 0x7FFFFFFFFFFFFull) != 0ull);
             }
        }
     }
   }
},

CASE( "Test FPDataPattern APIs" ) {
   SETUP( "setup IntDataPattern class" ) {
     using namespace Force;
     auto pDataPattern64 = new FpDataPattern(64);
     auto pDataPattern128 = new FpDataPattern(128);
     auto pDataPattern256 = new FpDataPattern(256);

     SECTION ("test choose large data 64-bit") {
       std::vector<uint64> datas;
       pDataPattern64->ChooseLargeData(datas);
       EXPECT(datas.size() == 1u);
     }
     
     SECTION ("test choose large data 128-bit") {
       std::vector<uint64> datas;
       pDataPattern128->ChooseLargeData(datas);
       EXPECT(datas.size() == 2u);
       EXPECT(datas[0] != datas[1]);
     }
     
     SECTION ("test choose large data 256-bit") {
       std::vector<uint64> datas;
       pDataPattern256->ChooseLargeData(datas);
       EXPECT(datas.size() == 4u);
       EXPECT(datas[0] != datas[3]);
     }
   }
},

};

int main( int argc, char * argv[] )
{
  Force::Logger::Initialize();
  Force::Random::Initialize();
  Force::DataFactory::Initialize();
  int ret = lest::run( specification, argc, argv );
  Force::DataFactory::Destroy();
  Force::Random::Destroy();
  Force::Logger::Destroy();
  return ret;

}      
