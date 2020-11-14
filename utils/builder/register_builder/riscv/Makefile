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
REGISTER_BUILDER=./register_builder.py
CSR_TREES_BUILDER=./csr_trees_builder.py

INSTALL_DIR=../../../../riscv/arch_data
TREES_INSTALL_DIR=../../../../py/DV/riscv/trees

.PHONY: all
all:
	@$(MAKE) make_dir
	@$(MAKE) registers
	@$(MAKE) install
	@$(MAKE) clean

.PHONY: make_dir
make_dir:
	@mkdir -p output

.PHONY: registers
registers: app_registers system_registers system_register_choices register_field_choices csr_trees
	@echo Done

.PHONY: app_registers
app_registers: app_registers_builder.py
	@echo 'Building Registers: application registers'
	@./app_registers_builder.py

.PHONY: system_registers
system_registers: $(REGISTER_BUILDER)
	@echo 'Building Registers: system registers'
	@$(REGISTER_BUILDER) --system_registers

.PHONY: csr_trees
csr_trees: $(CSR_TREES_BUILDER)
	@echo 'Building Registers: csr trees'
	@$(CSR_TREES_BUILDER) --system_registers_starter_file=input/system_registers_starter_rv64.xml

.PHONY: system_register_choices
system_register_choices: $(REGISTER_BUILDER)
	@echo 'Building Registers: system register choices'
	@$(REGISTER_BUILDER) --system_register_choices

.PHONY: register_field_choices
register_field_choices: $(REGISTER_BUILDER)
	@echo 'Building Registers: register field choices'
	@$(REGISTER_BUILDER) --register_field_choices

.PHONY: install
install:
	@echo 'Building Registers: installing register files'
	@cp -f output/app_registers_rv64.xml $(INSTALL_DIR)/app_registers.xml
	@cp -f output/system_registers_rv64.xml $(INSTALL_DIR)/system_registers.xml
	@cp -f output/system_register_choices_rv64.xml $(INSTALL_DIR)/system_register_choices.xml
	@cp -f output/register_field_choices_rv64.xml $(INSTALL_DIR)/register_field_choices.xml
	@cp -f output/csr_trees.py $(TREES_INSTALL_DIR)/csr_trees.py

.PHONY: clean
clean:
	@echo 'Building Registers: clean'
	@rm -f output/*.xml output/*.txt
