#
# Copyright (C) [2020] Futurewei Technologies, Inc.
#
# FORCE-RISCV is licensed under the Apache License, Version 2.0
#  (the "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
# OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the License for the specific language governing permissions and
# limitations under the License.
#

ALL_SRCS := $(wildcard *.cc fesvr/*.cc force_mod/*.cc riscv/*.cc softfloat/*.cc spike_main/*.cc)
ALL_SRCS := $(filter-out fesvr/dtm.cc fesvr/elf2hex.cc fesvr/htif.cc fesvr/htif_hexwriter.cc fesvr/htif_pthread.cc fesvr/syscall.cc fesvr/term.cc fesvr/tsi.cc riscv/clint.cc riscv/debug_module.cc riscv/devices.cc riscv/dts.cc riscv/insn_template.cc riscv/interactive.cc riscv/jtag_dtm.cc riscv/rom.cc spike_main/spike-dasm.cc spike_main/spike-log-parser.cc spike_main/termios-xspike.cc spike_main/xspike.cc, $(ALL_SRCS))

CLEAN_TARGETS = \
	./bin/\
	./make_area/\

include Makefile.common

INC_PATHS = -I. -I../../3rd_party/inc/ -I./fesvr -I./force_mod -I./riscv -I./softfloat -I./spike_main

NODEPS:=clean

vpath %.d $(DEP_DIR)

all:
	@$(MAKE) setup
	@$(MAKE) handcar
	@$(MAKE) install

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(ALL_DEPS)
endif
#
# Needed for all the APIs
#
$(DEP_DIR)/%.d: %.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INC_PATHS) -MM -MT '$(patsubst $(DEP_DIR)/%.d,$(OBJ_DIR)/%.o,$@)' $< -MF $@

$(OBJ_DIR)/%.o: %.cc %.d
	@mkdir -p $(dir $@)
	$(CXX) -c $(CFLAGS) -fPIC  $(INC_PATHS) -o $@ $<

#
# For cosim
#
bin/handcar_cosim.so: $(ALL_OBJS)
	$(CXX) -o $@ $^ $(LFLAGS) -shared -fPIC

.PHONY: setup
setup:
	@mkdir -p bin make_area/obj make_area/dep

.PHONY: handcar
handcar:
	@$(MAKE) bin/handcar_cosim.so

.PHONY: install
install: ./bin/handcar_cosim.so
	@cp bin/handcar_cosim.so ../../utils/handcar

.PHONY: clean
clean:
	rm -rf $(CLEAN_TARGETS)
	@cd patcher && $(MAKE) clean && cd ..
