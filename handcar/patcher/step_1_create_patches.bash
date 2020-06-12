#!/bin/bash

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

mkdir originals
mkdir modified
mkdir patches
mkdir patched

cp ../standalone/config.h ./originals
cp ../standalone/riscv/devices.h ./originals
cp ../standalone/riscv/disasm.h ./originals
cp ../standalone/riscv/mmu.cc ./originals
cp ../standalone/riscv/insns/mret.h ./originals
cp ../standalone/riscv/processor.cc ./originals
cp ../standalone/riscv/regnames.cc ./originals
cp ../standalone/riscv/sim.h ./originals
cp ../standalone/riscv/insns/sret.h ./originals
cp ../standalone/riscv/decode.h ./originals
cp ../standalone/spike_main/disasm.cc ./originals
cp ../standalone/riscv/execute.cc ./originals
cp ../standalone/riscv/mmu.h ./originals
cp ../standalone/softfloat/primitives.h ./originals
cp ../standalone/riscv/processor.h ./originals
cp ../standalone/riscv/sim.cc ./originals
cp ../standalone/riscv/simif.h ./originals
cp ../standalone/softfloat/specialize.h ./originals
cp ../standalone/spike_main/spike.cc ./originals

#These lines were used when first creating the patches. There are not normally to be used unless updating the patches. DO NOT UNCOMMENT unless you know what you are doing.
###
#cp ../spike_mod/config.h ./modified
#cp ../spike_mod/devices.h ./modified
#cp ../spike_mod/disasm.h ./modified
#cp ../spike_mod/mmu.cc ./modified
#cp ../spike_mod/mret.h ./modified
#cp ../spike_mod/processor.cc ./modified
#cp ../spike_mod/regnames.cc ./modified
#cp ../spike_mod/simlib.h ./modified
#cp ../spike_mod/sret.h ./modified
#cp ../spike_mod/decode.h ./modified
#cp ../spike_mod/disasm.cc ./modified
#cp ../spike_mod/execute.cc ./modified
#cp ../spike_mod/mmu.h ./modified
#cp ../spike_mod/primitives.h ./modified
#cp ../spike_mod/processor.h ./modified
#cp ../spike_mod/simlib.cc ./modified
#cp ../spike_mod/simif.h ./modified
#cp ../spike_mod/specialize.h ./modified
#cp ../spike_mod/handcar* ./modified
#rm patches/config.h.patch  
#rm patches/devices.h.patch  
#rm patches/disasm.h.patch    
#rm patches/mmu.cc.patch  
#rm patches/mret.h.patch        
#rm patches/processor.cc.patch  
#rm patches/regnames.cc.patch  
#rm patches/sim.h.patch    
#rm patches/sret.h.patch
#rm patches/decode.h.patch  
#rm patches/disasm.cc.patch  
#rm patches/execute.cc.patch  
#rm patches/mmu.h.patch   
#rm patches/primitives.h.patch  
#rm patches/processor.h.patch   
#rm patches/sim.cc.patch       
#rm patches/simif.h.patch  
#rm patches/specialize.h.patch
#rm patches/spike.cc.patch
#
#echo "#" >> patches/config.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/config.h.patch
#echo "#" >> patches/config.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/config.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/config.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/config.h.patch
#echo "#" >> patches/config.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/config.h.patch
#echo "#" >> patches/config.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/config.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/config.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/config.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/config.h.patch
#echo "# limitations under the License." >> patches/config.h.patch
#echo "#" >> patches/config.h.patch
#diff originals/config.h modified/config.h >> patches/config.h.patch  
#
#echo "#" >> patches/devices.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/devices.h.patch
#echo "#" >> patches/devices.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/devices.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/devices.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/devices.h.patch
#echo "#" >> patches/devices.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/devices.h.patch
#echo "#" >> patches/devices.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/devices.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/devices.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/devices.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/devices.h.patch
#echo "# limitations under the License." >> patches/devices.h.patch
#echo "#" >> patches/devices.h.patch
#diff originals/devices.h modified/devices.h >> patches/devices.h.patch  
#
#echo "#" >> patches/disasm.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/disasm.h.patch
#echo "#" >> patches/disasm.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/disasm.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/disasm.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/disasm.h.patch
#echo "#" >> patches/disasm.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/disasm.h.patch
#echo "#" >> patches/disasm.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/disasm.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/disasm.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/disasm.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/disasm.h.patch
#echo "# limitations under the License." >> patches/disasm.h.patch
#echo "#" >> patches/disasm.h.patch
#diff originals/disasm.h modified/disasm.h >> patches/disasm.h.patch    
#
#echo "#" >> patches/mmu.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/mmu.cc.patch
#echo "#" >> patches/mmu.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/mmu.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/mmu.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/mmu.cc.patch
#echo "#" >> patches/mmu.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/mmu.cc.patch
#echo "#" >> patches/mmu.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/mmu.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/mmu.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/mmu.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/mmu.cc.patch
#echo "# limitations under the License." >> patches/mmu.cc.patch
#echo "#" >> patches/mmu.cc.patch
#diff originals/mmu.cc modified/mmu.cc >> patches/mmu.cc.patch  
#
#echo "#" >> patches/mret.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/mret.h.patch
#echo "#" >> patches/mret.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/mret.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/mret.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/mret.h.patch
#echo "#" >> patches/mret.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/mret.h.patch
#echo "#" >> patches/mret.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/mret.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/mret.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/mret.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/mret.h.patch
#echo "# limitations under the License." >> patches/mret.h.patch
#echo "#" >> patches/mret.h.patch
#diff originals/mret.h modified/mret.h >> patches/mret.h.patch        
#
#echo "#" >> patches/processor.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/processor.cc.patch
#echo "#" >> patches/processor.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/processor.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/processor.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/processor.cc.patch
#echo "#" >> patches/processor.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/processor.cc.patch
#echo "#" >> patches/processor.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/processor.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/processor.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/processor.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/processor.cc.patch
#echo "# limitations under the License." >> patches/processor.cc.patch
#echo "#" >> patches/processor.cc.patch
#diff originals/processor.cc modified/processor.cc >> patches/processor.cc.patch  
#
#echo "#" >> patches/regnames.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/regnames.cc.patch
#echo "#" >> patches/regnames.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/regnames.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/regnames.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/regnames.cc.patch
#echo "#" >> patches/regnames.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/regnames.cc.patch
#echo "#" >> patches/regnames.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/regnames.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/regnames.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/regnames.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/regnames.cc.patch
#echo "# limitations under the License." >> patches/regnames.cc.patch
#echo "#" >> patches/regnames.cc.patch
#diff originals/regnames.cc modified/regnames.cc >> patches/regnames.cc.patch  
#
#echo "#" >> patches/sim.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/sim.h.patch
#echo "#" >> patches/sim.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/sim.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/sim.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/sim.h.patch
#echo "#" >> patches/sim.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/sim.h.patch
#echo "#" >> patches/sim.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/sim.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/sim.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/sim.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/sim.h.patch
#echo "# limitations under the License." >> patches/sim.h.patch
#echo "#" >> patches/sim.h.patch
#diff originals/sim.h modified/simlib.h >> patches/sim.h.patch    
#
#echo "#" >> patches/sret.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/sret.h.patch
#echo "#" >> patches/sret.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/sret.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/sret.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/sret.h.patch
#echo "#" >> patches/sret.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/sret.h.patch
#echo "#" >> patches/sret.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/sret.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/sret.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/sret.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/sret.h.patch
#echo "# limitations under the License." >> patches/sret.h.patch
#echo "#" >> patches/sret.h.patch
#diff originals/sret.h modified/sret.h >> patches/sret.h.patch
#
#echo "#" >> patches/decode.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/decode.h.patch
#echo "#" >> patches/decode.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/decode.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/decode.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/decode.h.patch
#echo "#" >> patches/decode.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/decode.h.patch
#echo "#" >> patches/decode.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/decode.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/decode.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/decode.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/decode.h.patch
#echo "# limitations under the License." >> patches/decode.h.patch
#echo "#" >> patches/decode.h.patch
#diff originals/decode.h modified/decode.h >> patches/decode.h.patch  
#
#echo "#" >> patches/disasm.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/disasm.cc.patch
#echo "#" >> patches/disasm.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/disasm.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/disasm.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/disasm.cc.patch
#echo "#" >> patches/disasm.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/disasm.cc.patch
#echo "#" >> patches/disasm.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/disasm.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/disasm.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/disasm.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/disasm.cc.patch
#echo "# limitations under the License." >> patches/disasm.cc.patch
#echo "#" >> patches/disasm.cc.patch
#diff originals/disasm.cc modified/disasm.cc >> patches/disasm.cc.patch  
#
#echo "#" >> patches/execute.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/execute.cc.patch
#echo "#" >> patches/execute.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/execute.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/execute.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/execute.cc.patch
#echo "#" >> patches/execute.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/execute.cc.patch
#echo "#" >> patches/execute.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/execute.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/execute.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/execute.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/execute.cc.patch
#echo "# limitations under the License." >> patches/execute.cc.patch
#echo "#" >> patches/execute.cc.patch
#diff originals/execute.cc modified/execute.cc >> patches/execute.cc.patch  
#
#echo "#" >> patches/mmu.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/mmu.h.patch
#echo "#" >> patches/mmu.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/mmu.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/mmu.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/mmu.h.patch
#echo "#" >> patches/mmu.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/mmu.h.patch
#echo "#" >> patches/mmu.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/mmu.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/mmu.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/mmu.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/mmu.h.patch
#echo "# limitations under the License." >> patches/mmu.h.patch
#echo "#" >> patches/mmu.h.patch
#diff originals/mmu.h modified/mmu.h >> patches/mmu.h.patch   
#
#echo "#" >> patches/primitives.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/primitives.h.patch
#echo "#" >> patches/primitives.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/primitives.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/primitives.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/primitives.h.patch
#echo "#" >> patches/primitives.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/primitives.h.patch
#echo "#" >> patches/primitives.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/primitives.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/primitives.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/primitives.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/primitives.h.patch
#echo "# limitations under the License." >> patches/primitives.h.patch
#echo "#" >> patches/primitives.h.patch
#diff originals/primitives.h modified/primitives.h >> patches/primitives.h.patch  
#
#echo "#" >> patches/processor.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/processor.h.patch
#echo "#" >> patches/processor.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/processor.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/processor.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/processor.h.patch
#echo "#" >> patches/processor.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/processor.h.patch
#echo "#" >> patches/processor.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/processor.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/processor.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/processor.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/processor.h.patch
#echo "# limitations under the License." >> patches/processor.h.patch
#echo "#" >> patches/processor.h.patch
#diff originals/processor.h modified/processor.h >> patches/processor.h.patch   
#
#echo "#" >> patches/sim.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/sim.cc.patch
#echo "#" >> patches/sim.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/sim.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/sim.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/sim.cc.patch
#echo "#" >> patches/sim.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/sim.cc.patch
#echo "#" >> patches/sim.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/sim.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/sim.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/sim.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/sim.cc.patch
#echo "# limitations under the License." >> patches/sim.cc.patch
#echo "#" >> patches/sim.cc.patch
#diff originals/sim.cc modified/simlib.cc >> patches/sim.cc.patch       
#
#echo "#" >> patches/simif.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/simif.h.patch
#echo "#" >> patches/simif.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/simif.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/simif.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/simif.h.patch
#echo "#" >> patches/simif.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/simif.h.patch
#echo "#" >> patches/simif.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/simif.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/simif.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/simif.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/simif.h.patch
#echo "# limitations under the License." >> patches/simif.h.patch
#echo "#" >> patches/simif.h.patch
#diff originals/simif.h modified/simif.h >> patches/simif.h.patch  
#
#echo "#" >> patches/specialize.h.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/specialize.h.patch
#echo "#" >> patches/specialize.h.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/specialize.h.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/specialize.h.patch
#echo "#  You may obtain a copy of the License at" >> patches/specialize.h.patch
#echo "#" >> patches/specialize.h.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/specialize.h.patch
#echo "#" >> patches/specialize.h.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/specialize.h.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/specialize.h.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/specialize.h.patch
#echo "# See the License for the specific language governing permissions and" >> patches/specialize.h.patch
#echo "# limitations under the License." >> patches/specialize.h.patch
#echo "#" >> patches/specialize.h.patch
#diff originals/specialize.h modified/specialize.h >> patches/specialize.h.patch
#
#echo "#" >> patches/spike.cc.patch
#echo "# Copyright (C) [2020] Futurewei Technologies, Inc." >> patches/spike.cc.patch
#echo "#" >> patches/spike.cc.patch
#echo "# FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");" >> patches/spike.cc.patch
#echo "#  you may not use this file except in compliance with the License." >> patches/spike.cc.patch
#echo "#  You may obtain a copy of the License at" >> patches/spike.cc.patch
#echo "#" >> patches/spike.cc.patch
#echo "#  http://www.apache.org/licenses/LICENSE-2.0" >> patches/spike.cc.patch
#echo "#" >> patches/spike.cc.patch
#echo "# THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER" >> patches/spike.cc.patch
#echo "# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR" >> patches/spike.cc.patch
#echo "# FIT FOR A PARTICULAR PURPOSE." >> patches/spike.cc.patch
#echo "# See the License for the specific language governing permissions and" >> patches/spike.cc.patch
#echo "# limitations under the License." >> patches/spike.cc.patch
#echo "#" >> patches/spike.cc.patch
#diff originals/spike.cc modified/handcar* >> patches/spike.cc.patch
#
###
