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
#include <lest/lest.hpp>
#include <Log.h>
#include <Random.h>
//------------------------------------------------
// include necessary header files here
//------------------------------------------------
#include <Defines.h>
#include <UtilityFunctions.h>
#include <VectorElementUpdates.h>
#include <SimAPI.h>
#include <vector>
#include <set>
#include <iostream>
#include <tuple>
#include <cstring>

using text = std::string;
using namespace Force;

//In real life, this methods would be called by the Handcar code. Instead it is called by the test code.
extern "C"{
void update_vector_element_mockup(uint32_t pid, const char *pRegName, uint32_t vRegIndex, uint32_t eltIndex, uint32_t eltByteWidth, const uint8_t* value, uint32_t  byteLength, const char* pAccessType, SimAPI& apihandle)
{
    apihandle.RecordVectorRegisterUpdate(pid, pRegName, vRegIndex, eltIndex, eltByteWidth, value, byteLength, pAccessType);
}
}

class SimAPIStub : public SimAPI
{
public:
SimAPIStub() : registerValue{}, registerMask{} {}
~SimAPIStub(){}
ASSIGNMENT_OPERATOR_ABSENT(SimAPIStub);
COPY_CONSTRUCTOR_ABSENT(SimAPIStub);

void ReadRegister(uint32 CpuID, const char *regname, uint64 *rval, uint64 *rmask) override
{
    (*rval) = registerValue[0];
    (*rmask) = registerMask[0];
} 

void PartialReadLargeRegister(uint32 CpuID, const char* regname, uint8_t* bytes, uint32_t length, uint32_t offset) override
{
    memcpy(bytes, &registerValue[0] + offset, length); 
}

void PartialWriteLargeRegister(uint32 CpuID, const char* regname, const uint8_t* bytes, uint32_t length, uint32_t offset) override {}

void InitializeIss(const ApiSimConfig& rConfig, const std::string &rSimSoFile, const std::string& rApiTraceFile) override {} 
void Terminate() override {}
void GetSimulatorVersion(std::string &sim_version) override {}
void ReadPhysicalMemory(uint32 mem_bank, uint64 address, uint32 size, unsigned char *pBytes) override {}
void WritePhysicalMemory(uint32 mem_bank, uint64 address, uint32  size, const unsigned char *pBytes) override {}
void WriteRegister(uint32 CpuID,const char *regname,uint64 rval,uint64 rmask) override {}
void InjectEvents(uint32 CpuID, uint32 interrupt_sets) override {}
void Step(uint32 cpuid,std::vector<RegUpdate> &rRegisterUpdates,std::vector<MemUpdate> &rMemUpdates, std::vector<MmuEvent> &rMmuEvents, std::vector<ExceptionUpdate> &rExceptions) override 
{
    GetVectorRegisterUpdates(rRegisterUpdates); 
    mVectorElementUpdates.clear(); //Approximates what happens in a real implementation of 'Step'
}
void WakeUp(uint32 cpuId) override {} //!< Wake up from lower power state.
void TurnOn(uint32 cpuId) override {} //!< Turn the Iss thread on.
void EnterSpeculativeMode(uint32 cpuId) override {} //!< The CPU thread enters speculative mode.
void LeaveSpeculativeMode(uint32 cpuId) override {} //!< The CPU thread leaves speculative mode.
void RecordExceptionUpdate(const SimException *pException) override {} //!< Record exceptions.

//Values used by ReadRegister. To be set directly in the tests.
uint64 registerValue[2];
uint64 registerMask[2];
};

const lest::test specification[] = {

CASE( "Testing VectorElementUpdate::physicalRegisterIndicesFromELement(...)" ) {

    SETUP( "Specify architectural details about vector registers" )  {
        //-----------------------------------------
        // include necessary setup code here
        //-----------------------------------------
        cuint32 physRegSize = 8;
        cuint32 numPhysRegs = 2;

        //-----------------------------------------
        // do some initial checking here
        //-----------------------------------------
        SECTION("Element is within bounds of the individual physical registers") {
            VectorElementUpdate update(0, 4);
            std::set<uint32> physicalRegisterIndices;
            update.GetPhysicalRegisterIndices(physRegSize, numPhysRegs, physicalRegisterIndices);
            EXPECT(physicalRegisterIndices.size() > 0u);
        }

        SECTION("Element overlaps multiple physical registers") {
            VectorElementUpdate update(0, 12);
            std::set<uint32> physicalRegisterIndices;
            update.GetPhysicalRegisterIndices(physRegSize, numPhysRegs, physicalRegisterIndices);
            EXPECT(physicalRegisterIndices.size() > 0u);
        }

        SECTION("Element partially overlaps one physical register, but is out of bounds") {
            VectorElementUpdate update(1, 12);
            std::set<uint32> physicalRegisterIndices;
            update.GetPhysicalRegisterIndices(physRegSize, numPhysRegs, physicalRegisterIndices);
            EXPECT(physicalRegisterIndices.size() > 0u);
        }

        SECTION("Element is completely out of bounds of the physical registers") {
            VectorElementUpdate update(1, 16);
            std::set<uint32> physicalRegisterIndices;
            update.GetPhysicalRegisterIndices(physRegSize, numPhysRegs, physicalRegisterIndices);
            EXPECT(physicalRegisterIndices.size() == 0u);
        }
    }
},

CASE( "Testing VectorElementUpdates::insert(...)" ) {
   SETUP( "Specify architectural details about vector registers and setup vector of VectorElementUpdates" )  {
     cuint32 physRegSize = 8;
     cuint32 numPhysRegs = 2;
     std::vector<std::string> aVecPhysRegNames = {"_1", "_2"};
     uint32 aVecLogRegWidth = 16;
     uint32 processorId = 0;
     uint32 aEltIndex = 0;
     uint32 aEltByteWidth = 4;
     const uint8_t aEntireRegValue[] = {0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu};
     std::vector<VectorElementUpdates> vecEltUpdates;
     vecEltUpdates.emplace_back(processorId, aVecLogRegWidth, aVecPhysRegNames, physRegSize, numPhysRegs);

     SECTION ("Insert one, value fits, access type read") {
        const char aRegName[] = "v1";
        uint32 aRegByteWidth = 16;
        const char aAccessType[] = "read";

        bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

        EXPECT(success);
     }

     SECTION ("Insert one, value fits, access type write") {
        const char aRegName[] = "v1";
        uint32 aRegByteWidth = 16;
        const char aAccessType[] = "write";

        bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

        EXPECT(success);
     }

     SECTION ("Insert one, value fits, access type unknown") {
        const char aRegName[] = "v1";
        uint32 aRegByteWidth = 16;
        const char aAccessType[] = "not_a_recognized_access_type";

        bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

        EXPECT(not success);
     }

     SECTION ("Insert one, register size larger than expected, access type read") {
        const char aRegName[] = "v1";
        uint32 aRegByteWidth = 32;
        const char aAccessType[] = "read";

        bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

        EXPECT(not success);
     }

     SECTION ("Insert one, register size smaller than expected, access type read") {
        const char aRegName[] = "v1";
        uint32 aRegByteWidth = 8;
        const char aAccessType[] = "read";

        bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

        EXPECT(not success);
     }

     SECTION ("Insert several, value fits, access type write") {
        const char aRegName[] = "v1";
        uint32 aRegByteWidth = 16;
        const char aAccessType[] = "write";

        uint32 aEltIndex1 = 1;
        uint32 aEltIndex2 = 2;
        uint32 aEltIndex3 = 3;

        bool success = true;
        success &= vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);
        success &= vecEltUpdates[0].insert(processorId, aRegName, aEltIndex1, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);
        success &= vecEltUpdates[0].insert(processorId, aRegName, aEltIndex2, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);
        success &= vecEltUpdates[0].insert(processorId, aRegName, aEltIndex3, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

        EXPECT(success);
     }

     SECTION ("Nominal read case, except aRegName is null") {
        const char* aRegName = nullptr;
        uint32 aRegByteWidth = 16;
        const char aAccessType[] = "read";

        bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

        EXPECT(not success);
     }

     SECTION ("Nominal read case, except aEntireRegValue is null") {
        const char aRegName[] = "v1";
        const uint8_t* aEntireRegValue = nullptr;
        uint32 aRegByteWidth = 16;
        const char aAccessType[] = "read";

        bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

        EXPECT(not success);
     }

     SECTION ("Nominal read case, except aAccessType is null") {
        const char aRegName[] = "v1";
        uint32 aRegByteWidth = 16;
        const char* aAccessType = nullptr;

        bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

        EXPECT(not success);
     }

     SECTION ("Trying to add a duplicate entry") {
        const char aRegName[] = "v1";
        uint32 aRegByteWidth = 16;
        const char aAccessType[] = "read";

        bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);
        success &= vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

        EXPECT(not success);
     }

     SECTION ("Nominal read and nominal write, same element") {
        const char aRegName[] = "v1";
        uint32 aRegByteWidth = 16;
        const char aAccessType0[] = "read";
        const char aAccessType1[] = "write";

        bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType0);
        success &= vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType1);

        EXPECT(success);
     }

   }
},

CASE( "Testing VectorElementUpdate::translateElementToRegisterUpdates(...)" ) {

    SETUP( "Specify architectural details about vector registers" )  {
        //-----------------------------------------
        // include necessary setup code here
        //-----------------------------------------
        cuint32 physRegSize = 8;
        cuint32 numPhysRegs = 2;
        std::vector<std::string> aVecPhysRegNames = {"_1", "_2"};
        uint32 aVecLogRegWidth = 16;
        uint32 aEltIndex = 0;
        uint32 aEltByteWidth = 4;
        uint32 processorId = 0;
        uint32 aRegByteWidth = 16;
        const char aRegName[] = "v1";
        const uint8_t aEntireRegValue[] = {0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu};
        std::vector<VectorElementUpdates> vecEltUpdates;
        vecEltUpdates.emplace_back(processorId, aVecLogRegWidth, aVecPhysRegNames, physRegSize, numPhysRegs);
        SimAPIStub stub;
        SimAPI* apihandle = &stub;

        //-----------------------------------------
        // do some initial checking here
        //-----------------------------------------
        SECTION("Insert a read update, then translate") {
          const char aAccessType[] = "read";
  
          bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

          std::vector<RegUpdate> regUpdates;
          vecEltUpdates[0].translateElementToRegisterUpdates(*apihandle, regUpdates);

          success &= (not regUpdates.empty());

          EXPECT(success);
        }

        SECTION("Insert a write update, then translate") {
          const char aAccessType[] = "write";
  
          bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

          std::vector<RegUpdate> regUpdates;
          vecEltUpdates[0].translateElementToRegisterUpdates(*apihandle, regUpdates);

          success &= (not regUpdates.empty());

          EXPECT(success);

        }

        SECTION("Insert an 'unknown' update, then attempt to translate") {
          const char aAccessType[] = "bogus";
  
          bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType);

          std::vector<RegUpdate> regUpdates;
          vecEltUpdates[0].translateElementToRegisterUpdates(*apihandle, regUpdates);

          success |= (not regUpdates.empty());

          EXPECT(not success);

        }

        SECTION("Insert a read update and a write update, then translate") {
          const char aAccessType1[] = "read";
          const char aAccessType2[] = "write";
  
          bool success = vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType1);
          success &= vecEltUpdates[0].insert(processorId, aRegName, aEltIndex, aEltByteWidth, aEntireRegValue, aRegByteWidth, aAccessType2);

          std::vector<RegUpdate> regUpdates;
          vecEltUpdates[0].translateElementToRegisterUpdates(*apihandle, regUpdates);

          success &= (regUpdates.size() == 2u);

          EXPECT(success);
        }
       
    }
},

CASE( "Testing SimAPI::RecordVectorRegisterUpdate" ) {

    SETUP( "Specify architectural details about vector registers" )  {
        //-----------------------------------------
        // include necessary setup code here
        //-----------------------------------------
        cuint32 physRegSize = 8;
        cuint32 numPhysRegs = 2;
        std::vector<std::string> aVecPhysRegNames = {"_0", "_1"};
        uint32 aVecLogRegWidth = 16;
        uint32 processorId = 0;
        std::vector<VectorElementUpdates> vecEltUpdates;
        vecEltUpdates.emplace_back(processorId, aVecLogRegWidth, aVecPhysRegNames, physRegSize, numPhysRegs);
        SimAPIStub stub;
        SimAPI* apihandle = &stub;

        std::vector<RegUpdate> rRegisterUpdates;
        std::vector<MemUpdate> rMemUpdates;
        std::vector<MmuEvent> rMmuUpdates;
        std::vector<ExceptionUpdate> rExceptions;

        SECTION("One element read update pushed via callback, then Step, inspect translated register update") {
          const char aAccessType[] = "read";
          uint32 aEltIndex = 0;
          uint32 aEltByteWidth = 4;
          const char aRegName[] = "v1";
          uint32_t vRegIndex = 0;
          std::vector<uint8_t> value{0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0};
 
          update_vector_element_mockup(processorId, aRegName, vRegIndex, aEltIndex, aEltByteWidth, &value[0], value.size(), aAccessType, *apihandle);
          stub.Step(processorId, rRegisterUpdates,rMemUpdates, rMmuUpdates, rExceptions);

          EXPECT(rRegisterUpdates.size() == 1u);
          RegUpdate temp = rRegisterUpdates.at(0);

          EXPECT(temp.CpuID == 0u); 
          EXPECT(temp.regname == std::string("v1_0"));
          bool value_comparison = (temp.rval == uint64_t(0x00ff00ff00ff00ffull));
          EXPECT(value_comparison);
          EXPECT(temp.access_type == std::string(aAccessType)); 
        }

        SECTION("One element write update pushed via callback, then Step, inspect translated register update") {
          const char aAccessType[] = "write";
          uint32 aEltIndex = 0;
          uint32 aEltByteWidth = 4;
          const char aRegName[] = "v1";
          uint32_t vRegIndex = 0;
          std::vector<uint8_t> value{0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0,0xff,0x0};

          memcpy(&stub.registerValue, &value[0], sizeof(stub.registerValue));
          memcpy(&stub.registerMask, &value[0], sizeof(stub.registerMask));

          update_vector_element_mockup(processorId, aRegName, vRegIndex, aEltIndex, aEltByteWidth, &value[0], value.size(), aAccessType, *apihandle);
          stub.Step(processorId, rRegisterUpdates,rMemUpdates, rMmuUpdates, rExceptions);

          EXPECT(rRegisterUpdates.size() == 1u);
          RegUpdate temp = rRegisterUpdates.at(0);

          EXPECT(temp.CpuID == 0u); 
          EXPECT(temp.regname == std::string("v1_0"));
          bool value_comparison = (temp.rval == uint64_t(0x00ff00ff00ff00ffull));
          EXPECT(value_comparison);
          EXPECT(temp.access_type == std::string(aAccessType)); 
        }

        SECTION("Every element of a vector register read, then Step, inspect translated register updates") {
          const char aAccessType[] = "read";
          std::vector<uint32> eltIndices = {0,1,2,3};
          std::vector<std::string> physRegNames = {"v2_0", "v2_1", "error", "error", "error"};
          uint32 aEltByteWidth = 4;
          const char aRegName[] = "v2";
          uint32_t vRegIndex = 1;
          std::vector<uint8_t> value{0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03};

          for(uint32 index : eltIndices)
          {
            update_vector_element_mockup(processorId, aRegName, vRegIndex, index, aEltByteWidth, &value[0], value.size(), aAccessType, *apihandle);
          }

          stub.Step(processorId, rRegisterUpdates,rMemUpdates, rMmuUpdates, rExceptions);

          EXPECT(rRegisterUpdates.size() == 2u);

          size_t counter = 0; 
          for(RegUpdate temp : rRegisterUpdates)
          {
            EXPECT(temp.CpuID == 0u); 
            bool value_comparison = (temp.rval == *(reinterpret_cast<uint64_t*>(&value[counter*8])));
            EXPECT(value_comparison);
            EXPECT(temp.access_type == std::string(aAccessType)); 
            EXPECT(temp.regname == physRegNames[counter++]);
          }
        }

        SECTION("Every element of a vector register write, then Step, inspect translated register updates") {
          const char aAccessType[] = "write";
          std::vector<uint32> eltIndices = {0,1,2,3};
          std::vector<std::string> physRegNames = {"v2_0", "v2_1", "error", "error", "error"};
          uint32 aEltByteWidth = 4;
          const char aRegName[] = "v2";
          uint32_t vRegIndex = 1;
          std::vector<uint8_t> value{0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03};
          memcpy(&stub.registerValue, &value[0], sizeof(stub.registerValue));
          memcpy(&stub.registerMask, &value[0], sizeof(stub.registerMask));

          for(uint32 index : eltIndices)
          {
            update_vector_element_mockup(processorId, aRegName, vRegIndex, index, aEltByteWidth, &value[0], value.size(), aAccessType, *apihandle);
          }

          stub.Step(processorId, rRegisterUpdates,rMemUpdates, rMmuUpdates, rExceptions);

          EXPECT(rRegisterUpdates.size() == 2u);

          size_t counter = 0; 
          for(RegUpdate temp : rRegisterUpdates)
          {
            EXPECT(temp.CpuID == 0u); 
            bool value_comparison = (temp.rval == *(reinterpret_cast<uint64_t*>(&value[counter*8])));
            EXPECT(value_comparison);
            EXPECT(temp.access_type == std::string(aAccessType)); 
            EXPECT(temp.regname == physRegNames[counter++]);
          }
        }

        SECTION("Every element of every vector register read, then Step, inspect translated register updates") {
          const char aAccessType[] = "read";
          std::vector<uint32> eltIndices = {0,1,2,3};
          std::vector<std::string> physRegSuffixes = {"_0", "_1"};
          size_t numVecRegs = 32;
          uint32 aEltByteWidth = 4;
          //const char aRegName[] = "v2";
          //uint32_t vRegIndex = 1;
          std::vector<uint8_t> value{0x00,0x10,0x20,0x30,0x01,0x11,0x21,0x31,0x02,0x12,0x22,0x32,0x03,0x13,0x23,0x33};
          memcpy(&stub.registerValue, &value[0], sizeof(stub.registerValue));
          memcpy(&stub.registerMask, &value[0], sizeof(stub.registerMask));

          std::vector<std::string> physRegNames;

          for(size_t vRegIndex = 0; vRegIndex < numVecRegs; ++vRegIndex)
          {
            std::string aRegName = std::string("v") + std::to_string(vRegIndex);

            for(const std::string& suffix : physRegSuffixes)
            {
                physRegNames.push_back(aRegName + suffix);
            }

            for(uint32 index : eltIndices)
            {
              update_vector_element_mockup(processorId, aRegName.c_str(), vRegIndex, index, aEltByteWidth, &value[0], value.size(), aAccessType, *apihandle);
            }
          }

          stub.Step(processorId, rRegisterUpdates, rMemUpdates, rMmuUpdates, rExceptions);

          EXPECT(rRegisterUpdates.size() == 64u);

          size_t counter = 0; 
          for(RegUpdate temp : rRegisterUpdates)
          {
            EXPECT(temp.CpuID == 0u); 
            size_t phys_reg_offset = (counter % 2)*8;
            bool value_comparison = (temp.rval == *(reinterpret_cast<uint64_t*>(value.data() + phys_reg_offset)));
            EXPECT(value_comparison);
            EXPECT(temp.access_type == std::string(aAccessType)); 
            EXPECT(temp.regname == physRegNames[counter++]);
          }
        }

        SECTION("Every element of every vector register write, then Step, inspect translated register updates") {
          const char aAccessType[] = "write";
          std::vector<uint32> eltIndices = {0,1,2,3};
          std::vector<std::string> physRegSuffixes = {"_0", "_1"};
          size_t numVecRegs = 32;
          uint32 aEltByteWidth = 4;
          //const char aRegName[] = "v2";
          //uint32_t vRegIndex = 1;
          std::vector<uint8_t> value{0x00,0x10,0x20,0x30,0x01,0x11,0x21,0x31,0x02,0x12,0x22,0x32,0x03,0x13,0x23,0x33};
          memcpy(&stub.registerValue, &value[0], sizeof(stub.registerValue));
          memcpy(&stub.registerMask, &value[0], sizeof(stub.registerMask));

          std::vector<std::string> physRegNames;

          for(size_t vRegIndex = 0; vRegIndex < numVecRegs; ++vRegIndex)
          {
            std::string aRegName = std::string("v") + std::to_string(vRegIndex);

            for(const std::string& suffix : physRegSuffixes)
            {
                physRegNames.push_back(aRegName + suffix);
            }

            for(uint32 index : eltIndices)
            {
              update_vector_element_mockup(processorId, aRegName.c_str(), vRegIndex, index, aEltByteWidth, &value[0], value.size(), aAccessType, *apihandle);
            }
          }

          stub.Step(processorId, rRegisterUpdates, rMemUpdates, rMmuUpdates, rExceptions);

          EXPECT(rRegisterUpdates.size() == 64u);

          size_t counter = 0; 
          for(RegUpdate temp : rRegisterUpdates)
          {
            EXPECT(temp.CpuID == 0u); 
            size_t phys_reg_offset = (counter % 2)*8;
            bool value_comparison = (temp.rval == *(reinterpret_cast<uint64_t*>(value.data() + phys_reg_offset)));
            EXPECT(value_comparison);
            EXPECT(temp.access_type == std::string(aAccessType)); 
            EXPECT(temp.regname == physRegNames[counter++]);
          }
        }

        SECTION("Every element of every vector register read and write, then Step, inspect translated register updates") {
          std::vector<std::string> accessTypes = {"read", "write"};
          std::vector<uint32> eltIndices = {0,1,2,3};
          std::vector<std::string> physRegSuffixes = {"_0", "_1"};
          size_t numVecRegs = 32;
          size_t numExpectedUpdates = numVecRegs * physRegSuffixes.size() * accessTypes.size();
          uint32 aEltByteWidth = 4;
          //const char aRegName[] = "v2";
          //uint32_t vRegIndex = 1;
          std::vector<uint8_t> value{0x00,0x10,0x20,0x30,0x01,0x11,0x21,0x31,0x02,0x12,0x22,0x32,0x03,0x13,0x23,0x33};
          memcpy(&stub.registerValue, &value[0], sizeof(stub.registerValue));
          memcpy(&stub.registerMask, &value[0], sizeof(stub.registerMask));

          std::vector<std::string> physRegNames;

          for(size_t vRegIndex = 0; vRegIndex < numVecRegs; ++vRegIndex)
          {
            std::string aRegName = std::string("v") + std::to_string(vRegIndex);

            for(const std::string& type : accessTypes)
            {
                for(const std::string& suffix : physRegSuffixes)
                {
                    physRegNames.push_back(aRegName + suffix);
                }

                for(uint32 index : eltIndices)
                {
                  update_vector_element_mockup(processorId, aRegName.c_str(), vRegIndex, index, aEltByteWidth, &value[0], value.size(), type.c_str(), *apihandle);
                }
            }
          }

          stub.Step(processorId, rRegisterUpdates, rMemUpdates, rMmuUpdates, rExceptions);

          EXPECT(rRegisterUpdates.size() == numExpectedUpdates);

          size_t counter = 0; 
          for(RegUpdate temp : rRegisterUpdates)
          {
            EXPECT(temp.CpuID == 0u); 
            size_t phys_reg_offset = (counter % 2)*8;
            size_t access_type_idx = (counter % 4) < 2 ? 0 : 1; //Our test pattern is two reads, then two writes, then we go on to the next register
            bool value_comparison = (temp.rval == *(reinterpret_cast<uint64_t*>(value.data() + phys_reg_offset)));
            EXPECT(value_comparison);
            EXPECT(temp.access_type == accessTypes[access_type_idx]); 
            EXPECT(temp.regname == physRegNames[counter++]);
          }
        }


    }
},



};

int main( int argc, char * argv[] )
{
  Force::Logger::Initialize();
  Force::Random::Initialize();
  int ret = lest::run( specification, argc, argv );
  Force::Random::Destroy();
  Force::Logger::Destroy();
  return ret;

}
