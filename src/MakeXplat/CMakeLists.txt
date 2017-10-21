# xplat\src\MakeXplat
# Copyright (C) 2017 Microsoft
# Created by Phil Smith (psmith@microsoft.com) on 10/19/2017

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project (MakeXplat)

set (XPLATAPPX_API 1)

include_directories(
	${include_directories}
	${CMAKE_PROJECT_ROOT}/src/inc
	)

add_executable(MakeXplat
	main.cpp
	)

ADD_DEPENDENCIES(MakeXplat xPlatAppx)
target_link_libraries(MakeXplat xPlatAppx)