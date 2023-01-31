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
#ifndef Force_TestIO_H
#define Force_TestIO_H

#include <map>

#include "Defines.h"

namespace Force {

  class Memory;
  class SymbolManager;
  class TestImage;
  class Generator;

  /*!
    \class TestIO
    \brief A Test IO model to allow a test memory image to be written to or from disk in ELF format.
  */
  class TestIO {
  public:
  TestIO(uint32 memBank, const Memory* pMem, const SymbolManager* pSymManager, bool createImage = true);
    ~TestIO();
    ASSIGNMENT_OPERATOR_ABSENT(TestIO);
    COPY_CONSTRUCTOR_ABSENT(TestIO);
  void WriteTestElf(const std::string& elfFilePath, bool bigEndian, uint64 entry, uint32 machineType); //!< write in-memory test image to the specified file
  void ReadTestElf(const std::string& elfFilePath, bool& bigEndian, uint64& entry, uint32 machineType);    //!< populate a memory object from the contents of an ELF file.
#ifndef UNIT_TEST
    void WriteTestAssembly(const std::map<uint32, Generator *>& generators, const std::string& disasmFilePath);//!< disassemble each instructions in-memory test image to the specified file
#endif

#ifdef UNIT_TEST
    unsigned CountSections(void);  //!< Count how many sections the Test image have
#endif
  private:
    uint32 mMemoryBank;
    const Memory* mpMemory; //!< Pointer to memory object.
    const SymbolManager* mpSymbolManager; //!< Pointer to symbol manager object.
    TestImage *mpTestImage;
};

}

#endif
