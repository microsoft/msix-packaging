# xplat\lib
# Copyright (C) 2017 Microsoft
# Created by Phil Smith (psmith@microsoft.com) on 10/19/2017
cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

ADD_CUSTOM_TARGET(LIBS)

<<<<<<< HEAD
# ZLIB
#   set(AMD64             OFF CACHE BOOL "Disable building i686 assembly implementation"  FORCE)
#   set(ASM686            OFF CACHE BOOL "Disable building amd64 assembly implementation" FORCE)
set(MINGW             OFF CACHE BOOL "Only build the static lib"                      FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Only build the static lib"                      FORCE)
set(HAVE_OFF64_T      OFF CACHE BOOL "no need to build zlib tests as part of build"   FORCE)
add_subdirectory(zlib)

# Xerces
set(DBUILD_SHARED_LIBS OFF CACHE BOOL "Only build static lib"                         FORCE)
add_subdirectory(xerces)
=======
#IF(WIN32)
#   set(AMD64             OFF CACHE BOOL "Disable building i686 assembly implementation"  FORCE)
#   set(ASM686            OFF CACHE BOOL "Disable building amd64 assembly implementation" FORCE)
    set(MINGW             OFF CACHE BOOL "Only build the static lib"                      FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Only build the static lib"                      FORCE)
    set(HAVE_OFF64_T      OFF CACHE BOOL "no need to build zlib tests as part of build"   FORCE)
    add_subdirectory(zlib)
#ENDIF()
>>>>>>> origin/feature
