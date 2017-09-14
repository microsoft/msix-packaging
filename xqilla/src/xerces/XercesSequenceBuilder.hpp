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

#ifndef _XERCESSEQUENCEBUILDER_HPP
#define _XERCESSEQUENCEBUILDER_HPP

#include <xqilla/events/SequenceBuilder.hpp>
#include <xqilla/runtime/Sequence.hpp>

#include "../dom-api/impl/XPathDocumentImpl.hpp"

class XercesSequenceBuilder : public SequenceBuilder
{
public:
  XercesSequenceBuilder(const DynamicContext *context);
  virtual ~XercesSequenceBuilder();

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
  virtual void atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value, const XMLCh *typeURI,
                               const XMLCh *typeName);
  virtual void endEvent();

  virtual Sequence getSequence() const { return seq_; }

  static void setElementTypeInfo(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *element, const XMLCh *typeURI, const XMLCh *typeName);
  static void setAttributeTypeInfo(XERCES_CPP_NAMESPACE_QUALIFIER DOMAttr *attr, const XMLCh *typeURI, const XMLCh *typeName);

private:
  const DynamicContext *context_;
  XPathDocumentImpl *document_;
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *currentParent_;
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *currentNode_;
  Sequence seq_;
};

#endif
