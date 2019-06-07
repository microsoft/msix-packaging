# MSIX\src
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
add_custom_target(SRC)
add_subdirectory(msix)
add_subdirectory(makemsix)
add_subdirectory(test)
