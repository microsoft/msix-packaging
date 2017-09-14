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

#include "XQillaXQCDynamicContext.hpp"
#include "XQillaXQCSequence.hpp"

#include <xqilla/utils/ContextUtils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include "../utils/DateUtils.hpp"

XERCES_CPP_NAMESPACE_USE;
using namespace std;

class VarEntry
{
public:
  VarEntry(const char *u, const char *n, XQC_Sequence *v, VarEntry *nxt)
    : uri(u),
      name(n),
      value(v),
      next(nxt)
  {
    XQillaXQCSequence *xseq = XQillaXQCSequence::get(v);
    seq = xseq->getResult()->toSequence(xseq->getContext());
  }

  ~VarEntry()
  {
    value->free(value);
  }

  void setValue(XQC_Sequence *v)
  {
    value->free(value);
    value = v;
  }

  string uri, name;
  XQC_Sequence *value;
  Sequence seq;
  VarEntry *next;
};


XQillaXQCDynamicContext::XQillaXQCDynamicContext(XQQuery *query)
  : vars_(0),
    context_(0),
    // Convert from seconds based to minutes based
    timezone_(ContextUtils::getTimezone() / DateUtils::g_secondsPerMinute),
    handler_(0)
{
  memset(&impl_, 0, sizeof(XQC_DynamicContext));

  impl_.set_variable = set_variable;
  impl_.get_variable = get_variable;
  impl_.set_context_item = set_context_item;
  impl_.get_context_item = get_context_item;
  impl_.set_implicit_timezone = set_implicit_timezone;
  impl_.get_implicit_timezone = get_implicit_timezone;
  impl_.set_error_handler = set_error_handler;
  impl_.get_error_handler = get_error_handler;
  impl_.get_interface = get_interface;
  impl_.free = free;
}

XQillaXQCDynamicContext::~XQillaXQCDynamicContext()
{
  VarEntry *tmp;
  while(vars_) {
    tmp = vars_;
    vars_ = vars_->next;
    delete tmp;
  }
}

void XQillaXQCDynamicContext::populateContext(DynamicContext *context) const
{
  VarEntry *var = vars_;
  while(var) {
    if(var->uri == "") {
      context->setExternalVariable(X(var->name.c_str()), var->seq);
    }
    else {
      context->setExternalVariable(X(var->uri.c_str()), X(var->name.c_str()), var->seq);
    }

    var = var->next;
  }

  if(context_.notNull()) {
    context->setContextItem(context_);
    context->setContextPosition(1);
    context->setContextSize(1);
  }

  context->setImplicitTimezone(context->getItemFactory()->
                               createDayTimeDuration(timezone_ * DateUtils::g_secondsPerMinute, context));
}

XQC_Error XQillaXQCDynamicContext::set_variable(XQC_DynamicContext *context, const char *uri, const char *name,
                                                XQC_Sequence *value)
{
  try {
    XQillaXQCDynamicContext *me = get(context);

    // Find the variable binding if it exists
    VarEntry **var = &me->vars_;
    while(*var) {
      if((*var)->name == name && (*var)->uri == uri)
        break;
      var = &(*var)->next;
    }

    if(*var) {
      if(value) {
        // Set the binding
        (*var)->setValue(value);
      }
      else {
        // Remove the binding
        VarEntry *tmp = *var;
        *var = (*var)->next;
        delete tmp;
      }
    }
    else if(value) {
      // Add a new binding
      me->vars_ = new VarEntry(uri, name, value, me->vars_);
    }

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCDynamicContext::get_variable(const XQC_DynamicContext *context, const char *uri, const char *name,
                                                XQC_Sequence **value)
{
  try {
    const XQillaXQCDynamicContext *me = get(context);

    // Find the variable binding if it exists
    VarEntry *var = me->vars_;
    while(var) {
      if(var->name == name && var->uri == uri)
        break;
      var = var->next;
    }

    if(var) {
      *value = (new XQillaXQCSequence(var->seq, 0))->getImpl();
    }
    else {
      *value = 0;
    }

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCDynamicContext::set_context_item(XQC_DynamicContext *context, XQC_Sequence *value)
{
  try {
    XQillaXQCDynamicContext *me = get(context);

    if(value == 0) {
      me->context_ = 0;
    }
    else {
      XQC_ItemType type;
      if(value->item_type(value, &type) == XQC_NO_CURRENT_ITEM)
        return XQC_NO_CURRENT_ITEM;

      me->context_ = XQillaXQCSequence::get(value)->getItem();
    }

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCDynamicContext::get_context_item(const XQC_DynamicContext *context, XQC_Sequence **value)
{
  try {
    const XQillaXQCDynamicContext *me = get(context);

    if(me->context_.isNull()) {
      *value = 0;
    }
    else {
      *value = (new XQillaXQCSequence(me->context_, 0))->getImpl();
    }

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCDynamicContext::set_implicit_timezone(XQC_DynamicContext *context, int timezone)
{
  try {
    XQillaXQCDynamicContext *me = get(context);

    if(abs(timezone) > 14 * 60) {
      return XQC_INTERNAL_ERROR;
    }

    me->timezone_ = timezone;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCDynamicContext::get_implicit_timezone(const XQC_DynamicContext *context, int *timezone)
{
  try {
    const XQillaXQCDynamicContext *me = get(context);

    *timezone = me->timezone_;
    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCDynamicContext::set_error_handler(XQC_DynamicContext *context, XQC_ErrorHandler *handler)
{
  try {
    XQillaXQCDynamicContext *me = get(context);

    me->handler_ = handler;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCDynamicContext::get_error_handler(const XQC_DynamicContext *context, XQC_ErrorHandler **handler)
{
  try {
    const XQillaXQCDynamicContext *me = get(context);

    *handler = me->handler_;
    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

void *XQillaXQCDynamicContext::get_interface(const XQC_DynamicContext *context, const char *name)
{
  return 0;
}

void XQillaXQCDynamicContext::free(XQC_DynamicContext *context)
{
  delete get(context);
}
