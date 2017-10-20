# xplat\src\xPlatAppx
# Copyright (C) 2017 Microsoft
# Created by Phil Smith (psmith@microsoft.com) on 10/19/2017

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake) # main (top) cmake dir

project(xPlatAppx)

set(LIB_SOURCES
	AppxBlockMapObject.cpp
	AppxPackageObject.cpp
	AppxPackaging_i.cpp
	InflateStream.cpp
	xPlatAppx.cpp
	ZipFileStream.cpp
	ZipObject.cpp
	PAL/FileSystem/Win32/DirectoryObject.cpp
	PAL/FileSystem/POSIX/DirectoryObject.cpp
)

# Define two variables in order not to repeat ourselves.
set(LIBRARY_NAME xPlatAppx)

# Define the library
add_library(${LIBRARY_NAME} SHARED ${LIB_SOURCES})

# Set the build version. It will be used in the name of the lib, with corresponding
# symlinks created. SOVERSION could also be specified for api version. 
set_target_properties(${LIBRARY_NAME} PROPERTIES
	VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
	FRAMEWORK FALSE
	PUBLIC_HEADER "${LIB_HEADERS}" # specify the public headers
)

IF(Win32)
	MESSAGE("xPlatAppx takes a static dependency on zlib")
	include_directories(
		${include_directories}
		${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlib
		${CMAKE_PROJECT_ROOT}/lib/zlib
	)
	add_dependencies(${PROJECT_NAME} zlibstatic)
	find_library(ZLIB libz.a)
	target_link_libraries(${PROJECT_NAME} zlibstatic)
ELSE()
	MESSAGE("xPlatAppx takes a dynamic dependency on zlib")
	# TODO: Where/how to bind?
ENDIF()
