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

#ifndef _XQILLAXQCDYNAMICCONTEXT_HPP
#define _XQILLAXQCDYNAMICCONTEXT_HPP

#include "XQillaXQCImplementation.hpp"

#include <xqilla/items/Item.hpp>

class VarEntry;

class XQillaXQCDynamicContext
{
public:
  XQillaXQCDynamicContext(XQQuery *query);
  ~XQillaXQCDynamicContext();

  void populateContext(DynamicContext *context) const;

  XQC_DynamicContext *getImpl()
  {
    return &impl_;
  }

  static XQillaXQCDynamicContext *get(XQC_DynamicContext *i)
  {
    return (XQillaXQCDynamicContext*)(((char*)i) - CLASS_OFFSET(XQillaXQCDynamicContext, impl_));
  }

  static const XQillaXQCDynamicContext *get(const XQC_DynamicContext *i)
  {
    return (const XQillaXQCDynamicContext*)(((const char*)i) - CLASS_OFFSET(XQillaXQCDynamicContext, impl_));
  }

private:
  XQillaXQCDynamicContext(const XQillaXQCDynamicContext &);
  XQillaXQCDynamicContext &operator=(const XQillaXQCDynamicContext &);

  static XQC_Error set_variable(XQC_DynamicContext *context, const char *uri, const char *name,
                                XQC_Sequence *value);
  static XQC_Error get_variable(const XQC_DynamicContext *context, const char *uri, const char *name,
                                XQC_Sequence **value);
  static XQC_Error set_context_item(XQC_DynamicContext *context, XQC_Sequence *value);
  static XQC_Error get_context_item(const XQC_DynamicContext *context, XQC_Sequence **value);
  static XQC_Error set_implicit_timezone(XQC_DynamicContext *context, int timezone);
  static XQC_Error get_implicit_timezone(const XQC_DynamicContext *context, int *timezone);
  static XQC_Error set_error_handler(XQC_DynamicContext *context, XQC_ErrorHandler *handler);
  static XQC_Error get_error_handler(const XQC_DynamicContext *context, XQC_ErrorHandler **handler);
  static void *get_interface(const XQC_DynamicContext *context, const char *name);
  static void free(XQC_DynamicContext *context);

  XQC_DynamicContext impl_;
  VarEntry *vars_;
  Item::Ptr context_;
  int timezone_;
  XQC_ErrorHandler *handler_;
};

#endif

