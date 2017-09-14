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

#include "XQillaXQCStaticContext.hpp"
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/ast/XQFunction.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/xerces/XercesConfiguration.hpp>
#include <xqilla/fastxdm/FastXDMConfiguration.hpp>

using namespace std;

class NSEntry
{
public:
  NSEntry(const char *p, const char *u, NSEntry *n)
    : prefix(p),
      uri(u),
      next(n)
  {
  }

  string prefix, uri;
  NSEntry *next;
};

XQillaXQCStaticContext::XQillaXQCStaticContext()
  : nslist_(0),
    elemns_(),
    funcns_(UTF8(XQFunction::XMLChFunctionURI)),
    xp1_(XQC_XPATH2_0),
    constr_(XQC_PRESERVE_CONS),
    order_(XQC_ORDERED),
    empty_(XQC_EMPTY_LEAST),
    bound_(XQC_STRIP_SPACE),
    preserve_(XQC_PRESERVE_NS),
    inherit_(XQC_INHERIT_NS),
    baseuri_(),
    handler_(0)
{
  memset(&impl, 0, sizeof(XQC_StaticContext));

  impl.create_child_context = create_child_context;
  impl.declare_ns = declare_ns;
  impl.get_ns_by_prefix = get_ns_by_prefix;
  impl.set_default_element_and_type_ns = set_default_element_and_type_ns;
  impl.get_default_element_and_type_ns = get_default_element_and_type_ns;
  impl.set_default_function_ns = set_default_function_ns;
  impl.get_default_function_ns = get_default_function_ns;
  impl.set_xpath_compatib_mode = set_xpath_compatib_mode;
  impl.get_xpath_compatib_mode = get_xpath_compatib_mode;
  impl.set_construction_mode = set_construction_mode;
  impl.get_construction_mode = get_construction_mode;
  impl.set_ordering_mode = set_ordering_mode;
  impl.get_ordering_mode = get_ordering_mode;
  impl.set_default_order_empty_sequences = set_default_order_empty_sequences;
  impl.get_default_order_empty_sequences = get_default_order_empty_sequences;
  impl.set_boundary_space_policy = set_boundary_space_policy;
  impl.get_boundary_space_policy = get_boundary_space_policy;
  impl.set_copy_ns_mode = set_copy_ns_mode;
  impl.get_copy_ns_mode = get_copy_ns_mode;
  impl.set_base_uri = set_base_uri;
  impl.get_base_uri = get_base_uri;
  impl.set_error_handler = set_error_handler;
  impl.get_error_handler = get_error_handler;
  impl.get_interface = get_interface;
  impl.free = free;
}

XQillaXQCStaticContext::~XQillaXQCStaticContext()
{
  NSEntry *tmp;
  while(nslist_) {
    tmp = nslist_;
    nslist_ = nslist_->next;
    delete tmp;
  }
}

XQC_Error XQillaXQCStaticContext::create_child_context(XQC_StaticContext *context, XQC_StaticContext **child_context)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    AutoDelete<XQillaXQCStaticContext> result(new XQillaXQCStaticContext);

    // Copy the namespace bindings
    NSEntry *tmp = me->nslist_;
    while(tmp) {
      declare_ns(result->getImpl(), tmp->prefix.c_str(), tmp->uri.c_str());
      tmp = tmp->next;
    }

    result->elemns_ = me->elemns_;
    result->funcns_ = me->funcns_;
    result->xp1_ = me->xp1_;
    result->constr_ = me->constr_;
    result->order_ = me->order_;
    result->empty_ = me->empty_;
    result->bound_ = me->bound_;
    result->preserve_ = me->preserve_;
    result->inherit_ = me->inherit_;
    result->baseuri_ = me->baseuri_;

    *child_context = result.adopt()->getImpl();
    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

static XercesConfiguration xercesConf;
static FastXDMConfiguration fastConf;

DynamicContext *XQillaXQCStaticContext::createContext(const XQC_StaticContext *context)
{
  AutoDelete<DynamicContext> result(XQilla::createContext(
    (XQilla::Language)(XQilla::XQUERY | XQilla::FULLTEXT | XQilla::UPDATE | XQilla::EXTENSIONS),
    &xercesConf));

  if(context) {
    const XQillaXQCStaticContext *me = get(context);
    XPath2MemoryManager *mm = result->getMemoryManager();

    // Copy the namespace bindings
    NSEntry *tmp = me->nslist_;
    while(tmp) {
      result->setNamespaceBinding(mm->getPooledString(tmp->prefix.c_str()),
                                  mm->getPooledString(tmp->uri.c_str()));
      tmp = tmp->next;
    }

    result->setDefaultElementAndTypeNS(mm->getPooledString(me->elemns_.c_str()));
    result->setDefaultFuncNS(mm->getPooledString(me->funcns_.c_str()));
    result->setBaseURI(mm->getPooledString(me->baseuri_.c_str()));

    result->setXPath1CompatibilityMode(me->xp1_ == XQC_XPATH1_0);
    result->setConstructionMode(me->constr_ == XQC_PRESERVE_CONS ? StaticContext::CONSTRUCTION_MODE_PRESERVE : StaticContext::CONSTRUCTION_MODE_STRIP);
    result->setNodeSetOrdering(me->order_ == XQC_ORDERED ? StaticContext::ORDERING_ORDERED : StaticContext::ORDERING_UNORDERED);
    result->setDefaultFLWOROrderingMode(me->empty_ == XQC_EMPTY_LEAST ? StaticContext::FLWOR_ORDER_EMPTY_LEAST : StaticContext::FLWOR_ORDER_EMPTY_GREATEST);
    result->setPreserveBoundarySpace(me->bound_ == XQC_PRESERVE_SPACE);
    result->setPreserveNamespaces(me->preserve_ == XQC_PRESERVE_NS);
    result->setInheritNamespaces(me->inherit_ == XQC_INHERIT_NS);
  }

  return result.adopt();
}

XQC_Error XQillaXQCStaticContext::declare_ns(XQC_StaticContext *context, const char *prefix, const char *uri)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    // Find the namespace binding if it exists
    NSEntry *tmp = me->nslist_;
    while(tmp) {
      if(tmp->prefix == prefix)
        break;
      tmp = tmp->next;
    }

    if(tmp) {
      // Set the binding
      tmp->uri = (uri == 0 ? "" : uri);
    }
    else {
      // Add a new binding
      me->nslist_ = new NSEntry(prefix, uri, me->nslist_);
    }

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::get_ns_by_prefix(XQC_StaticContext *context, const char *prefix, const char **result_ns)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    // Find the namespace binding if it exists
    NSEntry *tmp = me->nslist_;
    while(tmp) {
      if(tmp->prefix == prefix) {
        *result_ns = tmp->uri.c_str();
        return XQC_NO_ERROR;
      }
      tmp = tmp->next;
    }

    *result_ns = 0;
    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::set_default_element_and_type_ns(XQC_StaticContext *context, const char *uri)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    me->elemns_ = uri;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::get_default_element_and_type_ns(XQC_StaticContext *context, const char **uri)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    *uri = me->elemns_.c_str();

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::set_default_function_ns(XQC_StaticContext *context, const char *uri)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    me->funcns_ = uri;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::get_default_function_ns(XQC_StaticContext *context, const char **uri)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    *uri = me->funcns_.c_str();

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::set_xpath_compatib_mode(XQC_StaticContext *context, XQC_XPath1Mode mode)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    me->xp1_ = mode;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::get_xpath_compatib_mode(XQC_StaticContext *context, XQC_XPath1Mode* mode)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    *mode = me->xp1_;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::set_construction_mode(XQC_StaticContext *context, XQC_ConstructionMode mode)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    me->constr_ = mode;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::get_construction_mode(XQC_StaticContext *context, XQC_ConstructionMode* mode)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    *mode = me->constr_;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::set_ordering_mode(XQC_StaticContext *context, XQC_OrderingMode mode)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    me->order_ = mode;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::get_ordering_mode(XQC_StaticContext *context, XQC_OrderingMode* mode)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    *mode = me->order_;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::set_default_order_empty_sequences(XQC_StaticContext *context, XQC_OrderEmptyMode mode)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    me->empty_ = mode;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::get_default_order_empty_sequences(XQC_StaticContext *context, XQC_OrderEmptyMode* mode)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    *mode = me->empty_;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::set_boundary_space_policy(XQC_StaticContext *context, XQC_BoundarySpaceMode mode)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    me->bound_ = mode;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::get_boundary_space_policy(XQC_StaticContext *context, XQC_BoundarySpaceMode* mode)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    *mode = me->bound_;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::set_copy_ns_mode(XQC_StaticContext *context, XQC_PreserveMode preserve, XQC_InheritMode inherit)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    me->preserve_ = preserve;
    me->inherit_ = inherit;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::get_copy_ns_mode(XQC_StaticContext *context, XQC_PreserveMode* preserve, XQC_InheritMode* inherit)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    *preserve = me->preserve_;
    *inherit = me->inherit_;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::set_base_uri(XQC_StaticContext *context, const char *base_uri)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    me->baseuri_ = base_uri;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}


XQC_Error XQillaXQCStaticContext::get_base_uri(XQC_StaticContext *context, const char **base_uri)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    *base_uri = me->baseuri_.c_str();

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCStaticContext::set_error_handler(XQC_StaticContext *context, XQC_ErrorHandler *handler)
{
  try {
    XQillaXQCStaticContext *me = get(context);

    me->handler_ = handler;

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCStaticContext::get_error_handler(const XQC_StaticContext *context, XQC_ErrorHandler **handler)
{
  try {
    const XQillaXQCStaticContext *me = get(context);

    *handler = me->handler_;
    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

void *XQillaXQCStaticContext::get_interface(const XQC_StaticContext *context, const char *name)
{
  return 0;
}

void XQillaXQCStaticContext::free(XQC_StaticContext *context)
{
  delete get(context);
}
