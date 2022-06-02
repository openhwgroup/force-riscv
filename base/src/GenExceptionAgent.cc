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
#include "GenExceptionAgent.h"

#include <memory>

#include "AddressTable.h"
#include "AddressTableManager.h"
#include "Enums.h"
#include "ExceptionContext.h"
#include "ExceptionRecords.h"
#include "GenPC.h"
#include "GenRequest.h"
#include "Generator.h"
#include "Log.h"
#include "Register.h"
#include "StringUtils.h"
#include "VmManager.h"
#include "VmMapper.h"
#include ARCH_ENUM_HEADER

using namespace std;

/*!
  \file GenExceptionAgent.cc
  \brief Code handling all exception related requests.
*/

namespace Force {

  Object* GenExceptionAgent::Clone() const
  {
    return new GenExceptionAgent(*this);
  }

  void GenExceptionAgent::SetGenRequest(GenRequest* genRequest)
  {
    mpExceptionRequest = dynamic_cast<GenExceptionRequest* >(genRequest);
  }

  void GenExceptionAgent::HandleRequest()
  {
    switch (mpExceptionRequest->ExceptionRequestType()) {
    case EExceptionRequestType::HandleException:
      HandleException();
      break;
    case EExceptionRequestType::SystemCall:
      SystemCall();
      break;
    case EExceptionRequestType::UpdateHandlerInfo:
      UpdateHandlerInfo();
      break;
    default:
      LOG(fail) << "{GenExceptionAgent::HandleRequest} unsupported exception request type: " << EExceptionRequestType_to_string(mpExceptionRequest->ExceptionRequestType()) << endl;
      FAIL("unsupported-exception-request-type");
    }
  }

  void GenExceptionAgent::CleanUpRequest()
  {
    delete mpExceptionRequest;
    mpExceptionRequest = nullptr;
  }

  void GenExceptionAgent::ResetRequest()
  {
    mpExceptionRequest = nullptr;
  }

  void GenExceptionAgent::HandleException()
  {
    auto except_req = mpExceptionRequest->CastInstance<GenHandleException>();
    LOG(notice) << "{GenExceptionAgent::HandleException} exception ID 0x" << hex << except_req->Id() << ", " << except_req->Description() << endl;

    mpGenerator->UpdateVm();
    uint32 prev_exception_level = mpGenerator->PrivilegeLevel();

    if (IsSimExit(except_req)) {
      EGenModeTypeBaseType gen_mode_change = EGenModeTypeBaseType(EGenModeType::SimOff) | EGenModeTypeBaseType(EGenModeType::NoEscape);
      auto state_req = new GenStateRequest(EGenStateActionType::Push, EGenStateType::GenMode, gen_mode_change); // modify GenMode to ReExe.
      unique_ptr<GenRequest> storage_ptr(state_req); // responsible for releasing the storage when going out of scope.
      mpGenerator->StateRequest(state_req);
    }
    else if (IsLowPower(except_req)) {
      mpGenerator->SleepOnLowPower();
    }
    else if (not IsExceptionReturn(except_req)) {
      if (mpGenerator->InException()) {
        string err_info;
        if (!AllowExceptionInException(except_req, err_info)) {
          LOG(fail) << "{GenExceptionAgent::HandleException} unsupported exception-in-exception case. " << err_info << endl;
          FAIL("not-supported-exception-in-exception");
        }
      }

      if (NeedRecoveryAddress()) {
        auto addr_table_man = mpGenerator->GetAddressTableManager();
        addr_table_man->GenerateRecoveryAddress();
      }

      try {
        /* Report this new error */
        ExceptionRecord exception_record;
        ExceptionRecordManager* ex_manager = mpGenerator->GetExceptionRecordManager();

        exception_record.exception_code = except_req->Id();
        exception_record.tgt_exception_level = mpGenerator->PrivilegeLevel();
        exception_record.src_exception_level = prev_exception_level;
        exception_record.pc_value = mpGenerator->GetGenPC()->Value();
        exception_record.dfsc_ifsc_code = mpGenerator->GetExceptionSubCode();

        ex_manager->ReportNewExceptionRecord(exception_record);

      } catch (exception& error) {
        /* This is not an error value we care about. Don't do anything here. */
        LOG(notice) << "Unrecorded Exception Value: " << error.what() << endl;
      }

      mpGenerator->ExecuteHandler();

      AddPostExceptionRequests();
    }
    else {
      mpGenerator->ExceptionReturn();
    }
  }

  void GenExceptionAgent::SystemCall()
  {
    LOG(notice) << "{GenExceptionAgent::SystemCall} derived class should implement this!!!" << endl;
  }

  void GenExceptionAgent::UpdateHandlerInfo()
  {
    auto update_req = mpExceptionRequest->CastInstance<GenUpdateHandlerInfo>();

    const std::map<std::string, std::string> myMap = update_req->UpdaterHandlerParams();
    auto iter = myMap.find("Function");
    if (iter == myMap.end()) {
      // shouldn't happen since front-end has to provide a function name first
      LOG(fail) << "{GenExceptionAgent::UpdateHandlerInfo} function not provided " << endl;
      FAIL("empty-updatehandlerinfo-request-type");
    }

    std::string function = iter->second;

    if (function == "RecordExceptionSpecificAddressBounds") {
      RecordExceptionSpecificAddressBounds(myMap);
    }

    else if (function == "UpdateExceptionBounds") {
      // does not work for now, maybe can remove.
    }

    else if (function == "UpdateVectorBaseAddress") {
      LOG(notice) << "{GenExceptionAgent::UpdateHandlerInfo} given VBARS..." << endl;
      auto exception_manager = ExceptionManager::Instance();

      for (const auto &item : myMap) {
        if ( item.second != "UpdateVectorBaseAddress") { // "Function":"UpdateVectorBaseAddress" is in the map, need to filter it out
          EExceptionVectorType vector_type = GetExceptionVectorType(item.first);
          uint64 address = parse_uint64(item.second);
          LOG(notice) << "Key: " << item.first << "  Value:0x" << hex << address << endl;
          exception_manager->SetExceptionVectorBaseAddress(vector_type, address);
        }
      }
    }

    else {
      UpdateArchHandlerInfo();
    }
  }

  void GenExceptionAgent::UpdateArchHandlerInfo()
  {
    LOG(notice) << "{GenExceptionAgent::UpdateHandlerInfo} derived class should implement this!!!" << endl;
  }

  void GenExceptionAgent::DiagnoseExceptionDefault(EExceptionClassType exceptClass, string& rMsgStr) const
  {
    ExceptionContext* my_context = ExceptionContextInstance(exceptClass);
    std::unique_ptr<ExceptionContext> my_context_storage(my_context); // to release my_context when done.
    my_context->UpdateContext();
    rMsgStr += my_context->ToString();
  }

  void GenExceptionAgent::ProcessHandlerBounds(cuint32 bank, const string& rHandlerBounds, ExceptionManager::ListOfAddressBoundaries& rBoundariesList)
  {
    StringSplitter handler_pairs = StringSplitter(rHandlerBounds, ';');
    while (!handler_pairs.EndOfString()) {
      std::string next_pair = handler_pairs.NextSubString();
      StringSplitter addresses = StringSplitter(next_pair, ':');

      std::string exception_class = addresses.NextSubString();
      std::string start_address = addresses.NextSubString();
      std::string end_address = addresses.NextSubString();

      std::pair<uint64, uint64> next_handler_bounds = std::make_pair(std::stoll(start_address), std::stoll(end_address));
      ExceptionManager::Instance()->RecordSpecificHandlerBoundaries(bank, (EExceptionClassType)std::stoll(exception_class), next_handler_bounds);
      rBoundariesList.push_back(next_handler_bounds);
    }
  }

  ExceptionContext* GenExceptionAgent::ExceptionContextInstance(EExceptionClassType exceptClass) const
  {
    return new ExceptionContext(mpGenerator, exceptClass);
  }

}
