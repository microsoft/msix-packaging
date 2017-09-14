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

#include <string.h>

#include "XQillaXQCExpression.hpp"
#include "XQillaXQCDynamicContext.hpp"
#include "XQillaXQCSequence.hpp"

#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>

XERCES_CPP_NAMESPACE_USE;

XQillaXQCExpression::XQillaXQCExpression(XQQuery *query)
  : query_(query)
{
  memset(&impl_, 0, sizeof(XQC_Expression));

  impl_.create_context = create_context;
  impl_.execute = execute;
  impl_.get_interface = get_interface;
  impl_.free = free;
}

XQillaXQCExpression::~XQillaXQCExpression()
{
  delete query_;
}

XQC_Error XQillaXQCExpression::create_context(const XQC_Expression *expression, XQC_DynamicContext **context)
{
  try {
    const XQillaXQCExpression *me = get(expression);

    *context = (new XQillaXQCDynamicContext(me->query_))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCExpression::execute(const XQC_Expression *expression, const XQC_DynamicContext *context, XQC_Sequence **sequence)
{
  XQC_ErrorHandler *err = 0;
  if(context)
    context->get_error_handler(context, &err);

  try {
    const XQillaXQCExpression *me = get(expression);

    AutoDelete<DynamicContext> dcontext(me->query_->createDynamicContext());
    if(context) {
      XQillaXQCDynamicContext::get(context)->populateContext(dcontext);
    }

    *sequence = (new XQillaXQCSequence(me->query_->execute(dcontext), dcontext.adopt(), err))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(XPath2TypeMatchException &e) {
    XQillaXQCImplementation::reportError(err, XQC_TYPE_ERROR, e);
    return XQC_TYPE_ERROR;
  }
  catch(XQException &e) {
    XQillaXQCImplementation::reportError(err, XQC_DYNAMIC_ERROR, e);
    return XQC_DYNAMIC_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

void *XQillaXQCExpression::get_interface(const XQC_Expression *expression, const char *name)
{
  return 0;
}

void XQillaXQCExpression::free(XQC_Expression *expression)
{
  delete get(expression);
}

