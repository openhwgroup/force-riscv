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
include ../Makefile.target

SIM_SRCS := SimLoader.cc SimApiHANDCAR.cc
ALL_SRCS := $(ALL_SRCS) $(SIM_SRCS)

OPTIMIZATION = -O2
include ../../utils/make/Makefile.common

INC_PATHS = -I./inc -I../inc -I../../base/inc -I../../3rd_party/inc -I../../utils/handcar -I$(PYTHON_INC)

CFLAGS := $(CFLAGS) -DUNIT_TEST
NODEPS:=clean

vpath %.cc ../src ./src ../../3rd_party/src ../../base/src ../../utils/handcar
vpath %.d $(DEP_DIR)

all:
	@$(MAKE) make_dir
	@$(MAKE) inc/usage.h
	@$(MAKE) ../bin/fpix_riscv

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(ALL_DEPS)
endif

$(DEP_DIR)/%.d: %.cc
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_PATHS) -MM -MT '$(patsubst $(DEP_DIR)/%.d,$(OBJ_DIR)/%.o,$@)' $< -MF $@

$(OBJ_DIR)/%.o: %.cc %.d
	$(CC) -c $(CFLAGS) $(INC_PATHS) -o $@ $<

../bin/fpix_riscv: $(ALL_OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

.PHONY: make_dir
make_dir:
	@mkdir -p ../bin make_area make_area/obj make_area/dep

.PHONY: clean
clean:
	rm -rf make_area ../bin/fpix_riscv *.S *.ELF *.log *.railhouse inc/usage.h

inc/usage.h:  README.txt ../utils/readme2usage.py
	cat README.txt | ../utils/readme2usage.py >inc/usage.h

.PHONY: test
test:
	@echo test 1...
	rm -f *.log *.ELF *.S iss.railhouse
	../../bin/friscv -t ../../utils/smoke/riscv_force.py --noiss 1>force.log 2>&1
	@echo test plugin...
	../bin/fpix_riscv --decoding 1 -C 1 --exit_loop 1 --max_insts 1500 --plugin plugins/bin/ApiDump.so ./riscv_force.Default.ELF 1>fpix_plugins_test.log 2>&1
	@echo plugins still work!
