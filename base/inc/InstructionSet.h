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
#ifndef Force_InstructionSet_H
#define Force_InstructionSet_H

#include <Defines.h>
#include <string>
#include <map>
#include <vector>

namespace Force {

  class InstructionStructure;
  class AsmText;
  class ArchInfo;

  /*!
    \class InstructionSet
    \brief Architectural instruction container.
  */

  class InstructionSet {
  public:
    InstructionSet(); //!< Constructor.
    virtual ~InstructionSet(); //!< Destructor.

    ASSIGNMENT_OPERATOR_ABSENT(InstructionSet);
    COPY_CONSTRUCTOR_DEFAULT(InstructionSet);
    void Setup(const ArchInfo& archInfo);//!< Setup InstructionSet container, load instruction files.
    const InstructionStructure* LookUpById(const std::string& instrName) const; //!< Look up InstructionStructure object by instruction ID.
    void Dump() const; //!< Dump InstructionStructure object information.
  private:
    void AddInstruction(InstructionStructure* instr_struct); //!< Add a new instance of InstructionStructure.
    virtual AsmText* AsmTextInstance() const; //<! Return an AsmText object
  private:
    std::map<std::string, InstructionStructure* > mInstructionSet; //!< Map containing all instructions.
    const ArchInfo* mpArchInfo;
    friend class InstructionParser;
  };

}

#endif
