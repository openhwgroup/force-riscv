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
#include "ParseGuide.h"

using namespace std;
using namespace Force;

//***********************************************************************************************
// example fpix plugin - simple-minded register dependencies 'checker'. For a given dependencies
//      depth, count the # of read-after-write, write-after-read, and write-after-write 
//      occurences.
//***********************************************************************************************

namespace Force 
{
    struct RegAccess 
    {
        RegAccess(const string &arRname, const string &arAcctype) : mRname(arRname),mAcctype(arAcctype) {};
        string mRname;
        string mAcctype;
    };


class RegDependsChecker : public Force::SimPlugin 
{
    public:
        RegDependsChecker() : 
          _mRegisterAccesses(),
          _mDepDepth(-1),
          _mDependencyCounts(),
          _mInRandomInstructions(false),
          _mRandomInstructionsStart(0x80000000),
          _mStringExample("")
        { Init(); };
      
        const string Name() { return "RegDependsChecker"; }
      
        void Init() 
        {
            _mDependencyCounts["READ AFTER WRITE"] = 0;
            _mDependencyCounts["WRITE AFTER READ"] = 0;
            _mDependencyCounts["WRITE AFTER WRITE"] = 0;
      
            _mInRandomInstructions = false; // set to true when we appear to be in random code
        };
      
        bool IsSupported(ESimThreadEventType eventType) const 
        { 
            bool is_supported = false;
      
            switch(eventType) 
            {
                case ESimThreadEventType::PRE_STEP:
                case ESimThreadEventType::POST_STEP:
                case ESimThreadEventType::END_TEST:    
                is_supported = true;
                break;
      
                default: break;
            }
      
            return is_supported;
        };
      
        // Turn arguments from the top level that went uninterpreted into possible plugin options.
        void parsePluginsClargs(vector<string> &plugins_cl_args) 
        {
        }
      
        //Use the ParseGuide helper class to lookup and assign option values, stress free
        void parsePluginsOptions(std::map<std::string, std::string> & arPluginsOptions)
        {
            std::vector<Force::ParseGuide> guides;
            guides.emplace_back(std::string("dep_depth"), &_mDepDepth); 
            guides.emplace_back(std::string("rand_instr_start"), &_mRandomInstructionsStart);
            guides.emplace_back(std::string("string_example"), &_mStringExample);
      
            for(auto & guide : guides)
            {
                guide.parse(arPluginsOptions);
            }
      
            LOG(notice) << "In RegDependsChecker, options initialized: " << endl;
            LOG(notice) << "\tdep_depth: " << _mDepDepth << endl;
            LOG(notice) << "\trand_instr_start: " << _mRandomInstructionsStart << endl;
            LOG(notice) << "\tstring_example: " << _mStringExample << endl;
        } 
      
        // coerce all register names to upper case, 64 bits. return true if GP reg...
        bool GPReg(string &arDest, string &arSrc) 
        {
            arDest = arSrc;
            transform(arDest.begin(), arDest.end(), arDest.begin(), ::toupper);
            return arDest[0] == 'X';
        }
      
        // use pre-step event to look for start of random code...
        void onPreStep() 
        {
            if (_mInRandomInstructions) 
            {
              // already in random code...
            } 
            else 
            {
                uint64 rval = 0;
                uint64 rmask = 0;
                SimPtr()->ReadRegister(CpuID(),"PC",&rval,&rmask);
                if (rval >= _mRandomInstructionsStart) 
                    _mInRandomInstructions = true;
            }
        }
      
        // check gp register accesses on each step event...
        void onStep(vector<RegUpdate> *apRegUpdates, vector<MemUpdate> *apMemUpdates,
                    vector<MmuEvent> *apMMUUpdates, vector<ExceptionUpdate> *apExceptions) 
        {
            LOG(debug) << "RegDependsChecker: Received onStep event!" << endl;
      
            if (_mInRandomInstructions) 
            {
                LOG(debug) << "RegDependsChecker: Checking GP reg dependencies for Force generated random instruction..." << endl;
            } 
            else 
            {
                LOG(debug) << "RegDependsChecker: Not yet in Force random code..." << endl;
                return;
            }
      
            // the register-updates 'reported' by step may include both a read-access and a write-access for
            // general purpose registers. report reads, then writes...
            map<string, string> new_accesses; // used to record unique (by name) GP register read/write accesses
      
            for (uint32_t i = 0; i < (*apRegUpdates).size(); i++) 
            {
                if ((*apRegUpdates)[i].access_type == "read") 
                {
                    string rname;
                    if (GPReg(rname,(*apRegUpdates)[i].regname))    
                    {
                        CheckForHazard(rname, "READ");
                        new_accesses[rname] = "READ";
                    }
                }
            }
      
            for (uint32_t i = 0; i < (*apRegUpdates).size(); i++) 
            {
                if ((*apRegUpdates)[i].access_type == "write") 
                {
                    string rname;
                    if (GPReg(rname,(*apRegUpdates)[i].regname)) 
                    {
                        CheckForHazard(rname, "WRITE");
                        new_accesses[rname] = "WRITE"; // may overwrite previously recorded read access for the same register
                    }
                }
            }
      
            // record all new accesses...
            for (auto i = new_accesses.begin(); i != new_accesses.end(); i++) 
            {
                RecordRegisterAccess(i->first, i->second);
            }
        }
      
        void RecordRegisterAccess(const string &arRegName, const string &arAccessType) 
        {
            LOG(debug) << "RegDependsChecker: GP reg access: " << arRegName << "/" << arAccessType << endl;
      
            _mRegisterAccesses.push_back(RegAccess(arRegName,arAccessType));
          
            // track only last N dependencies...
            if (_mRegisterAccesses.size() > _mDepDepth) {
                _mRegisterAccesses.erase( _mRegisterAccesses.begin() );
            }
        }
      
        void CheckForHazard(string aPName, string aPAccessType) 
        {
            LOG(debug) << "RegDependsChecker: pipe size: " << _mRegisterAccesses.size() << ". Checking for new hazards..." << endl;
      
            for (auto i = _mRegisterAccesses.rbegin(); i != _mRegisterAccesses.rend(); i++) {
                if ( (*i).mRname == aPName) {
                    string access_sequence = aPAccessType + " AFTER " + (*i).mAcctype;
                    if (_mDependencyCounts.find(access_sequence) == _mDependencyCounts.end())
                        _mDependencyCounts[access_sequence] = 1;
                    else
                        _mDependencyCounts[access_sequence] += 1;
                    break;
                }
            }
      
            return;
        }
      
        // the test has ended. dump hazard counts...
        void atTestEnd() 
        {
            LOG(debug) << "RegDependsChecker: Received atTestEnd event!" << endl;
      
            cout << "Read After Write count:\t" << dec << _mDependencyCounts["READ AFTER WRITE"] << endl;
            cout << "Write After Read count:\t" << dec << _mDependencyCounts["WRITE AFTER READ"] << endl;
            cout << "Write After Write count:\t" << dec << _mDependencyCounts["WRITE AFTER WRITE"] << endl;
        };
    
    private:
        vector<struct RegAccess> _mRegisterAccesses;
        uint32_t _mDepDepth;
        map<string, int> _mDependencyCounts;
        bool _mInRandomInstructions;
        uint64_t _mRandomInstructionsStart;
        std::string _mStringExample;
};
}


// plugin + 'extern C' methods to be compiled into shared library:
extern "C" Force::SimPlugin* create()
{
  return new Force::RegDependsChecker();
}

extern "C" void destroy(Force::SimPlugin* t1)
{
  delete t1;
}

extern "C" int is_shared()
{
  return 0;  //!< create a separate plugin instance for each thread
}
