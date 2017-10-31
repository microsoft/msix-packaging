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
    # TODO: Apple deprecated OpenSSL because there isn't a stable ABI.
    # we should just build OpenSSL and statically include the library
    # pretty much like how everyone else uses OpenSSL. 

    # TODO: remove this temporary hack to unblock Thomas after getting
    # OpenSSL building and statically linking via the submodule dependency
    # https://cmake.org/cmake/help/v3.0/module/FindOpenSSL.html
    set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl") # path is populated by brew
    find_package(OpenSSL REQUIRED)

    IF(OPENSSL_FOUND)
        MESSAGE (STATUS "Using OpenSSL ${OPENSSL_VERSION}")
        include_directories(
            ${include_directories}
            ${OPENSSL_INCLUDE_DIRS}
        )

    ELSE()
        # ... and were done here...  :/
        MESSAGE (STATUS "OpenSSL NOT FOUND!")
        MESSAGE (STATUS "on MacOS run: 'brew install openssl'")
        RETURN()
    ENDIF()

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
    ../inc/AppxSignature.hpp
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
    AppxSignature.cpp
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

IF(OPENSSL_FOUND)
    # include the libraries needed to use OpenSSL
    target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
ENDIF()
