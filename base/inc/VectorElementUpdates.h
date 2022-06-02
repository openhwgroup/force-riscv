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
#ifndef Force_VectorElementUpdates_H
#define Force_VectorElementUpdates_H

#include <set>
#include <string>
#include <vector>

#include "Defines.h"

namespace Force {

  struct RegUpdate;
  class SimAPI;
  
  struct VectorElementUpdate {
    bool operator==(const VectorElementUpdate& rOther)
    {
      return (mElementIndex == rOther.mElementIndex); //No need to compare the byte length since this is not supposed to change mid instruction.
    }
    
    VectorElementUpdate(uint32 elementIndex, uint32 elementByteLength) : mElementIndex(elementIndex), mElementByteLength(elementByteLength) {}
    
    bool GetPhysicalRegisterIndices(cuint32 physRegSize, cuint32 numPhysRegs, std::set<uint32>& rPhysicalRegisterIndices) const; //Return value true means 'some indices were added', false means 'nothing new added'
    
    cuint32 mElementIndex;
    cuint32 mElementByteLength; 
  };

  class VectorElementUpdates {
  public:
    VectorElementUpdates(uint32 processorId, uint32 vectorLogicalRegisterWidth, const std::vector<std::string>& rVectorPhysicalRegisterNames, cuint32 physRegSize, cuint32 numPhysRegs) : 
      mProcessorId(processorId), mVectorLogicalRegisterWidth(vectorLogicalRegisterWidth), mVectorRegisterName(), mPhysRegSize(physRegSize), mNumPhysRegs(numPhysRegs), mVectorPhysicalRegisterNames(rVectorPhysicalRegisterNames), mRegisterValue(vectorLogicalRegisterWidth), mElementReadUpdates(), mElementWriteUpdates() {}
 
    DESTRUCTOR_DEFAULT(VectorElementUpdates); 
    ASSIGNMENT_OPERATOR_ABSENT(VectorElementUpdates);
  
    VectorElementUpdates(const VectorElementUpdates& other) :
      mProcessorId(other.mProcessorId),
      mVectorLogicalRegisterWidth(other.mVectorLogicalRegisterWidth),
      mVectorRegisterName(other.mVectorRegisterName),
      mPhysRegSize(other.mPhysRegSize),
      mNumPhysRegs(other.mNumPhysRegs),
      mVectorPhysicalRegisterNames(other.mVectorPhysicalRegisterNames),
      mRegisterValue(other.mRegisterValue),
      mElementReadUpdates(other.mElementReadUpdates),
      mElementWriteUpdates(other.mElementWriteUpdates)
    {}
  
    void insert(uint32 processorId, const char* pRegisterName, uint32 eltIndex, uint32 eltByteWidth, const uint8_t* pEntireRegValue, uint32 regByteWidth, const char* pAccessType);
  
    void translateElementToRegisterUpdates(SimAPI& rApiHandle, std::vector<RegUpdate>& rRegisterUpdates) const;
  
  private:
    void validateInsertArguments(uint32 processorId, const char* pRegisterName, uint32 eltIndex, uint32 eltByteWidth, const uint8_t* pEntireRegValue, uint32 regByteWidth, const char* pAccessType); //!< Fail if any of the provided arguments have invalid values for the insert() method.
  private:
    cuint32 mProcessorId;
    cuint32 mVectorLogicalRegisterWidth;
    std::string mVectorRegisterName;
    cuint32 mPhysRegSize;
    cuint32 mNumPhysRegs;
    const std::vector<std::string>& mVectorPhysicalRegisterNames;
    std::vector<uint8_t> mRegisterValue;
    std::vector<VectorElementUpdate> mElementReadUpdates;
    std::vector<VectorElementUpdate> mElementWriteUpdates;
  };

}
#endif

