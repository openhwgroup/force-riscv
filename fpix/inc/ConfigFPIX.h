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
#ifndef Fpix_Base_Config_H
#define Fpix_Base_Config_H

#include <string>
#include <vector>
#include <list>
#include <map>

#include <iostream>

using namespace std;

namespace Force {

  /*!
   \class Config
   \brief ISS Scripting Interface base level config file. Includes an 'entry' for all Command line options, supported or not.
  */

  struct plugin_node {
    plugin_node(const char*pName, const char*pFilepath) : mName(pName), mFilepath(pFilepath), mPluginsOptions("") {
    }

    plugin_node(const char*pName, const char*pFilepath, const char*pPluginsOptions) : mName(pName), mFilepath(pFilepath), mPluginsOptions(pPluginsOptions) {
    }

    const std::string Name() const { return mName; };
    const std::string Filepath() const { return mFilepath; };
    const std::string PluginsOptions() const { return mPluginsOptions; };
 
    std::string mName;
    std::string mFilepath;
    std::string mPluginsOptions;
  };

  class ConfigParserFPIX;

  class ConfigFPIX {
  public:                                    
    ConfigFPIX() :                                                                    
        mPlugins(), //!< Plugin shared object file-paths                            
        mPluginNodesMap(), //!< map from plugin names to plugin_node objects
        mGlobalPluginsOptions(""),
        mMainPath("uninitialized"), //!< Path to main program                          
        mConfigFile("uninitialized"), //!< Path to config file                                                               
        mRailhouseFile("uninitialized"), //!< Command line option is railhouse
        mSimulatorSharedObjectFile("uninitialized"), //!< Simulator shared object file
        mMaxInsts(-1),  //!< Command line option is max_insts
        mEnableDecoding(-1),  //!< Command line option is decoding
        mNumberOfCores(-1),  //!< Command line option is core_num
        mClusterCount(-1),  //!< Command line option is cluster_num
        mThreadsPerCpu(-1),  //!< Command line option is threads_per_cpu
        mPhysicalAddressSize(-1),  //!< Command line option is pa_size
        mVectorRegLen(-1),  //!< Command line option is vlen
        mMaxVectorElemWidth(-1),  //!< Command line option is elen
        mExitOnBranchToSelf(-1)  //!< Command line option is exit_loop
    {};
    virtual ~ConfigFPIX() {};

    // base class methods (NOT to be overridden):

    void Setup(const std::string& programPath); //!< Setup config search paths, etc
    void LoadConfigFile(const std::string& filePath, const std::string& programPath); //!< Load configuration file.
    const std::string LookUpFile(const std::string& filePath) const;
    const std::string &SimulatorSharedObjectFile() const { return mSimulatorSharedObjectFile; };
    void SetSimulatorSharedObjectFile(std::string &SimulatorSharedObjectFile) { mSimulatorSharedObjectFile = SimulatorSharedObjectFile; };

    bool TreatLowPowerAsNOP() const { return true; } //!< Treat low power as nop by default.
    // these methods should be common to all simulators:

    int ClusterCount() const { return mClusterCount; };
    void SetClusterCount(int ClusterCount) { mClusterCount = ClusterCount; };

    int NumberOfCores() const { return mNumberOfCores; };
    void SetNumberOfCores(int NumberOfCores) { mNumberOfCores = NumberOfCores; };

    int ThreadsPerCpu() const { return mThreadsPerCpu; };
    void SetThreadsPerCpu(int ThreadsPerCpu) { mThreadsPerCpu = ThreadsPerCpu; };

    int PhysicalAddressSize() const { return mPhysicalAddressSize; };
    void SetPhysicalAddressSize(int PhysicalAddressSize) { mPhysicalAddressSize = PhysicalAddressSize; };

    int VectorRegisterLength() const { return mVectorRegLen; };
    void SetVectorRegisterLength(int VectorRegLen) { mVectorRegLen = VectorRegLen; };

    int MaxVectorElementWidth() const { return mMaxVectorElemWidth; };
    void SetMaxVectorElementWidth(int MaxVectorElemWidth) { mMaxVectorElemWidth = MaxVectorElemWidth; };

    int MaxInsts() const { return mMaxInsts; };
    void SetMaxInsts(int MaxInsts) { mMaxInsts = MaxInsts; };

    int ExitOnBranchToSelf() const { return mExitOnBranchToSelf; };
    void SetExitOnBranchToSelf(int ExitOnBranchToSelf) { mExitOnBranchToSelf = ExitOnBranchToSelf; };


    // virtual class methods - sub-class accessor methods or remove call to OptionTBD if related feature is supported in simulator...

    void OptionTBD() const { throw std::runtime_error("Internal Error: Reference to unsupported simulator option."); };

    virtual const std::string &RailhouseFile() const { OptionTBD(); return mRailhouseFile; }; //<---sub-class or remove OptionTBD as option gets supported
    void SetRailhouseFile(std::string &RailhouseFile) { mRailhouseFile = RailhouseFile; }; //<---value defined in fpix config file

    void SetEnableDecoding(int EnableDecoding) { mEnableDecoding = EnableDecoding; };
    virtual int EnableDecoding() const { return mEnableDecoding; };

    void GetPlugins(std::vector<std::string> &paths) const {
      for (auto i = mPlugins.begin(); i != mPlugins.end(); i++) {
        paths.push_back((*i).Filepath());
      }
    };

    void GetPluginsInfo(std::map<std::string, Force::plugin_node> ** apPluginNodesMap, std::string ** apPluginsOptions) {
        *apPluginNodesMap = &mPluginNodesMap;
        *apPluginsOptions = &mGlobalPluginsOptions;
    };

    bool PluginExists(const std::string &pName, std::string &pFilepath) const {
      bool found = false;
      for (auto i = mPlugins.begin(); i != mPlugins.end(); i++) {
        if ( (*i).Name() == pName ) {
          pFilepath = (*i).Filepath();
          found = true;
          break;
        }
      }
      return found;
    };

    void AddPlugin(const char *pName, const char *pFilePath, const char* pPluginsOptions = "") {
      Force::plugin_node px(pName, pFilePath, pPluginsOptions);
      mPlugins.push_back(px);
      mPluginNodesMap.insert(std::pair<std::string, Force::plugin_node>(pName, px));
    };

    void appendPluginsOptions(std::string pPluginsOptions) {
        mGlobalPluginsOptions += std::string(",") + pPluginsOptions; 
    };

  private:

    std::vector<plugin_node> mPlugins; //!< Plugin shared object file-paths
    std::map<std::string, plugin_node> mPluginNodesMap; //!< Map from plugin name to plugin_node for that name
    std::string mGlobalPluginsOptions; //!< String containing comma separated values corresponding to options for one or all of the plugins. Combined from command line as well as standalone options nodes from the config xml file.

    std::string mMainPath; //!< Path to main program
    std::string mConfigFile; //!< Path to config file

    std::string mRailhouseFile;  //!< Command line option is railhouse
    std::string mSimulatorSharedObjectFile; //!< Simulator shared object file

    int mMaxInsts;  //!< Command line option is max_insts
    int mEnableDecoding;  //!< Command line option is decoding
    int mNumberOfCores;  //!< Command line option is core_num
    int mClusterCount;  //!< Command line option is cluster_num
    int mThreadsPerCpu;  //!< Command line option is threads_per_cpu
    int mPhysicalAddressSize;  //!< Command line option is pa_size
    int mVectorRegLen;  //!< Command line option is vlen
    int mMaxVectorElemWidth;  //!< Command line option is elen
    int mExitOnBranchToSelf; //!< Command line option is exit_loop
    
    friend class ConfigParserFPIX;
 };
}

#endif
