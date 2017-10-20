# Top level make project
# Copyright (C) 2017 Microsoft
# Created by Phil Smith (psmith@microsoft.com) on 10/19/2017

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Set build options
OPTION(Win32 "Build for Win32"   OFF)
OPTION(MacOS "Build for MacOS"   OFF)
OPTION(iOS   "Build for iOS"     OFF)
OPTION(AOSP  "Build for Android" OFF)
OPTION(LINUX "Build for Linux"   OFF)

# Enforce that target platform is specified.
IF((NOT Win32) AND (NOT MacOS) AND (NOT iOS) AND (NOT AOSP) AND (NOT Linux))
    MESSAGE( "You must specify one of: [Win32|MacOS|iOS|AOSP|Linux]" )
    MESSAGE( "For example, use cmake -DWin32=on .." )
    RETURN()
ENDIF()

# TODO: specify c++ 14
#ADD_DEFINITIONS(-std=c++14)

enable_testing() # needed on top-level CMakeLists.txt

# CMake useful variables
SET(CMAKE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

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
    MESSAGE(STATUS "GIT branch ${GIT_REFSPEC}")
    MESSAGE(STATUS "GIT revision ${GIT_SHA1}")
    set (VCS_REVISION ${GIT_SHA1})
ENDIF()

# Set the version number of your project here (format is MAJOR.MINOR.PATCHLEVEL - e.g. 1.0.0)
SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "0")
SET(VERSION_PATCH "0")
SET(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# Mac needed variables 
# [TODO: adapt as needed]
SET(CMAKE_MACOSX_RPATH ON)
#SET(CMAKE_SKIP_BUILD_RPATH FALSE)
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Target definitions:
# Sub projects must all define a custom target but with different names to avoid conflicts.
# Each of these define what we want to issue when building from the top directory while 
# being able to issue the same target in sub directories *when* we compile only a subproject.

add_custom_target(xPlatAppx_library)
add_custom_target(MakeXplat_tool)
ADD_DEPENDENCIES(xPlatAppx_library zlibstatic)
ADD_DEPENDENCIES(MakeXplat_tool xPlatAppx_library)

add_subdirectory(lib)
add_subdirectory(src)
