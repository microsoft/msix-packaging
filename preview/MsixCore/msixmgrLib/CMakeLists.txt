# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(msixcore-win-lib)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

message(STATUS "--------------------------------")
message(STATUS "MSIX Core Windows Lib")
message(STATUS "--------------------------------")

message(STATUS "CMake version: ${CMAKE_VERSION}")

# specify that this binary is to be built with C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Support CMake 3.12. See https://cmake.org/cmake/help/latest/policy/CMP0077.html
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if((CMAKE_BUILD_TYPE MATCHES RelWithDebInfo) OR (CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES MinSizeRel))
    ADD_DEFINITIONS(-DNDEBUG)
    message(STATUS "NDEBUG defined, assert should be turned-off" )
else()
    message(STATUS "NDEBUG NOT defined, assert should be turned-on" )
endif()

add_library(msixmgrlib STATIC "")

set(MSIXMGRMAIN_SRC_DIR ../msixmgr)

add_compile_definitions(UNICODE)
add_compile_definitions(_UNICODE)

target_sources(msixmgrlib
    PRIVATE
        GeneralUtil.cpp
        GeneralUtil.hpp
        msixmgrActions.cpp
        MsixTraceLoggingProvider.cpp
        MsixTraceLoggingProvider.hpp
        PackageManager.cpp
        PackageManager.hpp
        ${MSIXMGRMAIN_SRC_DIR}/AddRemovePrograms.cpp
        ${MSIXMGRMAIN_SRC_DIR}/AddRemovePrograms.hpp
        ${MSIXMGRMAIN_SRC_DIR}/AppExecutionAlias.cpp
        ${MSIXMGRMAIN_SRC_DIR}/AppExecutionAlias.hpp
        ${MSIXMGRMAIN_SRC_DIR}/AutoPlay.cpp
        ${MSIXMGRMAIN_SRC_DIR}/AutoPlay.hpp
        ${MSIXMGRMAIN_SRC_DIR}/Base32Encoding.cpp
        ${MSIXMGRMAIN_SRC_DIR}/Base32Encoding.hpp
        ${MSIXMGRMAIN_SRC_DIR}/BcryptLibrary.cpp
        ${MSIXMGRMAIN_SRC_DIR}/BcryptLibrary.hpp
        ${MSIXMGRMAIN_SRC_DIR}/ComInterface.cpp
        ${MSIXMGRMAIN_SRC_DIR}/ComInterface.hpp
        ${MSIXMGRMAIN_SRC_DIR}/CommandLineInterface.cpp
        ${MSIXMGRMAIN_SRC_DIR}/CommandLineInterface.hpp
        ${MSIXMGRMAIN_SRC_DIR}/ComServer.cpp
        ${MSIXMGRMAIN_SRC_DIR}/ComServer.hpp
        ${MSIXMGRMAIN_SRC_DIR}/Constants.hpp
        ${MSIXMGRMAIN_SRC_DIR}/CryptoProvider.cpp
        ${MSIXMGRMAIN_SRC_DIR}/CryptoProvider.hpp
        ${MSIXMGRMAIN_SRC_DIR}/ErrorHandler.cpp
        ${MSIXMGRMAIN_SRC_DIR}/ErrorHandler.hpp
        ${MSIXMGRMAIN_SRC_DIR}/Extractor.cpp
        ${MSIXMGRMAIN_SRC_DIR}/Extractor.hpp
        ${MSIXMGRMAIN_SRC_DIR}/FilePaths.cpp
        ${MSIXMGRMAIN_SRC_DIR}/FilePaths.hpp
        ${MSIXMGRMAIN_SRC_DIR}/FileTypeAssociation.cpp
        ${MSIXMGRMAIN_SRC_DIR}/FileTypeAssociation.hpp
        ${MSIXMGRMAIN_SRC_DIR}/FirewallRules.cpp
        ${MSIXMGRMAIN_SRC_DIR}/FirewallRules.hpp
        ${MSIXMGRMAIN_SRC_DIR}/FootprintFiles.hpp
        ${MSIXMGRMAIN_SRC_DIR}/InstallComplete.cpp
        ${MSIXMGRMAIN_SRC_DIR}/InstallComplete.hpp
        ${MSIXMGRMAIN_SRC_DIR}/IPackageHandler.hpp
        ${MSIXMGRMAIN_SRC_DIR}/MsixRequest.cpp
        ${MSIXMGRMAIN_SRC_DIR}/MsixRequest.hpp
        ${MSIXMGRMAIN_SRC_DIR}/MsixResponse.cpp
        ${MSIXMGRMAIN_SRC_DIR}/MsixResponse.hpp
        ${MSIXMGRMAIN_SRC_DIR}/Package.cpp
        ${MSIXMGRMAIN_SRC_DIR}/Package.hpp
        ${MSIXMGRMAIN_SRC_DIR}/PopulatePackageInfo.cpp
        ${MSIXMGRMAIN_SRC_DIR}/PopulatePackageInfo.hpp
        ${MSIXMGRMAIN_SRC_DIR}/PrepareDevirtualizedRegistry.cpp
        ${MSIXMGRMAIN_SRC_DIR}/PrepareDevirtualizedRegistry.hpp
        ${MSIXMGRMAIN_SRC_DIR}/ProcessPotentialUpdate.cpp
        ${MSIXMGRMAIN_SRC_DIR}/ProcessPotentialUpdate.hpp
        ${MSIXMGRMAIN_SRC_DIR}/Protocol.cpp
        ${MSIXMGRMAIN_SRC_DIR}/Protocol.hpp
        ${MSIXMGRMAIN_SRC_DIR}/PSFScriptExecuter.cpp
        ${MSIXMGRMAIN_SRC_DIR}/PSFScriptExecuter.hpp
        ${MSIXMGRMAIN_SRC_DIR}/RegistryDevirtualizer.cpp
        ${MSIXMGRMAIN_SRC_DIR}/RegistryDevirtualizer.hpp
        ${MSIXMGRMAIN_SRC_DIR}/RegistryKey.cpp
        ${MSIXMGRMAIN_SRC_DIR}/RegistryKey.hpp
        ${MSIXMGRMAIN_SRC_DIR}/StartMenuLink.cpp
        ${MSIXMGRMAIN_SRC_DIR}/StartMenuLink.hpp
        ${MSIXMGRMAIN_SRC_DIR}/StartupTask.cpp
        ${MSIXMGRMAIN_SRC_DIR}/StartupTask.hpp
        ${MSIXMGRMAIN_SRC_DIR}/ValidateTargetDeviceFamily.cpp
        ${MSIXMGRMAIN_SRC_DIR}/ValidateTargetDeviceFamily.hpp
        ${MSIXMGRMAIN_SRC_DIR}/VirtualFileHandler.cpp
        ${MSIXMGRMAIN_SRC_DIR}/VirtualFileHandler.hpp
        ${MSIXMGRMAIN_SRC_DIR}/Windows10Redirector.cpp
        ${MSIXMGRMAIN_SRC_DIR}/Windows10Redirector.hpp
        ${MSIXMGRMAIN_SRC_DIR}/WindowsCompatibility.manifest
        ${MSIXMGRMAIN_SRC_DIR}/WriteDevirtualizedRegistry.cpp
        ${MSIXMGRMAIN_SRC_DIR}/WriteDevirtualizedRegistry.hpp
)

target_include_directories(msixmgrlib PRIVATE inc)
target_include_directories(msixmgrlib PRIVATE ${MSIXMGRMAIN_SRC_DIR})
target_include_directories(msixmgrlib PRIVATE ../../../src/inc/public)