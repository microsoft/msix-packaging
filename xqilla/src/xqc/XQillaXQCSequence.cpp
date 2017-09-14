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

#include "XQillaXQCSequence.hpp"

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/functions/FunctionNumber.hpp>
#include <xqilla/functions/FunctionString.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

XQillaXQCSequence::XQillaXQCSequence(const Result &result, DynamicContext *context, XQC_ErrorHandler *err)
  : result_(result),
    context_(context),
    item_(),
    err_(err)
{
  memset(&impl_, 0, sizeof(XQC_Sequence));

  impl_.next = next;
  impl_.item_type = item_type;
  impl_.type_name = type_name;
  impl_.string_value = string_value;
  impl_.integer_value = integer_value;
  impl_.double_value = double_value;
  impl_.node_name = node_name;
  impl_.get_interface = get_interface;
  impl_.free = free;
}

XQillaXQCSequence::~XQillaXQCSequence()
{
  result_ = 0;
  item_ = 0;
  delete context_;
}

XQC_Error XQillaXQCSequence::next(XQC_Sequence *sequence)
{
  XQillaXQCSequence *me = get(sequence);

  try {
    me->item_ = me->result_->next(me->context_);
    if(me->item_.isNull()) return XQC_END_OF_SEQUENCE;

    return XQC_NO_ERROR;
  }
  catch(XPath2TypeMatchException &e) {
    XQillaXQCImplementation::reportError(me->err_, XQC_TYPE_ERROR, e);
    return XQC_TYPE_ERROR;
  }
  catch(XQException &e) {
    XQillaXQCImplementation::reportError(me->err_, XQC_DYNAMIC_ERROR, e);
    return XQC_DYNAMIC_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_ItemType typeOfItem(const Item::Ptr &item)
{
  if(item.isNull()) return XQC_EMPTY_TYPE;

  if(item->isNode()) {
    const XMLCh *nodekind = ((const Node*)item.get())->dmNodeKind();
    if(nodekind == Node::document_string) {
      return XQC_DOCUMENT_TYPE;
    }
    else if(nodekind == Node::element_string) {
      return XQC_ELEMENT_TYPE;
    }
    else if(nodekind == Node::attribute_string) {
      return XQC_ATTRIBUTE_TYPE;
    }
    else if(nodekind == Node::text_string) {
      return XQC_TEXT_TYPE;
    }
    else if(nodekind == Node::processing_instruction_string) {
      return XQC_PROCESSING_INSTRUCTION_TYPE;
    }
    else if(nodekind == Node::comment_string) {
      return XQC_COMMENT_TYPE;
    }
    else if(nodekind == Node::namespace_string) {
      return XQC_NAMESPACE_TYPE;
    }
  }
  else if(item->isAtomicValue()) {
    switch(((AnyAtomicType*)item.get())->getPrimitiveTypeIndex()) {
    case AnyAtomicType::ANY_SIMPLE_TYPE:
      return XQC_ANY_SIMPLE_TYPE;
    case AnyAtomicType::ANY_URI:
      return XQC_ANY_URI_TYPE;
    case AnyAtomicType::BASE_64_BINARY:
      return XQC_BASE_64_BINARY_TYPE;
    case AnyAtomicType::BOOLEAN:
      return XQC_BOOLEAN_TYPE;
    case AnyAtomicType::DATE:
      return XQC_DATE_TYPE;
    case AnyAtomicType::DATE_TIME:
      return XQC_DATE_TIME_TYPE;
    case AnyAtomicType::DAY_TIME_DURATION:
      return XQC_DAY_TIME_DURATION_TYPE;
    case AnyAtomicType::DECIMAL:
      return XQC_DECIMAL_TYPE;
    case AnyAtomicType::DOUBLE:
      return XQC_DOUBLE_TYPE;
    case AnyAtomicType::DURATION:
      return XQC_DURATION_TYPE;
    case AnyAtomicType::FLOAT:
      return XQC_FLOAT_TYPE;
    case AnyAtomicType::G_DAY:
      return XQC_G_DAY_TYPE;
    case AnyAtomicType::G_MONTH:
      return XQC_G_MONTH_TYPE;
    case AnyAtomicType::G_MONTH_DAY:
      return XQC_G_MONTH_DAY_TYPE;
    case AnyAtomicType::G_YEAR:
      return XQC_G_YEAR_TYPE;
    case AnyAtomicType::G_YEAR_MONTH:
      return XQC_G_YEAR_MONTH_TYPE;
    case AnyAtomicType::HEX_BINARY:
      return XQC_HEX_BINARY_TYPE;
    case AnyAtomicType::NOTATION:
      return XQC_NOTATION_TYPE;
    case AnyAtomicType::QNAME:
      return XQC_QNAME_TYPE;
    case AnyAtomicType::STRING:
      return XQC_STRING_TYPE;
    case AnyAtomicType::TIME:
      return XQC_TIME_TYPE;
    case AnyAtomicType::UNTYPED_ATOMIC:
      return XQC_UNTYPED_ATOMIC_TYPE;
    case AnyAtomicType::YEAR_MONTH_DURATION:
      return XQC_YEAR_MONTH_DURATION_TYPE;
    case AnyAtomicType::NumAtomicObjectTypes:
      // Shouldn't happen
      break;
    }
  }

  return XQC_EMPTY_TYPE;
}

XQC_Error XQillaXQCSequence::item_type(const XQC_Sequence *sequence, XQC_ItemType *type)
{
  try {
    const XQillaXQCSequence *me = get(sequence);

    if(me->item_.isNull()) {
      return XQC_NO_CURRENT_ITEM;
    }

    *type = typeOfItem(me->item_);

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCSequence::type_name(const XQC_Sequence *sequence, const char **uri, const char **name)
{
  try {
    const XQillaXQCSequence *me = get(sequence);

    if(me->item_.isNull()) {
      return XQC_NO_CURRENT_ITEM;
    }

    me->typeURI_ = UTF8(me->item_->getTypeURI());
    *uri = me->typeURI_.c_str();
    me->typeName_ = UTF8(me->item_->getTypeName());
    *name = me->typeName_.c_str();

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCSequence::string_value(const XQC_Sequence *sequence, const char **value)
{
  try {
    const XQillaXQCSequence *me = get(sequence);

    if(me->item_.isNull()) {
      return XQC_NO_CURRENT_ITEM;
    }

    me->stringValue_ = UTF8(FunctionString::string(me->item_, me->context_));
    *value = me->stringValue_.c_str();
    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCSequence::integer_value(const XQC_Sequence *sequence, int *value)
{
  try {
    const XQillaXQCSequence *me = get(sequence);

    if(me->item_.isNull()) {
      return XQC_NO_CURRENT_ITEM;
    }

    *value = FunctionNumber::number(me->item_, me->context_, 0)->asInt();
    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCSequence::double_value(const XQC_Sequence *sequence, double *value)
{
  try {
    const XQillaXQCSequence *me = get(sequence);

    if(me->item_.isNull()) {
      return XQC_NO_CURRENT_ITEM;
    }

    *value = FunctionNumber::number(me->item_, me->context_, 0)->asDouble();
    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCSequence::node_name(const XQC_Sequence *sequence, const char **uri, const char **name)
{
  try {
    const XQillaXQCSequence *me = get(sequence);

    if(me->item_.isNull()) {
      return XQC_NO_CURRENT_ITEM;
    }

    if(!me->item_->isNode()) {
      return XQC_NOT_NODE;
    }

    ATQNameOrDerived::Ptr qname = ((Node*)me->item_.get())->dmNodeName(me->context_);

    me->nodeURI_ = UTF8(qname->getURI());
    *uri = me->nodeURI_.c_str();
    me->nodeName_ = UTF8(qname->getName());
    *name = me->nodeName_.c_str();

    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

void *XQillaXQCSequence::get_interface(const XQC_Sequence *sequence, const char *name)
{
  return 0;
}

void XQillaXQCSequence::free(XQC_Sequence *sequence)
{
  delete get(sequence);
}
