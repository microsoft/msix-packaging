# MSIX\src
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.28.0 FATAL_ERROR)

add_subdirectory(msix)
add_subdirectory(makemsix)

if(MSIX_TESTS)
    add_subdirectory(test)
endif()
