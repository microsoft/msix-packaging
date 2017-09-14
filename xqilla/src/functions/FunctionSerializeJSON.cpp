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
#if defined(WIN32) && !defined(__CYGWIN__)
#define snprintf _snprintf
#endif

#include <xqilla/functions/FunctionSerializeJSON.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/events/SequenceBuilder.hpp>
#include <xqilla/events/QueryPathTreeFilter.hpp>
#include <xqilla/schema/DocumentCache.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

#include "../yajl/yajl_parse.h"

XERCES_CPP_NAMESPACE_USE;
using namespace std;

const XMLCh FunctionSerializeJSON::name[] = {
  chLatin_s, chLatin_e, chLatin_r, chLatin_i, chLatin_a, chLatin_l, chLatin_i, chLatin_z, chLatin_e, chDash, chLatin_j, chLatin_s, chLatin_o, chLatin_n, chNull
};
const unsigned int FunctionSerializeJSON::minArgs = 1;
const unsigned int FunctionSerializeJSON::maxArgs = 1;

/**
 * xqilla:serialize-json($json-xml as element()?) as xs:string?
 */
FunctionSerializeJSON::FunctionSerializeJSON(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQillaFunction(name, "($json-xml as element()?) as xs:string?", args, memMgr)
{
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

class JSONSerializer : public EventHandler
{
public:
  JSONSerializer(const LocationInfo *loc)
    : loc_(loc)
  {
  }

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding) {}
  virtual void endDocumentEvent() {}
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri) {}
  virtual void endEvent() {}
  virtual void piEvent(const XMLCh *target, const XMLCh *value) {}
  virtual void commentEvent(const XMLCh *value) {}
  virtual void atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value, const XMLCh *typeURI,
                               const XMLCh *typeName) {}

  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
  {
    if(uri != 0)
      XQThrow3(FunctionException, X("FunctionSerializeJSON::createSequence"), X("Unknown element name in xqilla:serialize-json()"), loc_);

    if(XPath2Utils::equals(localname, JSON2XML_item)) {
      if(stack_.back().type == UNKNOWN) {
        stack_.back() = ARRAY;
        buffer_.append('[');
      }
      else if(stack_.back().type == ARRAY) {
        if(stack_.back().seenOne)
          buffer_.append(',');
      }
      else {
        XMLBuffer buf;
        buf.append(X("Invalid input to xqilla:serialize-json() - found 'item', expecting "));
        typeToBuffer(stack_.back().type, buf);
        XQThrow3(FunctionException, X("FunctionSerializeJSON::createSequence"), buf.getRawBuffer(), loc_);
      }
    }
    else if(XPath2Utils::equals(localname, JSON2XML_pair)) {
      if(stack_.back().type == UNKNOWN) {
        stack_.back() = MAP;
        buffer_.append('{');
      }
      else if(stack_.back().type == MAP) {
        if(stack_.back().seenOne)
          buffer_.append(',');
      }
      else {
        XMLBuffer buf;
        buf.append(X("Invalid input to xqilla:serialize-json() - found 'pair', expecting "));
        typeToBuffer(stack_.back().type, buf);
        XQThrow3(FunctionException, X("FunctionSerializeJSON::createSequence"), buf.getRawBuffer(), loc_);
      }
    }
    else if(XPath2Utils::equals(localname, JSON2XML_json)) {
      stack_.push_back(UNKNOWN);
    }
    else {
      XQThrow3(FunctionException, X("FunctionSerializeJSON::createSequence"), X("Unknown element name in xqilla:serialize-json()"), loc_);
    }

    stack_.back().seenOne = true;
    stack_.push_back(UNKNOWN);
  }

  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName)
  {
    if(uri != 0)
      XQThrow3(FunctionException, X("FunctionSerializeJSON::createSequence"), X("Unknown element name in xqilla:serialize-json()"), loc_);

    if(stack_.back().type == ARRAY)
      buffer_.append(']');
    else if(stack_.back().type == MAP)
      buffer_.append('}');
    else if(stack_.back().type == STRING)
      buffer_.append('"');
    else if(stack_.back().type == UNKNOWN)
      buffer_.append(JSON2XML_null);

    stack_.pop_back();
  }

  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName)
  {
    if(uri != 0) {
      XMLBuffer buf;
      buf.append(X("Unknown attribute name {"));
      buf.append(uri);
      buf.append('}');
      buf.append(localname);
      buf.append(X(" in xqilla:serialize-json()"));
      XQThrow3(FunctionException, X("FunctionSerializeJSON::createSequence"), buf.getRawBuffer(), loc_);
    }

    if(XPath2Utils::equals(localname, JSON2XML_name) && stack_.size() >= 2 && (++stack_.rbegin())->type == MAP) {
      buffer_.append('"');
      buffer_.append(value);
      buffer_.append('"');
      buffer_.append(':');
    }
    else if(XPath2Utils::equals(localname, JSON2XML_type)) {
      if(XPath2Utils::equals(value, JSON2XML_object)) {
        stack_.back() = MAP;
        buffer_.append('{');
      }
      else if(XPath2Utils::equals(value, JSON2XML_array)) {
        stack_.back() = ARRAY;
        buffer_.append('[');
      }
      else if(XPath2Utils::equals(value, JSON2XML_string)) {
        stack_.back() = STRING;
        buffer_.append('"');
      }
      else if(XPath2Utils::equals(value, JSON2XML_boolean)) {
        stack_.back() = BOOLEAN;
      }
      else if(XPath2Utils::equals(value, JSON2XML_number)) {
        stack_.back() = NUMBER;
      }
      else if(XPath2Utils::equals(value, JSON2XML_null)) {
        stack_.back() = TNULL;
        buffer_.append(JSON2XML_null);
      }
    }
    else {
      XMLBuffer buf;
      buf.append(X("Unknown attribute name {"));
      buf.append(uri);
      buf.append('}');
      buf.append(localname);
      buf.append(X(" in xqilla:serialize-json()"));
      XQThrow3(FunctionException, X("FunctionSerializeJSON::createSequence"), buf.getRawBuffer(), loc_);
    }
  }

  virtual void textEvent(const XMLCh *value)
  {
    if(stack_.back().type == UNKNOWN) {
      stack_.back() = STRING;
      buffer_.append('"');
    }
    else if(stack_.back().type == MAP ||
            stack_.back().type == ARRAY ||
            stack_.back().type == TNULL) {
      XMLBuffer buf;
      buf.append(X("Invalid input to xqilla:serialize-json() - found text node, expecting "));
      typeToBuffer(stack_.back().type, buf);
      XQThrow3(FunctionException, X("FunctionSerializeJSON::createSequence"), buf.getRawBuffer(), loc_);
    }

    stack_.back().seenOne = true;


    escapeString(value, XPath2Utils::uintStrlen(value));
  }

  virtual void textEvent(const XMLCh *chars, unsigned int length)
  {
    if(stack_.back().type == UNKNOWN) {
      stack_.back() = STRING;
      buffer_.append('"');
    }
    else if(stack_.back().type == MAP ||
            stack_.back().type == ARRAY ||
            stack_.back().type == TNULL) {
      XMLBuffer buf;
      buf.append(X("Invalid input to xqilla:serialize-json() - found text node, expecting "));
      typeToBuffer(stack_.back().type, buf);
      XQThrow3(FunctionException, X("FunctionSerializeJSON::createSequence"), buf.getRawBuffer(), loc_);
    }

    stack_.back().seenOne = true;

    escapeString(chars, length);
  }

  bool isEmpty() const
  {
    return buffer_.isEmpty();
  }

  const XMLCh *getResult()
  {
    return buffer_.getRawBuffer();
  }

private:
  void escapeString(const XMLCh *chars, unsigned int length)
  {
    while(length) {
      switch(*chars) {
      case 0x22:
        buffer_.append('\\');
        buffer_.append('"');
        break;
      case 0x5C:
        buffer_.append('\\');
        buffer_.append('\\');
        break;
      case 0x2F:
        buffer_.append('\\');
        buffer_.append('/');
        break;
      case 0x08:
        buffer_.append('\\');
        buffer_.append('b');
        break;
      case 0x0C:
        buffer_.append('\\');
        buffer_.append('f');
        break;
      case 0x0A:
        buffer_.append('\\');
        buffer_.append('n');
        break;
      case 0x0D:
        buffer_.append('\\');
        buffer_.append('r');
        break;
      case 0x09:
        buffer_.append('\\');
        buffer_.append('t');
        break;
      default:
        buffer_.append(*chars);
        break;
      }
      --length;
      ++chars;
    }
  }

  enum Type { UNKNOWN, MAP, ARRAY, STRING, BOOLEAN, NUMBER, TNULL };

  void typeToBuffer(Type type, XMLBuffer &buf)
  {
    switch(type) {
    case UNKNOWN: buf.append(X("unknown")); break;
    case MAP: buf.append(X("map")); break;
    case ARRAY: buf.append(X("array")); break;
    case STRING: buf.append(X("string")); break;
    case BOOLEAN: buf.append(X("boolean")); break;
    case NUMBER: buf.append(X("number")); break;
    case TNULL: buf.append(X("null")); break;
    }
  }

  XMLBuffer buffer_;
  const LocationInfo *loc_;

  struct StackObject;
  friend struct StackObject;

  struct StackObject {
    StackObject(Type t) : type(t), seenOne(false) {}
    Type type;
    bool seenOne;
  };
  vector<StackObject> stack_;
};

Sequence FunctionSerializeJSON::createSequence(DynamicContext* context, int flags) const
{
  JSONSerializer serializer(this);
  _args[0]->generateAndTailCall(&serializer, context, false, false);

  if(serializer.isEmpty()) return Sequence(context->getMemoryManager());

  return Sequence(context->getItemFactory()->createString(serializer.getResult(), context));
}
