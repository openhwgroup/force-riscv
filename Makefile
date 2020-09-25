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

export FORCE_CC ?= g++
PYVER != python3 --version | sed -n -e 's/Python \(3\.[[:digit:]]\+\)\..*/\1/p'
export FORCE_PYTHON_VER ?= $(PYVER)

# this really ought to use pkg-config - there could be more than one include
# directory.
export FORCE_PYTHON_INC ?= /usr/include/python$(FORCE_PYTHON_VER)
export FORCE_PYTHON_LIB ?= /usr/lib/x86_64-linux-gnu/

all:
	@$(MAKE) riscv
	@$(MAKE) fpix

.PHONY: riscv
riscv:
	@cd riscv; $(MAKE)


.PHONY: fpix
fpix:
	@cd fpix; $(MAKE)

.PHONY: tests
tests:
	@rm -fr output
	@cd riscv; $(MAKE) tests
	@cd utils/regression/seedgen; $(MAKE) all

.PHONY: enums
enums:
	@cd utils/enum_classes; python3 ./create_enum_files.py

.PHONY: handlers
handlers:

.PHONY: clean
clean:
	@cd riscv; $(MAKE) clean
	@cd fpix; $(MAKE) clean
	@cd utils/regression/seedgen; $(MAKE) clean
