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

BASE_SRCS := $(notdir $(wildcard src/*.cc))
ALL_SRCS := $(BASE_SRCS) 
OPTIMIZATION = -O2
include ../../Makefile.common

INC_PATHS = -I../../inc -Iinc -I../../../3rd_party/inc

NODEPS:=clean 
vpath %.cc src
vpath %.d $(DEP_DIR)

all:
	@$(MAKE) make_dir
	@$(MAKE) bin/initialize_and_terminate

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(ALL_DEPS)
endif

$(DEP_DIR)/%.d: %.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INC_PATHS) -MM -MT '$(patsubst $(DEP_DIR)/%.d,$(OBJ_DIR)/%.o,$@)' $< -MF $@

$(OBJ_DIR)/%.o: %.cc %.d
	$(CXX) -c $(CFLAGS) -fPIC $(INC_PATHS) -o $@ $<

bin/initialize_and_terminate: $(ALL_OBJS)  
	$(CXX) -o $@ $^ $(LFLAGS) -fPIC

.PHONY: make_dir
make_dir:
	@mkdir -p make_area make_area/obj make_area/dep

.PHONY: clean
clean:
	rm -rf make_area
	rm -f bin/initialize_and_terminate
