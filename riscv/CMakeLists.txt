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
project(friscv)

include(CTest)
enable_testing()

# set c++11
set (CMAKE_CXX_STANDARD 11)

# definitions
add_definitions(-DARCH_ENUM_HEADER=<EnumsRISCV.h>)

# source files
aux_source_directory(./src ARCH_SRC_LIST)
aux_source_directory(./../3rd_party/src THIRD_PARTY_SRC_LIST)
aux_source_directory(./../base/src BASE_SRC_LIST)

# targets
add_executable(${PROJECT_NAME} ${ARCH_SRC_LIST} ${THIRD_PARTY_SRC_LIST} ${BASE_SRC_LIST} ./../utils/handcar/UopInterface.cc)

# include directories
target_include_directories(${PROJECT_NAME} PRIVATE
                          ./inc
                          ./../base/inc
                          ./../3rd_party/inc
                          ./../utils/handcar)

# link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE pybind11::module pybind11::embed ${CMAKE_DL_LIBS})

# install
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)