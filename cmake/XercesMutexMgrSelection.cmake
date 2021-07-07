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

# mutexmgr selection

option(threads "Threading support" ON)

if(threads)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  add_definitions(-D_THREAD_SAFE=1)
  find_package(Threads)

  if(TARGET Threads::Threads)
    if(WIN32)
      list(APPEND mutexmgrs windows)
    else()
      list(APPEND mutexmgrs POSIX)
      set(HAVE_PTHREAD 1)
    endif()
  endif()
else()
  set(mutexmgr nothreads)
endif(threads)
list(APPEND mutexmgrs nothreads)

string(REPLACE ";" "|" mutexmgr_help "${mutexmgrs}")
list(GET mutexmgrs 0 xerces_mutexmgr_default)
set(mutex-manager "${xerces_mutexmgr_default}" CACHE STRING "Mutex manager (${mutexmgr_help})")
set(mutexmgr "${mutex-manager}")

list(FIND mutexmgrs "${mutexmgr}" mutexmgr_found)
if(mutexmgr_found EQUAL -1)
  message(FATAL_ERROR "${mutexmgr} mutexmgr unavailable")
endif()

set(XERCES_USE_MUTEXMGR_POSIX 0)
set(XERCES_USE_MUTEXMGR_WINDOWS 0)
set(XERCES_USE_MUTEXMGR_NOTHREAD 0)
if(mutexmgr STREQUAL "POSIX")
  set(XERCES_USE_MUTEXMGR_POSIX 1)
elseif(mutexmgr STREQUAL "windows")
  set(XERCES_USE_MUTEXMGR_WINDOWS 1)
elseif(mutexmgr STREQUAL "nothreads")
  set(XERCES_USE_MUTEXMGR_NOTHREAD 1)
else()
  message(FATAL_ERROR "Invalid mutex manager: \"${mutexmgr}\"")
endif()
