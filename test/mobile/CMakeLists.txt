# MSIX\test\mobile
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

ADD_CUSTOM_TARGET(MOBILE)

if (IOS)
    add_subdirectory(common)
endif()