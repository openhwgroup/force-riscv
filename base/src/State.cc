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
#include <State.h>

#include <Generator.h>
#include <Log.h>
#include <Register.h>
#include <StateElement.h>
#include <UtilityFunctions.h>

#include <algorithm>

/*!
  \file State.cc
  \brief Code supporting collections of values relevant to the state of the simulation.
*/

using namespace std;

namespace Force {

  State::State(const Generator* pGenerator)
    : mStateElems(), mDuplicateMode(EStateElementDuplicateMode::Fail), mpGenerator(pGenerator)
  {
  }

  State::~State()
  {
    for (StateElement* state_elem : mStateElems) {
      delete state_elem;
    }
  }

  void State::AddMemoryStateElement(cuint64 memStartAddr, cuint64 memSize, cuint64 memVal, cuint32 priority)
  {
    if ((memSize < 1) or (memSize > smMaxMemStateElemSize)) {
      LOG(fail) << "{State::AddMemoryStateElement} must specify at least 1 and no more than " << smMaxMemStateElemSize << " bytes" << endl;
      FAIL("unsupported-number-of-bytes");
    }

    // Create a mask indicating which bytes are specified
    uint64 mask = 0;
    if (mpGenerator->IsDataBigEndian()) {
      mask = get_mask64((memSize * 8), ((smMaxMemStateElemSize - memSize) * 8));
    }
    else {
      mask = get_mask64(memSize * 8);
    }

    AddStateElement(new MemoryStateElement(memStartAddr, memVal, mask, priority));
  }

  void State::AddMemoryStateElementsAsBytes(cuint64 memStartAddr, const vector<uint8>& rMemValues, cuint32 priority)
  {
    uint64 aligned_mem_start_addr = get_aligned_value(memStartAddr, smMaxMemStateElemSize);
    uint64 offset = memStartAddr - aligned_mem_start_addr;
    uint64 mem_size = smMaxMemStateElemSize - offset;

    // Break the memory values into 8-byte aligned chunks
    uint64 bytes_used = 0;
    while (bytes_used < rMemValues.size()) {
      if (mem_size > (rMemValues.size() - bytes_used)) {
        mem_size = rMemValues.size() - bytes_used;
      }

      uint64 mem_val = 0;
      uint64 mask = 0;
      if (mpGenerator->IsDataBigEndian()) {
        mem_val = data_array_to_value_big_endian((rMemValues.data() + bytes_used), mem_size);
        mem_val >>= (offset * 8);
        mask = get_mask64((mem_size * 8), ((smMaxMemStateElemSize - mem_size - offset) * 8));
      }
      else {
        mem_val = data_array_to_value_little_endian((rMemValues.data() + bytes_used), mem_size);
        mem_val <<= (offset * 8);
        mask = get_mask64((mem_size * 8), (offset * 8));
      }

      AddStateElement(new MemoryStateElement(aligned_mem_start_addr, mem_val, mask, priority));

      bytes_used += mem_size;
      mem_size = smMaxMemStateElemSize;
      offset = 0;
      aligned_mem_start_addr += smMaxMemStateElemSize;
    }
  }

  void State::AddRegisterStateElement(const string& rRegName, const vector<uint64>& rRegValues, cuint32 priority)
  {
    const RegisterFile* reg_file = mpGenerator->GetRegisterFile();
    Register* reg = reg_file->RegisterLookup(rRegName);

    // Look up the largest containing register for this register
    Register* containing_reg = reg_file->GetContainingRegister(reg);
    if (containing_reg == nullptr) {
      containing_reg = reg;
    }

    for (RegisterField* reg_field : containing_reg->RegisterFields()) {
      bool is_vm_context_param = false;
      try_string_to_EVmContextParamType(reg_field->Name(), is_vm_context_param);

      if (is_vm_context_param) {
        // TODO(Noah): Fail here when a mechanism for identifying VM context parameter fields with
        // certainty is devised. Currently, we can determine if the field name matches, but there
        // may be multiple register fields with the same name spread across different system
        // registers.
        LOG(warn) << "{State::AddRegisterStateElement} Register " << containing_reg->Name() << " contains field " << reg_field->Name() << ", which may be a VM context parameter field. Use AddVmContextStateElement() for VM context parameter fields." << endl;
      }
    }

    uint64 value_size = 64;
    uint64 values_count = containing_reg->Size() / value_size;
    vector<uint64> values(values_count, 0);
    vector<uint64> masks(values_count, 0);
    if (containing_reg->Name() == reg->Name()) {
      values = rRegValues;
      fill(masks.begin(), masks.end(), MAX_UINT64);
    }
    else {
      // The specified register and containing register are different, so we need to determine which
      // bits of the containing register are represented by the specified register and build the
      // masks with those bits set.
      set<PhysicalRegister*> phys_registers;
      reg->GetPhysicalRegisters(phys_registers);

      for (PhysicalRegister* phys_reg : phys_registers) {
        uint64 mask = reg->GetPhysicalRegisterMask(*phys_reg);
        masks[phys_reg->SubIndexValue()] = mask;
        values[phys_reg->SubIndexValue()] = rRegValues[phys_reg->SubIndexValue()] << lowest_bit_set(mask);
      }
    }

    ERegisterType reg_type = containing_reg->RegisterType();
    auto state_elem = new RegisterStateElement(GetRegisterStateElementType(reg_type), containing_reg->Name(), containing_reg->IndexValue(), values, masks, priority);
    AddStateElement(state_elem);
  }

  void State::AddSystemRegisterStateElementByField(const std::string& rRegName, const std::string& rRegFieldName, cuint64 regFieldVal, cuint32 priority)
  {
    bool is_vm_context_param = false;
    try_string_to_EVmContextParamType(rRegFieldName, is_vm_context_param);
    if (is_vm_context_param) {
      // TODO(Noah): Fail here when a mechanism for identifying VM context parameter fields with
      // certainty is devised. Currently, we can determine if the field name matches, but there may
      // be multiple register fields with the same name spread across different system registers.
      LOG(warn) << "{State::AddSystemRegisterStateElementByField} Register field " << rRegFieldName << " may be a VM context parameter field. Use AddVmContextStateElement() for VM context parameter fields." << endl;
    }

    const RegisterFile* reg_file = mpGenerator->GetRegisterFile();
    Register* reg = reg_file->RegisterLookup(rRegName);

    RegisterField* reg_field = reg->RegisterFieldLookup(rRegFieldName);
    uint64 mask = reg_field->FieldMask();
    uint64 value = regFieldVal << reg_field->Lsb();

    auto state_elem = new RegisterStateElement(EStateElementType::SystemRegister, reg->Name(), reg->IndexValue(), {value}, {mask}, priority);
    AddStateElement(state_elem);
  }

  void State::AddVmContextStateElement(const std::string& rRegName, const std::string& rRegFieldName, cuint64 regFieldVal, cuint32 priority)
  {
    bool is_vm_context_param = false;
    try_string_to_EVmContextParamType(rRegFieldName, is_vm_context_param);
    if (not is_vm_context_param) {
      LOG(fail) << "{State::AddVmContextStateElement} Register field " << rRegFieldName << " is not a VM context parameter field." << endl;
      FAIL("unexpected-register-field");
    }

    AddStateElement(new VmContextStateElement(rRegName, rRegFieldName, regFieldVal, priority));
;
  }

  void State::AddPrivilegeLevelStateElement(const EPrivilegeLevelType privLevel, cuint32 priority)
  {
    AddStateElement(new PrivilegeLevelStateElement(privLevel, priority));
;
  }

  void State::AddPrivilegeLevelStateElementByName(const string& rPrivLevelName, cuint32 priority)
  {
    EPrivilegeLevelType priv_level = string_to_EPrivilegeLevelType(rPrivLevelName);
;
    AddPrivilegeLevelStateElement(priv_level, priority);
  }

  void State::AddPcStateElement(cuint64 pcVal, cuint32 priority)
  {
    AddStateElement(new PcStateElement(pcVal, priority));
  }

  void State::AddStateElement(StateElement* pStateElem)
  {
    // Determine whether the new StateElement is a duplicate
    auto itr = find_if(mStateElems.begin(), mStateElems.end(),
      [pStateElem](StateElement* pOldStateElem) { return pOldStateElem->IsDuplicate(*pStateElem); });

    if (itr == mStateElems.end()) {
      mStateElems.push_back(pStateElem);
    }
    else {
      StateElement* old_state_elem = *itr;

      if (old_state_elem->CanMerge(*pStateElem)) {
        old_state_elem->Merge(*pStateElem);
        delete pStateElem;
      }
      else {
        switch (mDuplicateMode) {
        case EStateElementDuplicateMode::Fail:
          LOG(fail) << "{State::AddStateElement} StateElement " << pStateElem->GetName() << " is a duplicate of " << old_state_elem->GetName() << endl;
          FAIL("duplicate-state-element");
          break;
        case EStateElementDuplicateMode::Replace:
          mStateElems.erase(itr);
          delete old_state_elem;
          old_state_elem = nullptr;
          mStateElems.push_back(pStateElem);
          break;
        case EStateElementDuplicateMode::Ignore:
          delete pStateElem;
          break;
        default:
          LOG(fail) << "{State::AddStateElement} unknown duplicate mode " << EStateElementDuplicateMode_to_string(mDuplicateMode);
          FAIL("unknown-duplicate-mode");
        }
      }
    }
  }

  EStateElementType State::GetRegisterStateElementType(const ERegisterType regType) const
  {
    EStateElementType state_elem_type = EStateElementType::GPR;
    switch(regType) {
    case ERegisterType::GPR:
      state_elem_type = EStateElementType::GPR;
      break;
    case ERegisterType::FPR:
      state_elem_type = EStateElementType::FloatingPointRegister;
      break;
    case ERegisterType::SIMDR:
      state_elem_type = EStateElementType::VectorRegister;
      break;
    case ERegisterType::SIMDVR:
      state_elem_type = EStateElementType::VectorRegister;
      break;
    case ERegisterType::VECREG:
      state_elem_type = EStateElementType::VectorRegister;
      break;
    case ERegisterType::PREDREG:
      state_elem_type = EStateElementType::PredicateRegister;
      break;
    case ERegisterType::SysReg:
      state_elem_type = EStateElementType::SystemRegister;
      break;
    case ERegisterType::SP:
      state_elem_type = EStateElementType::SystemRegister;
      break;
    case ERegisterType::ZR:
      state_elem_type = EStateElementType::GPR;
      break;
    default:
      LOG(fail) << "{State::GetRegsiterStateElementType} unexpected register type " << ERegisterType_to_string(regType);
      FAIL("unexpected-register-type");
    }

    return state_elem_type;
  }

}
