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
#include <BootOrderRISCV.h>
#include <Register.h>
#include <GenRequest.h>
#include <Log.h>

#include <iostream>
#include <iterator>

using namespace std;

/*!
  \file BootOrderRISCV.cc
  \brief Code handling RISC-V boot ordering details.
*/

namespace Force {

  BootOrderRISCV::BootOrderRISCV(const BootOrderRISCV& rOther)
    : BootOrder(rOther)
  {

  }

  BootOrderRISCV::~BootOrderRISCV()
  {
  }

  Object * BootOrderRISCV::Clone() const
  {
    return new BootOrderRISCV(*this);
  }

  void BootOrderRISCV::InitiateRegisterList(list<Register*>& regList)
  {
    if (mRegisterList.size() > 0)
    {
      LOG(fail) << "trying to initialize mRegisterList for the second time with mRegisterList already contains: " << mRegisterList.size() << endl;
      FAIL("boot-order-reglist-alreadyexisting-fail");
    }

    for (auto reg_ptr : regList)
    {
      mRegisterList.push_back(new RegisterElement(reg_ptr));
    }
  }

  void BootOrderRISCV::AdjustOrder()
  {
    // << "{BootOrderRISCV::AdjustOrder} begin" << endl;
    map<uint32, list<BootElement*>::iterator > FPR_size_map;
    size_t num_sys_regs = 0;

    for (auto it = mRegisterList.begin(); it != mRegisterList.end();)
    {
      auto reg_ptr = (*it)->GetRegister();
      const string& reg_name = reg_ptr->Name();
      auto reg_type = reg_ptr->RegisterType();

      LOG(info)  << "{BootOrderRISCV::AdjustOrder} reg_name=" << reg_name << " reg_type=" << ERegisterType_to_string(reg_type) << endl;
      switch (reg_type)
      {
        case ERegisterType::GPR:
        {
          if (reg_name == mpLastGPR->RegisterName())
          {
            // << "{BootOrderRISCV::AdjustOrder} Last GPR name=" << reg_name << endl;
            it = mRegisterList.erase(it);
          }
          else
          {
            ++it;
          }
          break;
        }
        case ERegisterType::FPR:
        {
          uint32 index = reg_ptr->IndexValue();
          auto size_map_it = FPR_size_map.find(index);
          //For S*, D*, Q* only keep the largest one
          if (size_map_it == FPR_size_map.end()) //index not present yet
          {
            // << "{BootOrderRISCV::AdjustOrder} adding new fpr name=" << reg_name << endl;
            FPR_size_map.insert(std::pair<uint32, list<BootElement*>::iterator >(index, it++));
          }
          else //index present
          {
            //existing register larger size than new reg
            if ((*(size_map_it->second))->GetRegister()->Size() > reg_ptr->Size())
            {
              // << "{BootOrderRISCV::AdjustOrder} existing fpr larger, removing name=" << reg_name << endl;
              delete *it;
              it = mRegisterList.erase(it);
            }
            else //existing register smaller than new reg, remove smaller reg and add new reg to fpr size map
            {
              // << "{BootOrderRISCV::AdjustOrder} existing fpr smaller, adding name=" << reg_name << endl;
              mRegisterList.erase(size_map_it->second);
              FPR_size_map.erase(size_map_it);
              FPR_size_map.insert(std::pair<uint32, list<BootElement*>::iterator >(index, it++));
            }
          }
          break;
        }
        case ERegisterType::SysReg: //TODO add custom SysReg ordering/processing if necessary
        {
         // << "{BootOrderRISCV::AdjustOrder} sysreg name=" << reg_name << endl;
          //Swap any system registers to a sublist we maintain in the front
          auto sys_reg_list_end = next(mRegisterList.begin(), num_sys_regs++);
          iter_swap(it, sys_reg_list_end);

          //Since we don't want to revisit any registers, advance our place in the list as normal.
          ++it;

          break;
        }
        default: //bypass
        {
          // << "{BootOrderRISCV::AdjustOrder} default regtype name=" << reg_name << endl;
          ++it;
          break;
        }
      }
    }
  }

  uint32 BootOrderRISCV::GetLastRegistersRequestOffset()
  {
    uint32 count = 0; // instr_count(mpLastGPR->Value())<<2;

    // TODO additional offset will be calculated.

    return count;
  }

  void BootOrderRISCV::LastRegistersLoadingRequest(vector<GenRequest*>& rRequests, bool load_all)
  {
    auto last_gpr_name = mpLastGPR->RegisterName();

    // TODO potentially loading more state registers here.

    rRequests.push_back(new GenLoadRegister(last_gpr_name, mpLastGPR->Value()));
  }

  //  void BootOrderRISCV::AppendInstructionBarrier(std::vector<GenRequest*>& rRequests)
  //{
    // TODO add instruction barrier if necessary.
  //}

}
