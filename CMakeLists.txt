# Top level make project
# Copyright (C) 2017 Microsoft
# Created by Phil Smith (psmith@microsoft.com) on 10/19/2017

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

MESSAGE (STATUS "--------------------------------")
MESSAGE (STATUS "xPlatAppx")
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
MESSAGE (STATUS "xPlatAppx version ${VERSION}")
MESSAGE (STATUS "xPlatAppx branch name ${GIT_BRANCH_NAME}")

# Configure Package.nuspec
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Package.nuspec.cmakein ${CMAKE_CURRENT_BINARY_DIR}/Package.nuspec CRLF)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.xPlat.AppxPackaging.targets ${CMAKE_BINARY_DIR}/build/native/Microsoft.xPlat.AppxPackaging.targets)
MESSAGE (STATUS "Package.Nuspec created")
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
    ENDIF()
    IF (AOSP)
        MESSAGE (STATUS "Building for Android")
    ENDIF()
    IF (LINUX)
        MESSAGE (STATUS "Building for Linux")
        # Static libraries must be position independent to be linked with a shared object.
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    ENDIF()
ENDIF()

# Enforce build type
IF(NOT CMAKE_BUILD_TYPE)
SET(CMAKE_BUILD_TYPE Debug CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel.  Use the -DCMAKE_BUILD_TYPE=[option] to specify."
    FORCE)
ENDIF()

MESSAGE (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

IF ((CMAKE_BUILD_TYPE MATCHES RelWithDebInfo) OR (CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES MinSizeRel))
    ADD_DEFINITIONS(-DNDEBUG)
    MESSAGE (STATUS "NDEBUG defined, assert should be turned-off" )
ELSE()
    MESSAGE (STATUS "NDEBUG NOT defined, assert should be turned-on" )
ENDIF()


IF(MACOS)
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
MESSAGE (STATUS "xPlatAppx")
MESSAGE (STATUS "--------------------------------")
MESSAGE (STATUS "libs processed")
add_subdirectory(src)
MESSAGE (STATUS "src processed")
ADD_DEPENDENCIES(SRC LIBS)
MESSAGE (STATUS "dependencies added")
add_subdirectory(sample)
MESSAGE (STATUS "sample processed")
MESSAGE (STATUS "DONE!")
