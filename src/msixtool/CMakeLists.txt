# MSIX\src\msixtool
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
project (msixtool)

# Define two variables in order not to repeat ourselves.
set(BINARY_NAME msixtool)

if(WIN32)
    set(DESCRIPTION "msixtool manifest")
    configure_file(${CMAKE_PROJECT_ROOT}/manifest.cmakein ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.exe.manifest CRLF)
    set(MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.exe.manifest)
endif()

add_executable(${BINARY_NAME}
    main.cpp
    ${MANIFEST}
    )

target_include_directories(${BINARY_NAME} PRIVATE ${CMAKE_BINARY_DIR}/src/msix)

add_dependencies(${BINARY_NAME} msix)
target_link_libraries(${BINARY_NAME} msix)

# msixtool used to be named makemsix. For now, add a custom rule to create a copy and name it makemsix.
if(WIN32)
    add_custom_command(TARGET ${BINARY_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy "${BINARY_NAME}.exe" "makemsix.exe"
                    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                    )
else()
    add_custom_command(TARGET ${BINARY_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy "${BINARY_NAME}" "makemsix"
                    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                    OUTPUT_QUIET)
endif()
