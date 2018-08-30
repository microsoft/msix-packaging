# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake) # main (top) cmake dir
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_subdirectory(ExtractContentsSample)
add_subdirectory(BundleSample)
add_subdirectory(OverrideStreamSample)

add_dependencies(ExtractContentsSample msix)
add_dependencies(BundleSample msix)
add_dependencies(OverrideStreamSample msix)
