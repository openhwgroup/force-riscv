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
ARCH_SRCS := $(notdir $(wildcard src/*.cc))
THIRD_PARTY_SRCS := $(notdir $(wildcard ../3rd_party/src/*.cc))
BASE_SRCS := $(notdir $(wildcard ../base/src/*.cc))
EXC_LIB_SRCS := ../utils/handcar/UopInterface.cc
ALL_SRCS := $(ARCH_SRCS) $(THIRD_PARTY_SRCS) $(BASE_SRCS) $(SIM_SRCS) $(EXC_LIB_SRCS)

OPTIMIZATION = -O2
VISIBILITY = -fvisibility=hidden
include ../utils/make/Makefile.common

ARCH_ENUM=RISCV

INC_PATHS = -I./inc -I../base/inc -I../3rd_party/inc -I$(PYTHON_INC)

NODEPS:=clean tests

vpath %.cc ./src ../3rd_party/src ../base/src
vpath %.d $(DEP_DIR)

all:
	@$(MAKE) make_dir
	@$(MAKE) ../bin/friscv
	@cd ../utils/handcar;make

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(ALL_DEPS)
endif

$(DEP_DIR)/%.d: %.cc
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_PATHS) -MM -MT '$(patsubst $(DEP_DIR)/%.d,$(OBJ_DIR)/%.o,$@)' $< -MF $@

$(OBJ_DIR)/%.o: %.cc %.d
	$(CC) -c $(CFLAGS) $(INC_PATHS) -o $@ $<

../bin/friscv: $(ALL_OBJS)
	$(CC) -o $@ $^ $(LFLAGS) 

.PHONY: make_dir
make_dir:
	@mkdir -p make_area make_area/obj make_area/dep

.PHONY: tests
tests:
	@cd ../utils/builder/test_builder/riscv; $(MAKE) all

.PHONY: deps
deps:
	@echo 'dependency files made'

.PHONY: clean
clean:
	rm -rf make_area
	rm -f ../bin/friscv
	@cd ../utils/handcar; make clean
