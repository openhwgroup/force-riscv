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
#include "GenQuery.h"

#include "Constraint.h"
#include "Log.h"
#include "PageInfoRecord.h"
#include "StringUtils.h"

/*!
  \file GenQuery.cc
  \brief Code for GenQuery based objects.

  GenQuery type should be "read-only" actions that shall not change any internal states or randomization sequences.
 */

using namespace std;

namespace Force {

  void GenQuery::UnsupportedQueryDetailAttribute(const std::string& attrName) const
  {
    LOG(fail) << "Unsupported query detail attribute: " << attrName << " of query: " << ToString() << endl;
    FAIL("unsupported-query-detail-attribute");
  }

  void GenQuery::UnsupportedQueryDetailValueType(const std::string& attrName, const std::string& valueType) const
  {
    LOG(fail) << "Unsupported query detail value type: " << valueType << " for attribute: " << attrName << " of query: " << ToString() << endl;
    FAIL("unsupported-query-detail- value-type");
  }

  string GenQuery::ToString() const
  {
    return EQueryType_to_string(QueryType()) + " : " + PrimaryString();
  }

  GenQuery* GenQuery::GenQueryInstance(const std::string& queryName)
  {
    EQueryType query_type = string_to_EQueryType(queryName);
    GenQuery* ret_query = nullptr;

    switch (query_type) {
    case EQueryType::RegisterIndex:
      ret_query = new GenRegisterIndexQuery(query_type);
      break;
    case EQueryType::RegisterReloadValue:
      ret_query = new GenRegisterReloadValueQuery(query_type);
      break;
    case EQueryType::RegisterInfo:
      ret_query = new GenRegisterInfoQuery(query_type);
      break;
    case EQueryType::InstructionRecord:
      ret_query = new GenInstructionRecordQuery(query_type);
      break;
    case  EQueryType::GenState:
      ret_query = new GenStateQuery(query_type);
      break;
    case EQueryType::PageInfo:
      ret_query = new GenPageInfoQuery(query_type);
      break;
    case EQueryType::BranchOffset:
      ret_query = new GenBranchOffsetQuery(query_type);
      break;
    case EQueryType::RegisterFieldInfo:
      ret_query = new GenRegisterFieldInfoQuery(query_type);
      break;
    case EQueryType::ChoicesTreeInfo:
      ret_query = new GenChoicesTreeInfoQuery(query_type);
      break;
    case EQueryType::SimpleExceptionsHistory:
    case EQueryType::AdvancedExceptionsHistory:
      ret_query = new GenExceptionsHistoryQuery(query_type);
      break;
    case EQueryType::GetVmContextDelta:
      ret_query = new GetVmContextDeltaQuery(query_type);
      break;
    case EQueryType::GetVmCurrentContext:
      ret_query = new VmCurrentContextQuery(query_type);
      break;
    case EQueryType::HandlerSetMemory:
      ret_query = new GenHandlerSetMemoryQuery(query_type);
      break;
    case EQueryType::ResourceEntropy:
      ret_query = new GenResourceEntropyQuery(query_type);
      break;
    case EQueryType::ExceptionVectorBaseAddress:
      ret_query = new GenExceptionVectorBaseAddressQuery(query_type);
      break;
    case EQueryType::RestoreLoopContext:
      ret_query = new GenRestoreLoopContextQuery(query_type);
      break;
    default:
        ret_query = new GenUtilityQuery(query_type);
        break;
    }

    return ret_query;
  }

  GenRegisterReloadValueQuery::~GenRegisterReloadValueQuery ()
  {
    for (auto it = mFieldConstraintMap.begin(); it != mFieldConstraintMap.end(); ++it)
    {
      delete it->second;
    }
  }

  void GenRegisterReloadValueQuery::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    ConstraintSet* constraint_set = new ConstraintSet(valueStr);
    AddFieldConstraint(attrName, constraint_set);
  }

  void GenRegisterReloadValueQuery::AddDetail(const std::string& attrName, uint64 value)
  {
    ConstraintSet* constraint_set = new ConstraintSet(value);
    AddFieldConstraint(attrName, constraint_set);
  }

  void GenRegisterReloadValueQuery::AddFieldConstraint(const std::string& fieldName, ConstraintSet* pConstr)
  {
    auto field_finder = mFieldConstraintMap.find(fieldName);
    if (field_finder != mFieldConstraintMap.end()) {
      delete field_finder->second;
      field_finder->second = nullptr;
    }
    mFieldConstraintMap[fieldName] = pConstr;
  }

  GenRegisterFieldInfoQuery::~GenRegisterFieldInfoQuery ()
  {
    for (auto it = mFieldConstraintMap.begin(); it != mFieldConstraintMap.end(); ++it)
    {
      delete it->second;
    }
  }

  void GenRegisterFieldInfoQuery::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    ConstraintSet* constraint_set = new ConstraintSet(valueStr);
    AddFieldConstraint(attrName, constraint_set);
  }

  void GenRegisterFieldInfoQuery::AddDetail(const std::string& attrName, uint64 value)
  {
    ConstraintSet* constraint_set = new ConstraintSet(value);
    AddFieldConstraint(attrName, constraint_set);
  }

  void GenRegisterFieldInfoQuery::AddFieldConstraint(const std::string& fieldName, ConstraintSet* pConstr)
  {
    auto field_finder = mFieldConstraintMap.find(fieldName);
    if (field_finder != mFieldConstraintMap.end()) {
      delete field_finder->second;
      field_finder->second = nullptr;
    }
    mFieldConstraintMap[fieldName] = pConstr;
  }

  void GenStateQuery::SetValue(uint64 value) const
  {
    mValue = value;
    mIsValue = true;
  }

  void GenPageInfoQuery::Copy (const PageInformation& pageInformation) const
  {
    mPageInformation.Copy (pageInformation);
  }

  void GenPageInfoQuery::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "Type")
    {
      UnsupportedQueryDetailValueType(attrName, "integer");
    }
    else if (attrName == "Bank")
    {
      mBank = value;
    }
    else if (attrName == "Addr")
    {
      mAddr = value;
    }
    else
    {
      UnsupportedQueryDetailAttribute(attrName);
    }
  }

  void GenPageInfoQuery::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    if (attrName == "Type")
    {
      mPageType = valueStr;
    }
    else if (attrName == "Bank")
    {
      mBank = parse_uint64(valueStr);
    }
    else if (attrName == "Addr")
    {
      mAddr = parse_uint64(valueStr);
    }
    else
    {
      UnsupportedQueryDetailAttribute(attrName);
    }
  }

  void GenUtilityQuery::AddDetail(const std::string& attrName, uint64 value)
  {
    std::string::size_type pos = attrName.find("arg");
    if (pos == std::string::npos)
      UnsupportedQueryDetailAttribute(attrName);

    std::string arg = attrName.substr(pos+3);
    uint32 arg_num = atoi(arg.c_str());
    if (mIntArgs.size() < arg_num)
      mIntArgs.resize(arg_num);

    mIntArgs[arg_num-1] = value;
    //LOG(notice) << "GenUtilityQuery::AddDetail " << attrName << " : " << arg  << hex << " : " << value << endl;
  }

  void GenUtilityQuery::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    std::string::size_type pos = attrName.find("arg");
    if (pos == std::string::npos)
      UnsupportedQueryDetailAttribute(attrName);

    std::string arg = attrName.substr(pos+3);
    uint32 arg_num = atoi(arg.c_str());
    if (mStrArgs.size() < arg_num)
      mStrArgs.resize(arg_num);

    mStrArgs[arg_num-1] = valueStr;;
    //LOG(notice) << "GenUtilityQuery::AddDetail " << attrName << " : " << arg  << hex << " : " << valueStr << endl;
  }

  void GenExceptionsHistoryQuery::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    if (attrName != "EC" && attrName != "PC" && attrName != "SRC_PRIVLEV" && attrName != "TGT_PRIVLEV" && attrName != "Last" && attrName != "FSC" )
      UnsupportedQueryDetailAttribute(attrName);
    mInputArgs[attrName] = valueStr;
    //LOG(notice) << "GenUtilityQuery::AddDetail " << attrName << " : " << arg  << hex << " : " << value << endl;
  }

  void GetVmContextDeltaQuery::AddDetail(const std::string& attrName, const uint64 value)
  {
    if (attrName != "VmContextId")
      UnsupportedQueryDetailAttribute(attrName);
    mInputArgs[attrName] = value;
    //LOG(notice) << "GetVmContextDeltaQuery::AddDetail " << attrName << " : " << endl;
  }

  GenBranchOffsetQuery::GenBranchOffsetQuery(EQueryType queryType)
    : GenQuery(queryType), mBranchAddress(0), mTargetAddress(0), mOffsetSize(0), mShift(0), mOffset(0), mHalfWords(0), mValid(false)
  {

  }

  void GenBranchOffsetQuery::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "BranchAddress") {
      mBranchAddress = value;
    }
    else if (attrName == "TargetAddress") {
      mTargetAddress = value;
    }
    else if (attrName == "OffsetSize") {
      mOffsetSize = value;
    }
    else if (attrName == "Shift") {
      mShift = value;
    }
    else {
      UnsupportedQueryDetailAttribute(attrName);
    }
  }

  void GenBranchOffsetQuery::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    uint64 value = parse_uint64(valueStr);
    AddDetail(attrName, value);
  }

  void  GenChoicesTreeInfoQuery::AddChoiceInfo(const std::string& choiceName, uint32 weight) const
  {
    mChoicesInfo[choiceName] = weight;
  }


  EntropyInfo& EntropyInfo::operator=(const EntropyInfo& rOther)
  {
    mState = rOther.mState;
    mEntropy = rOther.mEntropy;
    mOnThreshold = rOther.mOnThreshold;
    mOffThreshold = rOther.mOffThreshold;
    return *this;
  }

  GenResourceEntropyQuery::GenResourceEntropyQuery(EQueryType queryType) : GenQuery(queryType), mSourceEntropy(), mDestEntropy()
  {

  }

  GenResourceEntropyQuery::~GenResourceEntropyQuery()
  {

  }

  void GenResourceEntropyQuery::SetSourceEntropy(const EntropyInfo& info) const
  {
    mSourceEntropy = info;
  }

  void GenResourceEntropyQuery::SetDestEntropy(const EntropyInfo& info) const
  {
    mDestEntropy = info;
  }
}
