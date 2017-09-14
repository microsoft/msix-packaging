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

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/XQDocumentConstructor.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/exceptions/ASTException.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/events/EventHandler.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

XQDocumentConstructor::XQDocumentConstructor(ASTNode *value, XPath2MemoryManager* mm)
  : XQDOMConstructor(mm),
    m_value(value)
{
}

EventGenerator::Ptr XQDocumentConstructor::generateEvents(EventHandler *events, DynamicContext *context,
                                                     bool preserveNS, bool preserveType) const
{
  events->startDocumentEvent(0, 0);

  DocConstructFilter filter(events, this);
  m_value->generateAndTailCall(&filter, context, preserveNS, preserveType);

  events->endDocumentEvent();
  return 0;
}

ASTNode* XQDocumentConstructor::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  m_value = new (mm) XQContentSequence(m_value, mm);
  m_value->setLocationInfo(this);

  m_value = m_value->staticResolution(context);

  return this;
}

ASTNode *XQDocumentConstructor::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(m_value->getStaticAnalysis());

  if(m_value->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("XQDocumentConstructor::staticTyping"),
            X("It is a static error for the content expression of a document node constructor "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.getStaticType() = StaticType::DOCUMENT_TYPE;
  _src.creative(true);
  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
	  StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::SAMEDOC |
	  StaticAnalysis::ONENODE);
  return this;
}

const XMLCh* XQDocumentConstructor::getNodeType() const
{
  return Node::document_string;
}

ASTNode *XQDocumentConstructor::getValue() const
{
  return m_value;
}

void XQDocumentConstructor::setValue(ASTNode *value)
{
  m_value = value;
}

//////////////////////////////////////////////////////////////////////

DocConstructFilter::DocConstructFilter(EventHandler *next, const LocationInfo *location)
  : EventFilter(next),
    location_(location),
    level_(0)
{
}

void DocConstructFilter::startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
{
  ++level_;
  next_->startElementEvent(prefix, uri, localname);
}

void DocConstructFilter::endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                                         const XMLCh *typeURI, const XMLCh *typeName)
{
  next_->endElementEvent(prefix, uri, localname, typeURI, typeName);
  --level_;
}

void DocConstructFilter::attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                                        const XMLCh *typeURI, const XMLCh *typeName)
{
  if(level_ == 0) {
    XQThrow3(ASTException,X("DocConstructFilter::attributeEvent"),
             X("An attribute node cannot be a child of a document [err:XPTY0004]."), location_);
  }

  next_->attributeEvent(prefix, uri, localname, value, typeURI, typeName);
}

void DocConstructFilter::namespaceEvent(const XMLCh *prefix, const XMLCh *uri)
{
  if(level_ == 0) {
    XQThrow3(ASTException,X("DocConstructFilter::attributeEvent"),
             X("An namespace node cannot be a child of a document [err:XPTY0004]."), location_);
  }

  next_->namespaceEvent(prefix, uri);
}
