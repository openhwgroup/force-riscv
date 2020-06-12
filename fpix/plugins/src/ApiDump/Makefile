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
INC_PATHS = -I../../../../base/inc -I../../../../3rd_party/inc -I../../../inc

include Makefile.target
include ../../../../utils/make/Makefile.common

CFLAGS := $(CFLAGS)
PLUGIN_SRCS := $(shell find ./ -name '*.cc')
PLUGIN_HDRS := $(shell find ./ -name '*.h')

all:
	@mkdir -p ../../bin
	@make ../../bin/$(TARGET_NAME).so

../../bin/$(TARGET_NAME).so: $(PLUGIN_SRCS) $(PLUGIN_HDRS)
	$(CC) $(CFLAGS) $(INC_PATHS) -fPIC -shared -o $@ $(PLUGIN_SRCS)
 
.PHONY: clean
clean:
	rm -rf ../../bin/$(TARGET_NAME).so
