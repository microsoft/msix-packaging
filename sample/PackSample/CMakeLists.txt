# Copyright (C) 2019 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
project (PackSample)

# Define two variables in order not to repeat ourselves.
set(BINARY_NAME PackSample)

if(WIN32)
    set(DESCRIPTION "PackSample manifest")
    configure_file(${CMAKE_PROJECT_ROOT}/manifest.cmakein ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.exe.manifest CRLF)
    set(MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.exe.manifest)
endif()

add_executable(${BINARY_NAME}
    PackSample.cpp
    ${MANIFEST}
    )

target_include_directories(${BINARY_NAME} PRIVATE ${CMAKE_BINARY_DIR}/src/msix ../inc)

add_dependencies(${BINARY_NAME} msix)
target_link_libraries(${BINARY_NAME} msix)