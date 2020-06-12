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
#ifndef Force_SwitchPrivilegeSolverRISCV_H
#define Force_SwitchPrivilegeSolverRISCV_H

#include <Defines.h>

#include <map>
#include <string>
#include <vector>

namespace Force {

  class Generator;
  class VmMapper;

  /*!
    \struct SwitchPrivilegeResultInfo
    \brief Stores privilege switch result information.
  */
  struct SwitchPrivilegeResultInfo {
    SwitchPrivilegeResultInfo()
      : mSuccess(true), mTargetAddr(0), mIntermediateRetAddr(0), mTargetPrivLevel(4), mDataBlockPrivLevel(MAX_UINT32), mStatusVal(0), mInstrSeqCode(MAX_UINT32)
    {
    }

    bool mSuccess; //!< Success flag
    uint64 mTargetAddr; //!< Target address
    uint64 mIntermediateRetAddr; //!< Address to return to in the first step of a 2 ECALL sequence
    uint32 mTargetPrivLevel; //!< Target privilege level
    uint32 mDataBlockPrivLevel; //!< Privilege level at which the data block will be read
    uint64 mStatusVal; //! Target xstatus register value
    uint32 mInstrSeqCode; //!< Value indicating the instructions that need to be generated
  };

  /*!
    \class SwitchPrivilegeSolver
    \brief Class for solving privilege level switch scenarios.
  */
  class SwitchPrivilegeSolver {
  public:
    explicit SwitchPrivilegeSolver(Generator* pGenerator);
    COPY_CONSTRUCTOR_ABSENT(SwitchPrivilegeSolver);
    DESTRUCTOR_DEFAULT(SwitchPrivilegeSolver);
    ASSIGNMENT_OPERATOR_ABSENT(SwitchPrivilegeSolver);

    const SwitchPrivilegeResultInfo& Solve(const std::map<std::string, std::string>& rParams); //!< Handle privilege level switch request.
    const std::map<std::string, uint64>& GetVmReloadRegisters() const { return mVmReloadRegisters; } //!< Return the reload registers to update the VM context.
  private:
    void Setup(const std::map<std::string, std::string>& rParams); //!< Setup SwitchPrivilegeSolver.
    void ValidateAndGenerateTargetState(); //!< Validate and generate target privilege level and state.
    bool ValidateAndGenerateSwitchScheme(); //!< Validate and generate privilege level switch mechanism.
    void ValidateAndGenerateStatusValue(); //!< Validate and generate xstatus register value.
    void ValidateAndGenerateTargetAddress(); //!< Validate and generate the target address.
    void GenerateIntermediateReturnAddress(); //!< Generate an intermediate return address if required.
    void SetupCurrentState(); //!< Set up the current state.
    void SetupDefaultTargetState(); //!< Set up the default target state.
    void SetTargetStateValue(const std::string& rParamName, const std::string& rParamVal); //!< Set the target state value based on the specified target state parameter.
    void ValidateVmContextAndUpdate(const VmMapper* pTargetMapper); //!< Validate and update the VM context.
    VmMapper* GetVmMapper(const uint32 privLevel) const; //!< Get the VmMapper for the specified privilege level.
    bool IsTargetStateParameter(const std::string& rParamName) const; //!< Return true if the argument is a target state parameter name.
    uint32 ParseTargetStateParameterValue(const std::string& rParamVal) const; //!< Parse a target state parameter value.
    uint32 GetRegisterFieldValue(const std::string& rRegName, const std::string& rRegFieldName) const; //!< Get the current value for the specified register field.
    uint32 ComputeTargetStateValue(cuint32 currentVal, cuint32 targetVal, const std::string& rChoicesTreeName) const; //!< Determine the target state value based on the input value.
    uint32 ChooseValue(const std::string& rChoicesTreeName) const; //!< Randomly select a value from the available choices.
  private:
    Generator* mpGenerator; //!< Generator
    SwitchPrivilegeResultInfo mResult; //!< Privilege switch result
    bool mTargetAddrSpecified; //!< Flag indicating whether a target address was specified
    bool mSkipAddrValidation; //!< Flag indicating whether to skip address validation
    uint32 mAddrChoicesModId; //!< Choices modification ID for generating a target address
    uint32 mCurrentPrivLevel; //!< Current privilege level
    std::map<std::string, uint32> mCurrentState; //!< Bit values for current state
    std::map<std::string, uint32> mTargetState; //!< Bit values for target state
    std::map<std::string, uint64> mVmReloadRegisters; //!< Reload registers to update the VM context
    std::vector<std::string> mStateRegFieldNames; //!< Names of state register fields
  };

}

#endif  // Force_SwitchPrivilegeSolverRISCV_H
