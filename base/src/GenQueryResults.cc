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
#include <GenQuery.h>
#include <Log.h>

#include <pybind11/pybind11.h>

#include <PageInfoRecord.h>

/*!
  \file GenQueryResults
  \brief Module specifically designated to host GenQuery::GetResults method.
*/

using namespace std;

namespace Force {

  void GenRegisterIndexQuery::GetResults(py::object& rPyObject) const
  {
    rPyObject = py::int_(mIndex);
  }

  void GenRegisterReloadValueQuery::GetResults(py::object& rPyObject) const
  {
    rPyObject = py::int_(mReloadValue);
  }

  void GenRegisterInfoQuery::GetResults(py::object& rPyObject) const
  {
    py::dict ret_dict;
    py::list large_val_list;
    ret_dict["Width"] = py::int_(mWidth);
    ret_dict["Type"] = py::cast(mRegisterType);
    if (mValueValid == true)
    {
      if (mWidth > 64 && mValues.size())
      {
        for (auto val : mValues) large_val_list.append(val);
        ret_dict["LargeValue"] = large_val_list;
      }
      else
      {
        ret_dict["Value"] = py::int_(mValue);
      }
    }

    rPyObject = ret_dict;
  }

  void GenRegisterFieldInfoQuery::GetResults(py::object& rPyObject) const
  {
    py::tuple ret_tuple = py::tuple(3);
    ret_tuple[0] = mRegisterMask;
    ret_tuple[1] = ~mRegisterMask;
    ret_tuple[2] = mFieldsValue;

    LOG(notice) << "GetRegisterFieldInfoQuery: mask:0x" << hex << mRegisterMask << endl;

    rPyObject = ret_tuple;
  }

  void GenInstructionRecordQuery::GetResults(py::object& rPyObject) const
  {
    py::dict ret_dict;

    if (not IsValid()) {
      rPyObject = py::none();
      return;
    }

    auto va = VA();
    auto ipa = IPA();
    auto br_target = BRTarget();
    auto ls_target = LSTarget();

    ret_dict["Opcode"] = py::int_(Opcode());
    ret_dict["Name"] = py::cast(Name());
    ret_dict["Group"] = py::cast(Group());
    ret_dict["PA"] = py::int_(PA());
    ret_dict["Bank"] = py::int_(Bank());

    if (va.first)        ret_dict["VA"]       = py::int_(va.second);
    if (ipa.first)       ret_dict["IPA"]      = py::int_(ipa.second);
    if (ls_target.first) ret_dict["LSTarget"] = py::int_(ls_target.second);
    if (br_target.first) ret_dict["BRTarget"] = py::int_(br_target.second);

    py::dict destDict;
    const std::map<std::string, uint32> destList = Dests();
    for (auto &item : destList)
    {
      destDict[py::cast(item.first)] = py::int_(item.second);
    }
    ret_dict["Dests"] = destDict;

    py::dict srcDict;
    const std::map<std::string, uint32> srcList = Srcs();
    for (auto &item : srcList)
    {
      srcDict[py::cast(item.first)] = py::int_(item.second);
    }
    ret_dict["Srcs"] = srcDict;

    py::dict immDict;
    const std::map<std::string, uint32> immList = Imms();
    for (auto &item : immList)
    {
      immDict[py::cast(item.first)] = py::int_(item.second);
    }
    ret_dict["Imms"] = immDict;

    py::dict statusDict;
    const std::map<std::string, uint32> statusList = Status();
    for (auto &item : statusList)
    {
      statusDict[py::cast(item.first)] = py::int_(item.second);
    }
    ret_dict["Status"] = statusDict;

    py::dict addressingDict;
    const std::map<std::string, std::string> addr_name_map = AddressingName();
    const std::map<std::string, uint32> addr_index_map = AddressingIndex();
    auto it1 = addr_name_map.begin();
    auto it2 = addr_index_map.begin();
    while (it1 != addr_name_map.end() || it2 != addr_index_map.end())
    {
      py::tuple opt_tuple = py::tuple(2);
      string name;
      if (it1 != addr_name_map.end())
      {
        opt_tuple[1] = (*it1).second;
        name = (*it1).first;
        ++it1;
      }
      if (it2 != addr_index_map.end())
      {
        opt_tuple[0] = (*it2).second;
        ++it2;
      }
      addressingDict[py::cast(name)] = opt_tuple;
    }
    ret_dict["Addressing"] = addressingDict;

    rPyObject = ret_dict;
  }

  void GenStateQuery::GetResults(py::object& rPyObject) const
  {
    if (mIsValue) {
      rPyObject = py::int_(mValue);
    }
    else {
      rPyObject = py::str(mString);
    }
  }

  void GenUtilityQuery::GetResults(py::object& rPyObject) const
  {
    bool is_query_type = false;

    EQueryType query_type = try_string_to_EQueryType(PrimaryString(), is_query_type);

    if (!is_query_type)
    {
      query_type = QueryType();
    }

    LOG(debug) << "{GenUtilityQuery::GetResults} " << EQueryType_to_string(query_type) <<  endl;
    switch (query_type) {
        case EQueryType::SoftwareStepPrivLevs:
        {
          uint32 size = mList.size();
          py::tuple result_tuple = py::tuple(size);
          for (uint32 i = 0; i < size; i++)
          {
            result_tuple[i] = py::int_(mList[i]);
          }
          rPyObject = result_tuple;
          break;
        }
        case EQueryType::SoftwareStepReady:
          rPyObject = py::int_(mList[0]);
          break;

        case EQueryType::MaxAddress:
        case EQueryType::PickedValue:
          rPyObject = py::int_(mList[0]);
          break;

        case EQueryType::ValidAddressMask:
          rPyObject = py::int_(mList[0]);
          break;
        case EQueryType::GenData:
        {
          uint32 size = mList.size();
          if (size > 1)
          {
            py::tuple result_tuple = py::tuple(size);
            for (uint32 i = 0; i < size; ++ i)
            {
              result_tuple[i] = py::int_(mList[size - 1 - i]);
            }
            rPyObject = result_tuple;
          }
          else
          {
            rPyObject = py::int_(mList[0]);
          }
          break;
        }
        default:
          rPyObject = py::str(mString);
          break;
    }
  }

  void GenPageInfoQuery::GetResults(py::object& rPyObject) const
  {
    py::dict ret_dict;

    if (mValid == true)
    {
      // page
      py::dict page_dict;
      PageInfoRec page_info = mPageInformation.GetPageInfo();
      page_dict["MemoryType"] = page_info.memoryType;
      page_dict["MemoryAttr"] = page_info.memoryAttr;
      page_dict["Lower"] = page_info.lower;
      page_dict["Upper"] = page_info.upper;
      page_dict["PhysicalLower"] = page_info.physical_lower;
      page_dict["PhysicalUpper"] = page_info.physical_upper;
      page_dict["PhysPageId"] = page_info.phys_id;
      //page_dict["MemAttrValue"] = page_info.mem_attr_value;
      page_dict["MemAttrIndex"] = page_info.mem_attr_index;
      page_dict["PageSize"] = page_info.page_size;
      page_dict["Descriptor"] = page_info.descr_value;
      py::dict details_dict;
      for (auto iter = page_info.descr_details.begin(); iter != page_info.descr_details.end(); ++iter)
      {
        details_dict[py::cast(iter->first)] = iter->second;
      }
      page_dict["DescriptorDetails"] = details_dict;
      ret_dict["Page"] = page_dict;

      // table(s)
      for (auto item : mPageInformation.GetPageTableRecord())
      {
        py::dict table_dict;
        table_dict["Level"] = item.level;
        table_dict["DescriptorAddr"] = item.descr_addr;
        table_dict["Descriptor"] = item.descr_value;
        py::dict page_details_dict;
        for (auto iter = item.descr_details.begin(); iter != item.descr_details.end(); ++iter)
        {
          page_details_dict[py::cast(iter->first)] = iter->second;
        }
        table_dict["DescriptorDetails"] = page_details_dict;
        std::string name = "Table#" + std::to_string(item.level);
        ret_dict[py::cast(name)] = table_dict;
      }
    }
    else
    {
      LOG(notice) << "{GenPageInfoQuery::GetResults} - Can't find the page" << endl;
    }

    rPyObject = ret_dict;
  }

  void GenBranchOffsetQuery::GetResults(py::object& rPyObject) const
  {
    py::tuple result_tuple = py::tuple(3);
    result_tuple[0] = py::int_(mOffset);
    result_tuple[1] = py::int_(mValid);
    result_tuple[2] = py::int_(mHalfWords);
    rPyObject = result_tuple;
  }

  void GenChoicesTreeInfoQuery::GetResults(py::object& rPyObject) const
  {
    py::dict choice_dict;
    for (auto choice_info : mChoicesInfo) {
      choice_dict[choice_info.first.c_str()] = py::int_(choice_info.second);
    }
    rPyObject =  choice_dict;
  }

  void GenExceptionsHistoryQuery::GetResults(py::object& rPyObject) const
  {
    py::list resultant;

    for (auto error_record : (mErrorHistory)) {
      py::tuple record = py::tuple(5);

      record[0] = py::int_((uint32) error_record.exception_code);
      record[1] = py::int_((uint64) error_record.pc_value);
      record[2] = py::int_((uint32) error_record.tgt_exception_level);
      record[3] = py::int_((uint32) error_record.src_exception_level);
      record[4] = py::int_((uint32) error_record.dfsc_ifsc_code);

      resultant.append(record);
    }

    rPyObject = resultant;

  }

  void GetVmContextDeltaQuery::GetResults(py::object& rPyObject) const
  {
    py::dict ret_dict;

    for (const auto & x : mDeltaMap)
    {
      ret_dict[x.first.c_str()] = py::int_(x.second);
    }

    rPyObject = ret_dict;
  }

  void VmCurrentContextQuery::GetResults(py::object& rPyObject) const
  {
    rPyObject = py::int_(mVmCurrentContext);
  }

  void GenHandlerSetMemoryQuery::GetResults(py::object& rPyObject) const
  {
    py::tuple result_tuple = py::tuple(2);
    result_tuple[0] = py::int_(mMemoryBase);
    result_tuple[1] = py::int_(mMemorySize);
    rPyObject = result_tuple;
  }

  void GenExceptionVectorBaseAddressQuery::GetResults(py::object& rPyObject) const
  {
    rPyObject = py::int_(mVectorBaseAddress);
  }

  void GenResourceEntropyQuery::GetResults(py::object& rPyObject) const
  {
    py::dict ret_dict;

    py::dict source_dict;
    source_dict["State"] = py::str(EEntropyStateType_to_string(mSourceEntropy.mState));
    source_dict["Entropy"] = py::int_(mSourceEntropy.mEntropy);
    source_dict["OnThreshold"] = py::int_(mSourceEntropy.mOnThreshold);
    source_dict["OffThreshold"] = py::int_(mSourceEntropy.mOffThreshold);

    py::dict dest_dict;
    dest_dict["State"] = py::str(EEntropyStateType_to_string(mDestEntropy.mState));
    dest_dict["Entropy"] = py::int_(mDestEntropy.mEntropy);
    dest_dict["OnThreshold"] = py::int_(mDestEntropy.mOnThreshold);
    dest_dict["OffThreshold"] = py::int_(mDestEntropy.mOffThreshold);

    ret_dict["Source"] = source_dict;
    ret_dict["Dest"] = dest_dict;

    rPyObject = ret_dict;
  }

  void GenRestoreLoopContextQuery::GetResults(py::object& rPyObject) const
  {
    py::tuple result_tuple = py::tuple(3);
    result_tuple[0] = py::int_(mLoopId);
    result_tuple[1] = py::int_(mLoopBackAddress);
    result_tuple[2] = py::int_(mBranchRegIndex);
    rPyObject = result_tuple;
  }

}
