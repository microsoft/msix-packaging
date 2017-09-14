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

#ifndef _XQILLAXQCSEQUENCE_HPP
#define _XQILLAXQCSEQUENCE_HPP

#include "XQillaXQCImplementation.hpp"

#include <xqilla/runtime/Result.hpp>

class DynamicContext;

class XQillaXQCSequence
{
public:
  XQillaXQCSequence(const Result &result, DynamicContext *context, XQC_ErrorHandler *err = 0);
  ~XQillaXQCSequence();

  Result &getResult()
  {
    return result_;
  }

  DynamicContext *getContext()
  {
    return context_;
  }

  Item::Ptr &getItem()
  {
    return item_;
  }

  XQC_Sequence *getImpl()
  {
    return &impl_;
  }

  static XQillaXQCSequence *get(XQC_Sequence *i)
  {
    return (XQillaXQCSequence*)(((char*)i) - CLASS_OFFSET(XQillaXQCSequence, impl_));
  }

  static const XQillaXQCSequence *get(const XQC_Sequence *i)
  {
    return (const XQillaXQCSequence*)(((const char*)i) - CLASS_OFFSET(XQillaXQCSequence, impl_));
  }

private:
  XQillaXQCSequence(const XQillaXQCSequence &);
  XQillaXQCSequence &operator=(const XQillaXQCSequence &);

  static XQC_Error next(XQC_Sequence *sequence);
  static XQC_Error item_type(const XQC_Sequence *sequence, XQC_ItemType *type);
  static XQC_Error type_name(const XQC_Sequence *sequence, const char **uri, const char **name);
  static XQC_Error string_value(const XQC_Sequence *sequence, const char **value);
  static XQC_Error integer_value(const XQC_Sequence *sequence, int *value);
  static XQC_Error double_value(const XQC_Sequence *sequence, double *value);
  static XQC_Error node_name(const XQC_Sequence *sequence, const char **uri, const char **name);
  static void *get_interface(const XQC_Sequence *sequence, const char *name);
  static void free(XQC_Sequence *sequence);

  XQC_Sequence impl_;
  Result result_;
  DynamicContext *context_;
  Item::Ptr item_;
  XQC_ErrorHandler *err_;
  mutable std::string typeURI_, typeName_, stringValue_, nodeURI_, nodeName_;
};

#endif

