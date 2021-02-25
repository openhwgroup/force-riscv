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
#include "EnumsFPIX.h"

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
    Force::EOperandType mRegisterType;
    uint16_t mRegisterNumber;
    Force::EAccessAgeType mAccessType;
    uint64_t mCpuID;
  
    RegAccess(Force::EOperandType aRegisterType, uint16_t aRegisterNumber, Force::EAccessAgeType aAccessType, uint64_t aCpuID) : mRegisterType(aRegisterType), mRegisterNumber(aRegisterNumber), mAccessType(aAccessType), mCpuID(aCpuID) {};
    
    //Required in order to use find_if
    RegAccess() : mRegisterType(Force::EOperandType::Register), mRegisterNumber(999), mAccessType(Force::EAccessAgeType::Invalid), mCpuID(999) {};
    RegAccess(const struct RegAccess & other) = default;
    RegAccess & operator=(const struct RegAccess & other) = default; //Remind the reader that the default assignment operator and copy constructor are being used.
  
    bool operator==(const struct RegAccess & other) = delete; //Only should use a lambda with one of the functions below to use the precise intended comparison.
  
    bool is_same_register(const struct RegAccess & other) const 
    {
        return mRegisterType == other.mRegisterType and mRegisterNumber == other.mRegisterNumber and mCpuID == other.mCpuID;
    };
};


class DependencyRecord 
{
    private:
        uint32_t _mDependencyDepth;
        Force::EOperandType _mRegisterType;
        uint16_t _mRegisterNumber;
        Force::EAccessAgeType _mAccessType;
        Force::EDependencyType _mDependencyType;
        uint64_t _mCpuID;    
        uint64_t _mCount;    
        std::vector<uint64_t> _mDependencyInstructionCounts;

    public:
        //Life cycle methods
        DependencyRecord() = delete;

        DependencyRecord(uint32_t aDependencyDepth, Force::EOperandType aRegisterType, uint16_t aRegisterNumber, Force::EAccessAgeType aAccessType, Force::EDependencyType aDependencyType, uint64_t aCpuID = 0, uint64_t aCount = 0, uint64_t aInstructionCount = 0) : _mDependencyDepth(aDependencyDepth), _mRegisterType(aRegisterType), _mRegisterNumber(aRegisterNumber), _mAccessType(aAccessType), _mDependencyType(aDependencyType), _mCpuID(aCpuID), _mCount(aCount), _mDependencyInstructionCounts( aCount > 0 ? std::vector<uint64_t>(1, aInstructionCount) : std::vector<uint64_t>() ) {};

        DependencyRecord(uint32_t aDependencyDepth, Force::EOperandType aRegisterType, uint16_t aRegisterNumber, Force::EAccessAgeType aAccessType, Force::EDependencyType aDependencyType, uint64_t aCpuID = 0, uint64_t aCount = 0, const std::vector<uint64_t> & aInstructionCountVec = {}) : _mDependencyDepth(aDependencyDepth), _mRegisterType(aRegisterType), _mRegisterNumber(aRegisterNumber), _mAccessType(aAccessType), _mDependencyType(aDependencyType), _mCpuID(aCpuID), _mCount(aCount), _mDependencyInstructionCounts(aInstructionCountVec) {};

        DependencyRecord(const DependencyRecord& other) : _mDependencyDepth(other._mDependencyDepth), _mRegisterType(other._mRegisterType), _mRegisterNumber(other._mRegisterNumber), _mAccessType(other._mAccessType), _mDependencyType(other._mDependencyType),_mCpuID(other._mCpuID), _mCount(other._mCount), _mDependencyInstructionCounts(other._mDependencyInstructionCounts) {};

        ~DependencyRecord(){};

    private:
        //prefix increment oprator
        DependencyRecord& operator++()
        {
            ++_mCount;
            return *this;    
        };

    public:
        //postfix increment operator is left unimplemented because it isn't needed for how the dependency records are being used here. 
        DependencyRecord& operator=(const DependencyRecord& other)
        {
            if(this != &other)
            {
                _mDependencyDepth = other._mDependencyDepth;
                _mRegisterType = other._mRegisterType;
                _mRegisterNumber = other._mRegisterNumber;
                _mAccessType = other._mAccessType;
                _mDependencyType = other._mDependencyType;
                _mCpuID = other._mCpuID;
                _mCount = other._mCount;    
                _mDependencyInstructionCounts = other._mDependencyInstructionCounts;
            }
            
            return *this;
        };   

        //Relational operators need to disregard the value of mCount since they're only used for finding the correct record by the values of the other variables using a dummy record.
        bool operator==(const DependencyRecord& other) const 
        {
            return _mDependencyDepth == other._mDependencyDepth and _mRegisterType == other._mRegisterType and _mRegisterNumber == other._mRegisterNumber and _mAccessType == other._mAccessType and _mDependencyType == other._mDependencyType and _mCpuID == other._mCpuID;
        };

        //This operator follows the rule for lexicographic ordering
        bool operator<(const DependencyRecord& other) const 
        {
            if(_mDependencyDepth == other._mDependencyDepth)
            {
                if(_mRegisterType == other._mRegisterType)
                {
                    if(_mRegisterNumber == other._mRegisterNumber)
                    {
                        if(_mAccessType == other._mAccessType)
                        {
                            if(_mDependencyType == other._mDependencyType)
                            {
                                if(_mCpuID == other._mCpuID)
                                {
                                    return false;
                                }
                                else if(_mCpuID < other._mCpuID)
                                {
                                    return true;
                                }
                            }
                            else if(_mDependencyType < other._mDependencyType)
                            {
                                return true;
                            }
                        }
                        else if(_mAccessType < other._mAccessType)
                        {
                            return true;
                        }
                    }
                    else if(_mRegisterNumber < other._mRegisterNumber)
                    {
                        return true;
                    }
                }
                else if(_mRegisterType < other._mRegisterType)
                {
                    return true;
                }
            }
            else if(_mDependencyDepth < other._mDependencyDepth)
            {
                return true;
            }

            return false;
        };
        
        bool operator<=(const DependencyRecord& other) const 
        {
            return *this == other or *this < other;
        };

        DependencyRecord& countDependency(uint64_t aInstructionCountValue)
        {
            ++(*this);
            auto lb = std::lower_bound(_mDependencyInstructionCounts.begin(), _mDependencyInstructionCounts.end(), aInstructionCountValue);
            _mDependencyInstructionCounts.insert(lb, aInstructionCountValue);
            return *this;
        }

        //Bulk addition of counts from another record.
        DependencyRecord& operator+=(const DependencyRecord& other)
        {
            _mDependencyInstructionCounts.reserve(_mDependencyInstructionCounts.size() + other._mDependencyInstructionCounts.size());

            for(uint64_t count : other._mDependencyInstructionCounts)
            {
                countDependency(count);  
            }

            return *this;
        };

        //Accessors needed for filtering on collections of DependencyRecord while protecting access to member values
        uint32_t depth() const {return _mDependencyDepth;};
        Force::EAccessAgeType access_type() const {return _mAccessType;};
        Force::EDependencyType dependency_type() const {return _mDependencyType;};
        uint64_t count_value() const {return _mCount;};
        uint64_t cpuid_value() const {return _mCpuID;};
        const std::vector<uint64_t> & dependency_instruction_counts() const {return _mDependencyInstructionCounts;};

        //Output methods
        static std::string printHeadings()
        {
            return "Depth, Count, Register type, Register number, Access type, Dependency type, CpuID, Instruction counter values";
        };
        
        std::string printValues() const 
        {
            std::string outstring = std::to_string(_mDependencyDepth) + std::string(", ") + std::to_string(_mCount) + std::string(", ") + Force::EOperandType_to_string(_mRegisterType) + std::string(", ") + std::to_string(_mRegisterNumber) + std::string(", ") +  EAccessAgeType_to_string(_mAccessType) + std::string(", ") + EDependencyType_to_string(_mDependencyType) + std::string(", ") + std::to_string(_mCpuID);
            outstring += std::string(", [");
            for(uint64_t count : _mDependencyInstructionCounts)
            {
                outstring += std::to_string(count) + std::string(",");    
            }
            outstring += std::string("]");

            return outstring;      
        };
};


class RegDepCounter : public Force::SimPlugin 
{
    public:
        RegDepCounter() : 
          _mRegisterAccesses(),
          _mAccessStages(),
          _mDepDepth(30),
          _mNumBins(4),
          _mBasicDependencyCounts(),
          _mDependencyRecords(),
          _mInRandomInstructions(false),
          _mRandomInstructionsStart(0x80000000),
          _mCollectedCount(0),
          _mInstructionCount(0),
          _mDoDumpRecords(false)
        { Init(); };
      
        const string Name() { return "RegDepCounter"; }
      
        void Init() 
        {
            _mBasicDependencyCounts["Read After Write"] = 0;
            _mBasicDependencyCounts["Write After Read"] = 0;
            _mBasicDependencyCounts["Write After Write"] = 0;
      
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
        void parsePluginsClargs(vector<string> &plugins_cl_args) {  }
      
        //Use the ParseGuide helper class to lookup and assign option values, stress free
        void parsePluginsOptions(std::map<std::string, std::string> & arPluginsOptions)
        {
            std::vector<Force::ParseGuide> guides;
            guides.emplace_back(std::string("dep_depth"), &_mDepDepth); 
            guides.emplace_back(std::string("rand_instr_start"), &_mRandomInstructionsStart);
            guides.emplace_back(std::string("num_bins"), &_mNumBins);
      
            for(auto & guide : guides)
            {
                guide.parse(arPluginsOptions);
            }
      
            if(arPluginsOptions.count(std::string("debug")) > 0)
            {
                _mDoDumpRecords = true;
            }
      
            LOG(notice) << "In RegDepCounter, options initialized: " << endl;
            LOG(notice) << "\tdep_depth: " << _mDepDepth << endl;
            LOG(notice) << "\trand_instr_start: " << _mRandomInstructionsStart << endl;
            LOG(notice) << "\tdebug: " << _mDoDumpRecords << endl;
            LOG(notice) << "\tnum_bins: " << _mNumBins << endl;
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
        void onStep(vector<RegUpdate> *apRegUpdates, vector<MemUpdate> *apMemUpdates, vector<MmuEvent> *apMMUUpdates, vector<ExceptionUpdate> *apExceptions) 
        {
            LOG(debug) << "RegDepCounter: Received onStep event!" << endl;
      
            if (_mInRandomInstructions) 
            {
                LOG(debug) << "RegDepCounter: Checking GP reg dependencies for Force generated random instruction..." << endl;
            } 
            else 
            {
                LOG(debug) << "RegDepCounter: Not yet in Force random code..." << endl;
                ++_mInstructionCount; //The increment statement here means that mInstructionCount is incremented even if we are not yet in the random instructions. This allows easier comparison to the railhouse.

                return;
            }
      
            // the register-updates 'reported' by step may include both a read-access and a write-access for
            // general purpose registers. report reads, then writes...
            std::map<string, struct RegAccess > new_accesses; // used to record unique (by name) GP register read/write accesses
            std::vector< struct RegAccess> new_dependency_registers; // used to keep track of registers we have already counted dependency dependencies for, so we can avoid double counting, especially when we have both read and write accesses for the same register in the same step.
      
            //Check all the reads and then check all the write. If there is a conflict, new accesses are recorded as writes, and also in a conflict any dependencies are categorized as read accesses.
            const std::vector<std::pair<std::string, std::string> > access_check_order = {{"read", "Read"}, {"write", "Write"}}; //Capitalized version is for Enum compatibility
            for(auto rw_check_phase : access_check_order)
            {
                for (uint32_t i = 0; i < (*apRegUpdates).size(); i++) 
                {
                    if ((*apRegUpdates)[i].access_type == rw_check_phase.first) 
                    {
                        string rname;
                        if (GPReg(rname,(*apRegUpdates)[i].regname))
                        {
                            //Translate register name to EOperandType and number
                            //NOTE: there is a disconnect between the terminology used in the enum and the data source that creates the register update information.
                            Force::EOperandType reg_type;
                            if(rname.substr(0,1) == "X")
                            {
                                reg_type = Force::EOperandType::GPR;
                            }
                            uint16_t reg_num = std::stoul(std::string(std::next(rname.begin()), rname.end())); //There is an assumption here that the register number will fit the datatype. Overflow will result in value wraparound.
      
                            //Bundle the reg access info since we need to use the information in multiple ways and places
                            struct RegAccess reg_access_temp(reg_type, reg_num, Force::string_to_EAccessAgeType(rw_check_phase.second), (*apRegUpdates)[i].CpuID);
                            
                            //Only check for a dependency for this register if we have not already counted one.
                            auto same_register_lambda = [&reg_access_temp](const struct RegAccess & other){return reg_access_temp.is_same_register(other);};
                            if(std::find_if(new_dependency_registers.begin(), new_dependency_registers.end(), same_register_lambda) == new_dependency_registers.end())
                            {
                                if(CheckForDependency(reg_access_temp))
                                {
                                    new_dependency_registers.push_back(reg_access_temp);
                                }
                            }
                            // may overwrite previously recorded read access for the same register, which is what we want to happen.
                            new_accesses[rname] = reg_access_temp;
                        }
                    }
                }
            }
      
            //push back a new access stage
            _mAccessStages.push_back(std::vector<struct RegAccess>());
      
            // record all new accesses...
            for (auto i = new_accesses.begin(); i != new_accesses.end(); i++) 
            {
                RecordRegisterAccess(i->second);
            }
      
             //Increment the instruction counters
             ++_mInstructionCount;
             ++_mCollectedCount; //Only increments when we're in the random instructions
        }
      
        void RecordRegisterAccess(const struct RegAccess & arRegAccess)
        {
            LOG(debug) << "RegDepCounter: GP reg access: " << Force::EOperandType_to_string(arRegAccess.mRegisterType) << std::to_string(arRegAccess.mRegisterNumber) << "/" << Force::EAccessAgeType_to_string(arRegAccess.mAccessType) << endl;
      
            _mRegisterAccesses.push_back(arRegAccess);
            _mAccessStages.back().push_back(arRegAccess);
            
            // track only last N dependencies...
            if (_mRegisterAccesses.size() > _mDepDepth) 
            {
                _mRegisterAccesses.erase(_mRegisterAccesses.begin());
            }
      
            if(_mAccessStages.size() > _mDepDepth)
            {
                _mAccessStages.erase(_mAccessStages.begin());
            }
        }
      
        bool CheckForDependency(const struct RegAccess & arRegAccess) 
        {
            LOG(debug) << "RegDepCounter: pipe size: " << _mRegisterAccesses.size() << ". Checking for new dependencies..." << endl;
      
            //Iterate from the most recent to oldest accesses
            for(auto stage = _mAccessStages.rbegin(); stage != _mAccessStages.rend(); ++stage)
            {
                for(auto i = (*stage).begin(); i != (*stage).end(); ++i)
                {  
                    //A dependency is counted when a register is accessed again in the same access history window
                    if((*i).is_same_register(arRegAccess)) 
                    {
                        string access_sequence = Force::EAccessAgeType_to_string(arRegAccess.mAccessType) + " After " + Force::EAccessAgeType_to_string((*i).mAccessType);

                        if(_mBasicDependencyCounts.find(access_sequence) == _mBasicDependencyCounts.end())
                        {
                            _mBasicDependencyCounts[access_sequence] = 1;
                        }
                        else
                        {
                            _mBasicDependencyCounts[access_sequence] += 1;
                        }
      
                        //Compute dependency depth
                        uint32_t depth = std::distance(_mAccessStages.rbegin(), stage) + 1;
      
                        //Translate access sequence to EDependencyType 
                        Force::EDependencyType dep_type;
                        if((*i).mAccessType == Force::EAccessAgeType::Write)
                        {
                            dep_type = Force::EDependencyType::OnTarget;
                        }
                        else if((*i).mAccessType == Force::EAccessAgeType::Read)
                        {
                            dep_type = Force::EDependencyType::OnSource;
                        }
                        
                        //Create a record to search the dependency record for matches 
                        Force::EOperandType reg_type = arRegAccess.mRegisterType;
                        uint16_t reg_num = arRegAccess.mRegisterNumber;
                        DependencyRecord search_record(depth, reg_type, reg_num, arRegAccess.mAccessType, dep_type,  arRegAccess.mCpuID, 1, _mInstructionCount);
      
                        //If we find a match to the record in the collection, increment the record already in the collection.
                        //Otherwise we should insert the newly created record at the lower bound iterator for the container, thereby keeping it sorted.
                        auto lb = std::lower_bound(_mDependencyRecords.begin(), _mDependencyRecords.end(), search_record);
                        if(lb != _mDependencyRecords.end() and search_record == (*lb))
                        {
                            (*lb).countDependency(_mInstructionCount);
                        }
                        else
                        {
                            _mDependencyRecords.insert(lb, search_record);  
                        }
      
                        return true;
                    }
                }
            }
      
            return false;
        }
      
      
        // the test has ended. dump dependency counts...
        void atTestEnd() 
        {
          LOG(debug) << "\nRegDepCounter: Received atTestEnd event!" << endl;
      
          cout << "\nNumber of total instructions steps:\t" << dec << _mInstructionCount << endl;
          cout << "Number of random instructions steps:\t" << dec << _mCollectedCount << endl;
          cout << "Read After Write count:\t" << dec << _mBasicDependencyCounts["Read After Write"] << endl;
          cout << "Write After Read count:\t" << dec << _mBasicDependencyCounts["Write After Read"] << endl;
          cout << "Write After Write count:\t" << dec << _mBasicDependencyCounts["Write After Write"] << endl;
      
          //Declare the vectors in which we will cache the output from the filtered records.
          std::vector<std::string> write_after_write;
          write_after_write.reserve(_mDependencyRecords.size());
          std::vector<std::string> read_after_write;
          read_after_write.reserve(_mDependencyRecords.size());
          std::vector<std::string> write_after_read;
          write_after_read.reserve(_mDependencyRecords.size());
          std::vector<std::string> read_after_read;
          read_after_read.reserve(_mDependencyRecords.size());
      
          //Go through all the records once and assign their output lines to the right buffers
          std::ofstream stream;
          if(_mDoDumpRecords == true)
          {
              stream.open("dependency_record_dump.txt", std::ofstream::out | std::ofstream::app);
              if(stream.is_open())
              {
                  stream << "Dependency record debug dump." << endl;
                  stream << "\nNumber of total instructions steps: " << dec << _mInstructionCount << endl;
                  stream << "Number of random instructions steps: " << dec << _mCollectedCount << endl;
                  stream << "Read After Write count: " << dec << _mBasicDependencyCounts["Read After Write"] << endl;
                  stream << "Write After Read count: " << dec << _mBasicDependencyCounts["Write After Read"] << endl;
                  stream << "Write After Write count: " << dec << _mBasicDependencyCounts["Write After Write"] << endl << endl;;
                  stream << DependencyRecord::printHeadings() << endl;
              }
             
          }
      
          for(auto record : _mDependencyRecords)
          {
              if(record.access_type() == Force::EAccessAgeType::Read and record.dependency_type() == EDependencyType::OnTarget)        
              {
                  read_after_write.push_back(record.printValues());
              }
              else if(record.access_type() == Force::EAccessAgeType::Write and record.dependency_type() == EDependencyType::OnSource)        
              {
                  write_after_read.push_back(record.printValues());
              }
              else if(record.access_type() == Force::EAccessAgeType::Write and record.dependency_type() == EDependencyType::OnTarget)        
              {   
                  write_after_write.push_back(record.printValues());
              }
              else if(record.access_type() == Force::EAccessAgeType::Read and record.dependency_type() == EDependencyType::OnSource)        
              {   
                  read_after_read.push_back(record.printValues());
              }
      
              // dump all the records to file
              if(stream.is_open())
              {
                  stream << record.printValues() << endl;
              }
          }
      
          if(stream.is_open())
          {
              stream.close();
          }
      
          //Output all the raw data lines
          cout << "\nRead after read. Headings: " << endl;
          cout << DependencyRecord::printHeadings() << endl;
          for(auto outstring : read_after_read)
          {
              cout << outstring << endl;
          }
      
          cout << "\nRead after write. Headings: " << endl;
          cout << DependencyRecord::printHeadings() << endl;
          for(auto outstring : read_after_write)
          {
              cout << outstring << endl;
          }
      
          cout << "\nWrite after read. Headings: " << endl;
          cout << DependencyRecord::printHeadings() << endl;
          for(auto outstring : write_after_read)
          {
              cout << outstring << endl;
          }
      
          cout << "\nWrite after write. Headings: " << endl;
          cout << DependencyRecord::printHeadings() << endl;
          for(auto outstring : write_after_write)
          {
              cout << outstring << endl;
          }
      
          //sum over all register names and print it.
          std::vector<DependencyRecord> summed;
          for(auto record : _mDependencyRecords)
          {
              DependencyRecord temp(record.depth(), Force::EOperandType::Register, 0, record.access_type(), record.dependency_type(), record.cpuid_value(), record.count_value(), record.dependency_instruction_counts());
              auto lb = std::lower_bound(summed.begin(), summed.end(), temp);
              if(lb != summed.end() and temp == (*lb))
              {
                  (*lb) += temp;
              }
              else
              {
                  summed.insert(lb, temp);  
              }
          }
          
          cout << "\nRecord only by depth. Headings: " << endl;
          cout << DependencyRecord::printHeadings() << endl;
          for(auto record : summed)
          {
              cout << record.printValues() << endl;
          }
      
          //Collapse the records that were summed over all the registers and bin them
          std::vector<DependencyRecord> binned;
          int bin_size = _mDepDepth / _mNumBins;

          for(auto record : summed)
          {
              int bin_number = record.depth() / bin_size;

              if(bin_number >= static_cast<int>(_mNumBins))
                  bin_number = _mNumBins - 1;
                               
              DependencyRecord temp(bin_number, Force::EOperandType::Register, 0, record.access_type(), record.dependency_type(), record.cpuid_value(), record.count_value(), record.dependency_instruction_counts());
              auto lb = std::lower_bound(binned.begin(), binned.end(), temp);
              if(lb != binned.end() and temp == (*lb))
              {
                  (*lb) += temp;
              }
              else
              {
                  binned.insert(lb, temp);  
              }
          }
          
          cout << "\nRecord only by bin, where each bin is a range of depth values adding up to " << _mDepDepth << ". Headings: " << endl;
          cout << DependencyRecord::printHeadings() << endl;
          for(auto record : binned)
          {
              cout << record.printValues() << endl;
          }
        };
    
    private:
        vector<struct RegAccess> _mRegisterAccesses;
        std::vector<std::vector<struct RegAccess>> _mAccessStages;
        uint32_t _mDepDepth;
        uint32_t _mNumBins;
        map<string, int> _mBasicDependencyCounts;
        vector<DependencyRecord> _mDependencyRecords;
        bool _mInRandomInstructions;
        uint64_t _mRandomInstructionsStart;
        uint64_t _mCollectedCount; //!< those instructions after the first one at the random instruction start address are counted here.
        uint64_t _mInstructionCount; //!< every time the onStep method is called, this instruction count is incremented. It is meant to better related to the railhouse.
        bool _mDoDumpRecords; //!< save each record to disk
};
}

// plugin + 'extern C' methods to be compiled into shared library:

extern "C" Force::SimPlugin* create()
{
    return new Force::RegDepCounter();
}

extern "C" void destroy(Force::SimPlugin* t1)
{
    delete t1;
}

extern "C" int is_shared()
{
    return 0;  //!< create a separate plugin instance for each thread
}
