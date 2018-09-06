# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project (OverrideStreamSample)

# Define two variables in order not to repeat ourselves.
set(BINARY_NAME OverrideStreamSample)

if(WIN32)
    set(DESCRIPTION "OverrideStreamSample manifest")
    configure_file(../../manifest.cmakein ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.exe.manifest CRLF)
    set(MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.exe.manifest)
endif()


include_directories(
    ${include_directories}
    ${CMAKE_PROJECT_ROOT}/src/inc
    )

add_executable(${BINARY_NAME}
    OverrideStreamSample.cpp
    ${MANIFEST}
    )

# specify that this binary is to be built with C++14
set_property(TARGET ${BINARY_NAME} PROPERTY CXX_STANDARD 14)

ADD_DEPENDENCIES(${BINARY_NAME} msix)
if (LINUX OR AOSP)
    target_link_libraries(${BINARY_NAME} PRIVATE -latomic)
endif()
target_link_libraries(${BINARY_NAME} PRIVATE msix)
