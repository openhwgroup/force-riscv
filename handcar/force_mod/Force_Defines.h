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

#ifndef Force_Defines_H
#define Force_Defines_H

/*!
  \namespace Force
  \brief Application scope name space

  Namespace for the FORCE ISG code.
 */
namespace Force {

  typedef unsigned long long uint64;
  typedef unsigned int uint32;
  typedef unsigned char uint8;
  typedef long long int64;
  typedef signed int int32;
  typedef signed char int8;

  typedef const unsigned long long cuint64;
  typedef const unsigned int cuint32;
  typedef const unsigned char cuint8;
  typedef const long long cint64;
  typedef const signed int cint32;
  typedef const signed char cint8;
  typedef const bool cbool;

#define MAX_UINT32 0xffffffff
#define MAX_UINT64 0xffffffffffffffffULL
#define MAX_ENUM_SIZE 100
#define MAX_ASM_SIZE 128
#define MAX_PAGE_TABLE_LEVEL 3

#define ASSIGNMENT_OPERATOR_ABSENT(ClassName) ClassName& operator=(const ClassName&) = delete
#define ASSIGNMENT_OPERATOR_DEFAULT(ClassName) ClassName& operator=(const ClassName&) = default
#define COPY_CONSTRUCTOR_ABSENT(ClassName) ClassName(const ClassName&) = delete
#define COPY_CONSTRUCTOR_DEFAULT(ClassName) ClassName(const ClassName&) = default
#define DEFAULT_CONSTRUCTOR_ABSENT(ClassName) ClassName() = delete
#define DEFAULT_CONSTRUCTOR_DEFAULT(ClassName) ClassName() = default
#define DESTRUCTOR_DEFAULT(ClassName) ~ClassName() = default
#define SUBCLASS_DESTRUCTOR_DEFAULT(ClassName) ~ClassName() override = default
#define SUPERCLASS_DESTRUCTOR_DEFAULT(ClassName) virtual ~ClassName() = default
#define PICKY_IGNORED _Pragma("GCC diagnostic ignored \"-Weffc++\"")
#define PICKY_IGNORE_BLOCK_START _Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Weffc++\"")
#define PICKY_IGNORE_BLOCK_END _Pragma("GCC diagnostic pop")
}

#endif
