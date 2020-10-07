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
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>

#include <Defines.h>
#include <XmlTreeWalker.h>
#include <pugixml.h>
#include <PathUtils.h>
#include <Log.h>

#include <ConfigFPIX.h>

using namespace std;

/*!
  \file Config.cc
  \brief Code for ISS Scripting Interface base level config class
*/

namespace Force 
{

/*!
  \class ConfigParser
  \brief Parser class for config files.

  ConfigParser inherits pugi::xml_tree_walker.
*/
class ConfigParserFPIX : public pugi::xml_tree_walker 
{
    public:
        ConfigParserFPIX(ConfigFPIX* cfg) //!< Constructor, pass in pointer to Config object.
          : _mpConfig(cfg)
        {
        }
      
        ~ConfigParserFPIX() { } //!< Destructor, empty.
      
        ASSIGNMENT_OPERATOR_ABSENT(ConfigParserFPIX);
        COPY_CONSTRUCTOR_DEFAULT(ConfigParserFPIX);
      
        /*!
          Handles config file elements.
         */
        virtual bool for_each(pugi::xml_node& arNode)
        {
            const char * node_name = arNode.name();
            if ( (strcmp(node_name, "config") == 0) || (strcmp(node_name, "simulator_options") == 0) || (strcmp(node_name, "plugins") == 0) )
            {
                // known node names.
              //std::cout << "node: '" << node_name << "'" << std::endl;
            }
            else if (strcmp(node_name, "option") == 0) 
            {
                process_option(arNode);
            }
            else if (strcmp(node_name, "plugin") == 0) 
            {
                process_plugin(arNode);
            }
            else if (strcmp(node_name, "simulator_shared_object") == 0) 
            {
                const char *simulator_so_file = arNode.attribute("file").value();
                string full_file_path = _mpConfig->LookUpFile(simulator_so_file);
                _mpConfig->mSimulatorSharedObjectFile = full_file_path;
                LOG(info) << "   Simulator shared object: " << full_file_path << std::endl;
            }
	    else if (strcmp(node_name, "simulator_config_string") == 0)
            {
                const char *simulator_cfg_str = arNode.attribute("value").value();
                _mpConfig->mSimulatorConfigString = simulator_cfg_str;
                LOG(info) << "   Simulator config string: " << _mpConfig->mSimulatorConfigString << std::endl;
            }
            else
            {
                ostringstream oss;
                arNode.print(oss);
                LOG(fail) << "Unknown config file node name \'" << node_name << "\':" << endl;
                LOG(fail) << "failed node is " << oss.str() << std::endl;
                FAIL("config-unknown-node");
            }
      
            return true; // continue traversal
        }
      
        void process_plugin(pugi::xml_node& arNode)
        {
            const char* plugin_name = arNode.attribute("name").value();
            const char* file_name = arNode.attribute("file").value();
            const char* plugins_options = arNode.attribute("plugins_options").value();
            string full_file_path = _mpConfig->LookUpFile(file_name);
            Force::plugin_node px(plugin_name, full_file_path.c_str(), plugins_options);
            _mpConfig->mPlugins.push_back(px);
            _mpConfig->mPluginNodesMap.insert(std::pair<std::string, Force::plugin_node>(plugin_name, px));
            LOG(info) << "   Plugin shared object: " << full_file_path << std::endl;
            LOG(info) << "   Plugin options: " << plugins_options << std::endl;
        }
      
        void process_option(pugi::xml_node& arNode)
        {
            const char* name = arNode.attribute("name").value();
            const char* value = arNode.attribute("default_value").value();
      
            int ival = atoi(value);
      
            if (!strcmp(name,"railhouse")) _mpConfig->mRailhouseFile = value;      
            else if (!strcmp(name,"max_insts")) _mpConfig->mMaxInsts = ival;
            else if (!strcmp(name,"decoding")) _mpConfig->mEnableDecoding = ival;
            else if (!strcmp(name,"core_num")) _mpConfig->mNumberOfCores = ival;
            else if (!strcmp(name,"cluster_num")) _mpConfig->mClusterCount = ival;
            else if (!strcmp(name,"threads_per_cpu")) _mpConfig->mThreadsPerCpu = ival;
            else if (!strcmp(name,"pa_size")) _mpConfig->mPhysicalAddressSize = ival;
            else if (!strcmp(name,"vlen")) _mpConfig->mVectorRegLen = ival;
            else if (!strcmp(name,"elen")) _mpConfig->mMaxVectorElemWidth = ival;
            else if (!strcmp(name,"exit_loop")) _mpConfig->mExitOnBranchToSelf = ival;
            else if (!strcmp(name,"plugins_options")) _mpConfig->mGlobalPluginsOptions += value; 
            else
            {
                ostringstream oss;
                arNode.print(oss);
                LOG(fail) << "Unknown config file option: \'" << name << "\':" << endl;
                LOG(fail) << "failed node is " << oss.str() << std::endl;
                FAIL("config-unknown-node");
            }
        }
    
    private:
        ConfigFPIX* _mpConfig;  //!< Pointer to Config object.
};

void ConfigFPIX::Setup(const std::string& arProgramPath)
{
    using namespace std;
    const char * fp_env = getenv("FPIX_PATH");
    if (!fp_env) 
    {
        string bin_path = get_parent_path(arProgramPath.c_str());
        mMainPath = get_parent_path(bin_path);
    } 
    else 
    {
        mMainPath = fp_env;
    }
}

const std::string ConfigFPIX::LookUpFile(const std::string& arFilePath) const
{
    if (arFilePath.size() == 0) 
    {
        FAIL("look-up-empty-file-name");
    }

    if (arFilePath[0] == '/') 
    {
        //verify_file_path(arFilePath);
        return arFilePath;
    }

    string test_path = mMainPath + "/" + arFilePath;
    //verify_file_path(test_path);
    return test_path;
}

void ConfigFPIX::LoadConfigFile(const string& arFilePath, const std::string& arProgramPath) 
{
    if(arFilePath.find(std::string("base.config")) != std::string::npos)
    {
        LOG(info) << "   Loading base config file:\n      " << arFilePath << std::endl;
    }
    else
    {
        LOG(info) << "   Loading custom config file:\n      " << arFilePath << std::endl;
    }
  
    Setup(arProgramPath);
    ConfigParserFPIX cfg_parser(this);
  
    // We always load cmd line config first. It defines where the baseconfig is.
    string full_file_path = LookUpFile(arFilePath);
    mConfigFile = full_file_path;
  
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(mConfigFile.c_str());
  
    if (result) 
    {
        doc.traverse(cfg_parser);
    } 
    else 
    {
        LOG(fail) << "Failed parsing config file \"" << full_file_path << "\".  Error description: " << result.description() << endl;
    }
}
}
