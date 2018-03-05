# MSIX\lib
# Copyright (C) 2017 Microsoft
# Created by Phil Smith (psmith@microsoft.com) on 10/19/2017
cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

ADD_CUSTOM_TARGET(LIBS)

# ZLIB
#   set(AMD64             OFF CACHE BOOL "Disable building i686 assembly implementation"  FORCE)
#   set(ASM686            OFF CACHE BOOL "Disable building amd64 assembly implementation" FORCE)
set(MINGW               OFF CACHE BOOL "Only build the static lib"                      FORCE)
set(BUILD_SHARED_LIBS   OFF CACHE BOOL "Only build the static lib"                      FORCE)
set(HAVE_OFF64_T        OFF CACHE BOOL "no need to build zlib tests as part of build"   FORCE)
set(SKIP_INSTALL_SHARED ON  CACHE BOOL "Don't install zlib.dll"                         FORCE)
set(ZLIB_TESTS          OFF CACHE BOOL "Don't build zlib tests/examples"                FORCE)
add_subdirectory(zlib)

# Xerces
IF(XML_PARSER MATCHES xerces)
    MESSAGE (STATUS "XML_PARSER defined.  Configuring XERCES-C parser for inclusion as static library." )
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Only build static lib"                         FORCE)
    set(network           OFF CACHE BOOL "Disable network support"                       FORCE)
    set(XERCES_TESTS      OFF CACHE BOOL "Don't build test or samples"                   FORCE)
    add_subdirectory(xerces)
ENDIF()

# OpenSSL
IF(NOT WIN32)
    MESSAGE(STATUS "-----------------------------")
    MESSAGE(STATUS "OpenSLL configuration summary")
    MESSAGE(STATUS "-----------------------------")

    set(OpenSSL_SOURCE_PATH  "${CMAKE_CURRENT_SOURCE_DIR}/openssl" )
    set(OpenSLL_BINARY_PATH  "${CMAKE_CURRENT_BINARY_DIR}" )
    set(OpenSLL_INCLUDE_PATH "${CMAKE_CURRENT_BINARY_DIR}" )

    include(GetOpenSSLRevisionDescription)
    get_openssl_describe(OpenSSL_DESCRIPTION)
    MESSAGE(STATUS "OpenSSL Description '${OpenSSL_DESCRIPTION}'" )

    string(REGEX MATCH "OpenSSL_([0-9]*)_([0-9]*)_([0-9]*)" _dummy1 "${OpenSSL_DESCRIPTION}")
    SET(OpenSSL_MAJOR ${CMAKE_MATCH_1})
    SET(OpenSSL_MINOR ${CMAKE_MATCH_2})
    SET(OpenSSL_PATCH ${CMAKE_MATCH_3})
    SET(OpenSLL_VERSION "${OpenSSL_MAJOR}.${OpenSSL_MINOR}.${OpenSSL_PATCH}")

    set(OpenSSL_COMPILER_DEFINES OPENSSL_NO_ASM)
    # we include which means you need to use "OpenSSL_X_PATH" for your paths.
    include( crypto )
    set(OpenSSL_FOUND ON CACHE BOOL "OpenSSL ready for use" FORCE)

    MESSAGE(STATUS "OpenSLL_VERSION      '${OpenSLL_VERSION}'" )
    MESSAGE(STATUS "OpenSSL_SOURCE_PATH  '${OpenSSL_SOURCE_PATH}'" )
    MESSAGE(STATUS "OpenSLL_BINARY_PATH  '${OpenSLL_BINARY_PATH}'" )
    MESSAGE(STATUS "OpenSLL_INCLUDE_PATH '${OpenSLL_INCLUDE_PATH}'" )
ENDIF()

