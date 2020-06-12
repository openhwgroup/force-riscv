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
#include <MemoryReservation.h>
#include <Constraint.h>
#include <VmMapper.h>
#include <Log.h>

#include <sstream>

using namespace std;

namespace Force {

  MemoryReservation::MemoryReservation(const std::string& name)
    : mName(name), mInitialConstraints(), mCurrentConstraints()
  {
    mInitialConstraints.assign(EMemBankTypeSize, nullptr);
    mCurrentConstraints.assign(EMemBankTypeSize, nullptr);
  }

  MemoryReservation::~MemoryReservation()
  {
    for (auto init_item : mInitialConstraints) {
      delete init_item;
    }

    for (auto curr_item : mCurrentConstraints) {
      delete curr_item;
    }
  }

  void MemoryReservation::BankOutOfBound(EMemBankType memBank) const
  {
    LOG(fail) << "{MemoryReservation::BankOutOfBound} memory bank: " << EMemBankType_to_string(memBank) << " out of bound." << endl;
    FAIL("memory-bank-out-of-bound");
  }

  void MemoryReservation::AddConstraint(const std::string& range, EMemBankType memBank)
  {
    CheckBank(memBank);

    ConstraintSet* existing_constr = mInitialConstraints[int(memBank)];
    if (nullptr == existing_constr) {
      ConstraintSet* new_constr = new ConstraintSet(range);
      mInitialConstraints[int(memBank)] = new_constr;
    } else {
      // TODO to implement.
      LOG(fail) << "{MemoryReservation::AddConstraint} adding constraint string to existing constraint set not yet supported." << endl;
      FAIL("add-constraint-operation-not-supported");
    }
  }

  void MemoryReservation::AddVirtualConstraint(const std::string& range, const VmMapper* pVmMapper)
  {
    // TODO to implement.
    LOG(fail) << "{MemoryReservation::AddVirtualConstraint} method not yet implemented." << endl;
    FAIL("method-not-yet-implemented");
  }

  void MemoryReservation::AddRange(uint64 start, uint64 size, EMemBankType memBank)
  {
    CheckBank(memBank);

    ConstraintSet* existing_constr = mInitialConstraints[int(memBank)];
    uint64 end = start + (size - 1);
    if (end < start) {
      LOG(fail) << "{MemoryReservation::AddRange} adding range with size that will cause wrap-around." << endl;
      FAIL("add-range-with-wrap-around");
    }

    if (nullptr == existing_constr) {
      ConstraintSet* new_constr = new ConstraintSet(start, end);
      mInitialConstraints[int(memBank)] = new_constr;
    } else {
      existing_constr->AddRange(start, end);
    }
  }

  void MemoryReservation::AddVirtualRange(uint64 start, uint64 size, const VmMapper* pVmMapper)
  {
    // TODO not a complete solution, need to consider case when the VA ranges is translated into non-continuous physical ranges,
    uint64 pa = 0;
    uint32 bank = 0;
    auto trans_result = pVmMapper->TranslateVaToPa(start, pa, bank);
    if (trans_result != ETranslationResultType::Mapped) {
      LOG(fail) << "{MemoryReservation::AddVirtualRange} failed to translate address 0x" << hex << start << " state: " << ETranslationResultType_to_string(trans_result) << endl;
      FAIL("failed-translating-address");
    }

    AddRange(pa, size, EMemBankType(bank));
  }

  const ConstraintSet* MemoryReservation::GetReservation(EMemBankType memBank) const
  {
    CheckBank(memBank);

    auto current_constr = mCurrentConstraints[int(memBank)];
    if (nullptr != current_constr) {
      return current_constr;
    }

    auto init_constr = mInitialConstraints[int(memBank)];
    if (nullptr != init_constr) {
      auto new_curr_constr = init_constr->Clone();
      mCurrentConstraints[int(memBank)] = new_curr_constr;
      return new_curr_constr;
    }

    return nullptr;
  }

  const std::string MemoryReservation::ToString() const
  {
    stringstream out_str;

    out_str << "MemoryReservation: " << mName;
    // The assumption here is there is always initial constraint, but not always current constraint.
    EMemBankTypeBaseType bank_value = EMemBankTypeBaseType(0);
    for (; bank_value < EMemBankTypeSize; ++ bank_value) {
      ConstraintSet* init_constr = mInitialConstraints[int(bank_value)];
      ConstraintSet* curr_constr = mCurrentConstraints[int(bank_value)];
      if (nullptr != init_constr) {
        EMemBankType bank_type = EMemBankType(bank_value);
        out_str << " [" << EMemBankType_to_string(bank_type) << "] Initial: " << init_constr->ToSimpleString();
      }

      if (nullptr != curr_constr) {
        out_str << " Current: " << curr_constr->ToSimpleString();
      }
    }

    return out_str.str();
  }

}
