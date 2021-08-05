# MSIX\lib
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.
cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)

add_custom_target(LIBS)

if((NOT ((MACOS) OR (IOS) OR (AOSP))) OR USE_MSIX_SDK_ZLIB)
    # For mac and ios we use inbox libcompression apis.
    # ZLIB
    #   set(AMD64             OFF CACHE BOOL "Disable building i686 assembly implementation"  FORCE)
    #   set(ASM686            OFF CACHE BOOL "Disable building amd64 assembly implementation" FORCE)
    set(MINGW               OFF CACHE BOOL "Only build the static lib"                      FORCE)
    set(HAVE_OFF64_T        OFF CACHE BOOL "no need to build zlib tests as part of build"   FORCE)
    set(SKIP_INSTALL_SHARED ON  CACHE BOOL "Don't install zlib.dll"                         FORCE)
    set(ZLIB_TESTS          OFF CACHE BOOL "Don't build zlib tests/examples"                FORCE)
    if(USE_SHARED_ZLIB)
        set(BUILD_SHARED_LIBS   ON  CACHE BOOL  "Build dll"                                 FORCE)
    else()
        set(BUILD_SHARED_LIBS   OFF CACHE BOOL  "Build dll"                                 FORCE)
    endif()
    add_subdirectory(zlib)
endif()

# Xerces
if(XML_PARSER MATCHES xerces)
    message(STATUS "Configuring XERCES-C parser for inclusion as static library." )
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Only build static lib"                         FORCE)
    set(network           OFF CACHE BOOL "Disable network support"                       FORCE)
    set(XERCES_TESTS      OFF CACHE BOOL "Don't build test or samples"                   FORCE)
    add_subdirectory(xerces)
    if(NOT WIN32)
        get_target_property(XERCES_COMPILE_FLAGS xerces-c COMPILE_FLAGS)
        if(NOT ${XERCES_COMPILE_FLAGS})
            set(XERCES_COMPILE_FLAGS "")
        endif()
        string(APPEND XERCES_COMPILE_FLAGS "-Wno-cast-qual -Wno-unused-parameter -Wno-unused-variable -Wno-memset-transposed-args")
        set_target_properties(xerces-c PROPERTIES COMPILE_FLAGS "${XERCES_COMPILE_FLAGS}")
    endif()
endif()

# OpenSSL
if(CRYPTO_LIB MATCHES openssl)
    message(STATUS "-----------------------------")
    message(STATUS "OpenSLL configuration summary")
    message(STATUS "-----------------------------")

    set(OpenSSL_SOURCE_PATH  "${CMAKE_CURRENT_SOURCE_DIR}/openssl" )
    set(OpenSLL_BINARY_PATH  "${CMAKE_CURRENT_BINARY_DIR}" )
    set(OpenSLL_INCLUDE_PATH "${CMAKE_CURRENT_BINARY_DIR}" )

    set(OpenSLL_VERSION "1.1.1")

    set(OpenSSL_COMPILER_DEFINES OPENSSL_NO_ASM)
    # we include which means you need to use "OpenSSL_X_PATH" for your paths.
    include( crypto )
    set(OpenSSL_FOUND ON CACHE BOOL "OpenSSL ready for use" FORCE)

    message(STATUS "OpenSLL_VERSION      '${OpenSLL_VERSION}'" )
    message(STATUS "OpenSSL_SOURCE_PATH  '${OpenSSL_SOURCE_PATH}'" )
    message(STATUS "OpenSLL_BINARY_PATH  '${OpenSLL_BINARY_PATH}'" )
    message(STATUS "OpenSLL_INCLUDE_PATH '${OpenSLL_INCLUDE_PATH}'" )
endif()
