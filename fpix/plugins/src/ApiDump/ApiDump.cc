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
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <ctype.h>
#include <assert.h>

#include "SimAPI.h"
#include "SimEvent.h"
#include "SimPlugin.h"
#include "Log.h"

using namespace std;
using namespace Force;

//***********************************************************************************************
// ApiDump - for all plugin methods, stream all parameter values to stdout
//***********************************************************************************************

namespace Force 
{
class ApiDump: public Force::SimPlugin 
{
    public:
        ApiDump() {}
  
        const string Name() { return "ApiDump"; }
  
        bool IsSupported(ESimThreadEventType eventType) const 
        { 
            bool is_supported = false;
  
            switch(eventType) 
            {
                case ESimThreadEventType::START_TEST:
                case ESimThreadEventType::RESET:
                case ESimThreadEventType::BOOT_CODE:
                case ESimThreadEventType::FIRST_INSTRUCTION:
                case ESimThreadEventType::PRE_STEP:
                case ESimThreadEventType::POST_STEP:
                case ESimThreadEventType::REGISTER_UPDATE:
                case ESimThreadEventType::MEMORY_UPDATE:
                case ESimThreadEventType::MMU_EVENT:
                case ESimThreadEventType::EXCEPTION_EVENT:
                case ESimThreadEventType::END_TEST:    
                is_supported = true;
                break;
  
                default: break;
            }
  
            return is_supported;
        };
  
        void parsePluginsClargs(vector<string> & plugins_cl_args) 
        {
            cout << MethodName("parsePluginsClargs") << endl;
        }
  
        void parsePluginsOptions(map<string, string> & arPluginsOptions){}
  
        void atTestStart() 
        { 
            cout << MethodName("atTestStart") << endl;
        }
  
        void onReset() 
        { 
            cout << MethodName("onReset") << endl;
        }
  
        void onBootCode() 
        { 
            cout << MethodName("onBootCode") << endl;
        }
  
        void onMain() 
        { 
            cout << MethodName("onMain") << endl;
        }
  
        void onPreStep() 
        { 
            cout << MethodName("onPreStep") << endl;
        }
  
        void onRegisterUpdate(RegUpdate *apRegUpdate) 
        { 
            cout << MethodName("RegisterUpdate","") << endl;
            dumpRegUpdate(apRegUpdate);
        }
  
        void onMemoryUpdate(MemUpdate *apMemUpdate) 
        { 
            cout << MethodName("onMemoryUpdate","") << "\n";
            dumpMemoryUpdate(apMemUpdate); 
        }
  
        void onMmuEvent(MmuEvent *apMMUEvent) 
        { 
            cout << MethodName("onMmuEvent","") << endl;
            dumpMmuEvent(apMMUEvent);
        }
  
        void onException(ExceptionUpdate *apExceptionUpdate) 
        { 
            cout << MethodName("Exception") << endl;
            dumpException(apExceptionUpdate);
        }
  
        void atTestEnd() 
        {
            cout << MethodName("atTestEnd","Final PC") << endl;
        }
  
        void onStep(vector<RegUpdate> *apRegUpdates, vector<MemUpdate> *apMemUpdates,
                    vector<MmuEvent> *apMMUEvents, vector<ExceptionUpdate> *apExceptionUpdates) 
        {
  
            cout << MethodName("PostStep") << endl;
  
            for (auto i = apRegUpdates->begin(); i != apRegUpdates->end(); i++) 
            {
                dumpRegUpdate(&(*i),"Register update");
            }
  
            for (auto i = apMemUpdates->begin(); i != apMemUpdates->end(); i++) 
            {
                dumpMemoryUpdate(&(*i),"Memory update");
            }
  
            for (auto i = apMMUEvents->begin(); i != apMMUEvents->end(); i++) 
            {
                dumpMmuEvent(&(*i),"MMU event");
            }
  
            for (auto i = apExceptionUpdates->begin(); i != apExceptionUpdates->end(); i++) 
            {
                dumpException(&(*i),"Exception update");
            }
        }
   
        const string MethodName(const string &arMName,string aDumpPC = "") 
        { 
            stringstream ss;
            ss << "[ApiDump::" << arMName << ", cpuid: " << CpuID();
            if ( !aDumpPC.empty() ) 
            {
                uint64 rval = 0;
                uint64 rmask = 0;
                SimPtr()->ReadRegister(CpuID(),"PC",&rval,&rmask);
                ss << "," << aDumpPC << ": 0x" << hex << rval << dec;
            }
            ss << "]"; 
            return ss.str();
        }
  
        void dumpRegUpdate(RegUpdate *mpRegUpdate, string aPrefix="") 
        {
            if ( !aPrefix.empty() ) 
            {
                cout << "    " << aPrefix << ":\n";
            }
            cout << "\t" << "name: " << mpRegUpdate->regname << "\n";
            cout << "\t" << "value: 0x" << hex << mpRegUpdate->rval << dec << "\n";
            cout << "\t" << "mask: 0x" << hex << mpRegUpdate->mask << dec << "\n";
            cout << "\t" << "access-type: " << mpRegUpdate->access_type << "\n";
        }
  
        void dumpMemoryUpdate(MemUpdate *mpMemUpdate, string aPrefix="") 
        { 
            if ( !aPrefix.empty() ) 
            {
                cout << "    " << aPrefix << ":\n";
            }
            cout << "\tmemory bank: " << mpMemUpdate->mem_bank << "\n";
            cout << "\t" << "VA: 0x" << hex << mpMemUpdate->virtual_address << dec << "\n";
            cout << "\t" << "PA: 0x" << hex << mpMemUpdate->physical_address << dec << "\n";
            cout << "\t" << "access-type: " << mpMemUpdate->access_type << "\n";
            cout << "\t" << "size: " << mpMemUpdate->size << "\n";
            cout << "\t" << "bytes:";
            for (auto i = mpMemUpdate->bytes.begin(); i != mpMemUpdate->bytes.end(); i++) 
            {
                char tbuf[128];
                sprintf(tbuf," 0x%02x",(*i));
                cout << tbuf;
            }
            cout << endl;
        }
  
        void dumpMmuEvent(MmuEvent *apMMUEvent, string aPrefix="") 
        { 
            if ( !aPrefix.empty() ) 
            {
                cout << "    " << aPrefix << ":\n";
            }
            cout << "\t" << "VA: 0x" << hex << apMMUEvent->va << dec << "\n";
            cout << "\t" << "PA: 0x" << hex << apMMUEvent->pa << dec << "\n";
            cout << endl;
        }
  
        void dumpException(ExceptionUpdate *apExceptionUpdate, string aPrefix="") 
        { 
            if ( !aPrefix.empty() ) 
            {
                cout << "    " << aPrefix << ":\n";
            }
            cout << "\t" << "ID: 0x" << hex << apExceptionUpdate->mExceptionID << dec << "\n";
            cout << "\t" << "attributes: 0x" << hex << apExceptionUpdate->mExceptionAttributes << dec << "\n";
            cout << "\t" << "comments: " << apExceptionUpdate->mComments << "\n";
        }
    };
}

// plugin + 'extern C' methods to be compiled into shared library:

extern "C" Force::SimPlugin* create()
{
  return new Force::ApiDump();
}

extern "C" void destroy(Force::SimPlugin* t1)
{
  delete t1;
}

extern "C" int is_shared()
{
  return 1;  //!< only need a single instance
}
