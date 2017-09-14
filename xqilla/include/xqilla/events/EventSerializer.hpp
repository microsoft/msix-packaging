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

#ifndef _EVENTSERIALIZER_HPP
#define _EVENTSERIALIZER_HPP

#include <xqilla/events/EventHandler.hpp>

#include <xercesc/framework/XMLFormatter.hpp>

class XQILLA_API EventSerializer : public EventHandler
{
public:
  EventSerializer(const char *encoding, const char *xmlVersion, XERCES_CPP_NAMESPACE_QUALIFIER XMLFormatTarget *target,
                  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm =
                  XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);
  EventSerializer(XERCES_CPP_NAMESPACE_QUALIFIER XMLFormatTarget *target,
                  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm =
                  XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);
  ~EventSerializer();

  virtual void setLocationInfo(const LocationInfo *location) {}

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding);
  virtual void endDocumentEvent();
  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname);
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName);
  virtual void piEvent(const XMLCh *target, const XMLCh *value);
  virtual void textEvent(const XMLCh *value);
  virtual void textEvent(const XMLCh *chars, unsigned int length);
  virtual void commentEvent(const XMLCh *value);
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName);
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri);
  virtual void atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value,
                               const XMLCh *typeURI, const XMLCh *typeName);
  virtual void endEvent();

  void addNewlines(bool add) { addNewlines_ = add; }
  void useSelfClosingElement(bool value) { selfClosing_ = value; }
  void addXMLDeclarations(bool value) { xmlDecls_ = value; }

private:
  XERCES_CPP_NAMESPACE_QUALIFIER XMLFormatter formatter_;
  bool elementStarted_;
  unsigned int level_;
  XMLCh *version_, *encoding_;
  bool addNewlines_, selfClosing_, xmlDecls_;
  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm_;
};

#endif
