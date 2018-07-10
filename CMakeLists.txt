# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

MESSAGE (STATUS "--------------------------------")
MESSAGE (STATUS "MSIX Packaging SDK")
MESSAGE (STATUS "--------------------------------")

# Set build options
OPTION(WIN32 "Build for Win32"   OFF)
OPTION(MACOS "Build for MacOS"   OFF)
OPTION(IOS   "Build for iOS"     OFF)
OPTION(AOSP  "Build for Android" OFF)
OPTION(LINUX "Build for Linux"   OFF)

# Default version is 0.0.0
SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "0")
SET(VERSION_PATCH "0")
SET(GIT_BRANCH_NAME "master")

## Git (and its revision)
find_package(Git) # QUIET) # if we don't find git or FindGit.cmake is not on the system we ignore it.

## GetGitRevisionDescription module to retreive branch and revision information from Git
## Starting with Git 1.9 the module will be part of official cMake distribution, until then it has to be
## part of the application
## The Git module will trigger a reconfiguration for each pull that will bring a new revision on the local repository
SET(VCS_REVISION "-1")
IF(GIT_FOUND)
    include(GetGitRevisionDescription)
    get_git_head_revision(GIT_REFSPEC GIT_SHA1)
    MESSAGE(STATUS "GIT refspec ${GIT_REFSPEC}")
    MESSAGE(STATUS "GIT revision ${GIT_SHA1}")
    set (VCS_REVISION ${GIT_SHA1})

    git_describe(GIT_DESCRIPTION)
    MESSAGE(STATUS "GIT Description '${GIT_DESCRIPTION}'" )
    string(REGEX MATCH "v([0-9]+)\\.([0-9]+)\\-([0-9]+)" _dummy1 "${GIT_DESCRIPTION}")
    SET(VERSION_MAJOR ${CMAKE_MATCH_1})
    SET(VERSION_MINOR ${CMAKE_MATCH_2})
    SET(VERSION_PATCH ${CMAKE_MATCH_3})

    IF(NOT VERSION_PATCH)
    MESSAGE(STATUS "GIT Description is from NEW tag")
        string(REGEX MATCH "v([0-9]+)\\.([0-9]+)" _dummy2 "${GIT_DESCRIPTION}")
        SET(VERSION_MAJOR ${CMAKE_MATCH_1})
        SET(VERSION_MINOR ${CMAKE_MATCH_2})
        SET(VERSION_PATCH "0")
    ENDIF()

    IF(NOT ${GIT_REFSPEC})
        string(REGEX MATCH "refs/heads/([a-zA-Z0-9_/]+)" _dummy3 ${GIT_REFSPEC})
        SET(GIT_BRANCH_NAME ${CMAKE_MATCH_1})
    ELSE()
        # VSO doesn't checkout a branch do a pull, it checks out a hash and does a pull
        SET(GIT_BRANCH_NAME "master")
    ENDIF()

    MESSAGE(STATUS "GIT branch name '${GIT_BRANCH_NAME}'" )
ELSE()
    MESSAGE("git not found.")
ENDIF()

# Set the version number of your project here (format is MAJOR.MINOR.PATCHLEVEL - e.g. 1.0.0)
SET(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
MESSAGE (STATUS "MSIX Packaging SDK version ${VERSION}")
MESSAGE (STATUS "MSIX Packaging SDK branch name ${GIT_BRANCH_NAME}")

# Configure Package.nuspec
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Package.nuspec.cmakein ${CMAKE_CURRENT_BINARY_DIR}/Package.nuspec CRLF)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.MSIX.Packaging.targets ${CMAKE_BINARY_DIR}/build/native/Microsoft.MSIX.Packaging.targets)
MESSAGE (STATUS "Package.Nuspec created")
MESSAGE (STATUS "--------------------------------")

# Configure license txt
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LICENSE ${CMAKE_BINARY_DIR}/build/LICENSE)
MESSAGE (STATUS "LICENSE created")
MESSAGE (STATUS "--------------------------------")    

# Configure certificates
# list each certificate by name that is to be published in the nuget package
LIST(APPEND CERTS_TO_PUBLISH
    base64_MSFT_RCA_2010.cer
    base64_MSFT_RCA_2011.cer
    base64_STORE_PCA_2011.cer
    base64_Windows_Production_PCA_2011.cer
    base64_Windows_Production.cer
    Microsoft_MarketPlace_PCA_2011.cer
)
FOREACH(CERT_TO_PUBLISH ${CERTS_TO_PUBLISH})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/certs/${CERT_TO_PUBLISH} ${CMAKE_BINARY_DIR}/build/certs/${CERT_TO_PUBLISH})
ENDFOREACH()
MESSAGE (STATUS "Certificates published")
MESSAGE (STATUS "--------------------------------")    

# Enforce that target platform is specified.
IF((NOT WIN32) AND (NOT MACOS) AND (NOT IOS) AND (NOT AOSP) AND (NOT LINUX))
    MESSAGE (STATUS "You must specify one of: [WIN32|MACOS|IOS|AOSP|LINUX]" )
    MESSAGE (STATUS "For example, use cmake -DWIN32=on .." )
    RETURN()
ELSE()
    IF (WIN32)
        MESSAGE (STATUS "Building for WIN32")
    ENDIF()
    IF (MACOS)
        MESSAGE (STATUS "Building for MacOS")
    ENDIF()
    IF (IOS)
        MESSAGE (STATUS "Building for iOS")
        IF (IOS_DEPLOYMENT_TARGET VERSION_LESS 10.0)
            MESSAGE(FATAL_ERROR "Unsupported iOS version: ${IOS_DEPLOYMENT_TARGET}, this project requires at least iOS version 10.0")
        ENDIF()
        SET(PLATFORM_APPLE 1)
    ENDIF()
    IF (AOSP)
        MESSAGE (STATUS "Building for Android")
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    ENDIF()
    IF (LINUX)
        MESSAGE (STATUS "Building for Linux")
        # Static libraries must be position independent to be linked with a shared object.
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    ENDIF()
ENDIF()

# Enforce parser PAL
IF(NOT USE_VALIDATION_PARSER)
    MESSAGE (STATUS "To choose type of validation, Use the -DUSE_VALIDATION_PARSER=on to enable.  Default is 'off'")
ENDIF()

IF(NOT XML_PARSER)
    MESSAGE (STATUS "Choose the type of parser, options are: [xerces, msxml6].  Use the -DXML_PARSER=[option] to specify.")
    INCLUDE(CheckIncludeFileCXX)
    CHECK_INCLUDE_FILE_CXX(msxml6.h HAVE_MSXML6)

    IF (HAVE_MSXML6)
        SET(XML_PARSER msxml6 CACHE STRING "Using msxml6." FORCE)
    ELSE()
        SET(XML_PARSER xerces CACHE STRING "Using xerces" FORCE)
    ENDIF()
ENDIF()

# Enforce build type
IF(NOT CMAKE_BUILD_TYPE)
SET(CMAKE_BUILD_TYPE Debug CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel.  Use the -DCMAKE_BUILD_TYPE=[option] to specify."
    FORCE)
ENDIF()

# Enforce build libraries
IF(NOT USE_SHARED_ZLIB)
    MESSAGE (STATUS "Choose the type of dependency for zlib, Use the -DUSE_SHARED_ZLIB=on to have a shared dependency.  Default is 'off' (static)")
ENDIF()

IF((NOT USE_SHARED_OPENSSL) AND (NOT WIN32))
    MESSAGE (STATUS "Choose the type of dependency for openssl, Use the -DUSE_SHARED_OPENSSL=on to have a shared dependency.  Default is 'off' (static)")
ENDIF()

MESSAGE (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

IF ((CMAKE_BUILD_TYPE MATCHES RelWithDebInfo) OR (CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES MinSizeRel))
    ADD_DEFINITIONS(-DNDEBUG)
    MESSAGE (STATUS "NDEBUG defined, assert should be turned-off" )
ELSE()
    MESSAGE (STATUS "NDEBUG NOT defined, assert should be turned-on" )
ENDIF()


IF((MACOS) OR (IOS))
    if ((CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES MinSizeRel))
        MESSAGE (STATUS "optimized build, symbol generation turned-OFF" )
        # on optimized builds, do NOT turn-on symbol generation.
    else()
        MESSAGE (STATUS "non-optimized build, symbol generation turned-ON" )
        # Incredibly, for both clang and g++, while a single compile-and-link
        # invocation will create an executable.dSYM/ dir with debug info,
        # with separate compilation the final link does NOT create the
        # dSYM dir.
        # The "dsymutil" program will create the dSYM dir for us.
        # Strangely it takes in the executable and not the object
        # files even though it's the latter that contain the debug info.
        # Thus it will only work if the object files are still sitting around.        
        find_program(DSYMUTIL_PROGRAM dsymutil)
        if (DSYMUTIL_PROGRAM)
        set(CMAKE_C_LINK_EXECUTABLE
            "${CMAKE_C_LINK_EXECUTABLE}"
            "${DSYMUTIL_PROGRAM} <TARGET>")
        set(CMAKE_C_CREATE_SHARED_LIBRARY
            "${CMAKE_C_CREATE_SHARED_LIBRARY}"
            "${DSYMUTIL_PROGRAM} <TARGET>")
        set(CMAKE_CXX_LINK_EXECUTABLE
            "${CMAKE_CXX_LINK_EXECUTABLE}"
            "${DSYMUTIL_PROGRAM} <TARGET>")
        set(CMAKE_CXX_CREATE_SHARED_LIBRARY
            "${CMAKE_CXX_CREATE_SHARED_LIBRARY}"
            "${DSYMUTIL_PROGRAM} <TARGET>")
        endif ()
    endif()
ENDIF()

enable_testing() # needed on top-level CMakeLists.txt

# CMake useful variables
SET(CMAKE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Mac needed variables
# [TODO: adapt as needed]
SET(CMAKE_MACOSX_RPATH ON)
#SET(CMAKE_SKIP_BUILD_RPATH FALSE)
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_subdirectory(lib)
MESSAGE (STATUS " ")
MESSAGE (STATUS "--------------------------------")
MESSAGE (STATUS "MSIX Packaging SDK")
MESSAGE (STATUS "--------------------------------")
MESSAGE (STATUS "libs processed")
add_subdirectory(src)
MESSAGE (STATUS "src processed")
ADD_DEPENDENCIES(SRC LIBS)
MESSAGE (STATUS "dependencies added")
add_subdirectory(sample)
MESSAGE (STATUS "sample processed")
add_subdirectory(test)
MESSAGE (STATUS "tests processed")
MESSAGE (STATUS "DONE!")
