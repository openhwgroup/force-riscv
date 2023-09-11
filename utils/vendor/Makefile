#SHELL:=/bin/tcsh

FORCE_ROOT := $(shell pwd)/../..

.PHONY: all
all: vendor_clone vendor_install_spike
	@echo "Please use make install or make clean."

.PHONY: all_without_patch
all_without_patch: vendor_thead_clone vendor_install_spike
	@echo "Please use make install or make clean."

.PHONY: vendor_clone
vendor_clone:
	@echo "--- clone official spike/riscv-isa-sim"
	@./vendor.py rv_spike.vendor.hjson --verbose

.PHONY: vendor_update
vendor_update:
	@echo "--- update repo spike/riscv-isa-sim"
	@./vendor.py rv_spike.vendor.hjson --verbose --refresh-patches --update

.PHONY: vendor_thead_clone
vendor_thead_clone:
	@echo "--- clone official thead spike/riscv-isa-sim"
	@./vendor.py rv_thead_spike.vendor.hjson --verbose

.PHONY: vendor_thead_update
vendor_thead_update:
	@echo "--- update repo thead spike/riscv-isa-sim"
	@./vendor.py rv_thead_spike.vendor.hjson --verbose --update

.PHONY: 
vendor_install_spike:
	@echo "--- generate RISCV handcar.so"
	@cmake -S ${FORCE_ROOT}/build/vendor/rv_spike_dir -B ${FORCE_ROOT}/build/vendor/rv_spike_dir/build
	@cmake --build ${FORCE_ROOT}/build/vendor/rv_spike_dir/build --target handcar_cosim -- -j
	@mv ${FORCE_ROOT}/build/vendor/rv_spike_dir/build/handcar_cosim.so ${FORCE_ROOT}/utils/handcar/
	@echo "finished generating handcar.so"
