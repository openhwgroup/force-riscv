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
INC_PATHS = -I../../../base/inc -I../../../3rd_party/inc -I$(PYTHON_INC)


include Makefile.target
include ../../../utils/make/Makefile.common

#We need to replace LFLAGS with a new definition here because pybind11 does not want us to link explicitly with the Python lib
LFLAGS = -lpthread -static-libstdc++ -static-libgcc -lutil -ldl -rdynamic
MYPYBINDFLAGS = -shared -fPIC

#CFLAGS := $(CFLAGS) -DUNIT_TEST
CFLAGS := $(CFLAGS) -DUNIT_TEST 
NODEPS:=clean

vpath %.cc ../../../3rd_party/src ../../../base/src
vpath %.d $(DEP_DIR)

all:
	@make make_dir
	@make bin/$(TARGET_NAME)

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(ALL_DEPS)
endif

$(DEP_DIR)/%.d: %.cc
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_PATHS) -MM -MT '$(patsubst $(DEP_DIR)/%.d,$(OBJ_DIR)/%.o,$@)' $< -MF $@ 

#Note the -fPIC flag is necessary here
$(OBJ_DIR)/%.o: %.cc %.d
	$(CC) -fPIC -c $(CFLAGS) $(INC_PATHS) -o $@ $< 

bin/$(TARGET_NAME): $(ALL_OBJS)
	$(CC) $(MYPYBINDFLAGS) $(CFLAGS) $(INC_PATHS) seedgen.cc -o $@ $^ $(LFLAGS) 
	 
.PHONY: make_dir
make_dir:
	@mkdir -p bin make_area make_area/obj make_area/dep

.PHONY: clean
clean:
	rm -rf make_area bin
