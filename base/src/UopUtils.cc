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
#include <UopUtils.h>
#include <GenException.h>

using namespace std;

/*!
  \file UopUtils.cc
  \brief Code supporting use of the Uop interface.
*/

namespace Force {

  EUopParameterType string_to_EUopParameterType(const std::string& in_str)
  {
    if (in_str == "UopParamBool") {
      return UopParamBool;
    }
    else if (in_str == "UopParamUInt8") {
      return UopParamUInt8;
    }
    else if (in_str == "UopParamUInt32") {
      return UopParamUInt32;
    }
    else if (in_str == "UopParamUInt64") {
      return UopParamUInt64;
    }
    else if (in_str == "UopParamFp16") {
      return UopParamFp16;
    }
    else if (in_str == "UopParamFp32") {
      return UopParamFp32;
    }
    else if (in_str == "UopParamFp64") {
      return UopParamFp64;
    }
    else if (in_str == "UopParamFpException") {
      return UopParamFpException;
    }
    else if (in_str == "UopParamFpcr") {
      return UopParamFpcr;
    }
    else if (in_str == "UopParamFpRounding") {
      return UopParamFpRounding;
    }
    else {
      stringstream err_stream;
      err_stream << "Unknown EUopParameterType enum name: " << in_str;
      throw EnumTypeError(err_stream.str());
    }
  }

  EUop string_to_EUop(const string& in_str)
  {
    if (in_str == "UopAddWithCarry") {
      return UopAddWithCarry;
    }
    else if (in_str == "UopMul") {
      return UopMul;
    }
    else if (in_str == "UopMulAdd") {
      return UopMulAdd;
    }
    else if (in_str == "UopDiv") {
      return UopDiv;
    }
    else if (in_str == "UopDecodeBitMasks") {
      return UopDecodeBitMasks;
    }
    else if (in_str == "UopAdd") {
      return UopAdd;
    }
    else if (in_str == "UopSub") {
      return UopSub;
    }
    else if (in_str == "UopSubWithCarry") {
      return UopSubWithCarry;
    }
    else if (in_str == "UopAndShift32") {
      return UopAndShift32;
    }
    else if (in_str == "UopAndShift64") {
      return UopAndShift64;
    }
    else if (in_str == "UopBicShift32") {
      return UopBicShift32;
    }
    else if (in_str == "UopBicShift64") {
      return UopBicShift64;
    }
    else if (in_str == "UopEonShift32") {
      return UopEonShift32;
    }
    else if (in_str == "UopEonShift64") {
      return UopEonShift64;
    }
    else if (in_str == "UopEorShift32") {
      return UopEorShift32;
    }
    else if (in_str == "UopEorShift64") {
      return UopEorShift64;
    }
    else if (in_str == "UopOrrShift32") {
      return UopOrrShift32;
    }
    else if (in_str == "UopOrrShift64") {
      return UopOrrShift64;
    }
    else if (in_str == "UopMvnShift32") {
      return UopMvnShift32;
    }
    else if (in_str == "UopMvnShift64") {
      return UopMvnShift64;
    }
    else if (in_str == "UopOrnShift32") {
      return UopOrnShift32;
    }
    else if (in_str == "UopOrnShift64") {
      return UopOrnShift64;
    }
    else if (in_str == "UopMsub32") {
      return UopMsub32;
    }
    else if (in_str == "UopMsub64") {
      return UopMsub64;
    }
    else if (in_str == "UopMneg32") {
      return UopMneg32;
    }
    else if (in_str == "UopMneg64") {
      return UopMneg64;
    }
    else if (in_str == "UopSmaddl") {
      return UopSmaddl;
    }
    else if (in_str == "UopSmsubl") {
      return UopSmsubl;
    }
    else if (in_str == "UopSmnegl") {
      return UopSmnegl;
    }
    else if (in_str == "UopSmull") {
      return UopSmull;
    }
    else if (in_str == "UopSmulh") {
      return UopSmulh;
    }
    else if (in_str == "UopUmaddl") {
      return UopUmaddl;
    }
    else if (in_str == "UopUmsubl") {
      return UopUmsubl;
    }
    else if (in_str == "UopUmnegl") {
      return UopUmnegl;
    }
    else if (in_str == "UopUmull") {
      return UopUmull;
    }
    else if (in_str == "UopUmulh") {
      return UopUmulh;
    }
    else if (in_str == "UopAndImm32") {
      return UopAndImm32;
    }
    else if (in_str == "UopAndImm64") {
      return UopAndImm64;
    }
    else if (in_str == "UopEorImm32") {
      return UopEorImm32;
    }
    else if (in_str == "UopEorImm64") {
      return UopEorImm64;
    }
    else if (in_str == "UopOrrImm32") {
      return UopOrrImm32;
    }
    else if (in_str == "UopOrrImm64") {
      return UopOrrImm64;
    }
   else if (in_str == "UopAddExt32") {
      return UopAddExt32;
    }
    else if (in_str == "UopAddExt64") {
      return UopAddExt64;
    }
    else if (in_str == "UopAddShift32") {
      return UopAddShift32;
    }
    else if (in_str == "UopAddShift64") {
      return UopAddShift64;
    }
    else if (in_str == "UopSubExt32") {
      return UopSubExt32;
    }
    else if (in_str == "UopSubExt64") {
      return UopSubExt64;
    }
    else if (in_str == "UopSubShift32") {
      return UopSubShift32;
    }
    else if (in_str == "UopSubShift64") {
      return UopSubShift64;
    }
    else if (in_str == "UopAddsExt32") {
      return UopAddsExt32;
    }
    else if (in_str == "UopAddsExt64") {
      return UopAddsExt64;
    }
    else if (in_str == "UopAddsShift32") {
      return UopAddsShift32;
    }
    else if (in_str == "UopAddsShift64") {
      return UopAddsShift64;
    }
    else if (in_str == "UopSubsExt32") {
      return UopSubsExt32;
    }
    else if (in_str == "UopSubsExt64") {
      return UopSubsExt64;
    }
    else if (in_str == "UopSubsShift32") {
      return UopSubsShift32;
    }
    else if (in_str == "UopSubsShift64") {
      return UopSubsShift64;
    }
    else if (in_str == "UopNegShift32") {
      return UopNegShift32;
    }
    else if (in_str == "UopNegShift64") {
      return UopNegShift64;
    }
    else if (in_str == "UopNegsShift32") {
      return UopNegsShift32;
    }
    else if (in_str == "UopNegsShift64") {
      return UopNegsShift64;
    }
    else if (in_str == "UopNgc32") {
      return UopNgc32;
    }
    else if (in_str == "UopNgc64") {
      return UopNgc64;
    }
    else if (in_str == "UopNgcs32") {
      return UopNgcs32;
    }
    else if (in_str == "UopNgcs64") {
      return UopNgcs64;
    }
    else if (in_str == "UopAndsShift32") {
      return UopAndsShift32;
    }
    else if (in_str == "UopAndsShift64") {
      return UopAndsShift64;
    }
    else if (in_str == "UopAndsImm32") {
      return UopAndsImm32;
    }
    else if (in_str == "UopAndsImm64") {
      return UopAndsImm64;
    }
    else if (in_str == "UopBicsShift32") {
      return UopBicsShift32;
    }
    else if (in_str == "UopBicsShift64") {
      return UopBicsShift64;
    }
    else if (in_str == "UopFixedToFp") {
      return UopFixedToFp;
    }
    else if (in_str == "UopFpToFixed") {
      return UopFpToFixed;
    }
    else if (in_str == "UopFpRoundInt") {
      return UopFpRoundInt;
    }
    else if (in_str == "UopFpAdd") {
      return UopFpAdd;
    }
    else if (in_str == "UopFpSub") {
      return UopFpSub;
    }
    else if (in_str == "UopFpMul") {
      return UopFpMul;
    }
    else if (in_str == "UopFpMulAdd") {
      return UopFpMulAdd;
    }
    else if (in_str == "UopFpDiv") {
      return UopFpDiv;
    }
    else if (in_str == "UopFpAbs") {
      return UopFpAbs;
    }
    else if (in_str == "UopFpNeg") {
      return UopFpNeg;
    }
    else if (in_str == "UopFpSqrt") {
      return UopFpSqrt;
    }
    else if (in_str == "UopAddPACDA") {
      return UopAddPACDA;
    }
    else if (in_str == "UopAddPACDB") {
      return UopAddPACDB;
    }
    else if (in_str == "UopAddPACGA") {
      return UopAddPACGA;
    }
    else if (in_str == "UopAddPACIA") {
      return UopAddPACIA;
    }
    else if (in_str == "UopAddPACIB") {
      return UopAddPACIB;
    }
    else if (in_str == "UopAuthDA") {
      return UopAuthDA;
    }
    else if (in_str == "UopAuthDB") {
      return UopAuthDB;
    }
    else if (in_str == "UopAuthIA") {
      return UopAuthIA;
    }
    else if (in_str == "UopAuthIB") {
      return UopAuthIB;
    }
    else if (in_str == "UopStrip") {
      return UopStrip;
    }
    else {
      stringstream err_stream;
      err_stream << "Unknown EUop enum name: " << in_str;
      throw EnumTypeError(err_stream.str());
    }

    return UopAddWithCarry;
  }

}
