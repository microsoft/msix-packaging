# MSIX\src\makemsix
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
project (makemsix)

if(WIN32)
    set(DESCRIPTION "makemsix manifest")
    configure_file(${CMAKE_PROJECT_ROOT}/manifest.cmakein ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.exe.manifest CRLF)
    set(MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.exe.manifest)
endif()

add_executable(${PROJECT_NAME}
    main.cpp
    ${MANIFEST}
    )

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/src/msix)

add_dependencies(${PROJECT_NAME} msix)
target_link_libraries(${PROJECT_NAME} msix)
