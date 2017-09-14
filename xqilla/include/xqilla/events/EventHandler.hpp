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

#ifndef _EVENTHANDLER_HPP
#define _EVENTHANDLER_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/ast/LocationInfo.hpp>

#include <xercesc/util/XercesDefs.hpp>

class XQILLA_API EventHandler
{
public:
  virtual ~EventHandler() {};

  /** Recieves a LocationInfo object that is owned by the caller, and will be
      updated with the current location information as the parse progresses. */
  virtual void setLocationInfo(const LocationInfo *location) {}

  /** Handles a document node as an event */
  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding) = 0;
  /** Handles a document node as an event */
  virtual void endDocumentEvent() = 0;
  /** Handles the start of an element node as an event */
  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname) = 0;
  /** Handles the end of an element node as an event */
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName) = 0;
  /** Handles a processing instruction node as an event */
  virtual void piEvent(const XMLCh *target, const XMLCh *value) = 0;
  /** Handles a text node as an event */
  virtual void textEvent(const XMLCh *value) = 0;
  /** Handles a text node as an event */
  virtual void textEvent(const XMLCh *chars, unsigned int length) = 0;
  /** Handles a comment node as an event */
  virtual void commentEvent(const XMLCh *value) = 0;
  /** Handles an attribute node as an event */
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName) = 0;
  /** Handles a namespace binding as an event */
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri) = 0;
  /** Handles an atomic item as an event */
  virtual void atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value,
                               const XMLCh *typeURI, const XMLCh *typeName) {}
  /** Called when all events have been reported */
  virtual void endEvent() = 0;
};

class XQILLA_API EventFilter : public EventHandler
{
public:
  EventFilter(EventHandler *next)
    : next_(next)
  {
  }

  void setNextEventHandler(EventHandler *next)
  {
    next_ = next;
  }

  virtual void setLocationInfo(const LocationInfo *location)
  {
    next_->setLocationInfo(location);
  }

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding)
  {
    next_->startDocumentEvent(documentURI, encoding);
  }

  virtual void endDocumentEvent()
  {
    next_->endDocumentEvent();
  }

  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
  {
    next_->startElementEvent(prefix, uri, localname);
  }

  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName)
  {
    next_->endElementEvent(prefix, uri, localname, typeURI, typeName);
  }

  virtual void piEvent(const XMLCh *target, const XMLCh *value)
  {
    next_->piEvent(target, value);
  }

  virtual void textEvent(const XMLCh *value)
  {
    next_->textEvent(value);
  }

  virtual void textEvent(const XMLCh *chars, unsigned int length)
  {
    next_->textEvent(chars, length);
  }

  virtual void commentEvent(const XMLCh *value)
  {
    next_->commentEvent(value);
  }

  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName)
  {
    next_->attributeEvent(prefix, uri, localname, value, typeURI, typeName);
  }

  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri)
  {
    next_->namespaceEvent(prefix, uri);
  }

  virtual void atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value, const XMLCh *typeURI,
                               const XMLCh *typeName)
  {
    next_->atomicItemEvent(type, value, typeURI, typeName);
  }

  virtual void endEvent()
  {
    next_->endEvent();
  }

protected:
  EventHandler *next_;
};

static inline const XMLCh *emptyToNull(const XMLCh * const in)
{
  return (in == 0 || *in == 0) ? 0 : in;
}

#endif
