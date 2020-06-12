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
#ifndef Fpix_PluginManager_H
#define Fpix_PluginManager_H

#include <map>
#include <SimAPI.h>
#include <SimPlugin.h>
#include <ConfigFPIX.h>
#include <EnumsFPIX.h>
#include <PluginInterface.h>

using namespace std;

namespace Force {

  /*!
   \class PluginManager
   \brief Simulator plugins manager class.
   */

    class SimThreadEvent;

  class PluginManager {
  public:
    PluginManager(std::vector<ESimThreadEventType> event_types);  //!< create/hook-up plugin instances, senders for a sim-thread
    ~PluginManager();                        //!< delete event senders

    ASSIGNMENT_OPERATOR_ABSENT(PluginManager);
    COPY_CONSTRUCTOR_ABSENT(PluginManager);

    static int  Initialize(std::vector<std::string> &plugin_path_set, std::vector<std::string> &plugins_cl_args); //!< load plugin shared objects
    static int  Initialize(std::vector<std::string> &plugin_path_set, std::map<std::string, Force::plugin_node> * apPluginNodesMap, std::string * apGlobalPluginsOptions); //!< load plugin shared objects
    static void Destroy();    //!< unload plugins

    //!< after shared objects have been loaded, can retreive mgr instance:
    //!<
    //!<   create an instance of each plugin, hook up senders/receivers for plugin methods

    void Signal(SimThreadEvent &pData);  //!< event 'payload' data is required...

  private:
    std::vector<SimPlugin *> mPlugins;  //!< plugin instances for a particular sim-thread
    //std::map<std::string, Force::plugin_node> * mpPluginNodesMap; //!< pointer to map by plugin name that can provide options strings that were specified in the config file.
    //std::string * mpGlobalPluginsOptions; //!< pointer to universal plugins options from config file as well as universal and specific options collected from the command line.
    std::map<ESimThreadEventType, Sender<ESimThreadEventType> *> mSenders;   //<! a separate sender entry for each sim event type. index is event type
  };

}


#endif
