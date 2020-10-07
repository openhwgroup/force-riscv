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
#include <Config.h>
#include <PathUtils.h>
#include <StringUtils.h>
#include <XmlTreeWalker.h>
#include <Architectures.h>
#include <Log.h>

#include <pugixml.h>

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <chrono>

using namespace std;

/*!
  \file Config.cc
  \brief Code for top level generator configuration class
*/

namespace Force {

  /*!
    \class ConfigParser
    \brief Parser class for config files.

    ConfigParser inherits pugi::xml_tree_walker.  A config file can include other base line config files.
  */
  class ConfigParser : public pugi::xml_tree_walker {
  public:
    explicit ConfigParser(Config* cfg) //!< Constructor, pass in pointer to Config object.
      : mpArchitectures(nullptr), mpConfig(cfg)
    {
      mpArchitectures = Architectures::Instance();
    }

    ~ConfigParser() { } //!< Destructor, empty.

    ASSIGNMENT_OPERATOR_ABSENT(ConfigParser);
    COPY_CONSTRUCTOR_DEFAULT(ConfigParser);

    /*!
      Handles config file elements.
     */
    virtual bool for_each(pugi::xml_node& node)
    {
      const char * node_name = node.name();
      if (
        (strcmp(node_name, "config_file") == 0) ||
        (strcmp(node_name, "instruction_files") == 0) ||
        (strcmp(node_name, "default_classes") == 0) ||
        (strcmp(node_name, "limits") == 0) ||
        (strcmp(node_name, "register_files") == 0) ||
        (strcmp(node_name, "register_classes") == 0) ||
        (strcmp(node_name, "choices_files") == 0) ||
        (strcmp(node_name, "paging_files") == 0) ||
        (strcmp(node_name, "variable_files") == 0) ||
        (strcmp(node_name, "import_files") == 0)
      ) {
        // known node names.
      }
      else if (strcmp(node_name, "instruction_file") == 0) {
        const char* file_name = node.attribute("file").value();
        mpArchitectures->DefaultArchInfo()->AddInstructionFile(file_name);
      }
      else if (strcmp(node_name, "register_file") == 0) {
        const char* file_name = node.attribute("file").value();
        mpArchitectures->DefaultArchInfo()->AddRegisterFile(file_name);
      }
      else if (strcmp(node_name, "choices_file") == 0) {
        const char* file_name = node.attribute("file").value();
        mpArchitectures->DefaultArchInfo()->AddChoicesFile(file_name);
      }
      else if (strcmp(node_name, "paging_file") == 0) {
        const char* file_name = node.attribute("file").value();
        mpArchitectures->DefaultArchInfo()->AddPagingFile(file_name);
      }
      else if (strcmp(node_name, "variable_file") == 0) {
        const char* file_name = node.attribute("file").value();
        mpArchitectures->DefaultArchInfo()->AddVariableFile(file_name);
      }
      else if (strcmp(node_name, "simulator_api_module") == 0) {
        const char* file_name = node.attribute("file").value();
        // this if string.empty() is to make sure not to override the SimulatorApiModule name if already defined.
        if (mpArchitectures->DefaultArchInfo()->SimulatorApiModule().empty()) {
          mpArchitectures->DefaultArchInfo()->SetSimulatorApiModule(file_name);
        }
      }
      else if (strcmp(node_name, "simulator_shared_object") == 0) {
        const char* file_name = node.attribute("file").value();
        // this if string.empty() is to make sure not to override the SimulatorDLL name if already defined.
        if (mpArchitectures->DefaultArchInfo()->SimulatorDLL().empty()) {
          mpArchitectures->DefaultArchInfo()->SetSimulatorDLL(file_name);
        }
      }
      else if (strcmp(node_name, "simulator_config_string") == 0) {
        const char* file_name = node.attribute("value").value();
        // this if string.empty() is to make sure not to override the Simulator config string if already defined.
        if (mpArchitectures->DefaultArchInfo()->SimulatorConfigString().empty()) {
          mpArchitectures->DefaultArchInfo()->SetSimulatorConfigString(file_name);
        }
	bool rv32 = std::string(file_name).find("RV32") != std::string::npos;
	mpConfig->SetGlobalStateValue(EGlobalStateType::RV32, rv32);
      }
      else if (strcmp(node_name, "simulator_standalone") == 0) {
        const char* file_name = node.attribute("file").value();
        mpArchitectures->DefaultArchInfo()->SetSimulatorStandalone(file_name);
      }
      else if (strcmp(node_name, "default_class") == 0) {
        process_default_class(node);
      }
      else if (strcmp(node_name, "register_class") == 0) {
        process_default_class(node);
      }
      else if (strcmp(node_name, "limit") == 0) {
        process_limit(node);
      }
      else if (strcmp(node_name, "memory_file") == 0) {
        mpConfig->mMemoryFile = node.attribute("file").value();
      }
      else if (strcmp(node_name, "bnt_file") == 0) {
        mpConfig->mBntFile = node.attribute("file").value();
      }
      else if (strcmp(node_name, "choices_mod_file") == 0) {
        mpConfig->mChoicesModificationFile = node.attribute("file").value();
      }
      else if (strcmp(node_name, "import_file") == 0) {
        mpConfig->mImportFiles.push_back(node.attribute("file").value());
      }
      else {
        ostringstream oss;
        node.print(oss);
        LOG(fail) << "Unknown config file node name \'" << node_name << "\':" << endl;
        std::cout << "failed node is " << oss.str() << std::endl;
        FAIL("config-unknown-node");
      }

      return true; // continue traversal
    }


    void process_default_class(pugi::xml_node& node)
    {
      const char* category = node.attribute("category").value();
      if (strcmp(category, "instruction") == 0) {
        mpArchitectures->DefaultArchInfo()->mDefaultIClass = node.attribute("class").value();
      }
      else if (strcmp(category, "operand") == 0) {
        const char* opr_type = node.attribute("type").value();
        mpArchitectures->DefaultArchInfo()->mDefaultOperandClasses[opr_type] = node.attribute("class").value();
      }
      else if (strcmp(category, "pte") == 0) {
        mpArchitectures->DefaultArchInfo()->mDefaultPteClass = node.attribute("class").value();
      }
      else if (strcmp(category, "pte_attribute") == 0) {
        mpArchitectures->DefaultArchInfo()->mDefaultPteAttributeClass = node.attribute("class").value();
      }
      else if (strcmp(category, "register") == 0) {
        mpArchitectures->DefaultArchInfo()->mRegisterClasses[category] = node.attribute("class").value();
      }
      else if (strcmp(category, "physical_register") == 0) {
        mpArchitectures->DefaultArchInfo()->mRegisterClasses[category] = node.attribute("class").value();
      }
      else if (strcmp(category, "register_file") == 0) {
        mpArchitectures->DefaultArchInfo()->mRegisterClasses[category] = node.attribute("class").value();
      }
      else if (strcmp(category, "register_field") == 0) {
        mpArchitectures->DefaultArchInfo()->mRegisterClasses[category] = node.attribute("class").value();
      }
      else if (strcmp(category, "bit_field") == 0) {
        mpArchitectures->DefaultArchInfo()->mRegisterClasses[category] = node.attribute("class").value();
      }
      else {
        LOG(fail) << "Unsupported default class category \'" << category << "\'." << endl;
        FAIL("unsupported-default-class-category");
      }
    }

    void process_limit(pugi::xml_node& node)
    {
      const char* name = node.attribute("name").value();
      const char* value = node.attribute("value").value();
      ELimitType elimit_type = string_to_ELimitType(name);
      mpConfig->mLimits[elimit_type] = parse_uint64(value);
    }

  private:
    Architectures* mpArchitectures; //!< Pointer to Architectures object.
    Config* mpConfig;  //!< Pointer to Config object.
  };

  Config* Config::mspConfig = nullptr;

  void Config::Initialize()
  {
    if (nullptr == mspConfig) {
      mspConfig = new Config();
    }
  }

  void Config::Destroy()
  {
    delete mspConfig;
    mspConfig = nullptr;
  }

  /*!
    Initial setup, including setup main path "FORCE_PATH", which is the path to test generator supporting files.

    The "FORCE_PATH" environment variable is generally setup by the verification work flow scripts/code.  If it is not set, we will try to use the current working directory.
   */
  void Config::Setup(const std::string& programPath)
  {
    using namespace std;
    const char * fp_env = getenv("FORCE_PATH");
    if (!fp_env) {
      string bin_path = get_parent_path(programPath);
      mMainPath = get_parent_path(bin_path);
    } else {
      mMainPath = fp_env;
    }
    LOG(notice) << "FORCE_PATH = " << mMainPath << endl;
  }

  void Config::LoadConfigFile(const string& filePath, const std::string& programPath)
  {
    Setup(programPath);
    ConfigParser cfg_parser(this);

    // We always load cmd line config first. It defines where the baseconfig is.
    string full_file_path = LookUpFile(filePath);
    mConfigFile = full_file_path;
    parse_xml_file(full_file_path, "config", cfg_parser);

    // We then load all ImportFiles
    while ( not mImportFiles.empty()) {
      auto import_file_path = mImportFiles.front();
      string full_import_file_path = LookUpFile(import_file_path);
      parse_xml_file(full_import_file_path, "config", cfg_parser);
      mImportFiles.pop_front();
    }
  }

  const std::string Config::LookUpFile(const std::string& filePath) const
  {
    if (filePath.size() == 0) {
      LOG(fail) << "File name empty." << endl;
      FAIL("look-up-empty-file-name");
    }

    if (filePath[0] == '/') {
      verify_file_path(filePath);
      return filePath;
    }

    string test_path = mMainPath + "/" + filePath;
    verify_file_path(test_path);
    return test_path;
  }

  uint64 Config::LimitValue(ELimitType limitType) const
  {
    auto find_iter = mLimits.find(limitType);
    if (find_iter == mLimits.end()) {
      LOG(fail) << "Limit type \"" << ELimitType_to_string(limitType) << "\" not defined." << endl;
      FAIL("unsupported-limit-type");
    }
    return find_iter->second;
  }

  void Config::VerifyPeNumber(uint64 peNum, ELimitType limitType, const string& numType) const
  {
    uint64 num_limit = LimitValue(limitType);
    if (peNum < 1 || peNum > num_limit) {
      LOG(fail) << "Number of " << numType << " specified (" << peNum << ") out of bound, should be in [1, " << dec << num_limit << "] range." << endl;
      FAIL("pe-number-out-of-bound");
    }
  }

  void Config::SetNumChips(uint64 numChips)
  {
    VerifyPeNumber(numChips, ELimitType::ChipsLimit, "chips");
    mNumChips = numChips;
  }

  void Config::SetNumCores(uint64 numCores)
  {
    VerifyPeNumber(numCores, ELimitType::CoresLimit, "cores");
    mNumCores = numCores;
  }

  void Config::SetNumThreads(uint64 numThreads)
  {
    VerifyPeNumber(numThreads, ELimitType::ThreadsLimit, "threads");
    mNumThreads = numThreads;
  }

  uint64 Config::MaxInstructions() const
  {
    if (mMaxInstructions) {
      return mMaxInstructions;
    }
    else {
      return LimitValue(ELimitType::MaxInstructions);
    }
  }

  uint64 Config::MaxVectorLen() const
  {
    uint64 len_limit = LimitValue(ELimitType::MaxPhysicalVectorLen);
    if (len_limit % 128 != 0) {
      LOG(fail) << "MaxVectorLenLimit: configured value (" << len_limit << ") is not an integer multiple of 128!"<< endl;
      FAIL("invalid-max-vector-len");
    }
    return len_limit;
  }

  bool Config::ParseOptions(const std::string& optionsString)
  {
    StringSplitter ss(optionsString, ',');
    while (!ss.EndOfString()) {
      string sub_str = ss.NextSubString();
      if (not ParseOption(sub_str))
        return false;
    }

    return true;
  }

  bool Config::ParseOption(const std::string& optString)
  {
    string var, val;
    if (not parse_assignment(optString, var, val)) {
      return false;
    }

    SetOption(var, val);
    return true;
  }

  void Config::SetOption(const std::string& optName, const std::string& optValue)
  {
    bool has_error = false;
    uint64 opt_val = parse_uint64(optValue, &has_error);
    if (! has_error) {
      mOptionValues[optName] = opt_val;
    }
    else {
      mOptionStrings[optName] = optValue;
    }
  }

  uint64 Config::GetOptionValue(const string& optName, bool& valid) const
  {
    auto opt_finder = mOptionValues.find(optName);

    if (opt_finder != mOptionValues.end()) {
      valid = true;
      return opt_finder->second;
    } else {
      valid = false;
      return 0;
    }
  }

  const string Config::GetOptionString(const string& optName, bool& valid) const
  {
    auto opt_finder = mOptionStrings.find(optName);

    if (opt_finder != mOptionStrings.end()) {
      valid = true;
      return opt_finder->second;
    } else {
      valid = false;
      return "";
    }
  }

  void Config::SetGlobalStateValue(EGlobalStateType globalStateType, uint64 value)
  {
    mGlobalStateValues[globalStateType] = value;
  }

  void Config::SetGlobalStateString(EGlobalStateType globalStateType, const std::string &str)
  {
    mGlobalStateStrings[globalStateType] = str;
  }

  uint64 Config::GlobalStateValue(EGlobalStateType globalStateType, bool& exists) const
  {
    exists = false;
    auto state_iter = mGlobalStateValues.find(globalStateType);
    if (state_iter != mGlobalStateValues.end()) {
      exists = true;
      return state_iter->second;
    }

    return 0;
  }

  uint64 Config::GetGlobalStateValue(EGlobalStateType globalStateType) const
  {
    auto state_iter = mGlobalStateValues.find(globalStateType);
    if (state_iter == mGlobalStateValues.end()) {
      LOG(fail) << "{Config::GetGlobalStateValue} global state \"" << EGlobalStateType_to_string(globalStateType) << "\" not set." << endl;
      FAIL("required-global-state-not-set");
    }

    return state_iter->second;
  }

  const std::string Config::GlobalStateString(EGlobalStateType globalStateType, bool& exists) const
  {
    exists = false;
    auto state_iter = mGlobalStateStrings.find(globalStateType);
    if (state_iter != mGlobalStateStrings.end()) {
      exists = true;
      return state_iter->second;
    }

    return "";
  }

  const std::string Config::HeadOfImage() const
  {
    stringstream ss;
    ss << "$TYPE: Force Generator Test File" << std::endl;
    Architectures * arch_top = Architectures::Instance();
    ss << "$Command Line: " << mCommandLine << std::endl;
    ss << "$Simulator DLL: " << arch_top->DefaultArchInfo()->SimulatorDLL() << std::endl;
    std::time_t t = time(0);
    char now[64] = {0};
    strftime(now, sizeof(now) - 1, "%Y-%m-%d %H:%M:%S", localtime(&t));
    ss << "$Time Stamp: " << now << std::endl;
    ss << "# FORCE Path: " << mMainPath << std::endl;
    ss << "# Config File: " << mConfigFile << std::endl;
    return ss.str();
  }

}
