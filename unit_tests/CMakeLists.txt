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
project(unit_tests)

include(CTest)
enable_testing()

# set c++11
set (CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE UNIT_TEST_CMAKE_LIST ${PROJECT_SOURCE_DIR}/tests "CMakeLists.txt")
list(FILTER UNIT_TEST_CMAKE_LIST EXCLUDE REGEX "unit_tests/CMakeLists.txt")
foreach(unit_test_cmake IN LISTS UNIT_TEST_CMAKE_LIST)
    get_filename_component(unit_test_path ${unit_test_cmake} DIRECTORY)
    add_subdirectory(${unit_test_path})
endforeach()