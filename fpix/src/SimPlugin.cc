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
#include "SimPlugin.h"

#include "Log.h"
#include "SimEvent.h"

namespace Force {

//<! handle sim event notification...

void SimPlugin::HandleNotification(const Sender<ESimThreadEventType>* sender, ESimThreadEventType eventType, Object* pPayload) {
  SimThreadEvent *ev = (SimThreadEvent *) pPayload;

  mCpuID  = ev->CpuId();    //!< refresh current cpu ID,
  mSimPtr = ev->SimPtr();   //!<    sim-ptr

  mReturnCode = 0;          //!< plugin methods optionally can flag errors

  LOG(debug) << "SimPlugin '" << Name() << "': Handling sim-event notification, type: " << ESimThreadEventType_to_string(eventType) << "..." << endl;

  switch(eventType) {
    case ESimThreadEventType::START_TEST:
      atTestStart();
      break;
    case ESimThreadEventType::END_TEST:
      atTestEnd();
      break;
    case ESimThreadEventType::RESET:
      onReset();
      break;
    case ESimThreadEventType::BOOT_CODE:
      onBootCode();
      break;
    case ESimThreadEventType::FIRST_INSTRUCTION:
      onMain();
      break;
    case ESimThreadEventType::PRE_STEP:
      onPreStep();
      break;
    case ESimThreadEventType::POST_STEP:
      onStep( ev->RegisterUpdates(),ev->MemoryUpdates(),ev->MmuEvents(),ev->ExceptionUpdates() );
      break;
    case ESimThreadEventType::EXCEPTION_EVENT:
      onException( ev->ExceptUpdate() );
      break;
    case ESimThreadEventType::REGISTER_UPDATE:
      onRegisterUpdate( ev->RegisterUpdate() );
      break;
    case ESimThreadEventType::MEMORY_UPDATE:
      onMemoryUpdate( ev->MemoryUpdate() );
      break;
    default:
      break;
  }

  ev->SetReturnCode(mReturnCode);
}

}
