# xplat\src\xPlatAppx
# Copyright (C) 2017 Microsoft
# Created by Phil Smith (psmith@microsoft.com) on 10/19/2017

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake) # main (top) cmake dir

set(LIBRARY_NAME xPlatAppx)

project(${LIBRARY_NAME})

# Define PALs
IF(WIN32)
	set (XPLATAPPX_API=1)
	set (DirectoryObject PAL/FileSystem/Win32/DirectoryObject.cpp)
ELSE()
	set (DirectoryObject PAL/FileSystem/POSIX/DirectoryObject.cpp)
ENDIF()

MESSAGE (STATUS "PAL: DirectoryObject = ${DirectoryObject}")

set(LIB_PUBLIC_HEADERS
    ../inc/AppxPackaging.hpp
    ../inc/AppxWindows.hpp
)

set(LIB_PRIVATE_HEADERS
    ../inc/AppxBlockMapObject.hpp
    ../inc/AppxFactory.hpp
    ../inc/AppxPackageObject.hpp
    ../inc/ComHelper.hpp
    ../inc/DirectoryObject.hpp
    ../inc/Exceptions.hpp
    ../inc/FileStream.hpp
    ../inc/InflateStream.hpp
    ../inc/ObjectBase.hpp
    ../inc/StorageObject.hpp
    ../inc/StreamBase.hpp
    ../inc/VerifierObject.hpp
    ../inc/XmlObject.hpp
    ../inc/ZipFileStream.hpp
    ../inc/ZipObject.hpp
)

set(LIB_SOURCES
	AppxBlockMapObject.cpp
	AppxPackageObject.cpp
	AppxPackaging_i.cpp
	InflateStream.cpp
	xPlatAppx.cpp
	ZipFileStream.cpp
	ZipObject.cpp
	${DirectoryObject}
)

# Copy out public headers
 configure_file(../inc/AppxWindows.hpp   ${CMAKE_CURRENT_BINARY_DIR}/AppxWindows.hpp  )
 configure_file(../inc/AppxPackaging.hpp ${CMAKE_CURRENT_BINARY_DIR}/AppxPackaging.hpp)

# Define the library
add_library(${LIBRARY_NAME} SHARED ${LIB_SOURCES} ${LIB_PUBLIC_HEADERS} ${LIB_PRIVATE_HEADERS})

# specify that this library is to be built with C++14
set_property(TARGET ${LIBRARY_NAME} PROPERTY CXX_STANDARD 14)

# Set the build version. It will be used in the name of the lib, with corresponding
# symlinks created. SOVERSION could also be specified for api version. 
set_target_properties(${LIBRARY_NAME} PROPERTIES
	VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
	FRAMEWORK FALSE
	PUBLIC_HEADER "${LIB_HEADERS}" # specify the public headers
)

MESSAGE(STATUS "xPlatAppx takes a static dependency on zlib")
include_directories(
	${include_directories}
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlib
	${CMAKE_PROJECT_ROOT}/lib/zlib
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/xerces/src
	${CMAKE_PROJECT_ROOT}/lib/xerces/src
)

target_link_libraries(${PROJECT_NAME} zlibstatic)
target_link_libraries(${PROJECT_NAME} xerces-c)
