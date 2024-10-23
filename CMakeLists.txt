# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.29.0 FATAL_ERROR)
project(msix-sdk)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

message(STATUS "--------------------------------")
message(STATUS "MSIX Packaging SDK")
message(STATUS "--------------------------------")

message(STATUS "CMake version: ${CMAKE_VERSION}")

# specify that this binary is to be built with C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Support CMake 3.12. See https://cmake.org/cmake/help/latest/policy/CMP0077.html
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

# Default version is 0.0.0
set(VERSION_MAJOR "0")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")
set(GIT_BRANCH_NAME "master")

# CMake useful variables
set(MSIX_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(MSIX_BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

## Require that the target is little endian
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    message(FATAL_ERROR "Big endian targets are not supported at this time")
endif()

## Git (and its revision)
find_package(Git) # QUIET) # if we don't find git or FindGit.cmake is not on the system we ignore it.

## GetGitRevisionDescription module to retreive branch and revision information from Git
## Starting with Git 1.9 the module will be part of official cMake distribution, until then it has to be
## part of the application
## The Git module will trigger a reconfiguration for each pull that will bring a new revision on the local repository
set(VCS_REVISION "-1")
if(GIT_FOUND)
    include(GetGitRevisionDescription)
    get_git_head_revision(GIT_REFSPEC GIT_SHA1)
    message(STATUS "GIT refspec ${GIT_REFSPEC}")
    message(STATUS "GIT revision ${GIT_SHA1}")
    set(VCS_REVISION ${GIT_SHA1})

    git_describe(GIT_DESCRIPTION)
    message(STATUS "GIT Description '${GIT_DESCRIPTION}'" )
    string(REGEX MATCH "v([0-9]+)\\.([0-9]+)\\-([0-9]+)" _dummy1 "${GIT_DESCRIPTION}")
    set(VERSION_MAJOR ${CMAKE_MATCH_1})
    set(VERSION_MINOR ${CMAKE_MATCH_2})
    set(VERSION_PATCH ${CMAKE_MATCH_3})

    if(NOT VERSION_PATCH)
    message(STATUS "GIT Description is from NEW tag")
        string(REGEX MATCH "v([0-9]+)\\.([0-9]+)" _dummy2 "${GIT_DESCRIPTION}")
        set(VERSION_MAJOR ${CMAKE_MATCH_1})
        set(VERSION_MINOR ${CMAKE_MATCH_2})
        set(VERSION_PATCH "0")
    endif()

    if(NOT ${GIT_REFSPEC})
        string(REGEX MATCH "refs/heads/([a-zA-Z0-9_/]+)" _dummy3 ${GIT_REFSPEC})
        set(GIT_BRANCH_NAME ${CMAKE_MATCH_1})
    else()
        # VSO doesn't checkout a branch do a pull, it checks out a hash and does a pull
        set(GIT_BRANCH_NAME "master")
    endif()

    message(STATUS "GIT branch name '${GIT_BRANCH_NAME}'" )
else()
    message("git not found.")
endif()

# Set the version number of your project here (format is MAJOR.MINOR.PATCHLEVEL - e.g. 1.0.0)
set(MSIX_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
message(STATUS "MSIX Packaging SDK version ${MSIX_VERSION}")
message(STATUS "MSIX Packaging SDK branch name ${GIT_BRANCH_NAME}")
add_definitions(-DSDK_VERSION="${MSIX_VERSION}")
# Validates CMake options.
include(msix_options)

# Configure Package.nuspec
if(WIN32)
    set(MSIX_NUGET_NAME "Microsoft.MSIX.Packaging.Windows")
elseif(MACOS)
    set(MSIX_NUGET_NAME "Microsoft.MSIX.Packaging.MacOS")
elseif(IOS)
    set(MSIX_NUGET_NAME "Microsoft.MSIX.Packaging.iOS")
elseif(AOSP)
    set(MSIX_NUGET_NAME "Microsoft.MSIX.Packaging.AOSP")
else()
    set(MSIX_NUGET_NAME "Microsoft.MSIX.Packaging.Linux")
endif()

configure_file(${MSIX_PROJECT_ROOT}/Package.nuspec.cmakein ${MSIX_BINARY_ROOT}/Package.nuspec CRLF)
configure_file(${MSIX_PROJECT_ROOT}/Microsoft.MSIX.Packaging.targets ${MSIX_BINARY_ROOT}/build/native/${MSIX_NUGET_NAME}.targets)
message(STATUS "Package.Nuspec created")
message(STATUS "--------------------------------")

# Configure license txt
configure_file(${MSIX_PROJECT_ROOT}/LICENSE ${MSIX_BINARY_ROOT}/build/LICENSE)
configure_file(${MSIX_PROJECT_ROOT}/THIRD\ PARTY\ CODE\ NOTICE ${MSIX_BINARY_ROOT}/build/THIRD\ PARTY\ CODE\ NOTICE)
message(STATUS "LICENSE created")
message(STATUS "--------------------------------")

# Configure certificates
# list each certificate by name that is to be published in the nuget package
list(APPEND CERTS_TO_PUBLISH
    base64_MSFT_RCA_2010.cer
    base64_MSFT_RCA_2011.cer
    base64_STORE_PCA_2011.cer
    base64_Windows_Production_PCA_2011.cer
    base64_Windows_Production.cer
    Microsoft_MarketPlace_PCA_2011.cer
)
foreach(CERT_TO_PUBLISH ${CERTS_TO_PUBLISH})
    configure_file(${MSIX_PROJECT_ROOT}/resources/certs/${CERT_TO_PUBLISH} ${MSIX_BINARY_ROOT}/build/certs/${CERT_TO_PUBLISH})
endforeach()
message(STATUS "Certificates published")
message(STATUS "--------------------------------")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if((CMAKE_BUILD_TYPE MATCHES RelWithDebInfo) OR (CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES MinSizeRel))
    ADD_DEFINITIONS(-DNDEBUG)
    message(STATUS "NDEBUG defined, assert should be turned-off" )
else()
    message(STATUS "NDEBUG NOT defined, assert should be turned-on" )
endif()

if((MACOS) OR (IOS))
    if ((CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES MinSizeRel))
        message(STATUS "optimized build, symbol generation turned-OFF" )
        # on optimized builds, do NOT turn-on symbol generation.
    else()
        message(STATUS "non-optimized build, symbol generation turned-ON" )
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
        endif()
    endif()
    if(IOS)
        if(IOS_DEPLOYMENT_TARGET VERSION_LESS 10.0)
            message(FATAL_ERROR "Unsupported iOS version: ${IOS_DEPLOYMENT_TARGET}, this project requires at least iOS version 10.0")
        endif()
    endif()
elseif(AOSP OR LINUX)
    # Static libraries must be position independent to be linked with a shared object.
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Mac needed variables
# [TODO: adapt as needed]
set(CMAKE_MACOSX_RPATH ON)
#set(CMAKE_SKIP_BUILD_RPATH FALSE)
#set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_subdirectory(lib)
message(STATUS "libs processed")
add_subdirectory(src)
message(STATUS "src processed")
if (MSIX_SAMPLES)
    add_subdirectory(sample)
    message(STATUS "sample processed")
endif()
message(STATUS "DONE!")
