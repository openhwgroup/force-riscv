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
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

#include "ConfigFPIX.h"
#include "Log.h"
#include "PluginManager.h"
#include "SimApiHANDCAR.h"

using namespace Force;
using namespace std;

namespace Force {

  void load_program_options(ConfigFPIX &cfg, vector<string> &testFiles, vector<string> &plugins_cl_args, int argc, char *argv[], const string& rDefaultConfig);
  int simulate(SimAPI* pSimAPI, ConfigFPIX *cfg, vector<string> *test_files);
  void set_plugin_paths(vector<string> &plugin_path);
}


int main(int argc, char* argv[]) {
  cout << "fpix - Force/Instruction Set Simulator with Scripting Interface tool, version 0" << endl;

  ConfigFPIX my_config;
  vector<string> my_test_files;
  vector<string> plugins_cl_args;

  Logger::Initialize();
 
  // Load list of plugins and PreInitialize them
  

  // Obtain the options information from the plugins

  // load program config, process command line options, including those to be appended from the plugins.
  load_program_options(my_config, my_test_files, plugins_cl_args, argc, argv, "config/riscv_rv64.config");

  LOG(debug) << "Load/initialize simulator shared object..." << endl;

  SimApiHANDCAR sim_api;

  ApiSimConfig sim_dll_cfg(my_config.ClusterCount(), /* # of clusters */
                        my_config.NumberOfCores(), /* # cores */
                        my_config.ThreadsPerCpu(), /* # threads */
                        my_config.PhysicalAddressSize(), /* physical address size */
                        my_config.VectorRegisterLength(), /* vector register length */
                        my_config.MaxVectorElementWidth(), /* maximum vector element width */
                        "./fpix_sim.log", /* simulator debug trace file */
                        true, /* use trace file */
                        my_config.SimulatorConfigString(), /* simulator configuration string */
                        my_config.AutoInitMem() /* automatically initialize simulator memory on access */
                       );

  sim_api.InitializeIss(sim_dll_cfg, my_config.SimulatorSharedObjectFile(), "" /* no api trace file */);

  LOG(debug) << "Loading plugins..." << endl;

  vector<string> plugins;         //
  my_config.GetPlugins(plugins);  // setup the (static) set of plugin paths used by the PluginManager

  map<string, Force::plugin_node> * plugin_nodes_map = nullptr;
  string * global_plugins_options = nullptr;

  my_config.GetPluginsInfo(&plugin_nodes_map, &global_plugins_options);

  LOG(info) << "#### Dev test 1" << endl;
  for(auto element : (*plugin_nodes_map)){
      LOG(info) << "\tPropagated plugin: " << element.first << " with options string: " << element.second.PluginsOptions() << endl;
  }
  LOG(info) << "\tPropagated global plugins options: " << (*global_plugins_options) << endl;
  LOG(info) << "#### End dev test 1" << endl;

  //PluginManager::Initialize(plugins,plugins_cl_args);    //    plugin shared-objects

  PluginManager::Initialize(plugins,plugin_nodes_map,global_plugins_options);    //    plugin shared-objects
  

  // intialize the options of the plugins 

  // sanity-check: were input files specified?...

  if (my_test_files.empty()) {
    cerr << "error: no test files specified. run is aborted." << endl;
    return -1;
  }

  LOG(debug) << "running the simulation..." << endl;

  int rcode = simulate(&sim_api, &my_config, &my_test_files);

  LOG(debug) << "simulation complete, return-code: " << rcode << endl;

  // done with simapi, pluginmanager...

  sim_api.Terminate();         // unload simulator,
  PluginManager::Destroy();  //    plugin shared objects

  plugin_nodes_map = nullptr; // These pointers don't own the memory, nothing more needed.
  global_plugins_options = nullptr;

  Logger::Destroy();

  return rcode;
}




