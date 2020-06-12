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
#include <PluginManager.h>
#include <SimEvent.h>
#include <Log.h>
#include <StringUtils.h>
#include <algorithm>

//!< Initialization interface - 
//!<   Load plugin shared objects.
//!<   Create instances for 'shared' plugins (a 'shared' plugin is intended to be called from all threads).

namespace Force {
  static vector<PluginInterface *> *mPluginInterfaces = nullptr; //!< interfaces to all plugin shared objects
}

namespace Force {

int PluginManager::Initialize(vector<string> &plugin_paths, vector<string> &plugins_cl_args) { 
  mPluginInterfaces = new vector<PluginInterface *>;

  int rcode = 0;

  for (auto fp = plugin_paths.begin(); fp != plugin_paths.end() && !rcode; fp++) {
     PluginInterface *next_plugin_interface = new PluginInterface(*fp, plugins_cl_args);
     mPluginInterfaces->push_back(next_plugin_interface);
  }

  return rcode;
}

int PluginManager::Initialize(std::vector<std::string> &plugin_paths, std::map<std::string, Force::plugin_node> * apPluginNodesMap, std::string * apGlobalPluginsOptions) {
  mPluginInterfaces = new vector<PluginInterface *>;

  int rcode = 0;

  if(apPluginNodesMap == nullptr || apGlobalPluginsOptions == nullptr){
    rcode = 1;
  }
  else{
    //Break up the global options string so we can separate option name from any arguments.
    std::vector<std::pair<std::string, std::string>> split_global_options;
    Force::StringSplitter splitter((*apGlobalPluginsOptions), ',');

    while(! splitter.EndOfString() ){
        //chomp the split string
        std::string current_substring = splitter.NextSubString();
        
        //if an option with argument, split off the argument, otherwise leave rhs empty to indicate there was no argument.
        std::string lhs;
        std::string rhs;
        bool was_assignment = Force::parse_assignment(current_substring, lhs, rhs);
        if(! was_assignment){
            lhs = current_substring;
            rhs = "";
        }
        
        split_global_options.push_back(make_pair(lhs, rhs));    
    }
    
    LOG(notice) << "#### Dev test 3, size of plugin nodes map: " << apPluginNodesMap->size() << endl; 

    //Iterate over the plugins in the map, instantiate interfaces and provide appropriate options strings
    for(const auto & element : (*apPluginNodesMap)){
        std::map<std::string, std::string> plugins_options; //!< Local variable to contain the combined global and plugin associated relevant command line and config file options.

        std::vector<decltype(split_global_options)::const_iterator> for_erasure; //!< Contains copies of iterators to split_global_options, used to keep track of what to erase so that we don't re-consider options already assigned to specific plugins

        LOG(notice) << "\tPlugin: " << element.first << " with filepath: " << element.second.Filepath() << endl; 

        //Iterate over the globally specified options that we just split, distribute universal options and prune plugin specifics to make subsequent loop iterations faster.
        for(auto option_pair = split_global_options.cbegin(), next_it = option_pair; option_pair != split_global_options.cend(); option_pair = next_it){
            //Increment our second iterator so we can safely erase using the first one if needed
            next_it = std::next(next_it);

            //Is this a universal option? If so, just add it in for this plugin and keep going.
            auto dot_location = std::find(option_pair->first.begin(), option_pair->first.end(), '.'); //!< dot_location is an iterator that indicates the separation between plugin name and option name.
            if(dot_location == option_pair->first.end()){
                plugins_options.emplace(*option_pair);
                continue;    
            }

            //see if the plugin name is a substring of the option's first field, name section.
            auto search_it = std::search(option_pair->first.begin(), dot_location, element.first.begin(), element.first.end());
            bool for_this_plugin = true;
            if(search_it == dot_location){
                for_this_plugin = false;
            }

            //The plugin may be "named" by its relative file path check if instead lhs is a substring of the file_path. We're assuming that a substring match indicates we've identified the plugin. This will break down if plugins names are similar enough or differ just by file path.
            if(! for_this_plugin){
                search_it = std::search(element.first.begin(), element.first.end(), option_pair->first.begin(), dot_location);
                //LOG(notice) << "\t" << element.first << " vs. " << std::string(option_pair->first.begin(), dot_location) << endl;
                if(search_it != element.first.end()){
                    for_this_plugin = true;
                }
            }
            if(for_this_plugin){
                //split the lhs by '.' and get the last substring of the split as the option name, then add the pair to vector for this plugin
                plugins_options.emplace(std::string(std::next(dot_location), option_pair->first.end()),option_pair->second);

                //since this option was for this particular plugin, we should remove it from consideration by the other plugins
                for_erasure.push_back(option_pair);
            }

            //If this option is specific to another plugin, just do nothing
        }

        //Clear out the already assigned plugin specific options that were in the global options vector. This way the other plugin iterations don't needlessly consider them.
        for(auto it : for_erasure){
            split_global_options.erase(it);
        }

        //Break up the config file plugins options string so we can separate option name from any arguments.
        std::vector<std::pair<std::string, std::string>> split_local_options;
        if(element.second.PluginsOptions().size() > 0){
            std::string temp = element.second.PluginsOptions(); //Strange behavior was seen here if a temporary copy was not made. The first substring would be returned corrupted.
            Force::StringSplitter popt_splitter(temp, ',');

            while(! popt_splitter.EndOfString() ){
                std::string current_substring = popt_splitter.NextSubString();

                //if an option with argument, split off the argument
                std::string lhs;
                std::string rhs;
                bool was_assignment = Force::parse_assignment(current_substring, lhs, rhs);
                if(! was_assignment){
                    lhs = current_substring;
                    rhs = "";
                }
                
                plugins_options.emplace(lhs, rhs);    
            }
        }

        for(auto option : plugins_options) {
            LOG(notice) << "\t\t" << option.first << " = " << option.second << endl;
        }

        //Instantiate the interface object, providing the constructor with a copy of the local variable plugins_options
        PluginInterface *next_plugin_interface = new PluginInterface(element.second.Filepath(), plugins_options);
        mPluginInterfaces->push_back(next_plugin_interface);

    }
    LOG(notice) << "#### End dev test 3" << endl;
  }

  return rcode;
}
;

 //!< Destroy interface - unload plugins

void PluginManager::Destroy() {
  // delete all plugin interface objects...
  for (auto pi = mPluginInterfaces->begin(); pi != mPluginInterfaces->end(); pi++) {
    delete *pi;
  }
  delete mPluginInterfaces;
}

//!< create/hook-up plugin instances...

PluginManager::PluginManager(vector<ESimThreadEventType> event_types) :
mPlugins(), 
mSenders()
{
  for (auto pi = mPluginInterfaces->begin(); pi != mPluginInterfaces->end(); pi++) {
     SimPlugin *plugin_instance = (*pi)->CreateInstance();
     mPlugins.push_back(plugin_instance);
  }

  // Foreach event type:
  //    1. Create sender.
  //    2. Foreach plugin instance (shared and non-shared):
  //       Subscribe to this event, ie, attach plugin receiver to sender

  for (auto ev = event_types.begin(); ev != event_types.end(); ev++) {
     LOG(debug) << "Allocating sender for event type: '" << ESimThreadEventType_to_string(*ev) << "'..." << endl;
     mSenders[*ev] = new Sender<ESimThreadEventType>;  // allocate a sender for this event type...
     // allow plugins to subscribe to this event...
     for (auto pi = mPlugins.begin(); pi != mPlugins.end(); pi++) {
       if ((*pi)->IsSupported(*ev)) {
         LOG(debug) << "   Plugin '" << (*pi)->Name() << "' has subscribed to this event!" << endl;
         mSenders[*ev]->SignUp(*pi);
       }
     }
  }
}
    
//!< delete event senders...

PluginManager::~PluginManager() {
  // discard event senders...
  for (auto ev = mSenders.begin(); ev != mSenders.end(); ev++) {
    delete ev->second;
  }
}

//!< send sim-event notification to connected plugins... 

void PluginManager::Signal(SimThreadEvent &pData) {
  LOG(debug) << "Sending sim-event '" << pData.ToString() << "' (Id: " << ESimThreadEventType_to_string(pData.Type()) << ") to connected plugins..." << endl;
  mSenders[pData.Type()]->SendNotification(pData.Type(),&pData);
}

}
