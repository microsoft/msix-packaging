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

#ifndef _XQILLAXQCIMPLEMENTATION_HPP
#define _XQILLAXQCIMPLEMENTATION_HPP

#include <stddef.h>
#include <xqilla/xqilla-xqc.h>
#include <xqilla/simple-api/XQilla.hpp>
#if defined(_MSC_VER)
#include <BaseTsd.h>
#define ssize_t SSIZE_T
#endif

#define CLASS_OFFSET(class, member) (((ssize_t)&(((class*)1000)->member)) - 1000)

class XQException;

class XQillaXQCImplementation
{
public:
  XQillaXQCImplementation();
  ~XQillaXQCImplementation();

  XQC_Implementation *getImpl()
  {
    return &impl;
  }

  static XQillaXQCImplementation *get(XQC_Implementation *i)
  {
    return (XQillaXQCImplementation *)(((char*)i) - CLASS_OFFSET(XQillaXQCImplementation, impl));
  }

  static void reportError(XQC_ErrorHandler *err, XQC_Error code, const XQException &ex);

private:
  XQillaXQCImplementation(const XQillaXQCImplementation &);
  XQillaXQCImplementation &operator=(const XQillaXQCImplementation &);

  static XQC_Error create_context(XQC_Implementation *implementation, XQC_StaticContext **context);
  static XQC_Error prepare(XQC_Implementation *implementation, const char *string,
                           const XQC_StaticContext *context, XQC_Expression **expression);
  static XQC_Error prepare_file(XQC_Implementation *implementation, FILE *file,
                                const XQC_StaticContext *context, XQC_Expression **expression);
  static XQC_Error prepare_stream(XQC_Implementation *implementation, XQC_InputStream *stream,
                                  const XQC_StaticContext *context, XQC_Expression **expression);
  static XQC_Error parse_document(XQC_Implementation *implementation,
                                  const char *string, XQC_Sequence **sequence);
  static XQC_Error parse_document_file(XQC_Implementation *implementation,
                                       FILE *file, XQC_Sequence **sequence);
  static XQC_Error parse_document_stream(XQC_Implementation *implementation,
                                         XQC_InputStream *stream, XQC_Sequence **sequence);
  static XQC_Error create_empty_sequence(XQC_Implementation *implementation,
                                         XQC_Sequence **sequence);
  static XQC_Error create_singleton_sequence(XQC_Implementation *implementation,
                                             XQC_ItemType type, const char *value,
                                             XQC_Sequence **sequence);
  static XQC_Error create_integer_sequence(XQC_Implementation *implementation,
                                           int values[], unsigned int count,
                                           XQC_Sequence **sequence);
  static XQC_Error create_double_sequence(XQC_Implementation *implementation,
                                          double values[], unsigned int count,
                                          XQC_Sequence **sequence);
  static XQC_Error create_string_sequence(XQC_Implementation *implementation,
                                          const char *values[], unsigned int count,
                                          XQC_Sequence **sequence);
  static void *get_interface(const XQC_Implementation *implementation, const char *name);
  static void free(XQC_Implementation *implementation);

  XQilla xqilla;
  XQC_Implementation impl;
};

#endif
