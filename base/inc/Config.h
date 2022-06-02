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
#ifndef Force_Config_H
#define Force_Config_H

#include <list>
#include <map>
#include <string>

#include "Defines.h"
#include "Enums.h"
#include ARCH_ENUM_HEADER

namespace Force {

  /*!
    \class Config
    \brief Top level test generator configuration information.
  */

  class Config {
  public:
    static void Initialize();  //!< Initialization interface.
    static void Destroy();     //!< Destruction clean up interface.
    inline static Config* Instance() { return mspConfig; } //!< Access config info instance.

    void LoadConfigFile(const std::string& filePath, const std::string& programPath); //!< Load configuration file.
    const std::string LookUpFile(const std::string& filePath) const; //!< Look up full path for file.
    void SetTestTemplate(const std::string& testTemplate) { mTestTemplate = testTemplate; } //!< Set test template file name.
    const std::string& MainPath() const { return mMainPath; }
    const std::string& TestTemplate() const { return mTestTemplate; } //!< Return the test template file name.
    const std::string& MemoryFile() const { return mMemoryFile; } //!< Return the memory file name.
    const std::string& BntFile() const { return mBntFile; } //!< Return the Bnt file
    const std::string& ChoicesModificationFile() const { return mChoicesModificationFile; } //!< return the choices modification file name
    void SetChoicesModificationFile(const std::string& modFile) { mChoicesModificationFile = modFile; }
    uint64 LimitValue(ELimitType limitType) const; //!< Return limitation value of various aspects of the design
    bool OutputAssembly() const { return mOutputAssembly; } //!< Return whether to output assembly code.
    bool OutputImage() const { return mOutputImage; } //!< Return whether to output image.
    void SetOutputAssembly(bool output) { mOutputAssembly = output; } //!< Set flag to output assembly, or not.
    void SetOutputImage(bool output) { mOutputImage = output; } //!< Set flag to output image, or not.
    bool DoSimulate() const { return mDoSimulate; } //<! Return true if each generated instruction is to be simulated.
    void SetDoSimulate(bool dosim) { mDoSimulate = dosim; } //!< Set flag to simulate each generated instruction, or not.
    bool OutputWithSeed(uint64& initialSeed) const { initialSeed = mInitialSeed; return mOutputWithSeed; } //!< return true if output with seed
    void SetOutputWithSeed(bool seed, uint64 initialSeed = 0) {mOutputWithSeed = seed; mInitialSeed = initialSeed; } //!< set flag to output with seed or not
    void SetFailOnOperandOverrides() {mFailOverrides = true; }
    bool FailOnOperandOverrides() const {return mFailOverrides; }
    void SetMaxInstructions(uint64 maxInstr) { mMaxInstructions = maxInstr; } //!< Set max instructions allowed to be simulated.
    uint64 MaxInstructions() const; //!< Return max instructions allowed to be simulated.
    inline uint64 NumPEs() const { return mNumChips * mNumCores * mNumThreads; } //!< return number of PEs
    void SetNumChips(uint64 numChips); //!< Set number of chips to simulate with.
    uint64 NumChips() const { return mNumChips; } //!< Return number of chips to simulate with.
    void SetNumCores(uint64 numCores); //!< Set number of cores per chip to simulate with.
    uint64 NumCores() const { return mNumCores; } //!< Return number of cores per chip to simulate with.
    void SetNumThreads(uint64 numThreads); //!< Set number of threads per core to simulate with.
    uint64 NumThreads() const { return mNumThreads; } //!< Return number of threads per core to simulate with.
    const std::string& IssApiTraceFile() const { return mIssApiTraceFile; } //!< Return path to simulator API trace file
    void SetIssApiTraceFile(const std::string& apitrace_file) { mIssApiTraceFile = apitrace_file; } //!< Set path to simulator API trace file
    bool ParseOptions(const std::string& optionsString); //!< Parse options string.
    void SetOption(const std::string& optName, const std::string& optValue); //!< Set option value.
    uint64 GetOptionValue(const std::string& optName, bool& valid) const; //!< Return a option value, if available.
    const std::string GetOptionString(const std::string& optName, bool& valid) const; //!< Return a option string, if available.
    void SetGlobalStateValue(EGlobalStateType globalStateType, uint64 value); //!< Set global state value for the given type.
    void SetGlobalStateString(EGlobalStateType globalStateType, const std::string &str); //!< Set global state string for the given type.
    uint64 GlobalStateValue(EGlobalStateType globalStateType, bool& exists) const; //!< Return global state value for the given type.
    uint64 GetGlobalStateValue(EGlobalStateType globalStateType) const; //!< Return global state value for the given type, fail if not found.
    const std::string GlobalStateString(EGlobalStateType globalStateType, bool& exists) const; //!< Return global state string for the given type.
    void SetCommandLine(const std::string& commandLine) { mCommandLine = commandLine; } //!< set command line.
    const std::string HeadOfImage() const; //!< return the head string of the Image file.
    uint64 MaxVectorLen() const; //!< Return max vector register length allowed to be simulated.
  private:
    Config() : mMainPath(), mTestTemplate(), mMemoryFile(), mBntFile(), mChoicesModificationFile(), mIssApiTraceFile(), mLimits(), mOptionValues(), mOptionStrings(), mGlobalStateValues(), mGlobalStateStrings(), mImportFiles(), mOutputAssembly(true), mOutputImage(false), mDoSimulate(false), mOutputWithSeed(false), mInitialSeed(0), mMaxInstructions(0), mNumChips(1), mNumCores(1), mNumThreads(1), mFailOverrides(false), mConfigFile(), mCommandLine(), mMaxVectorLen(0) { }  //!< Constructor, private.
    virtual ~Config() { } //!< Destructor, private.
    void Setup(const std::string& programPath); //!< Config object setup.
    bool ParseOption(const std::string& optString); //!< Parse option string.
    void VerifyPeNumber(uint64 peNum, ELimitType limitType, const std::string& numType) const; //!< Verify if a PE number is valid.
  private:
    static Config* mspConfig;  //!< Pointer to singleton Config object.
    std::string mMainPath;     //!< Main path to the test generator supporting files.
    std::string mTestTemplate; //!< Name of the test template file passed in through command line switch.
    std::string mMemoryFile; //!< Name of the memory file.
    std::string mBntFile; //!< Name of the Bnt file
    std::string mChoicesModificationFile; //!< name of the choices modification file
    std::string mIssApiTraceFile; //!< Name of simulator API trace file.
    std::map<ELimitType, uint64> mLimits; //!< Limitation value of various aspects of the design.
    std::map<std::string, uint64> mOptionValues; //!< Test option values.
    std::map<std::string, std::string> mOptionStrings; //!< Test option strings.
    std::map<EGlobalStateType, uint64> mGlobalStateValues; //!< Holder of value type global states.
    std::map<EGlobalStateType, std::string> mGlobalStateStrings; //!< Holder of string type global states.
    std::list<std::string> mImportFiles; //!< the container for import files
    bool mOutputAssembly; //!< Whether to output assembly code.
    bool mOutputImage; //!< Whether to output image.
    bool mDoSimulate; //!< Whether or not to simulate during test generation.
    bool mOutputWithSeed; //!< Whether to output with seed
    uint64 mInitialSeed; //!< initial seed .
    uint64 mMaxInstructions; //!< Maximum instructions allowed to be simulated.
    uint64 mNumChips; //!< Number of chips to simulate with.
    uint64 mNumCores; //!< Number of cores per chip to simulate with.
    uint64 mNumThreads; //!< Number of threads per core to simulate with.
    bool mFailOverrides; //!< whether or not to fail on invalid operand overrides
    std::string mConfigFile; //!< Full Name of the config file.
    std::string mCommandLine; //!< The command line.
    uint64 mMaxVectorLen; //!< vector register max length limit
    friend class ConfigParser;
 };
}

#endif
