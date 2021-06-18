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
#include <MemoryTraitsRISCV.h>

using namespace std;

namespace Force {

  MemoryTraitsRegistryRISCV::MemoryTraitsRegistryRISCV()
    : MemoryTraitsRegistry()
  {
    AddMutuallyExclusiveTraits({EMemoryAttributeType::MainRegion, EMemoryAttributeType::IORegion, EMemoryAttributeType::EmptyRegion});
    AddMutuallyExclusiveTraits({EMemoryAttributeType::MainRegion, EMemoryAttributeType::EmptyRegion, EMemoryAttributeType::LRSC});
    AddMutuallyExclusiveTraits({EMemoryAttributeType::MainRegion, EMemoryAttributeType::EmptyRegion, EMemoryAttributeType::AMONone, EMemoryAttributeType::AMOSwap, EMemoryAttributeType::AMOLogical, EMemoryAttributeType::AMOArithmetic});
    AddMutuallyExclusiveTraits({EMemoryAttributeType::MainRegion, EMemoryAttributeType::EmptyRegion, EMemoryAttributeType::AMOAligned, EMemoryAttributeType::AMOMisaligned});
    AddMutuallyExclusiveTraits({EMemoryAttributeType::IORegion, EMemoryAttributeType::EmptyRegion, EMemoryAttributeType::RVWMO, EMemoryAttributeType::RVTSO});
    AddMutuallyExclusiveTraits({EMemoryAttributeType::MainRegion, EMemoryAttributeType::EmptyRegion, EMemoryAttributeType::RelaxedOrdering, EMemoryAttributeType::StrongOrderingChannel0, EMemoryAttributeType::StrongOrderingChannel1});
    AddMutuallyExclusiveTraits({EMemoryAttributeType::EmptyRegion, EMemoryAttributeType::CoherentL1, EMemoryAttributeType::CoherentL2, EMemoryAttributeType::CoherentL3, EMemoryAttributeType::Incoherent});
    AddMutuallyExclusiveTraits({EMemoryAttributeType::EmptyRegion, EMemoryAttributeType::CacheableMasterPrivate, EMemoryAttributeType::CacheableShared, EMemoryAttributeType::CacheableSlavePrivate, EMemoryAttributeType::Uncacheable});
    AddMutuallyExclusiveTraits({EMemoryAttributeType::MainRegion, EMemoryAttributeType::EmptyRegion, EMemoryAttributeType::ReadIdempotent, EMemoryAttributeType::ReadNonIdempotent});
    AddMutuallyExclusiveTraits({EMemoryAttributeType::MainRegion, EMemoryAttributeType::EmptyRegion, EMemoryAttributeType::WriteIdempotent, EMemoryAttributeType::WriteNonIdempotent});
  }

}
