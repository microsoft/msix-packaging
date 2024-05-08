# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.29.0 FATAL_ERROR)
project (OverrideStreamSample)

if(WIN32)
    set(DESCRIPTION "OverrideStreamSample manifest")
    configure_file(${MSIX_PROJECT_ROOT}/manifest.cmakein ${MSIX_SAMPLE_OUTPUT_DIRECTORY}/${PROJECT_NAME}.exe.manifest CRLF)
    set(MANIFEST ${MSIX_SAMPLE_OUTPUT_DIRECTORY}/${PROJECT_NAME}.exe.manifest)
endif()

add_executable(${PROJECT_NAME}
    OverrideStreamSample.cpp
    ${MANIFEST}
    )

target_include_directories(${PROJECT_NAME} PRIVATE ${MSIX_BINARY_ROOT}/src/msix ../inc)

if (LINUX OR AOSP)
    target_link_libraries(${PROJECT_NAME} PRIVATE -latomic)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE msix)

set_target_properties(${PROJECT_NAME} PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${MSIX_SAMPLE_OUTPUT_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${MSIX_SAMPLE_OUTPUT_DIRECTORY}"
  RUNTIME_OUTPUT_DIRECTORY "${MSIX_SAMPLE_OUTPUT_DIRECTORY}"
)

add_dependencies(${PROJECT_NAME} msix)
