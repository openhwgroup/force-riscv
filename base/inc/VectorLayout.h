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
#ifndef Force_VectorLayout_H
#define Force_VectorLayout_H

#include <Defines.h>

namespace Force {

  /*!
    \class VectorLayout
    \brief Struct defining the layout of vector registers within the context of a vector instruction.
  */
  struct VectorLayout {
    uint32 mElemSize; //!< The size in bits of each vector register element
    uint32 mElemCount; //!< The number of elements per vector register group
    uint32 mFieldCount; //!< The number of fields per structure in memory
    uint32 mRegCount; //!< The number of registers per vector register group
    uint32 mRegIndexAlignment; //!< A power of 2 to which vector register indices must be aligned
  };

}

#endif  // Force_VectorLayout_H
