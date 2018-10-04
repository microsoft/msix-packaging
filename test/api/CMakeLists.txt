# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
project(apitest)

# Define two variables in order not to repeat ourselves.
set(BINARY_NAME apitest)

if(XML_PARSER MATCHES msxml6)
    add_definitions(-DUSING_MSXML=1)
endif()

if(WIN32)
    set(DESCRIPTION "apitest manifest")
    configure_file(${CMAKE_PROJECT_ROOT}/manifest.cmakein ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.exe.manifest CRLF)
    set(MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.exe.manifest)
endif()

add_executable(${BINARY_NAME}
    ApiTest.cpp
    ${MANIFEST}
    )

target_include_directories(${BINARY_NAME} PRIVATE ${CMAKE_BINARY_DIR}/src/msix)

add_dependencies(${BINARY_NAME} msix)
target_link_libraries(${BINARY_NAME} msix)
