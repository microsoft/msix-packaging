# xplat\src\MakeXplat
# Copyright (C) 2017 Microsoft
# Created by Phil Smith (psmith@microsoft.com) on 10/19/2017

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project (MakeXplat)

set (XPLATAPPX_API 1)
add_executable(MakeXPlat main.cpp)