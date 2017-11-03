# xplat\src\MakeXplat
# Copyright (C) 2017 Microsoft
# Created by Phil Smith (psmith@microsoft.com) on 10/19/2017

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project (MakeXplat)

# Define two variables in order not to repeat ourselves.
set(BINARY_NAME ExtractContentsSample)

include_directories(
	${include_directories}
	${CMAKE_PROJECT_ROOT}/src/inc
	)

add_executable(${BINARY_NAME}
	ExtractContentsSample.cpp
	)

# specify that this binary is to be built with C++14
set_property(TARGET ${BINARY_NAME} PROPERTY CXX_STANDARD 14)

ADD_DEPENDENCIES(${BINARY_NAME} xPlatAppx)
target_link_libraries(${BINARY_NAME} xPlatAppx)