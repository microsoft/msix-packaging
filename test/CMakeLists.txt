# xplat\test
# Copyright (C) 2017 Microsoft
# Created by Phil Smith (psmith@microsoft.com) on 01/29/2018
cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

ADD_CUSTOM_TARGET(TEST)

IF (IOS)
    add_subdirectory(mobile)
ENDIF()