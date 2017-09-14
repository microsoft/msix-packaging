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

#ifndef _XQILLA_CONFIG_H
#define _XQILLA_CONFIG_H

#if defined(_MSC_VER)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Warning 4064: switch statement contains 'default' but no 'case' labels
// 
// Warning 4251: We have a lot of std::vector objects inside exported classes, and VC++ complains about it
// "class 'std::vector<XXXX> >' needs to have dll-interface to be used by clients of class 'YYYY'"
//
// Warning 4786: "identifier was truncated to '255' characters in the browser information"
//
// Warning 4101: "unreferenced local variable"
//
// Warning 4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
//
// Warning 4996 is deprecated/unsafe methods from Visual Studio 2005 onwards
//
// Warning 4786 is a warning that debug info was truncated to 255 chars
//   this happens in VC6 on large STL template classes.
#pragma warning(disable: 4065 4251 4786 4101 4290 4996 4786)

#endif

#if !defined(HAVE_CONFIG_H)

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* Intel & Alpha architechtures are small endian */
#undef WORDS_BIGENDIAN

/* Define to 1 if the FAXPP library is present. */
#undef HAVE_FAXPP

/* Define to 1 if the HTML Tidy library is present. */
#undef HAVE_LIBTIDY

/* Define to 1 if the Xerces-C library supports the
   RegularExpression::allMatches() method. */
#undef HAVE_ALLMATCHES

/* Define to 1 if the Xerces-C library supports the
   BinInputStream::getContentType() method. */
#undef HAVE_GETCONTENTTYPE

/* C code should not care about Xerces-C++ facilities. */
#ifdef __cplusplus
#include <xercesc/util/XercesVersion.hpp>
#if _XERCES_VERSION >= 30000
#define HAVE_ALLMATCHES 1
#define HAVE_GETCONTENTTYPE 1
#endif
#endif

#else

#include "xqilla_config_unix.h"

#endif

/* C code should not care about Xerces-C++ facilities. */
#ifdef __cplusplus
#include <xercesc/util/XercesVersion.hpp>
#include <xercesc/util/XercesDefs.hpp>

#if _XERCES_VERSION >= 30000
typedef XMLSize_t XercesSizeUint;
typedef XMLFilePos XercesFilePos;
typedef XMLFileLoc XercesFileLoc;
#else
typedef unsigned int XercesSizeUint;
typedef unsigned int XercesFilePos;
typedef XMLSSize_t XercesFileLoc;
#endif
#endif

#endif
