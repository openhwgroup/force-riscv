
 Copyright (C) [2020] Futurewei Technologies, Inc.

 FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

 THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 FIT FOR A PARTICULAR PURPOSE.
 See the License for the specific language governing permissions and
 limitations under the License.

To start a new plugin run the python script from this directory like so:

    ./start_plugin.py -n <plugin-name>

This will create a directory:

    ../src/<plugin-name>

Which will contain:

    <plugin-name>.cc
    Makefile
    Makefile.target

TIP: Customize these files and add more cc and h files as desired, the local Makefile will see and link them all together.

If you run make in the plugin directory itself, a bin directory will be created at the level of src where the <plugin-name>.so will be created.

However, if you run make in the plugins directory or in the main fpix directory, all the plugins will also be built automatically if needed.
