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

#include "XQillaXQCImplementation.hpp"
#include "XQillaXQCStaticContext.hpp"
#include "XQillaXQCExpression.hpp"
#include "XQillaXQCSequence.hpp"
#include "XQillaXQCInputStream.hpp"

#include <xqilla/utils/XStr.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/exceptions/ContextException.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/runtime/Sequence.hpp>

#include <xercesc/util/regx/RegularExpression.hpp>
#include <xercesc/util/regx/Match.hpp>

XERCES_CPP_NAMESPACE_USE;

XQillaXQCImplementation::XQillaXQCImplementation()
{
  memset(&impl, 0, sizeof(XQC_Implementation));

  impl.create_context = create_context;
  impl.prepare = prepare;
  impl.prepare_file = prepare_file;
  impl.prepare_stream = prepare_stream;
  impl.parse_document = parse_document;
  impl.parse_document_file = parse_document_file;
  impl.parse_document_stream = parse_document_stream;
  impl.create_empty_sequence = create_empty_sequence;
  impl.create_singleton_sequence = create_singleton_sequence;
  impl.create_integer_sequence = create_integer_sequence;
  impl.create_double_sequence = create_double_sequence;
  impl.create_string_sequence = create_string_sequence;
  impl.get_interface = get_interface;
  impl.free = free;
}

XQillaXQCImplementation::~XQillaXQCImplementation()
{
}

XQC_Error XQillaXQCImplementation::create_context(XQC_Implementation *implementation, XQC_StaticContext **context)
{
  try {
    *context = (new XQillaXQCStaticContext())->getImpl();
    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

static std::string regexFind(const char *regex, const XMLCh *str, int groupNo = 1)
{
  try {
    RegularExpression re(regex, "mH");

#ifdef HAVE_ALLMATCHES
    RefVectorOf<Match> matches(10, true);
    re.allMatches(str, 0, XMLString::stringLen(str), &matches);
    if(matches.size() >= 1) {
      Match *match = matches.elementAt(0);
      if(match->getNoGroups() >= groupNo && match->getStartPos(groupNo) != -1)
        return std::string(UTF8(str)).substr(match->getStartPos(groupNo), match->getEndPos(groupNo) - match->getStartPos(groupNo));
    }
#else
    Match match;
    if(re.matches(str, 0, XMLString::stringLen(str), &match)) {
      if(match.getNoGroups() >= groupNo && match.getStartPos(groupNo) != -1)
        return std::string(UTF8(str)).substr(match.getStartPos(groupNo), match.getEndPos(groupNo) - match.getStartPos(groupNo));
    }
#endif

  }
  catch(XMLException &ex) {
    printf("Regex exception: %s\n", UTF8(ex.getMessage()));
  }
  return "";
}

void XQillaXQCImplementation::reportError(XQC_ErrorHandler *err, XQC_Error code, const XQException &ex)
{
  if(err == 0) return;

  std::string qname = regexFind("\\[err:(\\c+)\\]", ex.getError());
  if(qname == "") {
    err->error(err, code, 0, 0, UTF8(ex.getError()), 0);
  }
  else {
    err->error(err, code, "http://www.w3.org/2005/xqt-errors", qname.c_str(), UTF8(ex.getError()), 0);
  }
}

static void reportErrorStatic(const XQC_StaticContext *context, XQC_Error code, const XQException &ex)
{
  if(context == 0) return;

  XQC_ErrorHandler *err = 0;
  context->get_error_handler(context, &err);

  XQillaXQCImplementation::reportError(err, code, ex);
}

XQC_Error XQillaXQCImplementation::prepare(XQC_Implementation *implementation, const char *string,
                                           const XQC_StaticContext *context, XQC_Expression **expression)
{
  try {
    CharInputSource is(string);
    XQQuery *query = XQilla::parse(is, XQillaXQCStaticContext::createContext(context));
    *expression = (new XQillaXQCExpression(query))->getImpl();
    return XQC_NO_ERROR;
  }
  // TBD Clean up exception classes - jpcs
  catch(XPath2TypeMatchException &e) {
    reportErrorStatic(context, XQC_TYPE_ERROR, e);
    return XQC_TYPE_ERROR;
  }
  catch(StaticErrorException &e) {
    reportErrorStatic(context, XQC_STATIC_ERROR, e);
    return XQC_STATIC_ERROR;
  }
  catch(XQException &e) {
    reportErrorStatic(context, XQC_DYNAMIC_ERROR, e);
    return XQC_DYNAMIC_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
  return XQC_NO_ERROR;
}

XQC_Error XQillaXQCImplementation::prepare_file(XQC_Implementation *implementation, FILE *file,
                                                const XQC_StaticContext *context, XQC_Expression **expression)
{
  try {
    FileInputSource is(file);
    XQQuery *query = XQilla::parse(is, XQillaXQCStaticContext::createContext(context));
    *expression = (new XQillaXQCExpression(query))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(ContextException &e) {
    return XQC_UNRECOGNIZED_ENCODING;
  }
  catch(XPath2TypeMatchException &e) {
    reportErrorStatic(context, XQC_TYPE_ERROR, e);
    return XQC_TYPE_ERROR;
  }
  catch(StaticErrorException &e) {
    reportErrorStatic(context, XQC_STATIC_ERROR, e);
    return XQC_STATIC_ERROR;
  }
  catch(XQException &e) {
    reportErrorStatic(context, XQC_DYNAMIC_ERROR, e);
    return XQC_DYNAMIC_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
  return XQC_NO_ERROR;
}

XQC_Error XQillaXQCImplementation::prepare_stream(XQC_Implementation *implementation, XQC_InputStream *stream,
                                                  const XQC_StaticContext *context, XQC_Expression **expression)
{
  try {
    XQCInputSource is(stream);
    XQQuery *query = XQilla::parse(is, XQillaXQCStaticContext::createContext(context));
    *expression = (new XQillaXQCExpression(query))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(ContextException &e) {
    return XQC_UNRECOGNIZED_ENCODING;
  }
  catch(XPath2TypeMatchException &e) {
    reportErrorStatic(context, XQC_TYPE_ERROR, e);
    return XQC_TYPE_ERROR;
  }
  catch(StaticErrorException &e) {
    reportErrorStatic(context, XQC_STATIC_ERROR, e);
    return XQC_STATIC_ERROR;
  }
  catch(XQException &e) {
    reportErrorStatic(context, XQC_DYNAMIC_ERROR, e);
    return XQC_DYNAMIC_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
  return XQC_NO_ERROR;
}

XQC_Error XQillaXQCImplementation::parse_document(XQC_Implementation *implementation,
                                                  const char *string, XQC_Sequence **sequence)
{
  try {
    AutoDelete<DynamicContext> context(XQillaXQCStaticContext::createContext(0));
    CharInputSource is(string);
    Node::Ptr doc = context->parseDocument(is);
    
    *sequence = (new XQillaXQCSequence(Result(doc), context.adopt()))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(XQException &e) {
    return XQC_PARSE_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCImplementation::parse_document_file(XQC_Implementation *implementation,
                                                       FILE *file, XQC_Sequence **sequence)
{
  try {
    AutoDelete<DynamicContext> context(XQillaXQCStaticContext::createContext(0));
    FileInputSource is(file);
    Node::Ptr doc = context->parseDocument(is);
    
    *sequence = (new XQillaXQCSequence(Result(doc), context.adopt()))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(XQException &e) {
    return XQC_PARSE_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCImplementation::parse_document_stream(XQC_Implementation *implementation,
                                                         XQC_InputStream *stream, XQC_Sequence **sequence)
{
  try {
    AutoDelete<DynamicContext> context(XQillaXQCStaticContext::createContext(0));
    XQCInputSource is(stream);
    Node::Ptr doc = context->parseDocument(is);
    
    *sequence = (new XQillaXQCSequence(Result(doc), context.adopt()))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(XQException &e) {
    return XQC_PARSE_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCImplementation::create_empty_sequence(XQC_Implementation *implementation,
                                                         XQC_Sequence **sequence)
{
  try {
    AutoDelete<DynamicContext> context(XQillaXQCStaticContext::createContext(0));
    *sequence = (new XQillaXQCSequence(Sequence(), context.adopt()))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

AnyAtomicType::AtomicObjectType xqcToXQillaType(XQC_ItemType type)
{
  switch(type) {
  case XQC_EMPTY_TYPE:
  case XQC_DOCUMENT_TYPE:
  case XQC_ELEMENT_TYPE:
  case XQC_ATTRIBUTE_TYPE:
  case XQC_TEXT_TYPE:
  case XQC_PROCESSING_INSTRUCTION_TYPE:
  case XQC_COMMENT_TYPE:
  case XQC_NAMESPACE_TYPE:
    break;

  case XQC_ANY_SIMPLE_TYPE:
    return AnyAtomicType::ANY_SIMPLE_TYPE;
  case XQC_ANY_URI_TYPE:
    return AnyAtomicType::ANY_URI;
  case XQC_BASE_64_BINARY_TYPE:
    return AnyAtomicType::BASE_64_BINARY;
  case XQC_BOOLEAN_TYPE:
    return AnyAtomicType::BOOLEAN;
  case XQC_DATE_TYPE:
    return AnyAtomicType::DATE;
  case XQC_DATE_TIME_TYPE:
    return AnyAtomicType::DATE_TIME;
  case XQC_DAY_TIME_DURATION_TYPE:
    return AnyAtomicType::DAY_TIME_DURATION;
  case XQC_DECIMAL_TYPE:
    return AnyAtomicType::DECIMAL;
  case XQC_DOUBLE_TYPE:
    return AnyAtomicType::DOUBLE;
  case XQC_DURATION_TYPE:
    return AnyAtomicType::DURATION;
  case XQC_FLOAT_TYPE:
    return AnyAtomicType::FLOAT;
  case XQC_G_DAY_TYPE:
    return AnyAtomicType::G_DAY;
  case XQC_G_MONTH_TYPE:
    return AnyAtomicType::G_MONTH;
  case XQC_G_MONTH_DAY_TYPE:
    return AnyAtomicType::G_MONTH_DAY;
  case XQC_G_YEAR_TYPE:
    return AnyAtomicType::G_YEAR;
  case XQC_G_YEAR_MONTH_TYPE:
    return AnyAtomicType::G_YEAR_MONTH;
  case XQC_HEX_BINARY_TYPE:
    return AnyAtomicType::HEX_BINARY;
  case XQC_NOTATION_TYPE:
    return AnyAtomicType::NOTATION;
  case XQC_QNAME_TYPE:
    return AnyAtomicType::QNAME;
  case XQC_STRING_TYPE:
    return AnyAtomicType::STRING;
  case XQC_TIME_TYPE:
    return AnyAtomicType::TIME;
  case XQC_UNTYPED_ATOMIC_TYPE:
    return AnyAtomicType::UNTYPED_ATOMIC;
  case XQC_YEAR_MONTH_DURATION_TYPE:
    return AnyAtomicType::YEAR_MONTH_DURATION;
  }

  return (AnyAtomicType::AtomicObjectType)-1;
}

Item::Ptr createItem(XQC_ItemType type, const char *value, DynamicContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  AnyAtomicType::AtomicObjectType xqtype = xqcToXQillaType(type);

  if(xqtype == (AnyAtomicType::AtomicObjectType)-1) return 0;

  return context->getItemFactory()->
    createDerivedFromAtomicType(xqtype, mm->getPooledString(value), context);
}

XQC_Error XQillaXQCImplementation::create_singleton_sequence(XQC_Implementation *implementation,
                                                             XQC_ItemType type, const char *value,
                                                             XQC_Sequence **sequence)
{
  try {
    AutoDelete<DynamicContext> context(XQillaXQCStaticContext::createContext(0));
    Item::Ptr item = createItem(type, value, context);
    if(item.isNull()) return XQC_INVALID_ARGUMENT;

    *sequence = (new XQillaXQCSequence(item, context.adopt()))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(XQException &e) {
    return XQC_INVALID_ARGUMENT;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCImplementation::create_integer_sequence(XQC_Implementation *implementation,
                                                           int values[], unsigned int count,
                                                           XQC_Sequence **sequence)
{
  try {
    AutoDelete<DynamicContext> context(XQillaXQCStaticContext::createContext(0));
    ItemFactory *fac = context->getItemFactory();

    Sequence seq;
    for(unsigned int i = 0; i < count; ++i) {
      seq.addItem(fac->createInteger(values[i], context));
    }

    *sequence = (new XQillaXQCSequence(seq, context.adopt()))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(XQException &e) {
    return XQC_INVALID_ARGUMENT;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCImplementation::create_double_sequence(XQC_Implementation *implementation,
                                                          double values[], unsigned int count,
                                                          XQC_Sequence **sequence)
{
  try {
    AutoDelete<DynamicContext> context(XQillaXQCStaticContext::createContext(0));
    ItemFactory *fac = context->getItemFactory();

    Sequence seq;
    for(unsigned int i = 0; i < count; ++i) {
      seq.addItem(fac->createDouble(values[i], context));
    }

    *sequence = (new XQillaXQCSequence(seq, context.adopt()))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(XQException &e) {
    return XQC_INVALID_ARGUMENT;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
}

XQC_Error XQillaXQCImplementation::create_string_sequence(XQC_Implementation *implementation,
                                                          const char *values[], unsigned int count,
                                                          XQC_Sequence **sequence)
{
  try {
    AutoDelete<DynamicContext> context(XQillaXQCStaticContext::createContext(0));
    XPath2MemoryManager *mm = context->getMemoryManager();
    ItemFactory *fac = context->getItemFactory();

    Sequence seq;
    for(unsigned int i = 0; i < count; ++i) {
      seq.addItem(fac->createString(mm->getPooledString(values[i]), context));
    }

    *sequence = (new XQillaXQCSequence(seq, context.adopt()))->getImpl();
    return XQC_NO_ERROR;
  }
  catch(XQException &e) {
    return XQC_INVALID_ARGUMENT;
  }
  catch(...) {
    return XQC_INTERNAL_ERROR;
  }
  return XQC_NO_ERROR;
}

void *XQillaXQCImplementation::get_interface(const XQC_Implementation *implementation, const char *name)
{
  return 0;
}

void XQillaXQCImplementation::free(XQC_Implementation *implementation)
{
  delete get(implementation);
}

extern "C" {

  XQC_Implementation *createXQillaXQCImplementation(int version)
  {
    try {
      if(version != 0 && version != XQC_VERSION_NUMBER)
        return 0;
      return (new XQillaXQCImplementation())->getImpl();
    }
    catch(...) {
      return 0;
    }
  }

}
