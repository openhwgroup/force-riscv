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
#include <PluginInterface.h>
#include <dlfcn.h>
#include <Log.h>

namespace Force {

//!< create set of handles (pointers) used to access a plugin shared object
PluginInterface::PluginInterface(string &plugin_path, vector<string> &plugins_cl_args) :
  mPath(plugin_path),
  mPluginsClargs(plugins_cl_args),
  mPluginsOptions(),
  mSharedObject(nullptr), // only used to close the shared object interface
  mCreate(nullptr),
  mDestroy(nullptr),
  mIsShared(nullptr),
  mPlugins()
{
  LOG(debug) << "Setup for plugin " << plugin_path << endl;

  void *handle;
  handle = dlopen(plugin_path.c_str(),RTLD_NOW);
  if (!handle) {
    printf("Unable to load plugin (path: '%s'). The system error message is: '%s'\n",plugin_path.c_str(), dlerror());
    exit(-1);
  }

  create_t* create = (create_t*)dlsym(handle,"create");
  if (!create) {
    printf("For plugin '%s' - 'create' function missing?: The system error message is: '%s'\n",plugin_path.c_str(), dlerror());
    exit(-1);
  }

  destroy_t* destroy = (destroy_t*)dlsym(handle,"destroy");
  if (!destroy) {
    printf("For plugin '%s' - 'destroy' function missing?: The system error message is: '%s'\n",plugin_path.c_str(), dlerror());
    exit(-1);
  }

  is_shared_t* is_shared = (is_shared_t*)dlsym(handle,"is_shared");
  if (!is_shared) {
    printf("For plugin '%s' - 'is_shared' function missing?: The system error message is: '%s'\n",plugin_path.c_str(), dlerror());
    exit(-1);
  }

  LOG(debug) << "Okay. we now have a set of pointers to allow SimPlugin instances to be created..." << endl;

  mPath = plugin_path;
  mSharedObject = handle; // only used to close the shared object interface
  mCreate = create;
  mDestroy = destroy;
  mIsShared = is_shared;

  // if this is a shared plugin, then create a single instance to be shared among all threads...

  if (IsShared()) {
    mPlugins.push_back( mCreate() );
    mPlugins[0]->parsePluginsClargs(mPluginsClargs);
  }
}

//!< create set of handles (pointers) used to access a plugin shared object
PluginInterface::PluginInterface(string plugin_path, map<string, string>  plugins_options) :
  mPath(plugin_path),
  mPluginsClargs(),
  mPluginsOptions(plugins_options),
  mSharedObject(nullptr), // only used to close the shared object interface
  mCreate(nullptr),
  mDestroy(nullptr),
  mIsShared(nullptr),
  mPlugins()
{
  LOG(debug) << "Setup for plugin " << plugin_path << endl;

  void *handle;
  handle = dlopen(plugin_path.c_str(),RTLD_NOW);
  if (!handle) {
    printf("Unable to load plugin (path: '%s'). The system error message is: '%s'\n",plugin_path.c_str(), dlerror());
    exit(-1);
  }

  create_t* create = (create_t*)dlsym(handle,"create");
  if (!create) {
    printf("For plugin '%s' - 'create' function missing?: The system error message is: '%s'\n",plugin_path.c_str(), dlerror());
    exit(-1);
  }

  destroy_t* destroy = (destroy_t*)dlsym(handle,"destroy");
  if (!destroy) {
    printf("For plugin '%s' - 'destroy' function missing?: The system error message is: '%s'\n",plugin_path.c_str(), dlerror());
    exit(-1);
  }

  is_shared_t* is_shared = (is_shared_t*)dlsym(handle,"is_shared");
  if (!is_shared) {
    printf("For plugin '%s' - 'is_shared' function missing?: The system error message is: '%s'\n",plugin_path.c_str(), dlerror());
    exit(-1);
  }

  LOG(debug) << "Okay. we now have a set of pointers to allow SimPlugin instances to be created..." << endl;

  mPath = plugin_path;
  mSharedObject = handle; // only used to close the shared object interface
  mCreate = create;
  mDestroy = destroy;
  mIsShared = is_shared;

  // if this is a shared plugin, then create a single instance to be shared among all threads...

  if (IsShared()) {
    mPlugins.push_back( mCreate() );
    mPlugins[0]->parsePluginsOptions(mPluginsOptions);
  }
}


//!< delete all plugin instances, close the shared object...

PluginInterface::~PluginInterface() {
  // delete all plugin instances...
  for (auto sp = mPlugins.begin(); sp != mPlugins.end(); sp++) {
    DestroyInstance(*sp);
  }
  // close the shared object interface...
  dlclose(mSharedObject);
}

SimPlugin *PluginInterface::CreateInstance() { 
  LOG(debug) << "Create plugin instance (plugin-path: " << mPath << ")" << endl;
  if (IsShared()) {
    return mPlugins[0];
  }

  SimPlugin *new_plugin_instance = mCreate(); 
  new_plugin_instance->parsePluginsOptions(mPluginsOptions);

  mPlugins.push_back(new_plugin_instance);
  return new_plugin_instance;
}

}


