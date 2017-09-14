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

#ifndef _XQILLAXQCSTATICCONTEXT_HPP
#define _XQILLAXQCSTATICCONTEXT_HPP

#include <string>

#include "XQillaXQCImplementation.hpp"

class NSEntry;

class XQillaXQCStaticContext
{
public:
  XQillaXQCStaticContext();
  ~XQillaXQCStaticContext();

  XQC_StaticContext *getImpl()
  {
    return &impl;
  }

  static XQillaXQCStaticContext *get(XQC_StaticContext *i)
  {
    return (XQillaXQCStaticContext*)(((char*)i) - CLASS_OFFSET(XQillaXQCStaticContext, impl));
  }

  static const XQillaXQCStaticContext *get(const XQC_StaticContext *i)
  {
    return (const XQillaXQCStaticContext*)(((const char*)i) - CLASS_OFFSET(XQillaXQCStaticContext, impl));
  }

  static DynamicContext *createContext(const XQC_StaticContext *context);

private:
  XQillaXQCStaticContext(const XQillaXQCStaticContext &);
  XQillaXQCStaticContext &operator=(const XQillaXQCStaticContext &);

  static XQC_Error create_child_context(XQC_StaticContext *context, XQC_StaticContext **child_context);
  static XQC_Error declare_ns(XQC_StaticContext *context, const char *prefix, const char *uri);
  static XQC_Error get_ns_by_prefix(XQC_StaticContext *context, const char *prefix, const char **result_ns);
  static XQC_Error set_default_element_and_type_ns(XQC_StaticContext *context, const char *uri);
  static XQC_Error get_default_element_and_type_ns(XQC_StaticContext *context, const char **uri);
  static XQC_Error set_default_function_ns(XQC_StaticContext *context, const char *uri);
  static XQC_Error get_default_function_ns(XQC_StaticContext *context, const char **uri);
  static XQC_Error set_xpath_compatib_mode(XQC_StaticContext *context, XQC_XPath1Mode mode);
  static XQC_Error get_xpath_compatib_mode(XQC_StaticContext *context, XQC_XPath1Mode* mode);
  static XQC_Error set_construction_mode(XQC_StaticContext *context, XQC_ConstructionMode mode);
  static XQC_Error get_construction_mode(XQC_StaticContext *context, XQC_ConstructionMode* mode);
  static XQC_Error set_ordering_mode(XQC_StaticContext *context, XQC_OrderingMode mode);
  static XQC_Error get_ordering_mode(XQC_StaticContext *context, XQC_OrderingMode* mode);
  static XQC_Error set_default_order_empty_sequences(XQC_StaticContext *context, XQC_OrderEmptyMode mode);
  static XQC_Error get_default_order_empty_sequences(XQC_StaticContext *context, XQC_OrderEmptyMode* mode);
  static XQC_Error set_boundary_space_policy(XQC_StaticContext *context, XQC_BoundarySpaceMode mode);
  static XQC_Error get_boundary_space_policy(XQC_StaticContext *context, XQC_BoundarySpaceMode* mode);
  static XQC_Error set_copy_ns_mode(XQC_StaticContext *context, XQC_PreserveMode preserve, XQC_InheritMode inherit);
  static XQC_Error get_copy_ns_mode(XQC_StaticContext *context, XQC_PreserveMode* preserve, XQC_InheritMode* inherit);
  static XQC_Error set_base_uri(XQC_StaticContext *context, const char *base_uri);
  static XQC_Error get_base_uri(XQC_StaticContext *context, const char **base_uri);
  static XQC_Error set_error_handler(XQC_StaticContext *context, XQC_ErrorHandler *handler);
  static XQC_Error get_error_handler(const XQC_StaticContext *context, XQC_ErrorHandler **handler);
  static void *get_interface(const XQC_StaticContext *context, const char *name);
  static void free(XQC_StaticContext *context);

  XQC_StaticContext impl;
  NSEntry *nslist_;
  std::string elemns_;
  std::string funcns_;
  XQC_XPath1Mode xp1_;
  XQC_ConstructionMode constr_;
  XQC_OrderingMode order_;
  XQC_OrderEmptyMode empty_;
  XQC_BoundarySpaceMode bound_;
  XQC_PreserveMode preserve_;
  XQC_InheritMode inherit_;
  std::string baseuri_;
  XQC_ErrorHandler *handler_;
};

#endif

