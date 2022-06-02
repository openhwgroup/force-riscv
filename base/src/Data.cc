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

#include <cstdlib>
#include <cstring>
#include <map>
#include <sstream>

#include "Choices.h"
#include "ChoicesModerator.h"
#include "Constraint.h"
#include "GenException.h"
#include "Generator.h"
#include "Log.h"
#include "Random.h"
#include "Register.h"
#include "StringUtils.h"

using namespace std;

namespace Force {

  IntData::IntData(const string& valueStr) : Data(), mpDataConstraint(nullptr), mpDataPattern(nullptr)
  {
    ConstructDataConstraint(valueStr);
    if (mpDataConstraint == nullptr)
      mpDataPattern = new IntDataPattern(mDataType);
  }

  void IntData::ConstructDataConstraint(const std::string& valueStr)
  {
    string str_val;
    ParseTypeValue(valueStr, mDataType, str_val);
    if (str_val == "zero") {
      mpDataConstraint = new ConstraintSet(0ul);
     }
     else if (str_val == "non_zero") {
       uint64 max_val = (mDataType == EDataType::INT64) ? -1ull : (1ull << (GetDataWidth() *8)) - 1;
       mpDataConstraint = new ConstraintSet(1, max_val);
     }
     else if (str_val == "non_all_ones") {
       uint64 sub_max_val =  (mDataType == EDataType::INT64) ? (-1ull - 1) : (1ull << (GetDataWidth() *8)) - 2;
       mpDataConstraint = new ConstraintSet(0, sub_max_val);
     }
     else if (str_val.substr(0,2) == "0b") { // TBD: parse value str like "0bx1x1x, 0bx0xx0x"
       auto data = SetupBitPatternConstraint(str_val);
       mpDataConstraint = new ConstraintSet(data);
     }
     else if (str_val != "") {
       mpDataConstraint = new ConstraintSet(str_val);
     }
  }

  IntData::IntData(const IntData& rOther): Data(rOther), mpDataConstraint(nullptr), mpDataPattern(nullptr)
  {
    if (nullptr != rOther.mpDataConstraint)
      mpDataConstraint = rOther.mpDataConstraint->Clone();
    if (nullptr != rOther.mpDataPattern)
      mpDataPattern = dynamic_cast<DataPattern* >(rOther.mpDataPattern->Clone());
  }

  IntData::~IntData()
  {
    if (nullptr != mpDataConstraint)
      delete mpDataConstraint;
    if (nullptr != mpDataPattern)
      delete mpDataPattern;
  }

  Object* IntData::Clone() const
  {
    return new IntData(*this);
  }

  const std::string IntData::ToString() const
  {
    stringstream out_stream;

    out_stream <<  EDataType_to_string(mDataType);
    if (nullptr != mpDataConstraint)
      out_stream << "(" << mpDataConstraint->ToSimpleString() << ")";
    else
      out_stream << "(" << mpDataPattern->ToString() << ")";

    return out_stream.str();
  }

  uint64 IntData::ChooseData() const
  {
    uint64 dataWidth = GetDataWidth();
    uint64 dataMask = (dataWidth >= 8 ) ? -1ull : ((1ull << (dataWidth * 8)) - 1);
    if (nullptr != mpDataConstraint)
      return mpDataConstraint->ChooseValue() & dataMask;
    else
      return mpDataPattern->ChooseData();
  }

  void IntData::ChooseLargeData(std::vector<uint64>& rDatas) const
  {
    LOG(fail) << "{IntData::ChooseLargeData} Not support large data, please use SIMDData::ChooseLargeData" << endl;
    FAIL("Not-support-large-data");
  }

  void IntData::Setup(const Generator& gen )
  {
    mpDataPattern->Setup(gen);
  }

  //!< ValueStr is formated as INTn()
  void IntData::ParseTypeValue(const string& valueStr, EDataType& dataType, string& val) const
  {
    size_t begin_pos = valueStr.find("(");
    size_t end_pos = valueStr.find(")");
    string strType = valueStr.substr(0, begin_pos);
    try {
      dataType = string_to_EDataType(strType);
    }
    catch (const EnumTypeError& enum_error) {
      LOG(fail) << "invalid Operand Data Type \"" << enum_error.what() << "\""<< endl;
      FAIL("invalid Operand Data Type");
    }

    if (begin_pos == string::npos && end_pos == string::npos)  {
      val = "";
      return;
    }

    if (begin_pos == string::npos || end_pos == string::npos) {
      LOG(fail) << "invalid Operand Data \"" << valueStr << "\""<< endl;
      FAIL("invalid Operand Data");
    }

    val = valueStr.substr(begin_pos+1 , end_pos - begin_pos - 1);
  }

  //!< the pattern string is like "0bxx1xxx0xx1x0xx"
  uint64 IntData::SetupBitPatternConstraint(const std::string& pattern)
  {
    string pattern_no_prefix = pattern.substr(2);
    auto pattern_len = pattern_no_prefix.length();
    if (pattern_len != 64 && pattern_len != 32) {
      LOG(fail) << "{IntData} unsupported bit pattern: " << pattern << endl;
      FAIL("unsupported-bit-pattern");
    }

    string ones_mask_pattern, zeroes_mask_pattern;
    for ( unsigned i = 0; i < pattern_len; i ++) {
      string bit_str = pattern_no_prefix.substr(i, 1);
      if (bit_str == "x") {
        ones_mask_pattern += "0";
        zeroes_mask_pattern += "1";
      }
      else {
        ones_mask_pattern += bit_str;
        zeroes_mask_pattern += bit_str;
      }
    }
    uint64 data = (Random::Instance()->Random64() & parse_bin64(zeroes_mask_pattern)) | parse_bin64(ones_mask_pattern);
    // << "{InitData} data value: 0x" << hex << data << " for bit pattern: " << pattern << endl;
    return data;
  }

  FPData::FPData(const string& valueStr) : Data(), mpExpConstraint(nullptr), mpSignConstraint(nullptr), mpFracConstraint(nullptr), mpDataPattern(nullptr)
  {
    string exp, sign, frac;
    ParseTypeValue(valueStr, mDataType, exp, sign, frac);
    if (exp == "")
      mpDataPattern = new FpDataPattern(mDataType);
    else {
      mpExpConstraint = new ConstraintSet(exp);
      mpSignConstraint = new ConstraintSet(sign);
      mpFracConstraint = new ConstraintSet(frac);
    }
  }

  FPData::FPData(const FPData& rOther): Data(rOther), mpExpConstraint(nullptr), mpSignConstraint(nullptr), mpFracConstraint(nullptr), mpDataPattern(nullptr)
  {
    if (nullptr !=  rOther.mpExpConstraint)
      mpExpConstraint = rOther.mpExpConstraint->Clone();
    if (nullptr != rOther.mpSignConstraint)
      mpSignConstraint = rOther.mpSignConstraint->Clone();
    if (nullptr != rOther.mpFracConstraint)
      mpFracConstraint = rOther.mpFracConstraint->Clone();
    if (nullptr != rOther.mpDataPattern)
      mpDataPattern = dynamic_cast<DataPattern* >(rOther.mpDataPattern->Clone());
  }

  FPData::~FPData()
  {
    if (nullptr != mpExpConstraint)
      delete mpExpConstraint;
    if (nullptr != mpSignConstraint)
      delete mpSignConstraint;
    if (nullptr != mpFracConstraint)
      delete mpFracConstraint;
    if (nullptr != mpDataPattern)
      delete mpDataPattern;
  }

  Object* FPData::Clone() const
  {
    return new FPData(*this);
  }

  const std::string FPData::ToString() const
  {
    stringstream out_stream;

    out_stream <<  EDataType_to_string(mDataType);
    if (nullptr != mpExpConstraint)
      out_stream << "(exp=" << mpExpConstraint->ToSimpleString()<<")";
    if (nullptr != mpSignConstraint)
      out_stream << "(sign=" << mpSignConstraint->ToSimpleString()<<")";
    if (nullptr != mpFracConstraint)
      out_stream << "(frac=" << mpFracConstraint->ToSimpleString()<<")";
    if (nullptr != mpDataPattern)
      out_stream << "(" << mpDataPattern->ToString() << ")";
    return out_stream.str();
  }

  uint64 FPData::ChooseData() const
  {
    if (mpDataPattern)
      return mpDataPattern->ChooseData();

    uint64 exp= mpExpConstraint->ChooseValue();
    uint64 sign = mpSignConstraint->ChooseValue();
    uint64 frac = mpFracConstraint->ChooseValue();
    uint64 value = 0ull;
    uint64 sign_mask;
    uint64 exp_mask;
    uint64 frac_mask;

    switch (mDataType) {
    case EDataType::FP16:
      sign_mask = 0x1ull;
      exp_mask = 0x1full;
      frac_mask = 0x3ffull;
      value = ((sign & sign_mask) << 15) | ((exp & exp_mask) << 10) | ((frac & frac_mask) << 0);
      break;
    case EDataType::FP32:
      sign_mask = 0x1ull;
      exp_mask = 0xffull;
      frac_mask = 0x7ffffful;
      value = ((sign & sign_mask) << 31) | ((exp & exp_mask) << 23) | ((frac & frac_mask) << 0);
      break;
    case EDataType::FP64:
      sign_mask = 0x1ull;
      exp_mask = 0x7ffull;
      frac_mask = 0xfffffffffffffull;
      value = ((sign & sign_mask) << 63) | ((exp & exp_mask) << 52) | ((frac & frac_mask) << 0);
      break;
    default:
      LOG(fail) << "invalid FP Data Type " << EDataType_to_string(mDataType) << endl;
      FAIL("invalid FP Data Type");
    }
    return value;
  }
  void FPData::ChooseLargeData(std::vector<uint64>& rDatas) const
  {
    LOG(fail) << "{FPData::ChooseLargeData} Not support large data, please use SIMDData::ChooseLargeData" << endl;
    FAIL("Not-support-large-data");
  }

  void FPData::Setup(const Generator& gen )
  {
     mpDataPattern->Setup(gen);
  }

  void FPData::ParseTypeValue(const string& valueStr, EDataType& dataType, string& exp, std::string& sign, string& frac)
  {
    size_t begin_pos = valueStr.find("(");
    string strType = valueStr.substr(0, begin_pos);
    try {
      dataType = string_to_EDataType(strType);
    }
    catch (const EnumTypeError& enum_error) {
      LOG(fail) << "invalid Operand FP Data Type \"" << enum_error.what() << "\""<< endl;
      FAIL("invalid Operand FP Data Type");
    }
    if (begin_pos == string::npos) {
      exp = sign = frac = "";
      return;
    }

    string strEval = valueStr.substr(begin_pos);
    vector<string> strAssigns;
    if (!parse_brackets_strings(strEval, strAssigns)) {
      LOG(fail) << "invalid Operand FP Data format \"" << valueStr << "\""<< endl;
      FAIL("invalid Operand FP Data");
    }

    for (auto assign : strAssigns) {
      string var, val;
      if (!parse_assignment(assign, var, val)) {
        LOG(fail) << "invalid Operand FP Data assignment \"" << valueStr << "\""<< endl;
        FAIL("invalid Operand FP Data");
      }
      if (var == "exp")
        exp = val;
      else if (var == "sign")
        sign = val;
      else if (var == "frac")
        frac = val;
      else {
        LOG(fail) << "invalid Operand FP Data assignment \"" << valueStr << "\""<< endl;
        FAIL("invalid Operand FP Data");
      }
    }
    return;
  }

  /*!
    \ class SIMDDataLane
    \ brief a class to contain value strings and its lane numbers.
   */
  struct SIMDDataLane {
    SIMDDataLane() : lanes(), laneStr("") { }
    vector<uint32> lanes;
    string laneStr;
  };

  // The format for the valueStr is like "[1,3]FP32(exp=10-50)(sign=1)(frac=0x100-0x400)[0,2]INT32(0x100-0x500)"
  SIMDData::SIMDData(const string& valueStr) : Data(), mLanes(0), mDatas()
  {
    vector<SIMDDataLane> simd_lanes;
    uint32 max_lane_no = 0;
    if (!ParseLanes(valueStr, simd_lanes, max_lane_no)) {
      LOG(fail) << "invalid Operand SIMD Data  \"" << valueStr << "\""<< endl;
      FAIL("invalid Operand SIMD Data");
    }
    mLanes = max_lane_no + 1;
    mDatas.resize(mLanes, nullptr); //
    Data* pData {nullptr};
    for (auto data_lane : simd_lanes) {
      if (data_lane.laneStr.substr(0,6) == "INT128" || data_lane.laneStr.substr(0,6) == "FIX128") {
        LOG(fail) << "invalid SIMD Data \"" << data_lane.laneStr << "\""<< endl;
        FAIL("invalid SIMD Data");
      }
      if (data_lane.laneStr.substr(0,3) == "INT" || data_lane.laneStr.substr(0,3) == "FIX")
        pData = new IntData(data_lane.laneStr);
      else if (data_lane.laneStr.substr(0,2) == "FP")
        pData = new FPData(data_lane.laneStr);
      else {
        LOG(fail) << "invalid SIMD Data \"" << data_lane.laneStr << "\""<< endl;
        FAIL("unknown-operand-data");
      }
      auto const shared_data = make_shared<const Data* > (pData);
      for (auto lane : data_lane.lanes)
        mDatas[lane] = shared_data;
      //<< "{SIMDData:SIMDData()} share data reference count " << shared_data.use_count() << endl;
    }
  }

  bool SIMDData::ParseLanes(const std::string& valueStr, vector<SIMDDataLane>& simd_lanes, uint32& lanes)
  {
    SIMDDataLane dataLane;
    size_t bracket_start = valueStr.find("[");
    if (bracket_start == string::npos)
      return true;
    size_t bracket_end = valueStr.find("]");
    if (bracket_end == string::npos)
      return false;

    string strLaneNo = valueStr.substr(bracket_start + 1, bracket_end - bracket_start - 1);
    StringSplitter splitter(strLaneNo, ',');
    while (!splitter.EndOfString()) {
      uint32 laneNo = parse_uint32(splitter.NextSubString());
      if (laneNo > lanes)
        lanes = laneNo;
      dataLane.lanes.push_back(laneNo);
    }

    size_t next_bracket_start = valueStr.find("[", bracket_end + 1);
    if (next_bracket_start == string::npos) {
      dataLane.laneStr= valueStr.substr(bracket_end + 1);
      simd_lanes.push_back(dataLane);
      return true;
    }
    dataLane.laneStr = valueStr.substr(bracket_end + 1 , next_bracket_start - bracket_end - 1);
    simd_lanes.push_back(dataLane);
    string next_valueStr = valueStr.substr(next_bracket_start);
    return ParseLanes(next_valueStr, simd_lanes, lanes);
  }

  SIMDData::SIMDData(const SIMDData& rOther): Data(rOther), mLanes(0), mDatas()
  {
    for (auto data : rOther.mDatas) {
      if (nullptr != data)
        mDatas.push_back(make_shared<const Data* >(dynamic_cast<Data*>((*data)->Clone())));
      else
        mDatas.push_back(make_shared<const Data* >());
    }
  }

  SIMDData::~SIMDData()
  {
     for (auto shared_data : mDatas) {
       if (*shared_data != nullptr) {
         delete (*shared_data);
         *shared_data = nullptr;
       }
       shared_data.reset();
       //<< "{~SIMDData:SIMDData()} share data reference count " << shared_data.use_count() << endl;
     }
  }

  Object* SIMDData::Clone() const
  {
    return new SIMDData(*this);
  }

  const std::string SIMDData::ToString() const
  {
    stringstream out_stream;

    for (uint32 i = 0; i < mLanes; i ++) {
      out_stream << "[" << i << "]";
      out_stream << (*mDatas[i])->ToString();
    }
    return out_stream.str();
  }

  uint32 SIMDData::GetDataWidth() const
  {
    uint32 dataWidth = 0;
    for (uint32 i = 0 ; i < mLanes; i ++) {
      dataWidth += (*mDatas[i])->GetDataWidth();
    }
    return dataWidth;
  }

  uint64 SIMDData::ChooseData() const
  {
    uint32 i;
    uint64 data = 0;
    for (i = mLanes - 1; i > 0; i --) {
      data |= (*mDatas[i])->ChooseData();
      data <<= (*mDatas[i])->GetDataWidth() << 3;
    }
    data |= (*mDatas[0])->ChooseData();

    return data;
  }

  void SIMDData::ChooseLargeData(std::vector<uint64>& rDatas) const
  {
    uint32 i ;
    uint64 data = 0ull;
    uint32 total_bytes = 0;

    for (i = 0; i <  mLanes; i ++) {
      data |=  (*mDatas[i])->ChooseData() << (total_bytes * 8);
      total_bytes +=  (*mDatas[i])->GetDataWidth();
      if (total_bytes >= 8) {
        rDatas.push_back(data);
        total_bytes = 0;
        data = 0ull;
      }
    }
    if (data)
      rDatas.push_back(data);
  }

  bool SIMDData::HasConstraint() const
  {
    for (unsigned i = 0; i < mLanes; i ++) {
      if (!(*mDatas[i])->HasConstraint())
        return false;
    }
    return true;
  }

  void SIMDData::Setup(const Generator& gen )
  {
    for (unsigned i = 0; i < mLanes; i ++) {
      if (!(*mDatas[i])->HasConstraint()) {
        auto pData = const_cast<Data* >((*mDatas[i]));
        pData->Setup(gen);
      }
    }
  }

  DataFactory* DataFactory::mspDataFactory = nullptr;
  void DataFactory::Initialize()
  {
    if (nullptr == mspDataFactory) {
      mspDataFactory = new DataFactory();
    }
  }

  void DataFactory::Destroy()
  {
    delete mspDataFactory;
    mspDataFactory = nullptr;
  }

#ifdef UNIT_TEST
  const Data* DataFactory::BuildData(const std::string& valueStr)
#else
  const Data* DataFactory::BuildData(const std::string& valueStr, const Generator& gen)
#endif
  {
    Data* pData = nullptr;

    if (valueStr.substr(0,3) == "INT" || valueStr.substr(0, 3) == "FIX")
      pData = new IntData(valueStr);
    else if (valueStr.substr(0,2) == "FP")
      pData = new FPData(valueStr);
    else if (valueStr.substr(0,1) == "[")
      pData = new SIMDData(valueStr);
    else {
      LOG(fail) << "{Operand Data Request} unknown operand data \"" << valueStr << "\"" << endl;
      FAIL("unknown-operand-data");
      return nullptr;
    }
#ifndef UNIT_TEST
    if (!pData->HasConstraint())
      pData->Setup(gen);
#endif
    return pData;
  }

#ifndef UNIT_TEST
  const DataPattern* DataFactory::BuildDataPattern(const Register *pReg, const Generator& gen)
  {
    DataPattern *pDataPattern = nullptr;
    switch (pReg->RegisterType()) {
    case ERegisterType::GPR:
    case ERegisterType::SP:
    case ERegisterType::SysReg:
    case ERegisterType::PREDREG:
      pDataPattern = new IntDataPattern(pReg->Size());
      break;
    case ERegisterType::FPR:
    case ERegisterType::SIMDR:
    case ERegisterType::SIMDVR:
    case ERegisterType::VECREG:
      {
        const ChoicesModerator* choices_mod = gen.GetChoicesModerator(EChoicesType::OperandChoices);
        try {
          auto choiceTree = choices_mod->CloneChoiceTree("FP-SIMD register data choices");
          auto choice = choiceTree->Choose();
          auto choice_value = choice->Value();
          if (choice_value == 0)
            pDataPattern = new IntDataPattern(pReg->Size());
          else if (choice_value == 1)
            pDataPattern = new FpDataPattern(pReg->Size());
          else {
            LOG(fail) << "{DataFactory::Build} Not handled choice item: " << choice->Name() << endl;
            FAIL("not-handled-choice");
          }
          delete choiceTree;
        }
        catch (const ChoicesError& choices_err) {
          LOG(fail) << "{DataFactory::Build} " << choices_err.what() << endl;
          FAIL("build-data-pattern-error");
        }
      }
      break;
    default:
      LOG(notice) << "use random data pattern for register: " << pReg->Name() << endl;
      pDataPattern = new RandomDataPattern(pReg->Size());
    }
    pDataPattern->Setup(gen);
    return pDataPattern;
  }
#endif

  uint64 DataPattern::GetDataMask() const
  {
      uint64 dataWidth = GetDataWidth();
      uint64 dataMask = (dataWidth >= 8 ) ? -1ull : ((1ull << (dataWidth * 8)) - 1);
      return dataMask;
  }

   RandomDataPattern::RandomDataPattern(EDataType dataType)
     : DataPattern(), mBaseDataNumber(1)
  {
    mpChoiceTree = nullptr;
    mDataType = dataType;
  }

  RandomDataPattern::RandomDataPattern(uint32 dataSize)
    : DataPattern(), mBaseDataNumber((dataSize + 63) / 64)
  {
    mpChoiceTree = nullptr;
    if (dataSize >= 64)
      mDataType = EDataType::INT64;
    else if (dataSize == 32)
      mDataType = EDataType::INT32;
    else if (dataSize == 16)
      mDataType = EDataType::INT16;
    else if (dataSize == 8)
      mDataType = EDataType::INT8;
  }

  Object* RandomDataPattern::Clone() const
  {
    return new RandomDataPattern(*this);
  }

  const std::string RandomDataPattern::ToString() const
  {
    return Type();
  }

  uint64 RandomDataPattern::ChooseData() const
  {
    return ChooseRandom();
  }

  void RandomDataPattern::ChooseLargeData(std::vector<uint64>& rDatas) const
  {
    for (unsigned i = 0 ; i < mBaseDataNumber; i ++)
      rDatas.push_back(ChooseRandom());
  }

  uint64 RandomDataPattern::ChooseRandom() const
  {
    uint64 dataWidth = GetDataWidth();
    uint64 dataMask = (dataWidth >= 8 ) ? -1ull : ((1ull << (dataWidth * 8)) - 1);
    uint64 random = Random::Instance()->Random64();
    return random & dataMask;
  }

  IntDataPattern::IntDataPattern(EDataType dataType) : RandomDataPattern(dataType), mAlmostSatValue(100)
  {

  }

  IntDataPattern::IntDataPattern(uint32 dataSize) : RandomDataPattern(dataSize), mAlmostSatValue(100)
  {
  }

  IntDataPattern:: ~IntDataPattern()
  {
    delete mpChoiceTree;
  }

  Object* IntDataPattern::Clone() const
  {
    return new IntDataPattern(*this);
  }

  const std::string IntDataPattern::ToString() const
  {
    return Type();
  }

  uint64 IntDataPattern::ChooseData() const
  {
    if (mpChoiceTree == nullptr) {
      return ChooseRandom();
    }

    auto choice = mpChoiceTree->Choose();
    mChoiceValue = choice->Value();
    mChoiceText = choice->Name();
    switch (mChoiceValue) {
    case 0:
      return ChooseAllOne();
    case 1:
      return ChooseAllZero();
    case 2:
      return ChooseLSBZero();
    case 3:
      return ChooseLSBOne();
    case 4:
      return ChooseMSBZero();
    case 5:
      return ChooseMSBOne();
    case 6:
      return ChooseSignedSaturate();
    case 7:
      return ChooseUnsignedSaturate();
    case 8:
      return ChooseSignedAlmostSat();
    case 9:
      return ChooseUnsignedAlmostSat();
    case 10:
      return ChooseSignedSatPlugOne();
    case 11:
      return ChooseUnsignedSatPlugOne();
    case 12:
      return ChooseRandom();
    default:
      LOG(fail) << "{IntDataPattern::ChooseData} " << "unknown choose item \"" << mChoiceText << "\""<< endl;
      FAIL("integer-data-pattern-choose-error");
    }
    return 0ull;
  }

  void IntDataPattern::ChooseLargeData(std::vector<uint64>& rDatas) const
  {
    if (mpChoiceTree == nullptr) {
      RandomDataPattern::ChooseLargeData(rDatas);
      return;
    }
    auto choice = mpChoiceTree->Choose();
    mChoiceValue = choice->Value();
    mChoiceText = choice->Name();
    switch (mChoiceValue) {
    case 0:
      for (unsigned i = 0 ; i < mBaseDataNumber; i ++) {
        rDatas.push_back(ChooseAllOne());
      }
      break;
    case 1:
      for (unsigned i = 0 ; i < mBaseDataNumber; i ++) {
        rDatas.push_back(ChooseAllZero());
      }
      break;
    case 2:
      rDatas.push_back(ChooseLSBZero());
      for (unsigned i = 1; i < mBaseDataNumber; i ++) {
        rDatas.push_back(ChooseRandom());
      }
      break;
    case 3:
      rDatas.push_back(ChooseLSBOne());
      for (unsigned i = 1; i < mBaseDataNumber; i ++) {
        rDatas.push_back(ChooseRandom());
      }
      break;
    case 4:
    case 5:
    case 6: // for saturated
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
      LOG(info) << " randomize the big data for \"" << mChoiceText << "\"" << endl;
      for (unsigned i = 0; i < mBaseDataNumber; i ++) {
        rDatas.push_back(ChooseRandom());
      }
      break;
    default:
      LOG(fail) << "{IntDataPattern::ChooseLargeData} " << "unknown choose item \"" << mChoiceText << "\""<< endl;
      FAIL("Integer-data-pattern-choose-error");
    }

  }

  void IntDataPattern::Setup(const Generator& gen )
  {
#ifndef UNIT_TEST
    const ChoicesModerator* choices_mod = gen.GetChoicesModerator(EChoicesType::OperandChoices);
    try {
      mpChoiceTree = choices_mod->CloneChoiceTree("Integer data pattern");
    }
    catch (const ChoicesError& choices_err) {
      LOG(fail) << "{IntDataPattern::Setup} " << choices_err.what() << endl;
      FAIL("integer-data-pattern-setup-error");
    }
#endif
  }

  IntDataPattern::IntDataPattern(const IntDataPattern& rOther) : RandomDataPattern(rOther), mAlmostSatValue(100)
  {
    if (rOther.mpChoiceTree)
      mpChoiceTree = dynamic_cast<const ChoiceTree* >(rOther.mpChoiceTree->Clone());
  }

  uint64 IntDataPattern::ChooseAllOne() const
  {
    uint64 dataMask = GetDataMask();
    return (-1ull) & dataMask;
  }

  uint64 IntDataPattern::ChooseAllZero() const
  {
    return 0ull;
  }

  uint64 IntDataPattern::ChooseLSBZero() const
  {
      uint64 dataWidth = GetDataWidth();
      uint64 dataMask = GetDataMask();
      if (1 == dataWidth) {
          return 0ull;
      }
      uint64 random = Random::Instance()->Random64();
      return (random << 8) & dataMask;
  }

  uint64 IntDataPattern::ChooseLSBOne() const
  {
      uint64 dataWidth = GetDataWidth();
      uint64 dataMask = GetDataMask();
      if (1 == dataWidth) {
          return -1ull & dataMask;
      }
      uint64 random = Random::Instance()->Random64();
      return ~(random << 8) & dataMask;
  }

  uint64 IntDataPattern::ChooseMSBZero() const
  {
      uint64 dataWidth = GetDataWidth();
      uint64 dataMask = GetDataMask();
      if (1 == dataWidth) {
          return 0ull;
      }
      if (dataWidth > 8) {
          LOG(error) << "Data width greater than 8 bytes" << endl;
          return 0ull;
      }
      uint64 random = Random::Instance()->Random64();
      return (random >> (8 - dataWidth + 1) * 8) & dataMask;
  }

  uint64 IntDataPattern::ChooseMSBOne() const
  {
      uint64 dataWidth = GetDataWidth();
      uint64 dataMask = GetDataMask();
      if (1 == dataWidth) {
          return -1ull & dataMask;
      }
      if (dataWidth > 8) {
          LOG(error) << "Data width greater than 8 bytes" << endl;
          return 0ull;
      }
      uint64 random = Random::Instance()->Random64();
      return ~(random >> (8 - dataWidth + 1) * 8) & dataMask;
  }

  uint64 IntDataPattern::ChooseSignedSaturate() const
  {
      uint64 result = -1ull;
      uint64 dataWidth = GetDataWidth();
      if (8 == dataWidth) {
        return ChooseRandom();
      }
      uint64  low = 1 << (dataWidth * 8);
      uint64 high = -1ull;
      ConstraintSet saturate_set(low, high);
      result = saturate_set.ChooseValue();
      return result;
  }

  uint64 IntDataPattern::ChooseUnsignedSaturate() const
  {
      uint64 result = 0ull;
      uint64 dataWidth = GetDataWidth();
      if (8 == dataWidth) {
        return ChooseRandom();
      }

      uint64  low = 1 << (dataWidth * 8);
      uint64 high = -1ull;
      ConstraintSet saturate_set(low, high);
      result = saturate_set.ChooseValue();
      return result;
  }

  uint64 IntDataPattern::ChooseSignedAlmostSat() const
  {
      uint64 dataWidth = GetDataWidth();
      uint64 low = (ChooseAllOne()>>1) - mAlmostSatValue;
      uint64 high = (1 << (dataWidth*8 - 1)) + mAlmostSatValue;
      ConstraintSet saturate_set(low, high);
      return saturate_set.ChooseValue();
  }

  uint64 IntDataPattern::ChooseUnsignedAlmostSat() const
  {
      uint64 high_max = ChooseAllOne();
      uint64 low_max = high_max - mAlmostSatValue;
      ConstraintSet saturate_set(low_max, high_max);
      uint64 low_min = ChooseAllZero();
      uint64 high_min = low_min + mAlmostSatValue;
      saturate_set.AddRange(low_min, high_min);
      return saturate_set.ChooseValue();
  }

  uint64 IntDataPattern::ChooseSignedSatPlugOne() const
  {
      return (ChooseSignedSaturate() + 1);
  }

  uint64 IntDataPattern::ChooseUnsignedSatPlugOne() const
  {
      return (ChooseUnsignedSaturate() + 1);
  }

  FpDataPattern::FpDataPattern(EDataType dataType) : RandomDataPattern(dataType)
  {
  }

  FpDataPattern::FpDataPattern(uint32 dataSize) : RandomDataPattern(dataSize)
  {
  }

  FpDataPattern:: ~FpDataPattern()
  {
    delete mpChoiceTree;
  }

  Object* FpDataPattern::Clone() const
  {
    return new FpDataPattern(*this);
  }

  const std::string FpDataPattern::ToString() const
  {
    return Type();
  }

  uint64 FpDataPattern::ChooseData() const
  {
    if (mpChoiceTree == nullptr)
      return ChooseRandom();

    auto choice = mpChoiceTree->Choose();
    mChoiceValue = choice->Value();
    mChoiceText = choice->Name();
    switch (mChoiceValue) {
    case 0:
      return ChooseNonZero();
    case 1:
      return ChoosePositiveZero();
    case 2:
      return ChooseNegativeZero();
    case 3:
      return ChoosePositiveInfinity();
    case 4:
      return ChooseNegativeInfinity();
    case 5:
      return ChooseQuietNan();
    case 6:
      return ChooseSignalNan();
    case 7:
      return ChooseRandom();
    default:
      LOG(fail) << "{FpDataPattern::ChooseData} " << "unknown choose item \"" << mChoiceText << "\""<< endl;
      FAIL("Floating-point-data-pattern-choose-error");
    }
    return 0ull;
  }

  void FpDataPattern::ChooseLargeData(std::vector<uint64>& rDatas) const
  {
    for (unsigned i = 0; i < mBaseDataNumber; i ++)
      rDatas.push_back(ChooseData());
  }

  void FpDataPattern::Setup(const Generator& gen)
  {
#ifndef UNIT_TEST
    const ChoicesModerator* choices_mod = gen.GetChoicesModerator(EChoicesType::OperandChoices);
    try {
      mpChoiceTree = choices_mod->CloneChoiceTree("Floating-point data pattern");
    }
    catch (const ChoicesError& choices_err) {
      LOG(fail) << "{FpDataPattern::Setup} " << choices_err.what() << endl;
      FAIL("floating-point-data-pattern-setup-error");
    }
#endif
  }

  uint64 FpDataPattern::ChooseNonZero() const
  {
    uint64 dataWidth = GetDataWidth();
    uint64 dataMask = (dataWidth >= 8 ) ? -1ull : ((1ull << (dataWidth * 8)) - 1);
    uint64 result = Random::Instance()->Random64(1ull, dataMask);
    return result;
  }

  uint64 FpDataPattern::ChoosePositiveZero() const
  {
      uint64 dataWidth = GetDataWidth();
      return (0ull<<(dataWidth*8 -1));
  }

  uint64 FpDataPattern::ChooseNegativeZero() const
  {
      uint64 dataWidth = GetDataWidth();
      return (1ull<<(dataWidth*8 -1));
  }

  uint64 FpDataPattern::ChoosePositiveInfinity() const
  {
      uint64 result = 0ull;
      uint64 dataWidth = GetDataWidth()*8;
      switch (dataWidth) {
      case 8:
        result = 0x3ull << 5;
        break;
      case 16:
        result = 0x1Full<<10;
        break;
      case 32:
        result = 0xFFull<<23;
        break;
      case 64:
        result = 0x7FFull<<52;
        break;
      default:
        {
          LOG(fail) << "Data width is not match in FpDataPattern's ChoosePositiveInfinity method" << endl;
          FAIL("Data width is not match in FpDataPattern's ChoosePositiveInfinity method");
        }
      }

      return result;
  }

  uint64 FpDataPattern::ChooseNegativeInfinity() const
  {
      uint64 result = 0ull;
      uint64 dataWidth = GetDataWidth()*8;
      switch (dataWidth) {
      case 8:
        result = 0x7ull << 5;
        break;
      case 16:
        result = 0x3Full<<10;
        break;
      case 32:
        result = 0x1FFull<<23;
        break;
      case 64:
        result = 0xFFFull<<52;
        break;
      default:
        {
          LOG(fail) << "Data width is not match in FpDataPattern's ChooseNegativeInfinity method" << endl;
          FAIL("Data width is not match in FpDataPattern's ChooseNegativeInfinity method");
        }
      }

      return result;
  }

  uint64 FpDataPattern::ChooseQuietNan() const
  {
    uint64 result = 0ull;
    uint64 exponent = 0ull;
    uint64 significantBit = 0ull;
    uint64 fraction = 0ull;
    uint64 sign = Random::Instance()->Random64(0ull, 1ull);
    uint64 dataWidth = GetDataWidth()*8;
    switch (dataWidth) {
    case 8:
      exponent = 0x3ull<<5;
      significantBit = 1ull<<4;
      fraction = Random::Instance()->Random64(1ull, 0xFull);
      break;
    case 16:
      exponent = 0x1Full<<10;
      significantBit = 1ull<<9;
      fraction = Random::Instance()->Random64(1ull, 0x1FFull);
      break;
    case 32:
      exponent = 0xFFull<<23;
      significantBit = 1ull<<22;
      fraction = Random::Instance()->Random64(1ull, 0x3FFFFFull);
      break;
    case 64:
      exponent = 0x7FFull<<52;
      significantBit = 1ull<<51;
      fraction = Random::Instance()->Random64(1ull, 0x7FFFFFFFFFFFFull);
      break;
    default:
      {
        LOG(fail) << "Data width is not match in FpDataPattern's ChooseQuietNan method" << endl;
        FAIL("Data width is not match in FpDataPattern's ChooseQuietNan method");
      }
    }

    result = (sign<<(dataWidth-1)) + exponent + significantBit + fraction;
    return result;
  }

  uint64 FpDataPattern::ChooseSignalNan() const
  {
      uint64 result = 0ull;
      uint64 exponent = 0ull;
      uint64 significantBit = 0ull;
      uint64 fraction = 0ull;
      uint64 sign = Random::Instance()->Random64(0ull, 1ull);
      uint64 dataWidth = GetDataWidth()*8;
      switch (dataWidth) {
      case 8:
        exponent = 0x3ull<<5;
        significantBit = 0ull<<4;
        fraction = Random::Instance()->Random64(1ull, 0xFull);
        break;
      case 16:
        exponent = 0x1Full<<10;
        significantBit = 0ull<<9;
        fraction = Random::Instance()->Random64(1ull, 0x1FFull);
        break;
      case 32:
        exponent = 0xFFull<<23;
        significantBit = 0ull<<22;
        fraction = Random::Instance()->Random64(1ull, 0x3FFFFFull);
        break;
      case 64:
        exponent = 0x7FFull<<52;
        significantBit = 0ull<<51;
        fraction = Random::Instance()->Random64(1ull, 0x7FFFFFFFFFFFFull);
        break;
      default:
        {
          LOG(error) << "Data width is not match in FpDataPattern's ChooseSignalNan method" << endl;
          FAIL("Data width is not match in FpDataPattern's ChooseSignalNan method");
        }
      }

      result = (sign<<(dataWidth-1)) + exponent + significantBit + fraction;
      return result;
  }

  FpDataPattern::FpDataPattern(const FpDataPattern& rOther) : RandomDataPattern(rOther)
  {
    if (rOther.mpChoiceTree)
      mpChoiceTree = dynamic_cast<const ChoiceTree* >(rOther.mpChoiceTree->Clone());
  }
}
