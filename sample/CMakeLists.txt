# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.29.0 FATAL_ERROR)

# For windows copy the library to the msix and samples directory
if(WIN32)
    add_custom_target(samples ALL
        COMMAND ${CMAKE_COMMAND} -E copy "bin/msix.dll" "samples/msix.dll"
        WORKING_DIRECTORY "${MSIX_BINARY_ROOT}"
        DEPENDS msix)
else()
    add_custom_target(samples ALL DEPENDS msix)
endif()

set(MSIX_SAMPLE_OUTPUT_DIRECTORY "${MSIX_BINARY_ROOT}/samples")

add_subdirectory(ExtractContentsSample)
add_subdirectory(BundleSample)
add_subdirectory(OverrideStreamSample)
add_subdirectory(OverrideLanguageSample)

if (MSIX_PACK)
    add_subdirectory(PackSample)
    add_dependencies(PackSample msix)
endif()
