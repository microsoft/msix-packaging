# MSIX\test\mobile\common
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.
cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake) # main (top) cmake dir
set(LIBRARY_NAME msixtestcommon)
project(${LIBRARY_NAME})

# Visibility variables for non-win32 platforms
IF(IOS)
    # on Apple platforms you can explicitly define which symbols are exported
    set(CMAKE_VISIBILITY_INLINES_HIDDEN     1)
    set(CMAKE_C_VISIBILITY_PRESET           hidden)    
    set(CMAKE_CXX_VISIBILITY_PRESET         hidden)            
    set(DEFINE_EXPORTS                      "-exported_symbols_list ${CMAKE_PROJECT_ROOT}/test/mobile/common/exports.def")    
ENDIF()

set(LIB_PUBLIC_HEADERS
    MobileTests.hpp
)

set(LIB_SOURCES
    MobileTests.cpp
)

# Define the library
add_library(${LIBRARY_NAME} SHARED ${LIB_SOURCES} ${LIB_PUBLIC_HEADERS} )

# specify that this library is to be built with C++14
set_property(TARGET ${LIBRARY_NAME} PROPERTY CXX_STANDARD 14)

# Set the build version. It will be used in the name of the lib, with corresponding
# symlinks created. SOVERSION could also be specified for api version.
set_target_properties(${LIBRARY_NAME} PROPERTIES
	VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
	FRAMEWORK FALSE
	PUBLIC_HEADER "${LIB_PUBLIC_HEADERS}" # specify the public headers
)

include_directories(
    ${include_directories}
    ${CMAKE_PROJECT_ROOT}/src/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE msix)

