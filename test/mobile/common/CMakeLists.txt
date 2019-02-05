# MSIX\test\mobile\common
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

# We only use this library for the iOS BVTs. Parent CMakeLists only includes
# this directory for iOS. If this fails building in non-iOS we are doing 
# something wrong...

project(msixtestcommon)

# on Apple platforms you can explicitly define which symbols are exported
set(CMAKE_VISIBILITY_INLINES_HIDDEN     1)
set(CMAKE_C_VISIBILITY_PRESET           hidden)    
set(CMAKE_CXX_VISIBILITY_PRESET         hidden)            
set(DEFINE_EXPORTS                      "-exported_symbols_list ${CMAKE_PROJECT_ROOT}/test/mobile/common/exports.def")    

include_directories(
    ${include_directories}
    ../../api
)

# Define the library
add_library(${PROJECT_NAME} SHARED MobileTests.cpp ../../api/ApiTests.cpp)

# Set the build version. It will be used in the name of the lib, with corresponding
# symlinks created. SOVERSION could also be specified for api version.
set_target_properties(${PROJECT_NAME} PROPERTIES
    FRAMEWORK FALSE
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/src/msix)
target_link_libraries(${PROJECT_NAME} PRIVATE msix)
