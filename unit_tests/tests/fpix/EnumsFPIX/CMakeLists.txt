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
project(EnumsFPIX_test)

include(CTest)
enable_testing()

# set c++11
set (CMAKE_CXX_STANDARD 11)

# definitions
add_definitions(-DARCH_ENUM_HEADER=<EnumsRISCV.h>)
add_definitions(-DUNIT_TEST)

set(ALL_SRCS 
    ./EnumsFPIX_test.cc
    ${CMAKE_SOURCE_DIR}/base/src/Log.cc
    ${CMAKE_SOURCE_DIR}/fpix/src/EnumsFPIX.cc
    ${CMAKE_SOURCE_DIR}/base/src/GenException.cc)

add_executable(${PROJECT_NAME} ${ALL_SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE
    ./
    ${CMAKE_SOURCE_DIR}/base/inc
    ${CMAKE_SOURCE_DIR}/fpix/inc
    ${CMAKE_SOURCE_DIR}/3rd_party/inc
    ${CMAKE_SOURCE_DIR}/unit_tests/utils/inc
    )

add_test(NAME ${PROJECT_NAME}
        COMMAND ${PROJECT_BINARY_DIR}/${PROJECT_NAME})
