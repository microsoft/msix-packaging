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

#include <xqilla/events/EventSerializer.hpp>
#include <xqilla/utils/XStr.hpp>

#include <assert.h>

XERCES_CPP_NAMESPACE_USE;

static const XMLCh startElem_start = '<';
static const XMLCh elem_end = '>';
static const XMLCh endElem_selfClose[] = { '/', '>', 0 };
static const XMLCh endElem_start[] = { '<', '/', 0 };
static const XMLCh colon = ':';
static const XMLCh pi_start[] = { '<', '?', 0 };
static const XMLCh space = ' ';
static const XMLCh pi_end[] = { '?', '>', 0 };
static const XMLCh comment_start[] = { '<', '!', '-', '-', 0 };
static const XMLCh comment_end[] = { '-', '-', '>', 0 };
static const XMLCh attr_equals_quote[] = { '=', '"', 0 };
static const XMLCh quote = '"';
static const XMLCh attr_xmlns[] = { ' ', 'x', 'm', 'l', 'n', 's', 0 };
static const XMLCh openCurly = '{';
static const XMLCh closeCurly = '}';
static const XMLCh openSquare = '[';
static const XMLCh closeSquare = ']';
static const XMLCh newline = '\n';

EventSerializer::EventSerializer(const char *encoding, const char *xmlVersion, XMLFormatTarget *target, MemoryManager *mm)
  : formatter_(encoding, xmlVersion, target, XMLFormatter::CharEscapes, XMLFormatter::UnRep_CharRef, mm),
    elementStarted_(false),
    level_(0),
    version_(XStr(xmlVersion, mm).adopt()),
    encoding_(XStr(encoding, mm).adopt()),
    addNewlines_(false),
    selfClosing_(true),
    xmlDecls_(true),
    mm_(mm)
{
}

EventSerializer::EventSerializer(XMLFormatTarget *target, MemoryManager *mm)
  : formatter_("UTF-16", "1.1", target, XMLFormatter::CharEscapes, XMLFormatter::UnRep_CharRef, mm),
    elementStarted_(false),
    level_(0),
    version_(XStr("1.1", mm).adopt()),
    encoding_(XStr("UTF-16", mm).adopt()),
    addNewlines_(false),
    selfClosing_(true),
    xmlDecls_(true),
    mm_(mm)
{
}

EventSerializer::~EventSerializer()
{
  mm_->deallocate(version_);
  mm_->deallocate(encoding_);
}

void EventSerializer::startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding)
{
  if(xmlDecls_) {
    formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;
    formatter_ << pi_start;
    formatter_ << X("xml version=\"");
    formatter_ << version_;
    formatter_ << X("\" encoding=\""); 
    formatter_ << encoding_;
    formatter_ << X("\"");
    formatter_ << pi_end;
    formatter_ << newline;
  }  
  ++level_;
}

void EventSerializer::endDocumentEvent()
{
  --level_;

  // Nothing to do

  if(addNewlines_ && level_ == 0) {
    formatter_ << newline;
  }
}

void EventSerializer::endEvent()
{
  // Nothing to do
}

void EventSerializer::startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
{
  formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;

  if(elementStarted_) {
    elementStarted_ = false;
    formatter_ << elem_end;
  }

  formatter_ << X("<");
  if(prefix != 0) {
    formatter_ << prefix;
    formatter_ << colon;
  }
  formatter_ << localname;

  elementStarted_ = true;
  ++level_;
}

void EventSerializer::endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                                      const XMLCh *typeURI, const XMLCh *typeName)
{
  --level_;
  formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;

  if(elementStarted_ && !selfClosing_) {
    elementStarted_ = false;
    formatter_ << elem_end;
  }

  if(elementStarted_) {
    elementStarted_ = false;
    formatter_ << endElem_selfClose;
  }
  else {
    formatter_ << endElem_start;
    if(prefix != 0) {
      formatter_ << prefix;
      formatter_ << colon;
    }
    formatter_ << localname;
    formatter_ << elem_end;
  }

  if(addNewlines_ && level_ == 0) {
    formatter_ << newline;
  }
}

void EventSerializer::piEvent(const XMLCh *target, const XMLCh *value)
{
  formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;

  if(elementStarted_) {
    elementStarted_ = false;
    formatter_ << elem_end;
  }

  formatter_ << pi_start;
  formatter_ << target;
  formatter_ << space;
  formatter_ << value;
  formatter_ << pi_end;

  if(addNewlines_ && level_ == 0) {
    formatter_ << newline;
  }
}

void EventSerializer::textEvent(const XMLCh *value)
{
  if(elementStarted_) {
    elementStarted_ = false;
    formatter_ << elem_end;
  }
  formatter_ << XMLFormatter::CharEscapes << XMLFormatter::UnRep_CharRef;
  formatter_ << value;

  if(addNewlines_ && level_ == 0) {
    formatter_ << newline;
  }
}

void EventSerializer::textEvent(const XMLCh *chars, unsigned int length)
{
  if(elementStarted_) {
    elementStarted_ = false;
    formatter_ << elem_end;
  }
  formatter_ << XMLFormatter::CharEscapes << XMLFormatter::UnRep_CharRef;
  formatter_.formatBuf(chars, length);

  if(addNewlines_ && level_ == 0) {
    formatter_ << newline;
  }
}

void EventSerializer::commentEvent(const XMLCh *value)
{
  formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;

  if(elementStarted_) {
    elementStarted_ = false;
    formatter_ << elem_end;
  }
  formatter_ << comment_start;
  formatter_ << value;
  formatter_ << comment_end;

  if(addNewlines_ && level_ == 0) {
    formatter_ << newline;
  }
}

void EventSerializer::attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                                     const XMLCh *typeURI, const XMLCh *typeName)
{
  if(elementStarted_) {
    formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;

    formatter_ << space;
    if(prefix != 0) {
      formatter_ << prefix;
      formatter_ << colon;
    }
    formatter_ << localname;
    formatter_ << attr_equals_quote;

    formatter_ << XMLFormatter::AttrEscapes << XMLFormatter::UnRep_CharRef;
    formatter_ << value;

    formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;
    formatter_ << quote;
  }
  else {
    assert(level_ == 0);

    formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;
    if(uri != 0) {
      formatter_ << openCurly;
      formatter_ << uri;
      formatter_ << closeCurly;
    }
    formatter_ << localname;
    formatter_ << attr_equals_quote;

    formatter_ << XMLFormatter::AttrEscapes << XMLFormatter::UnRep_CharRef;
    formatter_ << value;

    formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;
    formatter_ << quote;

    if(addNewlines_)
      formatter_ << newline;
  }
}

void EventSerializer::namespaceEvent(const XMLCh *prefix, const XMLCh *uri)
{
  if(elementStarted_) {
    formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;

    formatter_ << attr_xmlns;
    if(prefix != 0) {
      formatter_ << colon;
      formatter_ << prefix;
    }
    formatter_ << attr_equals_quote;

    formatter_ << XMLFormatter::AttrEscapes << XMLFormatter::UnRep_CharRef;
    formatter_ << uri;

    formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;
    formatter_ << quote;
  }
  else {
    assert(level_ == 0);

    formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;

    formatter_ << openSquare;
    if(prefix != 0) {
      formatter_ << prefix;
    }
    formatter_ << attr_equals_quote;

    formatter_ << XMLFormatter::AttrEscapes << XMLFormatter::UnRep_CharRef;
    formatter_ << uri;

    formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;
    formatter_ << quote;
    formatter_ << closeSquare;

    if(addNewlines_)
      formatter_ << newline;
  }
}

void EventSerializer::atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value,
                                      const XMLCh *typeURI, const XMLCh *typeName)
{
  assert(level_ == 0);

  formatter_ << XMLFormatter::NoEscapes << XMLFormatter::UnRep_Fail;
  formatter_ << value;

  if(addNewlines_)
    formatter_ << newline;
}

