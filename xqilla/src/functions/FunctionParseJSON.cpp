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

#include "../config/xqilla_config.h"

#include <stdio.h> // for sprintf
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#include <xqilla/functions/FunctionParseJSON.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/events/SequenceBuilder.hpp>
#include <xqilla/events/QueryPathTreeFilter.hpp>
#include <xqilla/schema/DocumentCache.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

#include "../yajl/yajl_parse.h"

XERCES_CPP_NAMESPACE_USE;
using namespace std;

const XMLCh FunctionParseJSON::name[] = {
  chLatin_p, chLatin_a, chLatin_r, chLatin_s, chLatin_e, chDash, chLatin_j, chLatin_s, chLatin_o, chLatin_n, chNull
};
const unsigned int FunctionParseJSON::minArgs = 1;
const unsigned int FunctionParseJSON::maxArgs = 1;

/**
 * xqilla:parse-json($xml as xs:string?) as element()?
 */
FunctionParseJSON::FunctionParseJSON(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQillaFunction(name, "($xml as xs:string?) as element()?", args, memMgr),
    queryPathTree_(0)
{
}

ASTNode *FunctionParseJSON::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();

  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::ONENODE);
  _src.creative(true);

  calculateSRCForArguments(context);
  return this;
}

const XMLCh JSON2XML_json[] = { 'j', 's', 'o', 'n', 0 };
const XMLCh JSON2XML_item[] = { 'i', 't', 'e', 'm', 0 };
const XMLCh JSON2XML_pair[] = { 'p', 'a', 'i', 'r', 0 };
const XMLCh JSON2XML_name[] = { 'n', 'a', 'm', 'e', 0 };
const XMLCh JSON2XML_type[] = { 't', 'y', 'p', 'e', 0 };
const XMLCh JSON2XML_array[] = { 'a', 'r', 'r', 'a', 'y', 0 };
const XMLCh JSON2XML_object[] = { 'o', 'b', 'j', 'e', 'c', 't', 0 };
const XMLCh JSON2XML_string[] = { 's', 't', 'r', 'i', 'n', 'g', 0 };
const XMLCh JSON2XML_boolean[] = { 'b', 'o', 'o', 'l', 'e', 'a', 'n', 0 };
const XMLCh JSON2XML_number[] = { 'n', 'u', 'm', 'b', 'e', 'r', 0 };
const XMLCh JSON2XML_null[] = { 'n', 'u', 'l', 'l', 0 };

struct JSON2XML_Env
{
  EventHandler *handler;
  enum Type { MAP, ARRAY };
  vector<Type> stack;

  void startValue()
  {
    if(stack.empty()) {
      // Do nothing
    }
    else if(stack.back() == JSON2XML_Env::ARRAY) {
      handler->startElementEvent(0, 0, JSON2XML_item);
    }
  }

  void endValue()
  {
    if(stack.empty()) {
      // Do nothing
    }
    else if(stack.back() == JSON2XML_Env::MAP) {
      handler->endElementEvent(0, 0, JSON2XML_pair,
                               SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               DocumentCache::g_szUntyped);
    }
    else {
      handler->endElementEvent(0, 0, JSON2XML_item,
                               SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               DocumentCache::g_szUntyped);
    }
  }

};

int json2xml_null(void *ctx)
{
  JSON2XML_Env *env = (JSON2XML_Env*)ctx;

  env->startValue();
  env->handler->attributeEvent(0, 0, JSON2XML_type, JSON2XML_null,
                               SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
  env->endValue();
  return 1;
}

int json2xml_boolean(void * ctx, int boolVal)
{
  JSON2XML_Env *env = (JSON2XML_Env*)ctx;

  env->startValue();
  env->handler->attributeEvent(0, 0, JSON2XML_type, JSON2XML_boolean,
                               SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
  env->handler->textEvent(boolVal ? SchemaSymbols::fgATTVAL_TRUE : SchemaSymbols::fgATTVAL_FALSE);
  env->endValue();
  return 1;
}

int json2xml_integer(void *ctx, long integerVal)
{
  JSON2XML_Env *env = (JSON2XML_Env*)ctx;

  char intString[256];
  snprintf(intString, 256,"%lld", (xq_int64_t)integerVal);

  env->startValue();
  env->handler->attributeEvent(0, 0, JSON2XML_type, JSON2XML_number,
                               SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
  env->handler->textEvent(X(intString));
  env->endValue();
  return 1;
}

int json2xml_double(void *ctx, double doubleVal)
{
  JSON2XML_Env *env = (JSON2XML_Env*)ctx;

  char doubleString[256];
  snprintf(doubleString, 256,"%lf", doubleVal);

  env->startValue();
  env->handler->attributeEvent(0, 0, JSON2XML_type, JSON2XML_number,
                               SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
  env->handler->textEvent(X(doubleString));
  env->endValue();
  return 1;
}

int json2xml_string(void *ctx, const unsigned char * stringVal,
                     unsigned int stringLen)
{
  JSON2XML_Env *env = (JSON2XML_Env*)ctx;

  AutoDeleteArray<char> str(new char[stringLen + 1]);
  memcpy(str.get(), stringVal, stringLen);
  str.get()[stringLen] = 0;

  env->startValue();
  env->handler->attributeEvent(0, 0, JSON2XML_type, JSON2XML_string,
                               SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
  env->handler->textEvent(X(str.get()));
  env->endValue();
  return 1;
}

int json2xml_map_key(void *ctx, const unsigned char * stringVal,
                     unsigned int stringLen)
{
  JSON2XML_Env *env = (JSON2XML_Env*)ctx;

  AutoDeleteArray<char> str(new char[stringLen + 1]);
  memcpy(str.get(), stringVal, stringLen);
  str.get()[stringLen] = 0;

  env->handler->startElementEvent(0, 0, JSON2XML_pair);
  env->handler->attributeEvent(0, 0, JSON2XML_name, X(str.get()),
                               SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
  return 1;
}

int json2xml_start_map(void *ctx)
{
  JSON2XML_Env *env = (JSON2XML_Env*)ctx;

  env->startValue();
  env->handler->attributeEvent(0, 0, JSON2XML_type, JSON2XML_object,
                               SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
  env->stack.push_back(JSON2XML_Env::MAP);
  return 1;
}


int json2xml_end_map(void *ctx)
{
  JSON2XML_Env *env = (JSON2XML_Env*)ctx;

  env->stack.pop_back();
  env->endValue();
  return 1;
}

int json2xml_start_array(void *ctx)
{
  JSON2XML_Env *env = (JSON2XML_Env*)ctx;

  env->startValue();
  env->handler->attributeEvent(0, 0, JSON2XML_type, JSON2XML_array,
                               SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
  env->stack.push_back(JSON2XML_Env::ARRAY);
  return 1;
}

int json2xml_end_array(void *ctx)
{
  JSON2XML_Env *env = (JSON2XML_Env*)ctx;

  env->stack.pop_back();
  env->endValue();
  return 1;
}

static yajl_callbacks json2xml_callbacks = {
  json2xml_null,
  json2xml_boolean,
  json2xml_integer,
  json2xml_double,
  json2xml_string,
  json2xml_start_map,
  json2xml_map_key,
  json2xml_end_map,
  json2xml_start_array,
  json2xml_end_array
};

void FunctionParseJSON::parseJSON(const XMLCh *jsonString, EventHandler *handler, DynamicContext *context, const LocationInfo *location)
{
  UTF8Str utf8(jsonString);

  JSON2XML_Env env;
  env.handler = handler;

  handler->startElementEvent(0, 0, JSON2XML_json);

  yajl_parser_config cfg = { 0 };
  yajl_handle yajl = yajl_alloc(&json2xml_callbacks, &cfg, &env);

  yajl_status stat = yajl_parse(yajl, (unsigned char*)utf8.str(),
                                (unsigned int)strlen(utf8.str()));
  if(stat != yajl_status_ok) {
    XMLBuffer buf;
    buf.append(X("JSON "));
    unsigned char *str = yajl_get_error(yajl, 1, (unsigned char*)utf8.str(),
                                        (unsigned int)strlen(utf8.str()));
    buf.append(X((char*)str));
    yajl_free_error(str);
    XQThrow3(FunctionException, X("FunctionParseJSON::parseJSON"), buf.getRawBuffer(), location);
  }

  handler->endElementEvent(0, 0, JSON2XML_json,
                           SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                           DocumentCache::g_szUntyped);
}

Sequence FunctionParseJSON::createSequence(DynamicContext* context, int flags) const
{
  Item::Ptr item = getParamNumber(1, context)->next(context);

  if(item.isNull()) return Sequence(context->getMemoryManager());

  AutoDelete<SequenceBuilder> builder(context->createSequenceBuilder());
  QueryPathTreeFilter qptf(queryPathTree_, builder.get());
  EventHandler *handler = queryPathTree_ ? (EventHandler*)&qptf : (EventHandler*)builder.get();

  parseJSON(item->asString(context), handler, context, this);

  handler->endEvent();
  return builder->getSequence();
}
