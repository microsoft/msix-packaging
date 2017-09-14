/*
 * Copyright (c) 2001, 2008,
 *     DecisionSoft Limited. All rights reserved.
 * Copyright (c) 2004, 2015 Oracle and/or its affiliates. All rights reserved.
 *     
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __XQILLAEXPORT_HPP
#define __XQILLAEXPORT_HPP

/*
 * This define tells VS8 not to give a warning when we use strcpy()
 * instead of the MS secure function strcpy_s().
 */
#ifdef _MSC_VER
#define WIN32 1
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif
#endif

/*
 * Building statically on Windows:
 *
 * 1) Compile Xerces-C with XERCES_STATIC_LIBRARY defined to get a static library
 * 2) Compile XQilla with XQILLA_API defined to nothing, to compile without
 *    the DLL import/exports
 */

/*
 * The following ifdef block is the standard way of creating macros which
 * make exporting from a DLL simpler. All files within this DLL are
 * compiled with the XQILLA_APIS symbol defined on the command line.
 * this symbol should not be defined on any project that uses this DLL.
 * This way any other project whose source files include this file see
 * XQILLA_API functions as being imported from a DLL, wheras this DLL
 * sees symbols defined with this macro as being exported.
 */
#if defined(WIN32) && !defined(__CYGWIN__)
#  if !defined(XQILLA_API)
#    if defined(XQILLA_APIS)
#      define XQILLA_API __declspec(dllexport)
#    else
#      define XQILLA_API __declspec(dllimport)
#    endif
#  endif

#  if !defined(__MINGW32__)
#    pragma warning(disable: 4251 4786 4101 4290)
#  endif
#else
#  define XQILLA_API 
#endif

#endif
