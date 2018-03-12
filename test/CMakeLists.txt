# MSIX\test
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.
cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

ADD_CUSTOM_TARGET(TEST)

IF (IOS)
    add_subdirectory(mobile)
ENDIF()