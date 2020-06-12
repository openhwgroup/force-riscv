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
#include <PageRequestRegulator.h>
#include <Generator.h>
#include <VmManager.h>
#include <VmMapper.h>
#include <GenRequest.h>
#include <Variable.h>
#include <Log.h>

#include <sstream>

using namespace std;

/*!
  \file PageRequestRegulator
  \brief Code for GenPageRequest regulating base class code.
*/

namespace Force {

  PageRequestRegulator::PageRequestRegulator()
    : Object(), mpGenerator(nullptr), mpNoDataAbortVariable(nullptr), mpNoInstrAbortVariable(nullptr), mDataExceptionTypes(), mInstrExceptionTypes()
  {

  }

  PageRequestRegulator::PageRequestRegulator(const PageRequestRegulator& rOther)
    : Object(rOther), mpGenerator(nullptr), mpNoDataAbortVariable(nullptr), mpNoInstrAbortVariable(nullptr), mDataExceptionTypes(rOther.mDataExceptionTypes), mInstrExceptionTypes(rOther.mInstrExceptionTypes)
  {

  }

  PageRequestRegulator::~PageRequestRegulator()
  {

  }

  const string PageRequestRegulator::ToString() const
  {
    stringstream ret_str;

    ret_str << Type() << " data exceptions: ";

    bool is_first = true;
    for (auto except_type : mDataExceptionTypes) {
      if (is_first) {
        is_first = false;
      } else {
        ret_str << ", ";
      }

      ret_str << EPagingExceptionType_to_string(except_type);
    }

    ret_str << " instruction exceptions: ";

    is_first = true;
    for (auto except_type : mInstrExceptionTypes) {
      if (is_first) {
        is_first = false;
      } else {
        ret_str << ", ";
      }

      ret_str << EPagingExceptionType_to_string(except_type);
    }
    return ret_str.str();
  }

  void PageRequestRegulator::Setup(const Generator* pGen)
  {
    mpGenerator = pGen;

    const VariableModerator* var_mod = mpGenerator->GetVariableModerator(EVariableType::Value);
    mpNoDataAbortVariable = dynamic_cast<const ValueVariable*>(var_mod->GetVariableSet()->GetVariable("No data abort allowed"));
    mpNoInstrAbortVariable = dynamic_cast<const ValueVariable*>(var_mod->GetVariableSet()->GetVariable("No instruction abort allowed"));
    // << "no data abort? " << mpNoDataAbortVariable->Value() << " no instr abort? " << mpNoInstrAbortVariable->Value() << endl;
  }

  void PageRequestRegulator::RegulatePageRequest(const VmMapper* pVmMapper, GenPageRequest* pPageReq) const
  {
    if (pPageReq->InstructionRequest()) {
      RegulateBranchPageRequest(pVmMapper, nullptr, pPageReq);
    }
    else {
      RegulateLoadStorePageRequest(pVmMapper, nullptr, pPageReq);
    }
  }

  void PageRequestRegulator::RegulateLoadStorePageRequest(const VmMapper* pVmMapper, const LoadStoreOperandStructure* pLsStruct, GenPageRequest* pPageReq) const
  {
    if (mpNoDataAbortVariable->Value() || pPageReq->GenBoolAttributeDefaultFalse(EPageGenBoolAttrType::NoDataAbort)) {
      PreventDataAbort(pPageReq);
      LOG(info) << "{PageRequestRegulator::RegulateLoadStorePageRequest} preventing all data abort." << endl;
    }
    else {
      for (auto except_type : mDataExceptionTypes) {
        auto except_name = string("Data ") + GetExceptionString(except_type);
        EExceptionConstraintType except_constr_type = pVmMapper->GetExceptionConstraintType(except_name);
        pPageReq->SetExceptionConstraint(except_type, except_constr_type);
        LOG(info) << "{PageRequestRegulator::RegulateLoadStorePageRequest} exception type: " << EPagingExceptionType_to_string(except_type) << " exception name: " << except_name << " exception constraint type: "
                  << EExceptionConstraintType_to_string(except_constr_type) << endl;
      }
    }

    pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::Regulated, true);
  }

  void PageRequestRegulator::PreventDataAbort(GenPageRequest* pPageReq) const
  {
    for (auto except_type : mDataExceptionTypes) {
      pPageReq->SetExceptionConstraint(except_type, EExceptionConstraintType::PreventHard);
    }
  }

  void PageRequestRegulator::RegulateBranchPageRequest(const VmMapper* pVmMapper, const BranchOperandStructure* pBrStruct, GenPageRequest* pPageReq) const
  {
    if (mpNoInstrAbortVariable->Value() || pPageReq->GenBoolAttributeDefaultFalse(EPageGenBoolAttrType::NoInstrAbort)) {
      PreventInstrAbort(pPageReq);
      LOG(info) << "{PageRequestRegulator::RegulateBranchPageRequest} preventing all instruction abort." << endl;
    }
    else {
      for (auto except_type : mInstrExceptionTypes) {
        auto except_name = string("Instruction ") + GetExceptionString(except_type);
        EExceptionConstraintType except_constr_type = pVmMapper->GetExceptionConstraintType(except_name);
        pPageReq->SetExceptionConstraint(except_type, except_constr_type);
        LOG(info) << "{PageRequestRegulator::RegulateBranchPageRequest} exception type: " << EPagingExceptionType_to_string(except_type) << " exception name: " << except_name << " exception constraint type: "
                  << EExceptionConstraintType_to_string(except_constr_type) << endl;
      }
    }

    pPageReq->SetGenBoolAttribute(EPageGenBoolAttrType::Regulated, true);
  }

  void PageRequestRegulator::PreventInstrAbort(GenPageRequest* pPageReq) const
  {
    for (auto except_type : mInstrExceptionTypes) {
      pPageReq->SetExceptionConstraint(except_type, EExceptionConstraintType::PreventHard);
    }
  }

}
