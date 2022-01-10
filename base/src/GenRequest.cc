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
#include <GenRequest.h>
#include <OperandRequest.h>
#include <OperandDataRequest.h>
#include <GenException.h>
#include <UtilityFunctions.h>
#include <StringUtils.h>
#include <VmUtils.h>
#include <Constraint.h>
#include <Log.h>
#include <Config.h>
#include <BntNode.h>
#include <Instruction.h>
#include <sstream>
#include <algorithm>
#include <cctype>

/*!
  \file GenRequest.cc
  \brief Code modeling various requests based on GenRequest class to Generator class and the GenRequestQueue container class.
 */

using namespace std;

namespace Force {

  static EMemDataType parse_data_type(const std::string& data_type)
  {
    if (data_type == "I") {
      return EMemDataType::Instruction;
    }
    else if (data_type == "D") {
      return EMemDataType::Data;
    }
    // else if (data_type == "ID" || data_type =="DI") {
    // return EMemDataType::Both;
    // }
    else {
      LOG(fail) << "{parse_data_type} unknown data type specified: " << data_type << endl;
    }
    return EMemDataType::Data;
  }

  const string GenRequest::ToString() const
  {
    return "GenRequest";
  }

  void GenRequest::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    auto value = parse_uint64(valueStr);
    AddDetail(attrName, value);
  }

  void GenRequest::UnsupportedRequestDetailAttribute(const std::string& attrName) const
  {
    LOG(fail) << "Unsupported request detail attribute: " << attrName << " of request: " << RequestType() << endl;
    FAIL("unsupported-request-detail-attribute");
  }

  GenInstructionRequest::GenInstructionRequest(const std::string& id)
    : GenRequest(), mInstructionId(id), mBoolAttributes(0), mOperandRequests(), mOperandDataRequests(), mInstructionConstraints(), mLSDataConstraints(), mLSTargetListConstraints()
  {
    mInstructionConstraints.assign(EInstrConstraintAttrTypeSize, nullptr);
  }

  GenInstructionRequest::GenInstructionRequest(const GenInstructionRequest& rOther)
    : GenRequest(rOther), mInstructionId(rOther.mInstructionId), mBoolAttributes(rOther.mBoolAttributes), mOperandRequests(), mOperandDataRequests(), mInstructionConstraints(), mLSDataConstraints(), mLSTargetListConstraints()
  {
    for (auto req_iter : rOther.mOperandRequests) {
      mOperandRequests[req_iter.first] = dynamic_cast<OperandRequest* >(req_iter.second->Clone());
    }

    for (auto req_iter : rOther.mOperandDataRequests) {
      mOperandDataRequests[req_iter.first] = dynamic_cast<OperandDataRequest* >(req_iter.second->Clone());
    }

    for (auto instr_constr : rOther.mInstructionConstraints) {
      if (nullptr != instr_constr) {
        mInstructionConstraints.push_back(instr_constr->Clone());
      }
      else {
        mInstructionConstraints.push_back(nullptr);
      }
    }
  }

  GenInstructionRequest::~GenInstructionRequest()
  {
    for (auto req_iter : mOperandRequests) {
      delete req_iter.second;
    }
    for (auto req_iter : mOperandDataRequests) {
      delete req_iter.second;
    }
    for (auto instr_constr: mInstructionConstraints) {
      delete instr_constr;
    }
    for (auto instr_constr: mLSDataConstraints) {
      delete instr_constr;
    }
    for (auto instr_constr: mLSTargetListConstraints) {
      delete instr_constr;
    }
  }

  const string GenInstructionRequest::ToString() const
  {
    return "GenInstructionRequest: " + mInstructionId;
  }

  void GenInstructionRequest::AddOperandRequest(const std::string& oprName, uint64 value)
  {
    auto find_iter = mOperandRequests.find(oprName);
    if (find_iter == mOperandRequests.end()) {
      mOperandRequests[oprName] = new OperandRequest(oprName, value);
    }
    else {
      auto existing_req = find_iter->second;
      existing_req->SetValueRequest(value);
    }
  }

  void GenInstructionRequest::AddOperandRequest(const std::string& oprName, const std::string& valueStr)
  {
    auto find_iter = mOperandRequests.find(oprName);
    if (find_iter == mOperandRequests.end()) {
      mOperandRequests[oprName] = new OperandRequest(oprName, valueStr);
    }
    else {
      auto existing_req = find_iter->second;
      existing_req->SetValueRequest(valueStr);
    }
  }

  void GenInstructionRequest::AddOperandDataRequest(const std::string& oprName, const std::string& valueStr) const
  {
    //<< "{GenInstructionRequest::AddOperandDataRequest oprName=" << oprName << " valstr=" << valueStr << endl;
    auto find_iter = mOperandDataRequests.find(oprName);
    if (find_iter == mOperandDataRequests.end()) {
      mOperandDataRequests[oprName] = new OperandDataRequest(oprName, valueStr);
    }
    else {
      auto existing_req = find_iter->second;
      existing_req->SetDataRequest(valueStr);
    }
  }

  void GenInstructionRequest::AddLSDataRequest(const std::string& valueStr)
  {
    char separator = ';';
    string constr_str = valueStr;
    strip_white_spaces(constr_str);
    bool empty_str_no_skip = true;

    StringSplitter ss(constr_str, separator, empty_str_no_skip);
    while (not ss.EndOfString()) {
      string sub_str = ss.NextSubString();
      if (not sub_str.empty())
      {
        mLSDataConstraints.push_back(new ConstraintSet(sub_str));
      }
      else
      {
        mLSDataConstraints.push_back(nullptr);
      }
    }
  }
  void GenInstructionRequest::AddLSTargetListRequest(const std::string& valueStr)
  {
    char separator = ';';
    string constr_str = valueStr;
    strip_white_spaces(constr_str);
    bool empty_str_no_skip = true;
    StringSplitter ss(constr_str, separator, empty_str_no_skip);
    while (not ss.EndOfString()) {
      string sub_str = ss.NextSubString();
      if (not sub_str.empty())
      {
        mLSTargetListConstraints.push_back(new ConstraintSet(sub_str));
      }
      else
      {
        mLSTargetListConstraints.push_back(nullptr);
      }
    }
  }

  void GenInstructionRequest::SetOperandDataRequest(const std::string& oprName, uint64 value, uint32 size) const
  {
    char print_buffer[256];
    snprintf(print_buffer, 256, "INT%d(0x%llx)", size, value);
    AddOperandDataRequest(oprName, print_buffer);
  }

  void GenInstructionRequest::SetOperandDataRequest(const std::string& oprName, const string& valueStr, uint32 size) const
  {
    char print_buffer[256];
    snprintf(print_buffer, 256, "INT%d(%s)", size, valueStr.c_str());
    AddOperandDataRequest(oprName, print_buffer);
  }

  void GenInstructionRequest::SetOperandDataRequest(const std::string& oprName, std::vector<uint64> values, uint32 size) const
  {
    stringstream ss;
    for (uint32 i = 0; i < values.size(); ++i){
      ss << dec << "[" << i << "]INT" << size << "(0x" << hex << values[i] << ")";
    }
    AddOperandDataRequest(oprName,ss.str());
  }

  void GenInstructionRequest::SetBoolAttribute(EInstrBoolAttrType attrType, bool isSet)
  {
    uint64 set_bit = 1ull << (uint32)(attrType);
    if (isSet) {
      mBoolAttributes |= set_bit;
    } else {
      mBoolAttributes &= ~set_bit;
    }
  }

  bool GenInstructionRequest::BoolAttribute(EInstrBoolAttrType attrType) const
  {
    return (mBoolAttributes >> (uint32)(attrType)) & 1;
  }

  void GenInstructionRequest::SetConstraintAttribute(EInstrConstraintAttrType attrType, ConstraintSet* pConstrSet)
  {
    auto existing_ptr = mInstructionConstraints[int(attrType)];
    if (nullptr != existing_ptr) {
      delete existing_ptr;
    }
    mInstructionConstraints[int(attrType)] = pConstrSet;
  }

  void GenInstructionRequest::AddDetail(const string& attrName, uint64 value)
  {
    bool convert_okay = false;
    EInstrBoolAttrType bool_attr = try_string_to_EInstrBoolAttrType(attrName, convert_okay);
    if (convert_okay) {
      SetBoolAttribute(bool_attr, bool(value));
    }
    else {
      EInstrConstraintAttrType constr_attr = try_string_to_EInstrConstraintAttrType(attrName, convert_okay);
      if (convert_okay) {
        SetConstraintAttribute(constr_attr, new ConstraintSet(value));
      }
      else {
        AddOperandRequest(attrName, value);
      }
    }
  }

  void  GenInstructionRequest::AddDetail(const string& attrName, const string& valueStr)
  {
    bool convert_okay = false;
    EInstrBoolAttrType bool_attr = try_string_to_EInstrBoolAttrType(attrName, convert_okay);
    if (convert_okay) {
      uint64 value = parse_uint64(valueStr);
      SetBoolAttribute(bool_attr, bool(value));
    }
    else {
      EInstrConstraintAttrType constr_attr = try_string_to_EInstrConstraintAttrType(attrName, convert_okay);
      if (convert_okay) {
        SetConstraintAttribute(constr_attr, new ConstraintSet(valueStr));
      }
      else {
        size_t data_pos = attrName.find(".Data");
        //<< "{GenInstructionRequest::AddDetail} data_pos=" << data_pos << endl;
        if (data_pos != string::npos) {
          AddOperandDataRequest(attrName.substr(0, data_pos), valueStr);
        }
        else if (attrName.find("LSData") != string::npos) {
          AddLSDataRequest(valueStr);
        }
        else if (attrName.find("LSTargetList") != string::npos) {
          AddLSTargetListRequest(valueStr);
        }
        else {
          AddOperandRequest(attrName, valueStr);
        }
      }
    }
  }

  const OperandRequest* GenInstructionRequest::FindOperandRequest(const string& opName) const
  {
    auto find_iter = mOperandRequests.find(opName);
    if (find_iter != mOperandRequests.end()) {
      return find_iter->second;
    }

    return nullptr;
  }

  const OperandDataRequest* GenInstructionRequest::FindOperandDataRequest(const std::string& opName) const
  {
    auto find_iter = mOperandDataRequests.find(opName);
    if (find_iter != mOperandDataRequests.end()) {
      return find_iter->second;
    }

    return nullptr;
  }

  void GenInstructionRequest::NotAppliedOperandRequests() const
  {
    auto failOnOverrides =  Config::Instance()->FailOnOperandOverrides();
    for (auto const& map_item : mOperandRequests) {
      if (!map_item.second->IsApplied() && !map_item.second->IsIgnored()) {
        if (failOnOverrides) {
          LOG(fail) << "{Instruction request} fail on operand override \"" << map_item.first << ":" << map_item.second->GetValueConstraint()->ToSimpleString() << "\""<<std::endl;
          FAIL("failed-on-operand override");
        }
        LOG(notice) << "{Instruction request} ignored operand override \"" << map_item.first << ":" << map_item.second->GetValueConstraint()->ToSimpleString() << "\""<<std::endl;
      }
    }

    for (auto const& map_item : mOperandDataRequests) {
      if (!map_item.second->IsApplied()) {
        if (failOnOverrides) {
          LOG(fail) << "{Instruction request} fail on operand data override \"" << map_item.first << ":" << map_item.second->ToString() << "\""<<std::endl;
          FAIL("failed-on-operand data override");
        }
        LOG(notice) << "{Instruction request} ignored operand data override \"" << map_item.first << ":" << map_item.second->ToString() << "\""<<std::endl;
      }
    }

  }

  GenSequenceRequest* GenSequenceRequest::GenSequenceRequestInstance(const string& rSequenceType)
  {
    auto sequence_type = string_to_ESequenceType(rSequenceType);

    switch (sequence_type) {
    case ESequenceType::ReExecution:
      return new GenReExecutionRequest();
      break;
    case ESequenceType::UpdatePeState:
      return new GenPeStateUpdateRequest();
      break;
    case ESequenceType::UpdateRegisterField:
      return new GenRegisterFieldUpdateRequest();
      break;
    case ESequenceType::InitializeAddrTables:
      return new GenInitializeAddrTablesRequest();
      break;
    default:
      return new GenSequenceRequest(sequence_type);
    }

    return nullptr;
  }

  GenSequenceRequest::GenSequenceRequest(ESequenceType seqType)
    : GenRequest(), mSequenceType(seqType), mIntArgs()
  {

  }

  GenSequenceRequest::GenSequenceRequest(const std::string& seqTypeStr)
    : GenRequest(), mSequenceType(string_to_ESequenceType(seqTypeStr)), mIntArgs()
  {
    mSequenceType = string_to_ESequenceType(seqTypeStr);
  }

  GenSequenceRequest::GenSequenceRequest(const GenSequenceRequest& rOther)
    : GenRequest(rOther), mSequenceType(rOther.mSequenceType), mIntArgs()
  {

  }

  const string GenSequenceRequest::ToString() const
  {
    return "GenSequenceRequest: " + ESequenceType_to_string(mSequenceType);
  }

  void GenSequenceRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    std::string::size_type pos = attrName.find("arg");
    if (pos == std::string::npos)
      UnsupportedRequestDetailAttribute(attrName);

    std::string arg = attrName.substr(pos+3,1);
    uint32 arg_num = atoi(arg.c_str());

    if (mIntArgs.size() < arg_num)
      mIntArgs.resize(arg_num);
    mIntArgs[arg_num-1] = value;
    LOG(info) << "{GenSequenceRequest::AddDetail} " << attrName << " : " << arg  << hex << " :0x" << mIntArgs[arg_num-1] << " : " << arg_num << endl;
  }

  GenLoadRegister::GenLoadRegister(const GenLoadRegister& rOther)
    : GenSequenceRequest(rOther), mRegisterName(rOther.mRegisterName), mRegisterValue(rOther.mRegisterValue), mInterRegName(rOther.mInterRegName)
  {

  }

  GenLoadRegister::GenLoadRegister(const std::string& regName, uint64 value, const std::string& inter_reg_name)
    : GenSequenceRequest(ESequenceType::LoadRegister), mRegisterName(regName), mRegisterValue(value), mInterRegName(inter_reg_name)
  {
  }

  GenReloadRegister::GenReloadRegister(const GenReloadRegister& rOther)
    : GenSequenceRequest(rOther), mInterRegName(rOther.mInterRegName), mReloadMap(rOther.mReloadMap), mReloadMethod(rOther.mReloadMethod), mReloadMethodForced(rOther.mReloadMethodForced), mType(rOther.mType)
  {
  }

  GenReloadRegister::GenReloadRegister()
    : GenSequenceRequest(ESequenceType::ReloadRegister), mInterRegName(), mReloadMap(), mReloadMethod(EReloadingMethodType(0)), mReloadMethodForced(false), mType(ERegisterType(0))
  {
  }

  void GenReloadRegister::AddReloadRegister(const string& regName, uint64 value)
  {
    mReloadMap.insert(pair<string, uint64>(regName, value));
  }

  GenBatchReloadRegisters::GenBatchReloadRegisters(const std::string& interRegName)
    : GenSequenceRequest(ESequenceType::BatchReloadRegisters), mInterRegName(interRegName), mRegisters()
  {
  }

  GenBatchReloadRegisters::GenBatchReloadRegisters(const GenBatchReloadRegisters& rOther)
    : GenSequenceRequest(rOther), mInterRegName(rOther.mInterRegName), mRegisters()
  {
    copy(rOther.mRegisters.begin(), rOther.mRegisters.end(), back_inserter(mRegisters));
  }

  void GenBatchReloadRegisters::AddReloadRegister(const string& regName)
  {
    mRegisters.push_back(regName);
  }

  GenSetRegister::GenSetRegister(Register* pReg, uint64 value)
    : GenSequenceRequest(ESequenceType::SetRegister), mpRegister(pReg), mRegisterValue(value)
  {
  }

  GenSetRegister::GenSetRegister(const GenSetRegister& rOther)
    : GenSequenceRequest(rOther), mpRegister(rOther.mpRegister), mRegisterValue(rOther.mRegisterValue)
  {
  }

  GenBranchToTarget::GenBranchToTarget(const GenBranchToTarget& rOther)
    : GenSequenceRequest(rOther), mTargetValue(rOther.mTargetValue), mNear(rOther.mNear), mNoBnt(rOther.mNoBnt)
  {

  }

  GenBranchToTarget::GenBranchToTarget(uint64 target, bool near, bool noBnt)
    : GenSequenceRequest(ESequenceType::BranchToTarget), mTargetValue(target), mNear(near), mNoBnt(noBnt)
  {
  }

  GenCommitInstruction::GenCommitInstruction(Instruction* instr, GenInstructionRequest* instrReq)
    : GenSequenceRequest(ESequenceType::CommitInstruction), mpInstruction(instr), mpInstructionRequest(instrReq)
  {
  }

  GenCommitInstruction::GenCommitInstruction(const GenCommitInstruction& rOther)
    : GenSequenceRequest(rOther), mpInstruction(nullptr), mpInstructionRequest(nullptr)
  {
    LOG(fail) << "{GenCommitInstruction::GenCommitInstruction} copy constructor not intended to be used." << endl;
    FAIL("unintended-copy-constructing");
  }

  GenCommitInstruction::~GenCommitInstruction()
  {
    if (nullptr != mpInstruction) {
      LOG(fail) << "{GenCommitInstruction::~GenCommitInstruction} expecting pointer to instruction to be nullptr at this point." << endl;
      FAIL("dangling-instruction-pointer");
    }

    if (nullptr != mpInstructionRequest) {
      LOG(fail) << "{GenCommitInstruction::~GenCommitInstruction} expecting pointer to instruction-request to be nullptr at this point." << endl;
      FAIL("dangling-instruction-request-pointer");
    }
  }

  Instruction* GenCommitInstruction::GiveInstruction()
  {
    Instruction* temp_instr = mpInstruction;
    mpInstruction = nullptr;
    return temp_instr;
  }

  GenInstructionRequest* GenCommitInstruction::GiveInstructionRequest()
  {
    GenInstructionRequest* temp_req = mpInstructionRequest;
    mpInstructionRequest = nullptr;
    return temp_req;
  }

  void GenCommitInstruction::CleanUp()
  {
    if (mpInstruction != nullptr) {
      mpInstruction->CleanUp();
      delete mpInstruction;
      mpInstruction = nullptr;
    }
    if (mpInstructionRequest != nullptr) {
      mpInstructionRequest->CleanUp();
      delete mpInstructionRequest;
      mpInstructionRequest = nullptr;
    }

  }

  GenRegisterReservation::GenRegisterReservation(const std::string& regName, bool doReserve, ERegAttrType regAttr)
    : GenSequenceRequest(ESequenceType::RegisterReservation), mRegisterName(regName), mDoReserve(doReserve), mReservationAttributes(regAttr)
  {

  }

  GenRegisterReservation::GenRegisterReservation(const GenRegisterReservation& rOther)
    : GenSequenceRequest(rOther), mRegisterName(rOther.mRegisterName), mDoReserve(rOther.mDoReserve), mReservationAttributes(rOther.mReservationAttributes)
  {

  }

  GenEscapeCollision::GenEscapeCollision()
    : GenSequenceRequest(ESequenceType::EscapeCollision)
  {

  }

  GenEscapeCollision::GenEscapeCollision(const GenEscapeCollision& rOther)
    : GenSequenceRequest(rOther)
  {

  }

  GenBntRequest::GenBntRequest(BntNode* pBntNode)
    : GenSequenceRequest(ESequenceType::BntNode), mpBntNode(pBntNode)
  {

  }

  GenBntRequest::GenBntRequest()
    : GenSequenceRequest(ESequenceType::BntNode), mpBntNode(nullptr)
  {

  }

  GenBntRequest::GenBntRequest(const GenBntRequest& rOther)
    : GenSequenceRequest(rOther), mpBntNode(nullptr)
  {

  }

  GenBntRequest::~GenBntRequest()
  {
    if (nullptr != mpBntNode) {
      LOG(fail) << "{GenBntRequest::~GenBntRequest} dangling BntNode pointer." << endl;
      FAIL("dangling-bnt-node-pointer");
    }
  }

  BntNode* GenBntRequest::GiveBntNode()
  {
    BntNode* node_ptr = mpBntNode;
    mpBntNode = nullptr;
    return node_ptr;
  }

  GenSpeculativeBntRequest::GenSpeculativeBntRequest(BntNode* pBntNode, ESpeculativeBntActionType actionType) : GenSequenceRequest(ESequenceType::SpeculativeBntNode), mpBntNode(pBntNode), mActionType(actionType)
  {

  }

  GenSpeculativeBntRequest::GenSpeculativeBntRequest() : GenSequenceRequest(ESequenceType::SpeculativeBntNode), mpBntNode(nullptr), mActionType(ESpeculativeBntActionType(0))
  {

  }

  GenSpeculativeBntRequest::GenSpeculativeBntRequest(const GenSpeculativeBntRequest& rOther) : GenSequenceRequest(rOther), mpBntNode(nullptr), mActionType(ESpeculativeBntActionType(0))
  {

  }

  GenSpeculativeBntRequest::~GenSpeculativeBntRequest()
  {
    mpBntNode = nullptr;
  }

  GenReExecutionRequest::GenReExecutionRequest(uint64 pc)
    : GenSequenceRequest(ESequenceType::ReExecution), mReExecutionAddress(pc), mMaxReExecutionInstructions(MAX_UINT32)
  {

  }

  GenReExecutionRequest::GenReExecutionRequest()
    : GenSequenceRequest(ESequenceType::ReExecution), mReExecutionAddress(0), mMaxReExecutionInstructions(MAX_UINT32)
  {

  }

  GenReExecutionRequest::GenReExecutionRequest(const GenReExecutionRequest& rOther)
    : GenSequenceRequest(rOther), mReExecutionAddress(0), mMaxReExecutionInstructions(rOther.mMaxReExecutionInstructions)
  {

  }

  GenReExecutionRequest::~GenReExecutionRequest()
  {

  }

  void GenReExecutionRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "Address") {
      mReExecutionAddress = value;
    }
    else if (attrName == "MaxReExecutionInstructions") {
      if (value != 0) {
        mMaxReExecutionInstructions = value;
      }
    }
    else {
      UnsupportedRequestDetailAttribute(attrName);
    }
  }

  GenPeStateUpdateRequest::GenPeStateUpdateRequest(uint32 id)
    : GenSequenceRequest(ESequenceType::UpdatePeState), mRecordId(id)
  {

  }

  GenPeStateUpdateRequest::GenPeStateUpdateRequest()
    : GenSequenceRequest(ESequenceType::UpdatePeState), mRecordId(0)
  {

  }

  GenPeStateUpdateRequest::GenPeStateUpdateRequest(const GenPeStateUpdateRequest& rOther)
    : GenSequenceRequest(rOther), mRecordId(0)
  {

  }

  GenPeStateUpdateRequest::~GenPeStateUpdateRequest()
  {

  }

  void GenPeStateUpdateRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "RecordId") {
      mRecordId = value;
    }
    else {
      UnsupportedRequestDetailAttribute(attrName);
    }
  }

  GenRegisterFieldUpdateRequest::GenRegisterFieldUpdateRequest(const std::string& reg_name, const std::string& field_name, uint64 value)
    : GenSequenceRequest(ESequenceType::UpdateRegisterField), mRegName(reg_name), mFieldName(field_name), mFieldValue(value), mMask(0), mValue(0)
  {

  }

  GenRegisterFieldUpdateRequest::GenRegisterFieldUpdateRequest()
    : GenSequenceRequest(ESequenceType::UpdateRegisterField), mRegName(), mFieldName(), mFieldValue(0), mMask(0), mValue(0)
  {

  }

  GenRegisterFieldUpdateRequest::GenRegisterFieldUpdateRequest(const GenRegisterFieldUpdateRequest& rOther)
    : GenSequenceRequest(rOther), mRegName(), mFieldName(), mFieldValue(0), mMask(0), mValue(0)
  {

  }

  GenRegisterFieldUpdateRequest::~GenRegisterFieldUpdateRequest()
  {

  }

  void GenRegisterFieldUpdateRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "FieldValue") {
      mFieldValue = value;
    } else if (attrName == "Mask") {
      mMask = value;
    } else if (attrName == "Value") {
      mValue = value;
    }
    else {
      UnsupportedRequestDetailAttribute(attrName);
    }
  }

  void GenRegisterFieldUpdateRequest::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    if (attrName == "RegName") {
      mRegName = valueStr;
    } else if (attrName == "FieldName") {
      mFieldName = valueStr;
    } else if (attrName == "FieldValue") {
      mFieldValue = parse_uint64(valueStr);
    } else if (attrName == "Mask") {
      mMask = parse_uint64(valueStr);
    } else if (attrName == "Value") {
      mValue = parse_uint64(valueStr);
    }
    else {
      UnsupportedRequestDetailAttribute(attrName);
    }
  }

  GenInitializeAddrTablesRequest::GenInitializeAddrTablesRequest() : GenSequenceRequest(ESequenceType::InitializeAddrTables), mAddrTableIndex(0), mFastMode(0)
  {
  }

  GenInitializeAddrTablesRequest::GenInitializeAddrTablesRequest(const GenInitializeAddrTablesRequest& rOther): GenSequenceRequest(rOther), mAddrTableIndex(rOther.mAddrTableIndex), mFastMode(rOther.mFastMode)
  {
  }

  void GenInitializeAddrTablesRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "table_index") {
      mAddrTableIndex = value;
    }
    else if (attrName == "fast_mode") {
      mFastMode = value;
    }
    else {
      UnsupportedRequestDetailAttribute(attrName);
    }
  }

  GenRestoreRequest::GenRestoreRequest(const ESequenceType seqType)
    : GenRestoreRequest(seqType, 0, 1, 1, set<ERestoreExclusionGroup>())
  {
  }

  GenRestoreRequest::GenRestoreRequest(const ESequenceType seqType, cuint32 loopRegIndex, cuint32 simCount, cuint32 restoreCount, const set<ERestoreExclusionGroup>& restoreExclusions)
    : GenSequenceRequest(seqType), mLoopRegIndex(loopRegIndex), mSimCount(simCount), mRestoreCount(restoreCount), mRestoreExclusions(restoreExclusions), mLoopId(0)
  {
  }

  GenRestoreRequest::GenRestoreRequest(const GenRestoreRequest& rOther)
    : GenSequenceRequest(rOther), mLoopRegIndex(rOther.mLoopRegIndex), mSimCount(rOther.mSimCount), mRestoreCount(rOther.mRestoreCount), mRestoreExclusions(rOther.mRestoreExclusions), mLoopId(rOther.mLoopId)
  {
  }

  void GenRestoreRequest::SetLoopId(cuint32 loopId)
  {
    mLoopId = loopId;
  }

  GenLoopReconvergeRequest::GenLoopReconvergeRequest(uint64 currentPC, uint64 lastPC)
    : GenSequenceRequest(ESequenceType::LoopReconverge), mCurrentPC(currentPC), mLastPC(lastPC)
  {

  }

  GenLoopReconvergeRequest::GenLoopReconvergeRequest(const GenLoopReconvergeRequest& rOther)
    : GenSequenceRequest(rOther), mCurrentPC(rOther.mCurrentPC), mLastPC(rOther.mLastPC)
  {
  }

  GenVirtualMemoryRequest* GenVirtualMemoryRequest::GenVirtualMemoryRequestInstance(const std::string& reqName)
  {
    EVmRequestType req_type = string_to_EVmRequestType(reqName);

    switch (req_type) {
    case EVmRequestType::GenPA:
      return new GenPaRequest();
      break;
    case EVmRequestType::GenVA:
      return new GenVaRequest();
      break;
    case EVmRequestType::GenVMVA:
      return new GenVmVaRequest();
      break;
    case EVmRequestType::GenVAforPA:
      return new GenVaForPaRequest();
      break;
    case EVmRequestType::GenVmContext:
      return new GenVmContextRequest();
      break;
    case EVmRequestType::GenPage:
      return new GenPageRequest();
      break;
    case EVmRequestType::PhysicalRegion:
      return new GenPhysicalRegionRequest();
      break;
    case EVmRequestType::UpdateVm:
      return new GenUpdateVmRequest();
      break;
    default:
      LOG(fail) << "{GenVirtualMemoryRequest::GenVirtualMemoryRequestInstance} unsupported virtual-memory-request type: " << reqName << endl;
      FAIL("unsupported-virtual-memory-request-type");
    }

    return nullptr;
  }

  GenVirtualMemoryRequest::GenVirtualMemoryRequest(EVmRequestType reqType)
    : GenRequestWithResult(), mVmRequestType(reqType), mDataType(EMemDataType::Data), mBankType(EMemBankType(0)), mPrivilegeLevel(EPrivilegeLevelType(0)), mSize(1), mAlign(1), mTag(0x1000), mPhysAddr(0), mPhysPageId(0),
      mPrivilegeLevelSpecified(false), mBankSpecified(false), mForceAlias(false), mFlatMap(false), mForceMemAttrs(false), mCanAlias(true), mForceNewAddr(false), mSharedMemory(false), mImplMemAttributes(), mArchMemAttributes(),
      mAliasImplMemAttributes(), mpMemoryRangesConstraint(nullptr), mVmContextParams(), mVmInfoBoolTypes(0), mVmInfoBoolTypeMask(0)
  {

  }

  GenVirtualMemoryRequest::GenVirtualMemoryRequest(const GenVirtualMemoryRequest& rOther)
    : GenRequestWithResult(rOther), mVmRequestType(rOther.mVmRequestType), mDataType(rOther.mDataType), mBankType(rOther.mBankType), mPrivilegeLevel(rOther.mPrivilegeLevel), mSize(rOther.mSize), mAlign(rOther.mAlign), mTag(rOther.mTag),
      mPhysAddr(rOther.mPhysAddr), mPhysPageId(rOther.mPhysPageId), mPrivilegeLevelSpecified(rOther.mPrivilegeLevelSpecified), mBankSpecified(rOther.mBankSpecified), mForceAlias(rOther.mForceAlias), mFlatMap(rOther.mFlatMap), mForceMemAttrs(rOther.mForceMemAttrs),
      mCanAlias(rOther.mCanAlias), mForceNewAddr(rOther.mForceNewAddr), mSharedMemory(rOther.mSharedMemory), mImplMemAttributes(), mArchMemAttributes(), mAliasImplMemAttributes(), mpMemoryRangesConstraint(nullptr),
      mVmContextParams(), mVmInfoBoolTypes(rOther.mVmInfoBoolTypes), mVmInfoBoolTypeMask(rOther.mVmInfoBoolTypeMask)
  {
    copy(rOther.mImplMemAttributes.begin(), rOther.mImplMemAttributes.end(), back_inserter(mImplMemAttributes));
    copy(rOther.mArchMemAttributes.begin(), rOther.mArchMemAttributes.end(), back_inserter(mArchMemAttributes));
    copy(rOther.mAliasImplMemAttributes.begin(), rOther.mAliasImplMemAttributes.end(), back_inserter(mAliasImplMemAttributes));

    if (rOther.mpMemoryRangesConstraint) {
      mpMemoryRangesConstraint = dynamic_cast<ConstraintSet* >(rOther.mpMemoryRangesConstraint->Clone());
    }

    for (auto& vm_param_iter : rOther.mVmContextParams) {
      mVmContextParams[vm_param_iter.first] = vm_param_iter.second;
    }
  }

  GenVirtualMemoryRequest::~GenVirtualMemoryRequest()
  {
    delete mpMemoryRangesConstraint;
  }

  const string GenVirtualMemoryRequest::ToString() const
  {
    stringstream out_str;
    out_str << RequestType() << ": " << EVmRequestType_to_string(mVmRequestType)
            << ", DataType=" << EMemDataType_to_string(mDataType)
            << ", BankType=" << EMemBankType_to_string(mBankType)
            << (mBankSpecified ? "(Specified)" : "(None)")
            << ", PrivelegeLevel=" << EPrivilegeLevelType_to_string(mPrivilegeLevel)
            << (mPrivilegeLevelSpecified ? "(Specified)" : "(None)")
            << ", Size=0x" << hex << mSize
            << ", Align=0x" << mAlign
            << ", Tag=0x" << mTag
            << ", PhysAddr=0x" << mPhysAddr
            << ", PhysPageId=0x" << mPhysPageId
            << ", ForceAlias=" << mForceAlias
            << ", FlatMap=" << mFlatMap
            << ", ForceMemAttrs=" << mForceMemAttrs
            << ", CanAlias=" << mCanAlias
            << ", ForceNewAddr=" << mForceNewAddr
            << ", SharedMemory=" << mSharedMemory;

    out_str << ", MemAttrImpl=[";
    for (const string& impl_mem_attr : mImplMemAttributes) {
      out_str << impl_mem_attr << ",";
    }
    out_str << "]";

    out_str << ", MemAttrArch=[";
    for (EMemoryAttributeType arch_mem_attr : mArchMemAttributes) {
      out_str << EMemoryAttributeType_to_string(arch_mem_attr) << ",";
    }
    out_str << "]";

    out_str << ", TargetAliasAttrs=[";
    for (const string& alias_impl_mem_attr : mAliasImplMemAttributes) {
      out_str << alias_impl_mem_attr << ",";
    }
    out_str << "]";

    if (nullptr !=  mpMemoryRangesConstraint) {
      out_str << ", MemoryRangesConstraint=" << mpMemoryRangesConstraint->ToSimpleString();
    }

    for (auto parm_pair : mVmContextParams) {
      out_str <<", " << EVmContextParamType_to_string(parm_pair.first) << "=0x" << hex << parm_pair.second;
    }

    for (EVmInfoBoolTypeBaseType i = 0; i < EVmInfoBoolTypeSize; ++ i) {
      EVmInfoBoolType vm_bool = EVmInfoBoolType(1 << i);
      if (mVmInfoBoolTypeMask & uint64(vm_bool)) {
        bool vm_bool_value = (mVmInfoBoolTypes & uint64(vm_bool)) ? true : false;
        out_str << ", " << EVmInfoBoolType_to_string(vm_bool) << "=" << vm_bool_value;
      }
    }

    return out_str.str();
  }

  void GenVirtualMemoryRequest::SetVmInfoBoolType(EVmInfoBoolType reqType, bool isSet)
  {
    uint64 set_bit = (uint64) reqType;
    mVmInfoBoolTypeMask |= set_bit;
    if (isSet)
        mVmInfoBoolTypes |= set_bit;
    else mVmInfoBoolTypes &= ~set_bit;
  }

  void GenVirtualMemoryRequest::SetImplementationMemoryAttributes(const vector<string>& rImplMemAttributes)
  {
    if (not mImplMemAttributes.empty())
    {
      LOG(fail) << "{GenVirtualMemoryRequest::SetImplementationMemoryAttributes} ImplMemAttributes is not empty" << endl;
      FAIL("impl-constraint-already-set");
    }

    if (rImplMemAttributes.empty())
    {
      LOG(warn) << "{GenVirtualMemoryRequest::SetImplementationMemoryAttributes} specified list of memory attributes is empty" << endl;
    }
    else
    {
      mImplMemAttributes = rImplMemAttributes;
    }
  }

  void GenVirtualMemoryRequest::SetArchitectureMemoryAttributes(const vector<EMemoryAttributeType>& rArchMemAttributes)
  {
    if (not mArchMemAttributes.empty())
    {
      LOG(fail) << "{GenVirtualMemoryRequest::SetArchitectureMemoryAttributes} ArchMemAttributes is not empty" << endl;
      FAIL("arch-constraint-already-set");
    }

    if (rArchMemAttributes.empty())
    {
      LOG(warn) << "{GenVirtualMemoryRequest::SetArchitectureMemoryAttributes} specified list of memory attributes is empty" << endl;
    }
    else
    {
      mArchMemAttributes = rArchMemAttributes;
    }
  }

  void GenVirtualMemoryRequest::SetAliasImplementationMemoryAttributes(const vector<string>& rAliasImplMemAttributes)
  {
    if (not mAliasImplMemAttributes.empty())
    {
      LOG(fail) << "{GenVirtualMemoryRequest::SetAliasImplementationMemoryAttributes} AliasImplMemAttributes is not empty" << endl;
      FAIL("target-alias-attrs-constraint-already-set");
    }

    if (rAliasImplMemAttributes.empty())
    {
      LOG(warn) << "{GenVirtualMemoryRequest::SetAliasImplementationMemoryAttributes} specified list of memory attributes is empty" << endl;
    }
    else
    {
      mAliasImplMemAttributes = rAliasImplMemAttributes;
    }
  }

  void GenVirtualMemoryRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    LOG(info) << "{GenVirtualMemoryRequest::AddDetail} " << attrName << " : 0x" << hex << value << " : " << RequestType() << endl;

    if (attrName == "Size" || attrName == "size"){
      mSize = value;
    }
    else if (attrName == "Align" || attrName == "align") {
      mAlign = value;
    }
    else if (attrName == "Tag") {
      mTag = value;
    }
    else if (attrName == "PrivilegeLevel") {
      SetPrivilegeLevel(EPrivilegeLevelType(value));
    }
    else if (attrName == "isInstr"){
      mDataType = (value) ? EMemDataType::Instruction : EMemDataType::Data;
    }
    else if (attrName == "Bank")
    {
      SetBankType(EMemBankType(value));
    }
    else if (attrName == "ForceAlias")
    {
      mForceAlias = bool(value);
    }
    else if (attrName == "PhysPageId")
    {
      mPhysPageId = value;
    }
    else if (attrName == "PA")
    {
      mPhysAddr = value;
    }
    else if (attrName == "FlatMap")
    {
      mFlatMap = bool(value);
    }
    else if (attrName == "ForceMemAttrs")
    {
      mForceMemAttrs = bool(value);
    }
    else if (attrName == "CanAlias")
    {
      mCanAlias = bool(value);
    }
    else if (attrName == "ForceNewAddr")
    {
      mForceNewAddr = bool(value);
    }
    else if (attrName == "Shared")
    {
      mSharedMemory = bool(value);
    }
    else if (attrName == "MemAttrArch") {
      mArchMemAttributes.push_back(EMemoryAttributeType(value));
    }

    bool convert_okay = false;
    EVmInfoBoolType bool_attr = try_string_to_EVmInfoBoolType(attrName, convert_okay);
    if (convert_okay) {
      SetVmInfoBoolType(bool_attr,bool(value));
    }
    else {
      EVmContextParamType context_attr = try_string_to_EVmContextParamType(attrName, convert_okay);
      if (convert_okay) {
        mVmContextParams[context_attr] = value;
        LOG(info) << "{GenVirtualMemoryRequest::AddDetail} " << attrName << " : " << EVmContextParamType_to_string(context_attr) << " = " << value << endl;
      }
    }
  }

  void GenVirtualMemoryRequest::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    LOG(info) << "{GenVirtualMemoryRequest::AddDetail} " << attrName << " : " << valueStr << " : " << RequestType() << endl;

    if (attrName == "Size") {
      mSize = parse_uint64(valueStr);
    }
    else if (attrName == "Align") {
      mAlign = parse_uint64(valueStr);
    }
    else if (attrName == "Type") {
      mDataType = parse_data_type(valueStr);
    }
    else if (attrName == "Bank")
    {
      SetBankType(string_to_EMemBankType(valueStr));
    }
    else if (attrName == "Tag") {
      mTag = parse_uint64(valueStr);
    }
    else if (attrName == "Range") {
      mpMemoryRangesConstraint = new ConstraintSet(valueStr);
    }
    else if (attrName == "PrivilegeLevel") {
      SetPrivilegeLevel(string_to_EPrivilegeLevelType(valueStr));
    }
    else if (attrName == "ForceAlias")
    {
      mForceAlias = bool(parse_uint64(valueStr));
    }
    else if (attrName == "PhysPageId")
    {
      mPhysPageId = parse_uint64(valueStr);
    }
    else if (attrName == "PA")
    {
      mPhysAddr = parse_uint64(valueStr);
    }
    else if (attrName == "FlatMap")
    {
      mFlatMap = bool(parse_uint64(valueStr));
    }
    else if (attrName == "MemAttrImpl")
    {
      AddImplementationMemoryAttributes(valueStr, mImplMemAttributes);
    }
    else if (attrName == "MemAttrArch") {
      AddArchitectureMemoryAttributes(valueStr, mArchMemAttributes);
    }
    else if (attrName == "ForceMemAttrs")
    {
      mForceMemAttrs = bool(parse_uint64(valueStr));
    }
    else if (attrName == "TargetAliasAttrs")
    {
      AddImplementationMemoryAttributes(valueStr, mAliasImplMemAttributes);
    }
    else if (attrName == "CanAlias")
    {
      mCanAlias = bool(parse_uint64(valueStr));
    }
    else if (attrName == "ForceNewAddr")
    {
      mForceNewAddr = bool(parse_uint64(valueStr));
    }
    else if (attrName == "Shared")
    {
      mSharedMemory = bool(parse_uint64(valueStr));
    }
    else {
      UnsupportedRequestDetailAttribute(attrName);
    }
  }

  void GenVirtualMemoryRequest::SetPrivilegeLevel(EPrivilegeLevelType priv)
  {
    mPrivilegeLevel = priv;
    mPrivilegeLevelSpecified = true;
  }

  void GenVirtualMemoryRequest::SetBankType(EMemBankType bank)
  {
    mBankType = bank;
    mBankSpecified = true;
  }

  bool GenVirtualMemoryRequest::VmSpecified() const
  {
    return (mPrivilegeLevelSpecified or (mVmContextParams.size() > 0) or (mVmInfoBoolTypeMask != 0));
  }

  void GenVirtualMemoryRequest::AddImplementationMemoryAttributes(const string& rValueStr, vector<string>& rMemAttributes)
  {
    StringSplitter ss(rValueStr, ',');
    while (!ss.EndOfString()) {
      string sub_str = ss.NextSubString();
      rMemAttributes.push_back(sub_str);
    }
  }

  void GenVirtualMemoryRequest::AddArchitectureMemoryAttributes(const string& rValueStr, vector<EMemoryAttributeType>& rMemAttributes)
  {
    StringSplitter ss(rValueStr, ',');
    bool is_integer = isdigit(rValueStr[0]);
    while (!ss.EndOfString()) {
      string sub_str = ss.NextSubString();

      if (is_integer) {
        uint64 attr_value = parse_uint64(sub_str);
        rMemAttributes.push_back(EMemoryAttributeType(attr_value));
      }
      else {
        rMemAttributes.push_back(string_to_EMemoryAttributeType(sub_str));
      }
    }
  }

  GenVaRequest::GenVaRequest()
    : GenVirtualMemoryRequest(EVmRequestType::GenVA), mVA(0)
  {

  }

  GenVaRequest::~GenVaRequest()
  {

  }

  GenVaRequest::GenVaRequest(const GenVaRequest& rOther)
    : GenVirtualMemoryRequest(rOther), mVA(rOther.mVA)
  {
  }

  GenVmVaRequest::GenVmVaRequest()
    : GenVirtualMemoryRequest(EVmRequestType::GenVMVA), mVA(0)
  {

  }

  GenVmVaRequest::~GenVmVaRequest()
  {

  }

  GenVmVaRequest::GenVmVaRequest(const GenVmVaRequest& rOther)
    : GenVirtualMemoryRequest(rOther), mVA(rOther.mVA)
  {
  }

  GenPaRequest::GenPaRequest()
    : GenVirtualMemoryRequest(EVmRequestType::GenPA), mPA(0)
  {

  }

  GenPaRequest::~GenPaRequest()
  {

  }

  GenPaRequest::GenPaRequest(const GenPaRequest& rOther)
    : GenVirtualMemoryRequest(rOther), mPA(rOther.mPA)
  {
  }

  GenVaForPaRequest::GenVaForPaRequest()
    : GenVirtualMemoryRequest(EVmRequestType::GenVAforPA), mVA(0)
  {

  }

  GenVaForPaRequest::~GenVaForPaRequest()
  {

  }

  GenVaForPaRequest::GenVaForPaRequest(const GenVaForPaRequest& rOther)
    : GenVirtualMemoryRequest(rOther), mVA(rOther.mVA)
  {

  }

  GenUpdateVmRequest::GenUpdateVmRequest()
    : GenVirtualMemoryRequest(EVmRequestType::UpdateVm), mInputArgs()
  {

  }

  GenUpdateVmRequest::~GenUpdateVmRequest()
  {

  }

  GenUpdateVmRequest::GenUpdateVmRequest(const GenUpdateVmRequest& rOther)
    : GenVirtualMemoryRequest(rOther), mInputArgs()
  {

  }

  GenVmContextRequest::GenVmContextRequest()
    : GenVirtualMemoryRequest(EVmRequestType::GenVmContext), mId(0), mInputArgs()
  {

  }

  GenVmContextRequest::~GenVmContextRequest()
  {

  }

  GenVmContextRequest::GenVmContextRequest(const GenVmContextRequest& rOther)
    : GenVirtualMemoryRequest(rOther), mId(rOther.mId), mInputArgs()
  {

  }

  void GenVmContextRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    mInputArgs[attrName] = value;
  }

  GenPhysicalRegionRequest::GenPhysicalRegionRequest()
    : GenVirtualMemoryRequest(EVmRequestType::PhysicalRegion), mRegionType(EPhysicalRegionType(0)), mMemoryBank(EMemBankType(0)), mBaseAddress(0), mFastMode(false)
  {

  }

  GenPhysicalRegionRequest::GenPhysicalRegionRequest(const GenPhysicalRegionRequest& rOther)
    : GenVirtualMemoryRequest(rOther), mRegionType(EPhysicalRegionType(0)), mMemoryBank(EMemBankType(0)), mBaseAddress(0), mFastMode(false)
  {

  }

  GenPhysicalRegionRequest::~GenPhysicalRegionRequest()
  {

  }

  void GenPhysicalRegionRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "Bank") {
      mMemoryBank = EMemBankType(value);
    }
    else if (attrName == "FastMode") {
      mFastMode = value;
    }
    else {
      GenVirtualMemoryRequest::AddDetail(attrName, value);
    }
  }

  void GenPhysicalRegionRequest::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    if (attrName == "RegionType") {
      mRegionType = string_to_EPhysicalRegionType(valueStr);
    }
    else {
      GenVirtualMemoryRequest::AddDetail(attrName, valueStr);
    }
  }

  GenPageRequest::GenPageRequest()
    : GenVirtualMemoryRequest(EVmRequestType::GenPage), mVA(0), mIPA(0), mPA(0), mPageId(0), mAttributeMask(0), mGenBoolAttributes(0), mGenBoolAttrMask(0), mMemAccessType(EMemAccessType(0)),
      mPageSizes(), mPteAttributes(), mGenAttributes(), mExceptionConstraints()
  {

  }

  GenPageRequest::GenPageRequest(const GenPageRequest& rOther)
    : GenVirtualMemoryRequest(rOther), mVA(rOther.mVA), mIPA(rOther.mIPA), mPA(rOther.mPA), mPageId(rOther.mPageId), mAttributeMask(rOther.mAttributeMask), mGenBoolAttributes(rOther.mGenBoolAttributes), mGenBoolAttrMask(rOther.mGenBoolAttrMask),
      mMemAccessType(rOther.mMemAccessType), mPageSizes(), mPteAttributes(), mGenAttributes(), mExceptionConstraints(rOther.mExceptionConstraints)
  {
    transform(rOther.mPageSizes.begin(), rOther.mPageSizes.end(), back_inserter(mPageSizes), [](PageSizeInfo* ps_info) { return new PageSizeInfo(*ps_info); });

    for (auto pte_attr_iter : rOther.mPteAttributes) {
      mPteAttributes[pte_attr_iter.first] = pte_attr_iter.second->Clone();
    }

    for (auto gen_attr_iter : rOther.mGenAttributes) {
      mGenAttributes[gen_attr_iter.first] = gen_attr_iter.second->Clone();
    }
  }

  GenPageRequest::~GenPageRequest()
  {
    for (auto ps_info: mPageSizes) {
      delete ps_info;
    }

    for (auto pte_attr_iter : mPteAttributes) {
      delete (pte_attr_iter.second);
    }

    for (auto gen_attr_iter : mGenAttributes) {
      delete (gen_attr_iter.second);
    }
  }

  void GenPageRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "VA") {
      mVA = value;
      SetAttributeMask(EPageRequestAttributeType::VA);
      return;
    }
    else if (attrName == "IPA") {
      mIPA = value;
      SetAttributeMask(EPageRequestAttributeType::IPA);
      return;
    }
    else if (attrName == "PA") {
      mPA = value;
      SetAttributeMask(EPageRequestAttributeType::PA);
      return;
    }
    else if (attrName == "PageSize") {
      PageSizeInfo* ps_info = new PageSizeInfo();
      PageSizeInfo::ValueToPageSizeInfo(value, *ps_info);
      mPageSizes.push_back(ps_info);
      return;
    }

    bool convert_okay = false;
    EPageGenBoolAttrType bool_attr = try_string_to_EPageGenBoolAttrType(attrName, convert_okay);
    if (convert_okay) {
      SetGenBoolAttribute(bool_attr, bool(value));
      return;
    }

    EPteAttributeType pte_attr = try_string_to_EPteAttributeType(attrName, convert_okay);
    if (convert_okay) {
      SetPteAttributeConstraint(pte_attr, new ConstraintSet(value));
      return;
    }

    EPageGenAttributeType gen_attr = try_string_to_EPageGenAttributeType(attrName, convert_okay);
    if (convert_okay) {
      SetGenAttributeConstraint(gen_attr, new ConstraintSet(value));
      return;
    }

    EPagingExceptionType except_type = try_string_to_EPagingExceptionType(attrName, convert_okay);
    if (convert_okay) {
      if (value >= EExceptionConstraintTypeSize) {
        LOG(fail) << "{GenPageRequest::AddDetail} value=0x" << hex << value << " out of range for \"EExceptionConstraintType\"." << endl;
        FAIL("enum-out-of-range");
      }
      SetExceptionConstraint(except_type, EExceptionConstraintType(value));
    } else {
      GenVirtualMemoryRequest::AddDetail(attrName, value);
    }
  }

  void GenPageRequest::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    if (attrName == "VA") {
      mVA = parse_uint64(valueStr);
      SetAttributeMask(EPageRequestAttributeType::VA);
      return;
    }
    else if (attrName == "IPA") {
      mIPA = parse_uint64(valueStr);
      SetAttributeMask(EPageRequestAttributeType::IPA);
      return;
    }
    else if (attrName == "PA") {
      mPA = parse_uint64(valueStr);
      SetAttributeMask(EPageRequestAttributeType::PA);
      return;
    }
    else if (attrName == "PageSize") {
      StringSplitter ss(valueStr, ',');
      while (!ss.EndOfString()) {
        string sub_str = ss.NextSubString();
        PageSizeInfo* ps_info = new PageSizeInfo();
        PageSizeInfo::StringToPageSizeInfo(sub_str, *ps_info);
        mPageSizes.push_back(ps_info);
      }
      return;
    }

    bool convert_okay = false;
    EPageGenBoolAttrType bool_attr = try_string_to_EPageGenBoolAttrType(attrName, convert_okay);
    if (convert_okay) {
      uint64 value = parse_uint64(valueStr);
      SetGenBoolAttribute(bool_attr, bool(value));
      return;
    }

    EPteAttributeType pte_attr = try_string_to_EPteAttributeType(attrName, convert_okay);
    if (convert_okay) {
      SetPteAttributeConstraint(pte_attr, new ConstraintSet(valueStr));
      return;
    }

    EPageGenAttributeType gen_attr = try_string_to_EPageGenAttributeType(attrName, convert_okay);
    if (convert_okay) {
      SetGenAttributeConstraint(gen_attr, new ConstraintSet(valueStr));
      return;
    }

    EPagingExceptionType except_type = try_string_to_EPagingExceptionType(attrName, convert_okay);
    if (convert_okay) {
      auto enum_value = string_to_EExceptionConstraintType(valueStr);
      SetExceptionConstraint(except_type, EExceptionConstraintType(enum_value));
    } else {
      GenVirtualMemoryRequest::AddDetail(attrName, valueStr);
    }
  }

  void GenPageRequest::SetAttributeMask(EPageRequestAttributeType reqType)
  {
    uint64 set_bit = 1ull << (uint32)(reqType);
    mAttributeMask |= set_bit;
  }

  bool GenPageRequest::GetAttributeValue(EPageRequestAttributeType attrType, uint64& value) const
  {
    switch (attrType) {
    case EPageRequestAttributeType::VA:
      value = mVA;
      break;
    case EPageRequestAttributeType::IPA:
      value = mIPA;
      break;
    case EPageRequestAttributeType::PA:
      value = mPA;
      break;
    case EPageRequestAttributeType::AliasPageId:
      value = mPageId;
      break;
    case EPageRequestAttributeType::PageSize:
    case EPageRequestAttributeType::MemAttrArch:
    case EPageRequestAttributeType::MemAttrImpl:
      // these are not simple attribute values, therefore return false
      return false;
      break;
    default:
      LOG(fail) << "{GenPageRequest::GetAttributeValue} unexpected attribute case: " << EPageRequestAttributeType_to_string(attrType) << endl;
      FAIL("unexpected-attribute-case");
    }

    return (mAttributeMask >> (uint32)(attrType)) & 1;
  }

  void GenPageRequest::SetAttributeValue(EPageRequestAttributeType attrType, uint64 value)
  {
    switch (attrType) {
    case EPageRequestAttributeType::VA:
      mVA = value;
      break;
    case EPageRequestAttributeType::IPA:
      mIPA = value;
      break;
    case EPageRequestAttributeType::PA:
      mPA = value;
      break;
    case EPageRequestAttributeType::AliasPageId:
      mPageId = value;
      break;
    default:
      LOG(fail) << "{GenPageRequest::SetAttributeValue} unexpected attribute case: " << EPageRequestAttributeType_to_string(attrType) << endl;
      FAIL("unexpected-attribute-case");
    }

    SetAttributeMask(attrType);
  }

  void GenPageRequest::SetGenBoolAttribute(EPageGenBoolAttrType attrType, bool isSet)
  {
    uint64 set_bit = 1ull << (uint32)(attrType);
    if (isSet) {
      mGenBoolAttributes |= set_bit;
    } else {
      mGenBoolAttributes &= ~set_bit;
    }
    mGenBoolAttrMask |= set_bit;
  }

  bool GenPageRequest::GetGenBoolAttribute(EPageGenBoolAttrType attrType, bool& value) const
  {
    value = (mGenBoolAttributes >> (uint32)(attrType)) & 1;
    return (mGenBoolAttrMask >> (uint32)(attrType)) & 1;
  }

  bool GenPageRequest::GenBoolAttribute(EPageGenBoolAttrType attrType) const
  {
    bool value = (mGenBoolAttributes >> (uint32)(attrType)) & 1;
    if (0 == ((mGenBoolAttrMask >> (uint32)(attrType)) & 1)) {
      LOG(fail) << "{GenPageRequest::GetGenBoolAttribute} attribute not specified: " << EPageGenBoolAttrType_to_string(attrType) << endl;
      FAIL("gen-bool-attribute-not-specified");
    }

    return value;
  }

  // limit reference to the InstrAddr attribute by wrapping it in this method, since we might want to merge InstrAddr attribute and mDataType, also it is used quite a lot.
  bool GenPageRequest::InstructionRequest() const
  {
    bool value = (mGenBoolAttributes >> (uint32)(EPageGenBoolAttrType::InstrAddr)) & 1;
    if (0 == ((mGenBoolAttrMask >> (uint32)(EPageGenBoolAttrType::InstrAddr)) & 1)) {
      LOG(fail) << "{GenPageRequest::InstructionRequest} attribute not specified: " << EPageGenBoolAttrType_to_string(EPageGenBoolAttrType::InstrAddr) << endl;
      FAIL("instruction-request-attribute-not-specified");
    }

    return value;
  }

  void GenPageRequest::SetPteAttributeConstraint(EPteAttributeType attrType, ConstraintSet* constrSet)
  {
    auto attr_iter = mPteAttributes.find(attrType);
    if (attr_iter != mPteAttributes.end()) {
      // if there is existing constraint set, need to delete it first.
      delete (mPteAttributes[attrType]);
      mPteAttributes[attrType] = nullptr;
    }

    mPteAttributes[attrType] = constrSet;
  }

  void GenPageRequest::SetPteAttribute(EPteAttributeType attrType, uint64 value)
  {
    auto new_constr = new ConstraintSet(value);
    SetPteAttributeConstraint(attrType, new_constr);
  }


  const ConstraintSet* GenPageRequest::PteAttributeConstraint(EPteAttributeType attrType) const
  {
    auto attr_iter = mPteAttributes.find(attrType);
    if (attr_iter != mPteAttributes.end()) {
      return attr_iter->second;
    }

    return nullptr;
  }

  void GenPageRequest::SetGenAttributeConstraint(EPageGenAttributeType attrType, ConstraintSet* constrSet)
  {
    auto attr_iter = mGenAttributes.find(attrType);
    if (attr_iter != mGenAttributes.end()) {
      // if there is existing constraint set, need to delete it first.
      delete (mGenAttributes[attrType]);
      mGenAttributes[attrType] = nullptr;
    }

    mGenAttributes[attrType] = constrSet;
  }

  void GenPageRequest::SetGenAttributeValue(EPageGenAttributeType attrType, uint64 value)
  {
    auto new_constr = new ConstraintSet(value);
    SetGenAttributeConstraint(attrType, new_constr);
  }

  const ConstraintSet* GenPageRequest::GenAttributeConstraint(EPageGenAttributeType attrType) const
  {
    auto attr_iter = mGenAttributes.find(attrType);
    if (attr_iter != mGenAttributes.end()) {
      return attr_iter->second;
    }

    return nullptr;
  }

  void GenPageRequest::SetExceptionConstraint(EPagingExceptionType exceptType, EExceptionConstraintType constrType)
  {
    mExceptionConstraints[exceptType] = constrType;
  }

  EExceptionConstraintType GenPageRequest::GetExceptionConstraint(EPagingExceptionType exceptType) const
  {
    auto find_iter = mExceptionConstraints.find(exceptType);
    if (find_iter != mExceptionConstraints.end()) {
      return find_iter->second;
    }

    return EExceptionConstraintType::Allow; // this should be default.
  }

  const string GenPageRequest::ToString() const
  {
    stringstream out_str;

    out_str << GenVirtualMemoryRequest::ToString();

    out_str << hex;

    for (EPageRequestAttributeTypeBaseType i = 0; i < EPageRequestAttributeTypeSize; ++ i) {
      EPageRequestAttributeType req_attr_type = EPageRequestAttributeType(i);
      uint64 attr_value = 0;
      if (GetAttributeValue(req_attr_type, attr_value)) {
        out_str << ", " << EPageRequestAttributeType_to_string(req_attr_type) << "=0x" << attr_value;
      }
    }

    for (EPageGenBoolAttrTypeBaseType i = 0; i < EPageGenBoolAttrTypeSize; ++ i) {
      EPageGenBoolAttrType gen_bool = EPageGenBoolAttrType(i);
      if ( ( mGenBoolAttrMask >> uint64(gen_bool) ) & 1 ) {
        bool gen_bool_value = ( (mGenBoolAttributes >> uint64(gen_bool)) & 1) ? true : false;
        out_str << ", " << EPageGenBoolAttrType_to_string(gen_bool) << "=" << gen_bool_value;
      }
    }

    out_str << ", MemAccessType=" << EMemAccessType_to_string(mMemAccessType);

    for (auto parm_pair : mVmContextParams) {
      out_str <<", " << EVmContextParamType_to_string(parm_pair.first) << "=0x" << hex << parm_pair.second;
    }

    if (mPageSizes.size()) {
      out_str << ", Page sizes";
    }

    for (auto pte_attr_pair : mPteAttributes) {
      out_str << ", " << EPteAttributeType_to_string(pte_attr_pair.first) << "=" << pte_attr_pair.second->ToSimpleString();
    }

    for (auto gen_attr_pair : mGenAttributes) {
      out_str << ", " << EPageGenAttributeType_to_string(gen_attr_pair.first) << "=" << gen_attr_pair.second->ToSimpleString();
    }

    for (auto excep_constr_pair : mExceptionConstraints) {
      out_str << ", " << EPagingExceptionType_to_string(excep_constr_pair.first) << "=" << EExceptionConstraintType_to_string(excep_constr_pair.second);
    }

    return out_str.str();
  }

  GenStateRequest* GenStateRequest::GenStateRequestInstance(const std::string& rStateType)
  {
    EGenStateType state_type = string_to_EGenStateType(rStateType);

    switch (state_type) {
    case EGenStateType::Loop:
    case EGenStateType::PostLoopAddress:
    case EGenStateType::LoopReconvergeAddress:
      return new GenLoopRequest(state_type);
      break;
    case EGenStateType::LinearBlock:
      return new GenLinearBlockRequest();
      break;
    case EGenStateType::BntHook:
      return new GenBntHookRequest();
    default:
      return new GenStateRequest(state_type);
    }

    return nullptr;
  }

  GenStateRequest::GenStateRequest(EGenStateType stateType)
    : GenRequestWithResult(), mActionType(EGenStateActionType::Push), mStateType(stateType), mIsValue(false), mValue(0), mString()
  {
  }

  GenStateRequest::GenStateRequest(EGenStateActionType actType, EGenStateType stateType, uint64 value)
    : GenRequestWithResult(), mActionType(actType), mStateType(stateType), mIsValue(true), mValue(value), mString()
  {

  }

  GenStateRequest::GenStateRequest(const GenStateRequest& rOther)
    : GenRequestWithResult(rOther), mActionType(EGenStateActionType::Push), mStateType(EGenStateType::GenMode), mIsValue(false), mValue(0), mString()
  {

  }

  void GenStateRequest::SetAction(const std::string& actionName)
  {
    mActionType = string_to_EGenStateActionType(actionName);
  }

  const string GenStateRequest::ToString() const
  {
    stringstream out_stream;

    out_stream << RequestType() << ": action=" << EGenStateActionType_to_string(mActionType) << " state=" << EGenStateType_to_string(mStateType) << " value=";
    if (mIsValue) {
      out_stream << "0x" << hex << mValue;
    }
    else {
      out_stream << mString;
    }
    return out_stream.str();
  }

  void GenStateRequest::SetPrimaryValue(uint64 value)
  {
    mIsValue = true;
    mValue = value;
    mString = "";
  }

  void GenStateRequest::SetPrimaryString(const string& valueStr)
  {
    mIsValue = false;
    mValue = 0;
    mString = valueStr;
  }

  GenStateTransitionRequest::GenStateTransitionRequest(const State* pTargetState, const EStateTransitionType stateTransType, const EStateTransitionOrderMode orderMode, const vector<EStateElementType>& rStateElemTypeOrder)
    : GenRequest(), mpTargetState(pTargetState), mStateTransType(stateTransType), mOrderMode(orderMode), mStateElemTypeOrder(rStateElemTypeOrder)
  {
  }

  GenStateTransitionRequest::GenStateTransitionRequest(const GenStateTransitionRequest& rOther)
    : GenRequest(rOther), mpTargetState(rOther.mpTargetState), mStateTransType(rOther.mStateTransType), mOrderMode(rOther.mOrderMode), mStateElemTypeOrder(rOther.mStateElemTypeOrder)
  {
  }

  GenOneTimeStateTransitionRequest::GenOneTimeStateTransitionRequest(const State* pTargetState, const EStateTransitionType stateTransType, const EStateTransitionOrderMode orderMode, const vector<EStateElementType>& rStateElemTypeOrder)
    : GenStateTransitionRequest(pTargetState, stateTransType, orderMode, rStateElemTypeOrder)
  {
  }

  GenOneTimeStateTransitionRequest::GenOneTimeStateTransitionRequest(const GenOneTimeStateTransitionRequest& rOther)
    : GenStateTransitionRequest(rOther)
  {
  }

  GenLoopRequest::GenLoopRequest(EGenStateType stateType)
    : GenStateRequest(stateType), mLoopRegIndex(0), mLoopId(0), mLoopBackAddress(0)
  {

  }

  GenLoopRequest::GenLoopRequest(const GenLoopRequest& rOther)
    : GenStateRequest(rOther), mLoopRegIndex(rOther.mLoopRegIndex), mLoopId(rOther.mLoopId), mLoopBackAddress(rOther.mLoopBackAddress)
  {

  }

  void GenLoopRequest::SetPrimaryString(const std::string& valueStr)
  {
    auto value = parse_uint64(valueStr);
    SetPrimaryValue(value);
  }

  void GenLoopRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "LoopRegIndex") {
      mLoopRegIndex = value;
    }
    else if (attrName == "LoopId") {
      mLoopId = value;
    }
    else {
      UnsupportedRequestDetailAttribute(attrName);
    }
  }

  GenLinearBlockRequest::GenLinearBlockRequest()
    : GenStateRequest(EGenStateType::LinearBlock), mBlockId(0), mExecute(false), mEmpty(false), mBlockEndAddr(0)
  {

  }

  GenLinearBlockRequest::GenLinearBlockRequest(const GenLinearBlockRequest& rOther)
    : GenStateRequest(rOther), mBlockId(rOther.mBlockId), mExecute(rOther.mExecute), mEmpty(rOther.mEmpty), mBlockEndAddr(rOther.mBlockEndAddr)
  {

  }

  void GenLinearBlockRequest::SetPrimaryString(const std::string& valueStr)
  {
    auto value = parse_uint64(valueStr);
    SetPrimaryValue(value);
  }

  void GenLinearBlockRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "BlockId") {
      mBlockId = value;
    }
    else if (attrName == "Execute") {
      mExecute = value;
    }
    else {
      UnsupportedRequestDetailAttribute(attrName);
    }
  }

  GenBntHookRequest::GenBntHookRequest() : GenStateRequest(EGenStateType::BntHook), mBntSequence(), mBntFunction(), mBntId(0)
  {

  }

  GenBntHookRequest::GenBntHookRequest(const GenBntHookRequest& rOther) : GenStateRequest(rOther), mBntSequence(rOther.mBntSequence),
                                                                             mBntFunction(rOther.mBntFunction), mBntId(rOther.mBntId)
  {

  }

  const std::string GenBntHookRequest::ToString() const
  {
    stringstream out_stream;
    out_stream << RequestType() << ": action=" << EGenStateActionType_to_string(mActionType);
    if (mBntSequence != "")
      out_stream << " sequence name: " << mBntSequence;
    if (mBntFunction != "")
      out_stream << " function name: " << mBntFunction;
    if (mBntId)
      out_stream << " Bnt ID:" << mBntId;

    return out_stream.str();
  }

  void GenBntHookRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "bntId")
      mBntId = value;
    else
      UnsupportedRequestDetailAttribute(attrName);
  }

  void GenBntHookRequest::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    if (attrName == "Seq")
      mBntSequence = valueStr;
    else if (attrName == "Func")
      mBntFunction = valueStr;
    else
      UnsupportedRequestDetailAttribute(attrName);
  }

  GenExceptionRequest* GenExceptionRequest::GenExceptionRequestInstance(const std::string& rReqName)
  {
    EExceptionRequestType req_type = string_to_EExceptionRequestType(rReqName);

    switch (req_type) {
    case EExceptionRequestType::HandleException:
      return new GenHandleException();
      break;
    case EExceptionRequestType::SystemCall:
      LOG(notice) << "{GenExceptionRequest::GenExceptionRequestInstance} creating new system call request..." << endl;
      return new GenSystemCall();
      break;
    case EExceptionRequestType::UpdateHandlerInfo:
      LOG(notice) << "{GenExceptionRequest::GenExceptionRequestInstance} creating new handler info update request..." << endl;
      return new GenUpdateHandlerInfo();
      break;
    default:
      LOG(fail) << "{GenExceptionRequest::GenExceptionRequestInstance} unsupported exception-request type: " << rReqName << endl;
      FAIL("unsupported-exception-request-type");
    }

    return nullptr;
  }

  GenExceptionRequest::GenExceptionRequest(EExceptionRequestType reqType)
    : GenRequestWithResult(), mExceptionRequestType(reqType)
  {

  }

  GenExceptionRequest::GenExceptionRequest(const std::string& rSeqTypeStr)
    : GenRequestWithResult(),  mExceptionRequestType(string_to_EExceptionRequestType(rSeqTypeStr))
  {
    mExceptionRequestType = string_to_EExceptionRequestType(rSeqTypeStr);
  }

  GenExceptionRequest::GenExceptionRequest(const GenExceptionRequest& rOther)
    : GenRequestWithResult(rOther), mExceptionRequestType(rOther.mExceptionRequestType)
  {

  }

  const string GenExceptionRequest::ToString() const
  {
    return string("GenExceptionRequest: ") + EExceptionRequestType_to_string(mExceptionRequestType);
  }

  GenHandleException::GenHandleException(uint32 exceptId, const std::string& rDesc)
    : GenExceptionRequest(EExceptionRequestType::HandleException), mId(exceptId), mDescription(rDesc)
  {

  }

  GenHandleException::GenHandleException(const GenHandleException& rOther)
    : GenExceptionRequest(rOther), mId(rOther.mId), mDescription(rOther.mDescription)
  {

  }

  GenHandleException::~GenHandleException()
  {

  }

  GenSystemCall::GenSystemCall(const GenSystemCall& rOther)
    : GenExceptionRequest(rOther), mSysCallParms(), mSysCallResult(0), mSysCallResults()
  {

  }

  GenSystemCall::~GenSystemCall()
  {

  }

  void GenSystemCall::AddDetail(const string& attrName, uint64 value)
  {
    mSysCallParms[attrName] = std::to_string(value);
  }

  void GenSystemCall::AddDetail(const string& attrName, const string& valueStr)
  {
    mSysCallParms[attrName] = valueStr;
  }

  GenUpdateHandlerInfo::GenUpdateHandlerInfo(const GenUpdateHandlerInfo& rOther)
    : GenExceptionRequest(rOther), mUpdateHandlerParams(),  mUpdateHandlerResult(0)
  {

  }

  GenUpdateHandlerInfo::~GenUpdateHandlerInfo()
  {

  }

  void GenUpdateHandlerInfo::AddDetail(const string& attrName, uint64 value)
  {
    mUpdateHandlerParams[attrName] = std::to_string(value);
  }

  void GenUpdateHandlerInfo::AddDetail(const string& attrName, const string& valueStr)
  {
    mUpdateHandlerParams[attrName] = valueStr;
  }

  GenCallBackRequest::~GenCallBackRequest()
  {

  }

  const std::string GenCallBackRequest::ToString() const
  {
    return "GenCallBackRequest";
  }

  GenCallBackBntRequest::~GenCallBackBntRequest()
  {
    if (not mpBntNode->IsSpeculative()) // BntNodeManger owns speculative bnt node.
      delete mpBntNode;

    mpBntNode = nullptr;
  }

  GenLoadLargeRegister::GenLoadLargeRegister(const std::string& rRegName, const std::vector<uint64>& rValues, const std::string& rInterRegName, uint32 immOffset)
    : GenSequenceRequest(ESequenceType::LoadLargeRegister), mRegisterName(rRegName), mRegisterValues(rValues), mInterRegName(rInterRegName), mImmOffset(immOffset)
  {
  }

  GenLoadLargeRegister::GenLoadLargeRegister(const GenLoadLargeRegister& rOther)
    : GenSequenceRequest(rOther), mRegisterName(rOther.mRegisterName), mRegisterValues(rOther.mRegisterValues), mInterRegName(rOther.mInterRegName), mImmOffset(rOther.mImmOffset)
  {
  }

  GenFreePageRequest::GenFreePageRequest() : GenVirtualMemoryRequest(EVmRequestType::GenFreePage), mPageNum(0), mRequestPageSizes(), mpRequestRanges(nullptr), mValid(false), mStartAddr(0), mpResolvedRanges(nullptr), mResolvedPageSizes()
  {
    mpRequestRanges = new ConstraintSet();
    mpResolvedRanges = new ConstraintSet();
  }

  GenFreePageRequest::~GenFreePageRequest()
  {
    delete mpRequestRanges;
    delete mpResolvedRanges;
  }

  void GenFreePageRequest::AddDetail(const std::string& attrName, uint64 value)
  {
    if (attrName == "Number") {
      mPageNum = value;
    }
    else {
      UnsupportedRequestDetailAttribute(attrName);
    }
  }

  void GenFreePageRequest::AddDetail(const std::string& attrName, const std::string& valueStr)
  {
    if (attrName == "Range") {
      mpRequestRanges->MergeConstraintSet(ConstraintSet(valueStr));
    }
    else if (attrName == "PageSize"){
      SplitToPageSize(valueStr);
    }
    else
      UnsupportedRequestDetailAttribute(attrName);

  }

  void GenFreePageRequest::SplitToPageSize(const std::string& pageSizeStr)
  {
    StringSplitter splitter(pageSizeStr, ',', true);
    while (not splitter.EndOfString()) {
      auto page_size = splitter.NextSubString();
      mRequestPageSizes.push_back(kmg_number(page_size));

    }
  }

  void GenFreePageRequest::RegulateRequest()
  {
    if (mPageNum < 2) {
      LOG(fail) << "{GenFreePageRequest::RegulateRequest} Page Number should be at least 2, current value is " << mPageNum << endl;
      FAIL("Invalid-page-number");
    }
    mRequestPageSizes.resize(mPageNum, 0);
    mResolvedPageSizes.resize(mPageNum, 0);
  }

}
