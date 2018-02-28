# MSIX\src\makemsix
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project (makemsix)

# Define two variables in order not to repeat ourselves.
set(BINARY_NAME makemsix)

include_directories(
	${include_directories}
	${CMAKE_PROJECT_ROOT}/src/inc
	)

add_executable(${BINARY_NAME}
	main.cpp
	)

# specify that this binary is to be built with C++14
set_property(TARGET ${BINARY_NAME} PROPERTY CXX_STANDARD 14)

ADD_DEPENDENCIES(${BINARY_NAME} msix)
target_link_libraries(${BINARY_NAME} msix)