# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(apitest)

# Define two variables in order not to repeat ourselves.
set(BINARY_NAME apitest)

IF (XML_PARSER MATCHES xerces)
    add_definitions(-DUSING_XERCES=1)
ENDIF()

if(XML_PARSER MATCHES javaxml)
    add_definitions(-DUSING_JAVAXML=1)
endif()

IF (XML_PARSER MATCHES msxml6)
    add_definitions(-DUSING_MSXML=1)
ENDIF()

if(WIN32)
    set(DESCRIPTION "apitest manifest")
    configure_file(../../manifest.cmakein ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.exe.manifest CRLF)
    set(MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.exe.manifest)
endif()

include_directories(
    ${include_directories}
    ${CMAKE_PROJECT_ROOT}/src/inc
    )

add_executable(${BINARY_NAME}
    ApiTest.cpp
    ${MANIFEST}
    )

# specify that this binary is to be built with C++14
set_property(TARGET ${BINARY_NAME} PROPERTY CXX_STANDARD 14)

add_dependencies(${BINARY_NAME} msix)
target_link_libraries(${BINARY_NAME} msix)
