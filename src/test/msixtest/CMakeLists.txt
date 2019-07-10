# MSIX\test\msixtest
# Copyright (C) 2019 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
project (msixtest)

if(WIN32)
    set(DESCRIPTION "msixtest manifest")
    configure_file(${CMAKE_PROJECT_ROOT}/manifest.cmakein ${MSIX_TEST_OUTPUT_DIRECTORY}/${PROJECT_NAME}.exe.manifest CRLF)
    set(MANIFEST ${MSIX_TEST_OUTPUT_DIRECTORY}/${PROJECT_NAME}.exe.manifest)
    if(XML_PARSER MATCHES msxml6)
        add_definitions(-DMSIX_MSXML6=1)
    endif()
endif()

set(MsixTestFiles)

list(APPEND MsixTestFiles
    msixtest.cpp
    unpack.cpp
    api_packagereader.cpp
    api_manifestreader.cpp
    api_blockmapreader.cpp
    testData/UnpackTestData.cpp
    testData/BlockMapTestData.cpp
)

if(SKIP_BUNDLES)
    list(APPEND MsixTestFiles
        no_bundle.cpp)
else()
    list(APPEND MsixTestFiles
        unbundle.cpp
        api_bundlereader.cpp
        api_bundlemanifestreader.cpp
        testData/UnbundleTestData.cpp
    )
endif()

if(MSIX_PACK)
    list(APPEND MsixTestFiles
        api_packagewriter.cpp
        testData/PackTestData.cpp
        )
endif()

if (WIN32)
    list(APPEND MsixTestFiles
        "PAL/File/WIN32/FileHelpers.cpp"
    )
else()
    list(APPEND MsixTestFiles
        "PAL/File/POSIX/FileHelpers.cpp"
    )
endif()

# For mobile, we create a shared library that will be added to the apps to be
# tested. For non-mobile we create a msixtest executable and we provide our own main
if (AOSP OR IOS)
    add_library(${PROJECT_NAME} SHARED
        ${MsixTestFiles}
        )
else()
    add_executable(${PROJECT_NAME}
        main.cpp
        msixtest.cpp
        ${MsixTestFiles}
        ${MANIFEST}
        )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/src/msix ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# Output test binaries into a test directory
set_target_properties(${PROJECT_NAME} PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${MSIX_TEST_OUTPUT_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${MSIX_TEST_OUTPUT_DIRECTORY}"
  RUNTIME_OUTPUT_DIRECTORY "${MSIX_TEST_OUTPUT_DIRECTORY}"
)

add_dependencies(${PROJECT_NAME} msix)
target_link_libraries(${PROJECT_NAME} msix)
