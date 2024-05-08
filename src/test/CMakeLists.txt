# MSIX\src\test
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.29.0 FATAL_ERROR)

set(MSIX_TEST_OUTPUT_DIRECTORY "${MSIX_BINARY_ROOT}/msixtest")

# Copy the test packages to the test output directory to don't have to use
# ..\..\src\test\testData\<path>
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/testData" DESTINATION "${MSIX_TEST_OUTPUT_DIRECTORY}")

# Used for flat tests. Remove assets.appx to make it invalid
file(COPY "${MSIX_TEST_OUTPUT_DIRECTORY}/testData/unpack/flat/"
    DESTINATION "${MSIX_TEST_OUTPUT_DIRECTORY}/testData/unpack/badflat/"
    FILES_MATCHING
        PATTERN "*"
        PATTERN "assets*" EXCLUDE)

if(MSIX_PACK)
    # Remove AppxManifest.xml
    file(COPY "${MSIX_TEST_OUTPUT_DIRECTORY}/testData/pack/input/"
        DESTINATION "${MSIX_TEST_OUTPUT_DIRECTORY}/testData/pack/input_nomanifest/"
        FILES_MATCHING
            PATTERN "*"
            PATTERN "AppxManifest.xml" EXCLUDE)
    
    # Remove AppxManifest.xml and add it it another folder
    file(COPY "${MSIX_TEST_OUTPUT_DIRECTORY}/testData/pack/input/"
        DESTINATION "${MSIX_TEST_OUTPUT_DIRECTORY}/testData/pack/input_misplace_manifest/"
        FILES_MATCHING
            PATTERN "*"
            PATTERN "AppxManifest.xml" EXCLUDE)
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/testData/pack/input/AppxManifest.xml"
         DESTINATION "${MSIX_TEST_OUTPUT_DIRECTORY}/testData/pack/input_misplace_manifest/folder/")

    # Add bad file in the input directory
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/testData/pack/input/"
         DESTINATION "${MSIX_TEST_OUTPUT_DIRECTORY}/testData/pack/input_wrongfile")
    file(WRITE "${MSIX_TEST_OUTPUT_DIRECTORY}/testData/pack/input_wrongfile/_Rels/.rels"
        "This is a fake file")
endif()

add_subdirectory(msixtest)
