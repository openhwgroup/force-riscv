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
#include <vector>
#include <Log.h>

#include <ConfigFPIX.h>
#include <SimAPI.h>
#include <SimThread.h>
#include <PluginManager.h>

using namespace Force;
using namespace std;

namespace Force {

//!< top-level simulation routine...

int simulate(SimAPI* pSimAPI, ConfigFPIX *cfg, vector<string> *test_files) {

  uint64_t entry_point = 0;

  if (!SimUtils::LoadTest(entry_point, pSimAPI, test_files) || !SimUtils::LoadTestMarkers() ) {
    cerr << "Aborting simulation" << endl;
    return -1;
  }

  // create a sim-thread for each cpu ID identified by the simulator...

  vector<uint32_t> cpu_ids;
  SimUtils::AssignCpuIDs( cpu_ids,cfg->ClusterCount(),cfg->NumberOfCores(),cfg->ThreadsPerCpu() );

  vector<SimThread *> sim_threads;

  for (auto cpu_id = cpu_ids.begin(); cpu_id != cpu_ids.end(); cpu_id++) {
     sim_threads.push_back(new SimThread(*cpu_id, cfg, pSimAPI, entry_point));
  }
  
  // we process until all sim-threads are done OR some error has occurred...

  bool processing_events = true; // true as long as there is more work to do...

  int rcode = 0; // any non-zero value will represent an error...

  while( processing_events && !rcode ) {

    bool more_to_do = false; // will be true if at least one sim-thread has more work to do...

    bool all_stop = true;
    for (auto st = sim_threads.begin(); st != sim_threads.end() && !rcode; st++) {
      all_stop &= (*st)->EndTestReached();  // can only be true if all threads have hit end-test
    }

    // one iteration on each sim-thread...

    for (auto st = sim_threads.begin(); st != sim_threads.end() && !rcode; st++) {
      if ((*st)->Done()) {
        // this thread is done...
        continue;
      } 

      if ( (rcode = (*st)->ProcessNextEvent(all_stop)) != 0 ) {
        // any error (non-zero return code) will cause simulation to abort...
      } 
      else {
        more_to_do = true; // will assume this sim-thread is NOT done...
      }

    }

    processing_events = more_to_do; // theres more to do, ie, more events to process, if at least one thread is not done...
  }

  // discard sim-thread objects...

  for (auto st = sim_threads.begin(); st != sim_threads.end(); st++) {
     delete *st;
  }

  return rcode;
}

}
