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
#ifndef Force_UopInterface_H
#define Force_UopInterface_H

#include <stdint.h>

#ifdef __cplusplus
#include <string>
extern "C" {
#endif

  /*!
    \brief interface handling interactions between FORCE and UopExecutor.
  */

enum EUopParameterType {
  UopParamBool,
  UopParamUInt8,
  UopParamUInt32,
  UopParamUInt64,
  UopParamFp16,
  UopParamFp32,
  UopParamFp64,
  UopParamFpException,
  UopParamFpcr,
  UopParamFpRounding
};

enum EFpException {
  FpInvalidOp,
  FpDivideByZero,
  FpOverFlow,
  FpUnderFlow,
  FpInexact,
  FpInputDenorm
};

enum EFpRounding {
  FpRoundingTieEven,
  FpRoundingPosInf,
  FpRoundingNegInf,
  FpRoundingZero,
  FpRoundingTieAway,
  FpRoundingOdd
};

enum EUop {
  UopAddWithCarry = 0,
  UopMul = 1,
  UopMulAdd = 2,
  UopDiv = 3,
  UopDecodeBitMasks = 4,
  UopAdd = 5,
  UopSub = 6,
  UopSubWithCarry = 7,
  UopAndShift32 = 8,
  UopAndShift64 = 9,
  UopBicShift32 = 10,
  UopBicShift64 = 11,
  UopEonShift32 = 12, 
  UopEonShift64 = 13, 
  UopEorShift32 = 14,
  UopEorShift64 = 15,
  UopOrrShift32 = 16, 
  UopOrrShift64 = 17, 
  UopMvnShift32 = 18,
  UopMvnShift64 = 19,
  UopOrnShift32 = 20,
  UopOrnShift64 = 21,
  UopMsub32 = 22,
  UopMsub64 = 23,
  UopMneg32 = 24,
  UopMneg64 = 25,
  UopSmaddl = 26,
  UopSmsubl = 27, 
  UopSmnegl = 28,
  UopSmull = 29, 
  UopSmulh = 30,
  UopUmaddl = 31,
  UopUmsubl = 32,
  UopUmnegl = 33,
  UopUmull = 34,
  UopUmulh = 35,
  UopAndImm32 = 36,
  UopAndImm64 = 37,
  UopEorImm32 = 38,
  UopEorImm64 = 39,
  UopOrrImm32 = 40,
  UopOrrImm64 = 41,
  UopAddExt32 = 42,
  UopAddExt64 = 43,
  UopAddShift32 = 44,
  UopAddShift64 = 45,
  UopSubExt32 = 46,
  UopSubExt64 = 47,
  UopSubShift32 = 48,
  UopSubShift64 = 49,
  UopAddsExt32 = 50,
  UopAddsExt64 = 51,
  UopAddsShift32 = 52,
  UopAddsShift64 = 53,
  UopSubsExt32 = 54,
  UopSubsExt64 = 55,
  UopSubsShift32 = 56,
  UopSubsShift64 = 57,
  UopNegShift32 = 58,
  UopNegShift64 = 59,
  UopNegsShift32 = 60,
  UopNegsShift64 = 61,
  UopNgc32 = 62,
  UopNgc64 = 63,
  UopNgcs32 = 64,
  UopNgcs64 = 65,
  UopAndsShift32 = 66,
  UopAndsShift64 = 67,
  UopAndsImm32 = 68,
  UopAndsImm64 = 69,
  UopBicsShift32 = 70,
  UopBicsShift64 = 71,
  UopFixedToFp = 1000,
  UopFpToFixed = 1001,
  UopFpRoundInt = 1002,
  UopFpAdd = 1003,
  UopFpSub = 1004,
  UopFpMul = 1005,
  UopFpMulAdd = 1006,
  UopFpDiv = 1007,
  UopFpAbs = 1008,
  UopFpNeg = 1009,
  UopFpSqrt = 1010,
  UopAddPACDA = 2000,
  UopAddPACDB = 2001,
  UopAddPACGA = 2002,
  UopAddPACIA = 2003,
  UopAddPACIB = 2004,
  UopAuthDA = 2005,
  UopAuthDB = 2006,
  UopAuthIA = 2007,
  UopAuthIB = 2008,
  UopStrip = 2009
};

struct UopParameter {
  EUopParameterType type;
  uint64_t value;
};

bool execute_uop(uint32_t cpuid, EUop uop, UopParameter* inputParams, uint8_t inputParamCount, UopParameter* outputParams, uint8_t *outputParamCount); //<! execute the operation specify by uop.

#ifdef __cplusplus
};
#endif

#endif

