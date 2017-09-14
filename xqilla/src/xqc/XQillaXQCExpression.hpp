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

#ifndef _XQILLAXQCEXPRESSION_HPP
#define _XQILLAXQCEXPRESSION_HPP

#include "XQillaXQCImplementation.hpp"

class XQQuery;

class XQillaXQCExpression
{
public:
  XQillaXQCExpression(XQQuery *query);
  ~XQillaXQCExpression();

  XQC_Expression *getImpl()
  {
    return &impl_;
  }

  static XQillaXQCExpression *get(XQC_Expression *i)
  {
    return (XQillaXQCExpression*)(((char*)i) - CLASS_OFFSET(XQillaXQCExpression, impl_));
  }

  static const XQillaXQCExpression *get(const XQC_Expression *i)
  {
    return (const XQillaXQCExpression*)(((const char*)i) - CLASS_OFFSET(XQillaXQCExpression, impl_));
  }

private:
  XQillaXQCExpression(const XQillaXQCExpression &);
  XQillaXQCExpression &operator=(const XQillaXQCExpression &);

  static XQC_Error create_context(const XQC_Expression *expression, XQC_DynamicContext **context);
  static XQC_Error execute(const XQC_Expression *expression, const XQC_DynamicContext *context, XQC_Sequence **sequence);
  static void *get_interface(const XQC_Expression *expression, const char *name);
  static void free(XQC_Expression *expression);

  XQC_Expression impl_;
  XQQuery *query_;
};

#endif

