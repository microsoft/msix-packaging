# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

include_directories(
    ${include_directories}
    ../../common
    ../../../../src/inc
    ../../../api
)

add_library(native-lib SHARED
            ../../common/MobileTests.cpp
            src/main/cpp/native-lib.cpp
            ../../../api/ApiTests.cpp
)

# Add our msix library
add_library(msix-lib SHARED IMPORTED GLOBAL)
set_target_properties(msix-lib PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/x86/libmsix.so)

target_link_libraries(native-lib msix-lib android log)