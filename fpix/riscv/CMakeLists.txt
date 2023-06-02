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
project(fpix_riscv)

include(CTest)
enable_testing()

# set c++11
set (CMAKE_CXX_STANDARD 11)

# definitions
add_definitions(-DARCH_ENUM_HEADER=<EnumsRISCV.h>)
add_definitions(-DUNIT_TEST)

# custom command
add_custom_command(
  OUTPUT usage.h
  COMMAND cat ${PROJECT_SOURCE_DIR}/README.txt|${PROJECT_SOURCE_DIR}/../utils/readme2usage.py>usage.h
  DEPENDS ${PROJECT_SOURCE_DIR}/../utils/readme2usage.py ${PROJECT_SOURCE_DIR}/README.txt
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# source files
aux_source_directory(./../src FPIX_SRC_LIST)

set(SOURCES
    ./src/main.cc
    ./../../base/src/XmlTreeWalker.cc
    ./../../base/src/Log.cc
    ./../../base/src/Random.cc
    ./../../base/src/SimAPI.cc
    ./../../base/src/GenException.cc
    ./../../base/src/PathUtils.cc
    ./../../base/src/StringUtils.cc
    ./../../base/src/VectorElementUpdates.cc
    ./../../3rd_party/src/pugixml.cc
    ./../../utils/handcar/SimLoader.cc
    ./../../utils/handcar/SimApiHANDCAR.cc
    ${FPIX_SRC_LIST}
    ${CMAKE_CURRENT_BINARY_DIR}/usage.h
    )

# executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME}
                    PRIVATE
                    ./../inc
                    ./../../base/inc
                    ./../../3rd_party/inc
                    ./../../utils/handcar)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION fpix/bin)