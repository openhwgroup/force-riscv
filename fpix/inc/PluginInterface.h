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
#ifndef Fpix_PluginInterface_H
#define Fpix_PluginInterface_H

#include <vector>
#include <map>
#include <SimAPI.h>
#include <SimPlugin.h>

using namespace std;

namespace Force {

    /*!
     \class PluginInterface
     \brief Keep track of plugin shared object handle and 'extern C' function pointers
    */

    typedef SimPlugin* create_t();
    typedef void destroy_t(SimPlugin*);
    typedef int is_shared_t();

    class PluginInterface {
    public:
      ASSIGNMENT_OPERATOR_ABSENT(PluginInterface);
      COPY_CONSTRUCTOR_ABSENT(PluginInterface);
      PluginInterface(std::string &plugin_path, std::vector<std::string> &plugins_cl_args);
      PluginInterface(std::string plugin_path, std::map<std::string, std::string> plugins_options);
      ~PluginInterface();

      const std::string Path() const { return mPath; }; //!< return path to the plugin
      SimPlugin *CreateInstance(); //!< create new SimPlugin instance
      int IsShared() const { return mIsShared(); }; //!< return true if this plugin is shared among all threads

    protected:
      // only the plugin interface should use this method:
      void DestroyInstance(SimPlugin *plugin_instance) const { mDestroy(plugin_instance); }; //!< delete a SimPlugin instance

    private:
      std::string           mPath;               //!< plugin filepath
      std::vector<std::string> mPluginsClargs;    //!< unparsed arguments to be forwarded to the plugins
      std::map<std::string, std::string> mPluginsOptions; //!< combined command line and config file options ready for interpretation by the plugin code
      void            *mSharedObject;       //!< handle to plugin shared object
      create_t        *mCreate;             //!< use to create plugin instance
      destroy_t       *mDestroy;            //!< use to delete     "      "
      is_shared_t     *mIsShared;           //!< use to invoke plugin IsShared method

      std::vector<SimPlugin *> mPlugins;         //!< plugin instances
    };
}


#endif
