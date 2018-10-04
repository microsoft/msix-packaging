# MSIX\test\mobile\common
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

# This file is also compiled for Android using Gradle. It uses Android SDK's CMake that which is 3.6
cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

project(msixtestcommon)

# Visibility variables for non-win32 platforms
if(IOS)
    # on Apple platforms you can explicitly define which symbols are exported
    set(CMAKE_VISIBILITY_INLINES_HIDDEN     1)
    set(CMAKE_C_VISIBILITY_PRESET           hidden)    
    set(CMAKE_CXX_VISIBILITY_PRESET         hidden)            
    set(DEFINE_EXPORTS                      "-exported_symbols_list ${CMAKE_PROJECT_ROOT}/test/mobile/common/exports.def")    
endif()

# Define the library
add_library(${PROJECT_NAME} SHARED MobileTests.cpp MobileTests.hpp)

if(IOS)
    # Set the build version. It will be used in the name of the lib, with corresponding
    # symlinks created. SOVERSION could also be specified for api version.
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${MSIX_VERSION}  # ${MSIX_VERSION} was defined in the main CMakeLists.
        FRAMEWORK FALSE
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/src/msix)
target_link_libraries(${PROJECT_NAME} PRIVATE msix)
