Copyright 2019-2021 T-Head Semiconductor Co., Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

# Work with hardware code in external repositories

force-riscv is not a closed ecosystem. It needs to integrate code from third parties. OpenTitan's vendor tool is chosen as the integration tool. Please refer to the vendor docoment from OpenTitan's github repo(https://github.com/lowRISC/opentitan/blob/c774704db9ebe117f7c6961bc9f567b0695faf06/doc/contributing/hw/vendor.md).

## Makefile targets

all:
 clone riscv-isa-sim official repo based on version defined in rv_spike.lock.hjson, apply the patches, build riscv-isa-sim and install handcar_cosim.so into utils/handcar/handcar_cosim.so

all_without_patch:
 clone riscv-isa-sim thead forkedrepo based on version defined in rv_thead_spike.lock.hjson, build riscv-isa-sim and install handcar_cosim.so into utils/handcar/handcar_cosim.so

vendor_clone:
 clone riscv-isa-sim official repo based on version defined in rv_spike.lock.hjson

vendor_update:
 update patches based on rv_spike.vendor.hjson, and update rv_spike.lock.hjson

vendor_thead_clone:
 clone riscv-isa-sim thead repo based on version defined in rv_thead_spike.lock.hjson

vendor_thead_update:
 based on rv_thead_spike.vendor.hjson, update rv_thead_spike.lock.hjson

all is the default target to build handcar_cosim.so

## handcar_cosim.so update policy

feature_force_vendor_develop branch of https://github.com/T-head-Semi/riscv-isa-sim.git is used to maintain the latest stable riscv-isa-sim code for handcar_cosim.so generation.

feature_force_vendor_bases branch of https://github.com/T-head-Semi/riscv-isa-sim.git is used to track the base version of official riscv-isa-sim repo for handcar integration code development. It is used in rv_spike.vendor.hjson to make patches.

feature_force_vendor_develop branch of https://github.com/T-head-Semi/riscv-isa-sim.git can be used directly as the stable version for handcar code development.