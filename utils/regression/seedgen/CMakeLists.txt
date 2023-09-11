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
project(seedgen)

include(CTest)
enable_testing()

# set c++11
set (CMAKE_CXX_STANDARD 11)

# definitions
add_definitions(-DUNIT_TEST)

set(ALL_SRCS 
    ./seedgen.cc
    ${CMAKE_SOURCE_DIR}/base/src/Log.cc
    ${CMAKE_SOURCE_DIR}/base/src/Random.cc
    ${CMAKE_SOURCE_DIR}/base/src/GenException.cc
    ${CMAKE_SOURCE_DIR}/base/src/StringUtils.cc
    ${CMAKE_SOURCE_DIR}/base/src/Enums.cc)

# link libraries
add_library(${PROJECT_NAME} MODULE ${ALL_SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE
    ./
    ${CMAKE_SOURCE_DIR}/base/inc
    ${CMAKE_SOURCE_DIR}/3rd_party/inc
    ${CMAKE_SOURCE_DIR}/unit_tests/utils/inc
    )
target_link_libraries(${PROJECT_NAME} PRIVATE pybind11::module ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

add_test(NAME ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${PROJECT_SOURCE_DIR}/..:$ENV{PYTHONPATH}
            ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/demo.py 
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})


# install
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

install(TARGETS ${PROJECT_NAME} DESTINATION utils/regression/seedgen/bin)