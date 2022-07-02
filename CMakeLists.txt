# Copyright 2019-2021 T-Head Semiconductor Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.0.0)
project(force-riscv VERSION 0.9.0)

include(CTest)
enable_testing()

# set c++11
set (CMAKE_CXX_STANDARD 11)

# pybind11 related
add_subdirectory(./3rd_party)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  message(STATUS "Setting default CMAKE_INSTALL_PREFIX path to ${PROJECT_SOURCE_DIR}")
  set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}" CACHE STRING "The path to use for make install" FORCE)
endif()

#------------------------
# other targets
add_subdirectory(./riscv)
add_subdirectory(./utils/handcar)
add_subdirectory(./fpix)

#------------------------
# unit tests
add_subdirectory(./unit_tests)
