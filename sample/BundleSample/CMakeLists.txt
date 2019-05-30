# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
project (BundleSample)

if(WIN32)
    set(DESCRIPTION "BundleSample manifest")
    configure_file(${CMAKE_PROJECT_ROOT}/manifest.cmakein ${MSIX_SAMPLE_OUTPUT_DIRECTORY}/${PROJECT_NAME}.exe.manifest CRLF)
    set(MANIFEST ${MSIX_SAMPLE_OUTPUT_DIRECTORY}/${PROJECT_NAME}.exe.manifest)
endif()

add_executable(${PROJECT_NAME}
    BundleSample.cpp
    ${MANIFEST}
    )

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/src/msix)

target_link_libraries(${PROJECT_NAME} msix)

set_target_properties(${PROJECT_NAME} PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${MSIX_SAMPLE_OUTPUT_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${MSIX_SAMPLE_OUTPUT_DIRECTORY}"
  RUNTIME_OUTPUT_DIRECTORY "${MSIX_SAMPLE_OUTPUT_DIRECTORY}"
)

add_dependencies(${PROJECT_NAME} msix)
