# MSIX\test\msixtest
# Copyright (C) 2019 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.29.0 FATAL_ERROR)
project (msixtest)

if(WIN32)
    set(DESCRIPTION "msixtest manifest")
    configure_file(${MSIX_PROJECT_ROOT}/manifest.cmakein ${MSIX_TEST_OUTPUT_DIRECTORY}/${PROJECT_NAME}.exe.manifest CRLF)
    set(MANIFEST ${MSIX_TEST_OUTPUT_DIRECTORY}/${PROJECT_NAME}.exe.manifest)
    if(XML_PARSER MATCHES msxml6)
        add_definitions(-DMSIX_MSXML6=1)
    endif()
endif()

# Enable differentiation of based on being used in the tests. 
# Keep usage of this to a minimum; it is primarily intended for Exceptions.hpp
add_definitions(-DMSIX_TEST=1)

set(MsixTestFiles)

# Functional tests
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
    add_definitions(-DMSIX_PACK=1)
    list(APPEND MsixTestFiles
        pack.cpp
        api_packagewriter.cpp
        testData/PackTestData.cpp
        )
    if (WIN32)
        list(APPEND MsixTestFiles
            PAL/Pack/Windows/PackValidation.cpp
        )
    else()
        list(APPEND MsixTestFiles
            PAL/Pack/NonWindows/PackValidation.cpp
        )
    endif()
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

# Unit tests
list(APPEND MsixTestFiles
    TimeHelpers_ut.cpp
)

list(APPEND MsixTestFiles
    ${MSIX_PROJECT_ROOT}/src/msix/common/TimeHelpers.cpp
)

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

target_include_directories(${PROJECT_NAME} PRIVATE
    ${MSIX_PROJECT_ROOT}/src/inc/public
    ${MSIX_PROJECT_ROOT}/lib/catch2
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${MSIX_PROJECT_ROOT}/src/inc/shared
    ${MSIX_PROJECT_ROOT}/src/inc/common)

# Output test binaries into a test directory
set_target_properties(${PROJECT_NAME} PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${MSIX_TEST_OUTPUT_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${MSIX_TEST_OUTPUT_DIRECTORY}"
  RUNTIME_OUTPUT_DIRECTORY "${MSIX_TEST_OUTPUT_DIRECTORY}"
)

set(MsixTestLibs)

list(APPEND MsixTestLibs msix)

if (AOSP)
    # Catch2 depends on the Android logging library, so we need to make it
    # available at link-time to avoid unresolved symbol errors
    list(APPEND MsixTestLibs log)
endif()

add_dependencies(${PROJECT_NAME} msix)
target_link_libraries(${PROJECT_NAME} ${MsixTestLibs})

# For windows copy the library
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${MSIX_BINARY_ROOT}/bin/${CMAKE_CFG_INTDIR}/msix.dll" "${MSIX_TEST_OUTPUT_DIRECTORY}/msixtest/msix.dll")
endif()
