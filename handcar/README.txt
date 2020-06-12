#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#

The easy way to build handcar:

1) Clone riscv-isa-sim git version 5d5ee23f574583145cd2093a1fdab677e313e1d2 to directory standalone. Retrieve from https://github.com/riscv/riscv-isa-sim

2) There, configure and make. Refer to build instructions provided at https://github.com/riscv/riscv-isa-sim .
   Some of the files (like the instruction files) created in the configuring and build process are needed by handcar.

3) In the main handcar directory call the script "regenerate_and_build.bash"
