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
TEST_BUILDER=./riscv_test_builder.py
XML_INPUT_DIR=input/xml/
INSTALL_DIR=../../../../tests/riscv/instructions/
ARCH_DATA_DIR=../../../../riscv/arch_data/

.PHONY: all
all:
	@$(MAKE) setup
	@$(MAKE) riscv_tests
	@$(MAKE) install
	@$(MAKE) clean

input:
	@mkdir input

output:
	@mkdir output

$(XML_INPUT_DIR):
	@mkdir $(XML_INPUT_DIR)

$(XML_INPUT_DIR)g_instructions.xml:
	@cp -f $(ARCH_DATA_DIR)g_instructions.xml $(XML_INPUT_DIR)

$(XML_INPUT_DIR)g_instructions_rv64.xml:
	@cp -f $(ARCH_DATA_DIR)g_instructions_rv64.xml $(XML_INPUT_DIR)

$(XML_INPUT_DIR)c_instructions.xml:
	@cp -f $(ARCH_DATA_DIR)c_instructions.xml $(XML_INPUT_DIR)

$(XML_INPUT_DIR)c_instructions_rv64.xml:
	@cp -f $(ARCH_DATA_DIR)c_instructions_rv64.xml $(XML_INPUT_DIR)

$(XML_INPUT_DIR)c_instructions_rv32.xml:
	@cp -f $(ARCH_DATA_DIR)c_instructions_rv32.xml $(XML_INPUT_DIR)

$(XML_INPUT_DIR)v_instructions.xml:
	@cp -f $(ARCH_DATA_DIR)v_instructions.xml $(XML_INPUT_DIR)

$(XML_INPUT_DIR)priv_instructions.xml:
	@cp -f $(ARCH_DATA_DIR)priv_instructions.xml $(XML_INPUT_DIR)

$(XML_INPUT_DIR)zfh_instructions.xml:
	@cp -f $(ARCH_DATA_DIR)zfh_instructions.xml $(XML_INPUT_DIR)

$(XML_INPUT_DIR)zfh_instructions_rv64.xml:
	@cp -f $(ARCH_DATA_DIR)zfh_instructions_rv64.xml $(XML_INPUT_DIR)

$(INSTALL_DIR):
	@mkdir $(INSTALL_DIR)

.PHONY: setup
setup: input output $(XML_INPUT_DIR)
	@cp -f $(ARCH_DATA_DIR)*_instructions.xml $(XML_INPUT_DIR)

.PHONY: riscv_tests
riscv_tests: $(TEST_BUILDER) $(XML_INPUT_DIR)g_instructions.xml $(XML_INPUT_DIR)g_instructions_rv64.xml \
            $(XML_INPUT_DIR)c_instructions.xml $(XML_INPUT_DIR)c_instructions_rv64.xml $(XML_INPUT_DIR)c_instructions_rv32.xml \
            $(XML_INPUT_DIR)v_instructions.xml $(XML_INPUT_DIR)priv_instructions.xml \
            $(XML_INPUT_DIR)zfh_instructions.xml $(XML_INPUT_DIR)zfh_instructions_rv64.xml
	@$(TEST_BUILDER)

.PHONY: install
install: output $(INSTALL_DIR)
	@rm -rf $(INSTALL_DIR)*
	@cp -rf output/* $(INSTALL_DIR)

.PHONY: clean
clean:
	@rm -rf output/* debug_output.txt $(XML_INPUT_DIR)/*
