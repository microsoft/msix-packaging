# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
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

# Set build options
option(WIN32 "Build for Win32"   OFF)
option(MACOS "Build for MacOS"   OFF)
option(IOS   "Build for iOS"     OFF)
option(AOSP  "Build for Android" OFF)
option(LINUX "Build for Linux"   OFF)

option(USE_VALIDATION_PARSER "Turn on to validates using the resouce schemas. Default (OFF) validates XML files are just valid XML" OFF)
option(USE_SHARED_ZLIB "Choose the type of dependency for zlib, Use the -DUSE_SHARED_ZLIB=on to have a shared dependency. Default is 'off' (static)" OFF)
option(USE_STATIC_MSVC "Windows only. Pass /MT as a compiler flag to use the staic version of the run-time library. Default is 'off' (dynamic)" OFF)
option(SKIP_BUNDLES "Removes bundle functionality from the MSIX SDK. Default is 'off'" OFF)

set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel.  Use the -DCMAKE_BUILD_TYPE=[option] to specify.")
set(XML_PARSER "" CACHE STRING "Choose the type of parser, options are: [xerces, msxml6, javaxml].  Use the -DXML_PARSER=[option] to specify.")
set(CRYPTO_LIB "" CACHE STRING "Choose the cryptography library to use, options are: [openssl, crypt32].  Use the -DCRYPTO_LIB=[option] to specify.")

# Default version is 0.0.0
set(VERSION_MAJOR "0")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")
set(GIT_BRANCH_NAME "master")

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

# Configure Package.nuspec
if(WIN32)
    set(MSIX_NUGET_NAME "Microsoft.MSIX.Packaging.Windows")
elseif(MACOS)
    set(MSIX_NUGET_NAME "Microsoft.MSIX.Packaging.MacOS")
elseif(IOS)
    set(MSIX_NUGET_NAME "Microsoft.MSIX.Packaging.iOS")
elseif(AOSP)
    set(MSIX_NUGET_NAME "Microsoft.MSIX.Packaging.AOSP")
elseif(LINUX)
    set(MSIX_NUGET_NAME "Microsoft.MSIX.Packaging.Linux")
else()
    set(MSIX_NUGET_NAME "Microsoft.MSIX.Packaging") # cmake ..
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Package.nuspec.cmakein ${CMAKE_CURRENT_BINARY_DIR}/Package.nuspec CRLF)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Microsoft.MSIX.Packaging.targets ${CMAKE_BINARY_DIR}/build/native/${MSIX_NUGET_NAME}.targets)
message(STATUS "Package.Nuspec created")
message(STATUS "--------------------------------")

# Configure license txt
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LICENSE ${CMAKE_BINARY_DIR}/build/LICENSE)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/THIRD\ PARTY\ CODE\ NOTICE ${CMAKE_BINARY_DIR}/build/THIRD\ PARTY\ CODE\ NOTICE)
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
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/certs/${CERT_TO_PUBLISH} ${CMAKE_BINARY_DIR}/build/certs/${CERT_TO_PUBLISH})
endforeach()
message(STATUS "Certificates published")
message(STATUS "--------------------------------")    

# Enforce that target platform is specified.
if((NOT WIN32) AND (NOT MACOS) AND (NOT IOS) AND (NOT AOSP) AND (NOT LINUX))
    message(STATUS "You must specify one of: [WIN32|MACOS|IOS|AOSP|LINUX]" )
    message(STATUS "For example, use cmake -DWIN32=on .." )
    return()
else()
    if(WIN32)
        message(STATUS "Building for WIN32")
        if(USE_STATIC_MSVC)
            # By default these flags have /MD set. Modified it to use /MT instead.
            foreach(buildType RELEASE MINSIZEREL RELWITHDEBINFO)
                set(cxxFlag "CMAKE_CXX_FLAGS_${buildType}")
                string(REPLACE "/MD" "/MT" ${cxxFlag} "${${cxxFlag}}")
            endforeach()
            set(cxxFlagDebug "CMAKE_CXX_FLAGS_DEBUG")
            string(REPLACE "/MDd" "/MTd" ${cxxFlagDebug} "${${cxxFlagDebug}}")
        endif()
    endif()
    if(MACOS)
        message(STATUS "Building for MacOS")
    endif()
    if(IOS)
        message(STATUS "Building for iOS")
        if(IOS_DEPLOYMENT_TARGET VERSION_LESS 10.0)
            message(FATAL_ERROR "Unsupported iOS version: ${IOS_DEPLOYMENT_TARGET}, this project requires at least iOS version 10.0")
        endif()
        set(PLATFORM_APPLE 1)
    endif()
    if(AOSP)
        message(STATUS "Building for Android")
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    endif()
    if(LINUX)
        message(STATUS "Building for Linux")
        # Static libraries must be position independent to be linked with a shared object.
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    endif()
endif()

if(NOT XML_PARSER)
    include(CheckIncludeFileCXX)
    check_include_file_cxx(msxml6.h HAVE_MSXML6)
    if(HAVE_MSXML6)
        set(XML_PARSER msxml6 CACHE STRING "Using XML Parser: msxml6" FORCE)
    elseif (AOSP)
        set(XML_PARSER javaxml CACHE STRING "Using XML Parser: javaxml" FORCE)
    else()
        set(XML_PARSER xerces CACHE STRING "Using XML Parser: xerces" FORCE)
    endif()
endif()

if(NOT CRYPTO_LIB)
    include(CheckIncludeFileCXX)
    check_include_file_cxx(wincrypt.h HAVE_CRYPT32)
    if(HAVE_CRYPT32)
        set(CRYPTO_LIB crypt32 CACHE STRING "Using Crypto Lib: crypt32" FORCE)
    else()
        set(CRYPTO_LIB openssl CACHE STRING "Using Crypto Lib: openssl" FORCE)
    endif()
endif()

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
        endif ()
    endif()
endif()

# CMake useful variables
set(CMAKE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

# Create a bin directory for samples and msixtest to be self contained
set(MSIX_SAMPLE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/samples")
set(MSIX_TEST_OUTPUT_DIRECTORY   "${CMAKE_CURRENT_BINARY_DIR}/msixtest")

# Mac needed variables
# [TODO: adapt as needed]
set(CMAKE_MACOSX_RPATH ON)
#set(CMAKE_SKIP_BUILD_RPATH FALSE)
#set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_subdirectory(lib)
message(STATUS " ")
message(STATUS "--------------------------------")
message(STATUS "MSIX Packaging SDK")
message(STATUS "--------------------------------")
message(STATUS "libs processed")
add_subdirectory(src)
message(STATUS "src processed")
ADD_DEPENDENCIES(SRC LIBS)
message(STATUS "dependencies added")
add_subdirectory(sample)
message(STATUS "sample processed")
message(STATUS "DONE!")
