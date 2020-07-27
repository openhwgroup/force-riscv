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
#include <VectorElementUpdates.h>
#include <SimAPI.h>
#include <string.h>
#include <vector>
#include <set>
#include <algorithm>

namespace Force
{

bool VectorElementUpdate::GetPhysicalRegisterIndices(cuint32 physRegSize, cuint32 numPhysRegs, std::set<uint32>& rPhysicalRegisterIndices) const
{
  uint32 elt_offset = mElementIndex*mElementByteLength;
  uint32 elt_upper_bound = elt_offset + mElementByteLength;   
  bool contributed = false;
  uint32 bottomIndex = elt_offset / physRegSize;
  for(uint32 index = bottomIndex; index < numPhysRegs; ++index)
  {
    //Check that the first byte of the physical register actually intersects with the element.
    if((index * physRegSize) < elt_upper_bound)
    {
      std::pair<std::set<uint32>::iterator, bool> result = rPhysicalRegisterIndices.insert(index);
      contributed |= result.second;
    }
  }

  return contributed;
}

bool VectorElementUpdates::insert(uint32 processorId, const char* pRegisterName, uint32 eltIndex, uint32 eltByteWidth, const uint8_t* pEntireRegValue, uint32 regByteWidth, const char* pAccessType)
{
  bool success = validateInsertArguments(processorId, pRegisterName, eltIndex, eltByteWidth, pEntireRegValue, regByteWidth, pAccessType);
  if(not success) {
    return false;
  }

  VectorElementUpdate tentative_update(eltIndex, eltByteWidth);

  if(strcmp(pAccessType, "read") == 0)
  {
    if(std::find(mElementReadUpdates.begin(), mElementReadUpdates.end(), tentative_update) == mElementReadUpdates.end())
    {

      mElementReadUpdates.push_back(tentative_update);

      //Copy the value and register name if this was the first element added to the read updates vector.
      if(mElementReadUpdates.size() == 1)
      {
        for(uint32 byteNumber = 0; byteNumber < regByteWidth; ++byteNumber)
        {
          mRegisterValue.at(byteNumber) = (pEntireRegValue[byteNumber]);      
        }

        mVectorRegisterName = pRegisterName;
      }
    }  
    else
    {
      success = false;
    }
  }
  else if(strcmp(pAccessType, "write") == 0)
  {
    if(std::find(mElementWriteUpdates.begin(), mElementWriteUpdates.end(), tentative_update) == mElementWriteUpdates.end())
    {
      mElementWriteUpdates.push_back(tentative_update);

      //Don't copy the value now because at this point the instruction hasn't completed. We need the name of the register though so we can read it from the simulator. 
      //If this was the first writes vector update, copy the register name. 
      if(mElementWriteUpdates.size() == 1)
      {
        mVectorRegisterName = pRegisterName;
      }
    }  
    else
    {
      success = false;
    }
  }
  else
  {
    success = false;
  }

  return success;
}

void VectorElementUpdates::translateElementToRegisterUpdates(SimAPI& rApiHandle, std::vector<RegUpdate>& rRegisterUpdates) const
{

  if(not mElementReadUpdates.empty() or not mElementWriteUpdates.empty())
  {
    std::string access_type_temp = "read";
    std::set<uint32> indices;

    for(const VectorElementUpdate& update : mElementReadUpdates)
    {
      update.GetPhysicalRegisterIndices(mPhysRegSize, mNumPhysRegs, indices);
    }

    for(uint32 phys_reg_idx : indices)
    {
      uint64 rval_temp = 0x0ull;
      uint64 mask_temp = 0xffffffffffffffffull;  
      memcpy(&rval_temp, &mRegisterValue.at(mPhysRegSize * phys_reg_idx), mPhysRegSize);

      std::string regname_temp = mVectorRegisterName + mVectorPhysicalRegisterNames.at(phys_reg_idx);
      RegUpdate reg_update_temp(mProcessorId, regname_temp.c_str(), rval_temp, mask_temp, access_type_temp.c_str());
      rRegisterUpdates.push_back(reg_update_temp);
    }    

    std::vector<uint8_t> rval_buff;
    if(not mElementWriteUpdates.empty())
    {
      access_type_temp = "write";
      rval_buff.resize(mVectorLogicalRegisterWidth);
      rApiHandle.PartialReadLargeRegister(mProcessorId, mVectorRegisterName.c_str(), &rval_buff.at(0), mVectorLogicalRegisterWidth, 0);
    }

    indices.clear(); //Clear out because write updates are next
    for(const VectorElementUpdate& update : mElementWriteUpdates)
    {
      update.GetPhysicalRegisterIndices(mPhysRegSize, mNumPhysRegs, indices);
    }

    for(uint32 phys_reg_idx : indices)
    {
      uint64 rval_temp = 0x0ull;
      uint64 mask_temp = 0xffffffffffffffffull;  
      memcpy(&rval_temp, &rval_buff.at(mPhysRegSize * phys_reg_idx), mPhysRegSize);

      std::string regname_temp = mVectorRegisterName + mVectorPhysicalRegisterNames.at(phys_reg_idx);
      RegUpdate reg_update_temp(mProcessorId, regname_temp.c_str(), rval_temp, mask_temp, access_type_temp.c_str());
      rRegisterUpdates.push_back(reg_update_temp);
    }
  }
}

bool VectorElementUpdates::validateInsertArguments(uint32 processorId, const char* pRegisterName, uint32 eltIndex, uint32 eltByteWidth, const uint8_t* pEntireRegValue, uint32 regByteWidth, const char* pAccessType)
{
  bool valid = true;
  if(pRegisterName == nullptr || pEntireRegValue == nullptr || pAccessType == nullptr) {
    valid = false;
  }

  if(regByteWidth != mVectorLogicalRegisterWidth) {
    valid = false;
  }

  //Tests if element fits in the vector register
  if((eltIndex + uint32(1))*eltByteWidth > regByteWidth) {
    valid = false;
  }

  if(processorId != mProcessorId) {
    valid = false;
  }

  return valid;
}

} 
