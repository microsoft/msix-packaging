# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake) # main (top) cmake dir
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_subdirectory(ExtractContentsSample)

ADD_DEPENDENCIES(ExtractContentsSample msix)