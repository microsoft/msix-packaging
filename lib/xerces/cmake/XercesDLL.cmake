# CMake build for xerces-c
#
# Written by Roger Leigh <rleigh@codelibre.net>
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Option for selection of shared or static libraries, exported as
# cache variable

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")

# Add a d postfix to Debug libraries on Windows

if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

# DLL export and import macros

set(XERCES_PLATFORM_EXPORT)
set(XERCES_PLATFORM_IMPORT)
set(XERCES_DLL_EXPORT)
set(XERCES_STATIC_LIBRARY)
if(NOT BUILD_SHARED_LIBS)
  set(XERCES_STATIC_LIBRARY 1)
else()
  if(WIN32)
    set(XERCES_PLATFORM_EXPORT "__declspec(dllexport)")
    set(XERCES_PLATFORM_IMPORT "__declspec(dllimport)")
    set(XERCES_DLL_EXPORT 1)
  endif()
endif()

# Versioning information

file(STRINGS "${PROJECT_SOURCE_DIR}/version.incl" icu_verinfo
  REGEX "^[^#].*=.*")

foreach(item IN LISTS icu_verinfo)
  string(REGEX REPLACE "^([^=]+)=(.*)"
    "\\1" lib_key "${item}")
  string(REGEX REPLACE "^([^=]+)=(.*)"
    "\\2" lib_value "${item}")
  set(LIB_${lib_key} "${lib_value}")
endforeach()

string(REGEX REPLACE "^([0-9]+)_.*"
  "\\1"
  LIB_INTERFACE_VER_U_MAJOR
  "${LIB_INTERFACE_VER_U}")
