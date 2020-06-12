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

BASE_SRCS = $(shell find src -name '*.cc' -type f)
COSIM_SRCS = $(shell find so_build/cosim/src -name '*.cc' -type f)
FORCE_SRCS = $(shell find force_mod -name '*.cc' -type f)

ALL_SRCS := $(BASE_SRCS) $(COSIM_SRCS) $(FORCE_SRCS)

include Makefile.common 

INC_PATHS = -I./inc -I./so_build/cosim/inc -I./force_mod -I../3rd_party/inc/

NODEPS:=clean

vpath %.cc src so_build/cosim/src force_mod
vpath %.d $(DEP_DIR)

all:
	./filesurgeon.py
	@$(MAKE) make_dir
	@$(MAKE) bin/handcar_cosim.so

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(ALL_DEPS)
endif

#
# Original Spike standalone build
#
#STANDALONE_BASE_PATH = standalone
#STANDALONE_ORIGINAL_PATH = ../3rd_party/spike
#STANDALONE_MODIFIED_PATH = standalone/patched_source
#STANDALONE_REPLACEMENTS_PATH = standalone/spike_mod_standalone
#
#.PHONY: standalone
#standalone: 
#	cp -R $(STANDALONE_ORIGINAL_PATH)/* $(STANDALONE_MODIFIED_PATH);
#	cp -R $(STANDALONE_REPLACEMENTS_PATH)/* $(STANDALONE_MODIFIED_PATH);
#	cd $(STANDALONE_BASE_PATH); patched_source/configure --enable-commitlog; $(MAKE) -j10

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

.PHONY: make_dir
make_dir:
	@mkdir -p make_area make_area/obj make_area/dep

.PHONY: clean
clean:
	rm -rf make_area
	rm -rf bin/handcar*.so

