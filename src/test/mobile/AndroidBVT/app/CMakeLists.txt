# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

include_directories(
    ${include_directories}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp
)

add_library(native-lib SHARED
            src/main/cpp/native-lib.cpp
)

# Add our msix and test library
add_library(msix-lib SHARED IMPORTED GLOBAL)
add_library(msixtest-lib SHARED IMPORTED GLOBAL)
set_target_properties(msix-lib PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/x86/libmsix.so)
set_target_properties(msixtest-lib PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/x86/libmsixtest.so)

get_target_property(LIB_COMPILE_FLAGS native-lib COMPILE_FLAGS)
if(NOT ${LIB_COMPILE_FLAGS})
    set(LIB_COMPILE_FLAGS "")
endif()
string(APPEND LIB_COMPILE_FLAGS "-Wno-writable-strings")
set_target_properties(native-lib PROPERTIES COMPILE_FLAGS "${LIB_COMPILE_FLAGS}")

target_link_libraries(native-lib PUBLIC msix-lib msixtest-lib android log)
