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
#ifndef Force_PcSpacing_H
#define Force_PcSpacing_H

#include <Defines.h>
#include <vector>

namespace Force {

  class Generator;
  class ConstraintSet;
  class VmMapper;
  class GenPC;

  /*!
    \class PcSpacing
    \brief Consider PC vicinity spacing from all PEs when generating instruction or data targets
  */

  class PcSpacing {
  public:
    static void Initialize();  //!< Initialization interface.
    static void Destroy();     //!< Destruction clean up interface.
    inline static PcSpacing* Instance() { return mspPcSpacing; } //!< Access PcSpacing instance.
    void SignUp(const Generator* pGen); //!< Generator signup with the PC spacing module.
    const ConstraintSet* GetPcSpaceConstraint(); //!< Get PC spaces constraint.
    const ConstraintSet* GetBranchPcSpaceConstraint(const VmMapper* pVmMapper, uint32 instrSize); //!< Get PC spaces constraint for branch instruction.
  private:
    PcSpacing(); //!< Default constructor.
    COPY_CONSTRUCTOR_ABSENT(PcSpacing);
    ~PcSpacing(); //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(PcSpacing);
    void AugmentConstraint(cuint64 pcValue, cuint32 instrSpace, ConstraintSet* constr); //!< Add the range starting at pcValue of length instrSpace to the specified constraint.
  private:
    ConstraintSet* mpPcConstraint; //!< Pointer to a PC spacing constraint object for normal access.
    ConstraintSet* mpBranchPcConstraint; //!< Pointer to a PC spacing constraint object for own branch access.
    std::vector<const Generator* > mGenerators; //!< List of generators with distinct PCs.
    static PcSpacing* mspPcSpacing; //!< Static pointer to PcSpacing object.
  };

}

#endif
