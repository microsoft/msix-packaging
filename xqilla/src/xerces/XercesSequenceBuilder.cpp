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
#include "XercesSequenceBuilder.hpp"
#include "XercesNodeImpl.hpp"

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>

#include "../dom-api/impl/XPathDocumentImpl.hpp"
#include "../dom-api/XQillaImplementation.hpp"
#include "../dom-api/impl/XPathNamespaceImpl.hpp"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/impl/DOMTypeInfoImpl.hpp>
#include <xercesc/dom/impl/DOMAttrImpl.hpp>
#include <xercesc/dom/impl/DOMElementNSImpl.hpp>
#include <xercesc/framework/psvi/PSVIItem.hpp>

XERCES_CPP_NAMESPACE_USE;

XercesSequenceBuilder::XercesSequenceBuilder(const DynamicContext *context)
  : context_(context),
    document_(0),
    currentParent_(0),
    currentNode_(0),
    seq_(context->getMemoryManager())
{
}

XercesSequenceBuilder::~XercesSequenceBuilder()
{
  delete document_;
}

void XercesSequenceBuilder::startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding)
{
  if(document_ == 0) {
    document_ = new (context_->getMemoryManager()) XPathDocumentImpl(XQillaImplementation::getDOMImplementationImpl(), context_->getMemoryManager());
  }

  currentParent_ = document_;
  currentNode_   = document_;

  document_->setDocumentURI(documentURI);
#if _XERCES_VERSION >= 30000
  document_->setInputEncoding(encoding);
#else
  document_->setActualEncoding(encoding);
#endif
}

void XercesSequenceBuilder::endDocumentEvent()
{
  currentNode_ = currentParent_;
  currentParent_ = currentParent_->getParentNode();

  if(currentParent_ == 0) {
    seq_.addItem(new XercesNodeImpl(currentNode_, (XercesURIResolver*)context_->getDefaultURIResolver()));
    document_ = 0;
    currentNode_ = 0;
  }
}

void XercesSequenceBuilder::endEvent()
{
}

void XercesSequenceBuilder::startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
{
  if(document_ == 0) {
    document_ = new (context_->getMemoryManager()) XPathDocumentImpl(XQillaImplementation::getDOMImplementationImpl(), context_->getMemoryManager());
  }

  DOMElement *elem = document_->createElementNS(uri, localname);
  if(prefix != 0)
    elem->setPrefix(prefix);

  if(currentParent_ != 0)
    currentParent_->appendChild(elem);

  currentParent_ = elem;
  currentNode_ = elem;
}

void XercesSequenceBuilder::setElementTypeInfo(DOMElement *element, const XMLCh *typeURI, const XMLCh *typeName)
{
  // TBD detect xs:untyped? - jpcs
  DOMDocument *document = element->getOwnerDocument();
  DOMTypeInfoImpl* pInfo = new (document) DOMTypeInfoImpl(typeURI, typeName);
  pInfo->setNumericProperty(DOMPSVITypeInfo::PSVI_Validity, PSVIItem::VALIDITY_VALID);
#if _XERCES_VERSION >= 30000
  ((DOMElementNSImpl*)element)->setSchemaTypeInfo(pInfo);
#else
  ((DOMElementNSImpl*)element)->setTypeInfo(pInfo);
#endif
}

void XercesSequenceBuilder::endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                                            const XMLCh *typeURI, const XMLCh *typeName)
{
  currentNode_ = currentParent_;
  currentParent_ = currentParent_->getParentNode();

  if(typeName)
    setElementTypeInfo((DOMElement*)currentNode_, typeURI, typeName);
  else
    setElementTypeInfo((DOMElement*)currentNode_, SchemaSymbols::fgURI_SCHEMAFORSCHEMA, DocumentCache::g_szUntyped);

  if(currentParent_ == 0) {
    seq_.addItem(new XercesNodeImpl(currentNode_, (XercesURIResolver*)context_->getDefaultURIResolver()));
    document_ = 0;
    currentNode_ = 0;
  }
}

void XercesSequenceBuilder::piEvent(const XMLCh *target, const XMLCh *value)
{
  if(document_ == 0) {
    document_ = new (context_->getMemoryManager()) XPathDocumentImpl(XQillaImplementation::getDOMImplementationImpl(), context_->getMemoryManager());
  }

  DOMProcessingInstruction *pi = document_->createProcessingInstruction(target, value);

  if(currentParent_ != 0)
    currentParent_->appendChild(pi);
  currentNode_ = pi;

  if(currentParent_ == 0) {
    seq_.addItem(new XercesNodeImpl(currentNode_, (XercesURIResolver*)context_->getDefaultURIResolver()));
    document_ = 0;
    currentNode_ = 0;
  }
}

void XercesSequenceBuilder::textEvent(const XMLCh *value)
{
  if(document_ == 0) {
    document_ = new (context_->getMemoryManager()) XPathDocumentImpl(XQillaImplementation::getDOMImplementationImpl(), context_->getMemoryManager());
  }

  if(currentNode_ != 0 && currentNode_->getNodeType() == DOMNode::TEXT_NODE) {
    // Combine adjacent text nodes
    ((DOMText *)currentNode_)->appendData(value);
  }
  else if(currentParent_ == 0 || (value != 0 && *value != 0)) {
		// Text nodes with a zero length value can only exist
		// when they have no parent
    DOMText *node = document_->createTextNode(value);

    if(currentParent_ != 0)
      currentParent_->appendChild(node);
    currentNode_ = node;
  }

  if(currentParent_ == 0) {
    seq_.addItem(new XercesNodeImpl(currentNode_, (XercesURIResolver*)context_->getDefaultURIResolver()));
    document_ = 0;
    currentNode_ = 0;
  }
}

void XercesSequenceBuilder::textEvent(const XMLCh *chars, unsigned int length)
{
  if(document_ == 0) {
    document_ = new (context_->getMemoryManager()) XPathDocumentImpl(XQillaImplementation::getDOMImplementationImpl(), context_->getMemoryManager());
  }

  if(currentNode_->getNodeType() == DOMNode::TEXT_NODE) {
    // Combine adjacent text nodes
    XMLBuffer buf;
    buf.append(chars, length);
    ((DOMText *)currentNode_)->appendData(buf.getRawBuffer());
  }
  else if(currentParent_ == 0 || length != 0) {
		// Text nodes with a zero length value can only exist
		// when they have no parent
    XMLBuffer buf;
    buf.append(chars, length);
    DOMText *node = document_->createTextNode(buf.getRawBuffer());

    if(currentParent_ != 0)
      currentParent_->appendChild(node);
    currentNode_ = node;
  }

  if(currentParent_ == 0) {
    seq_.addItem(new XercesNodeImpl(currentNode_, (XercesURIResolver*)context_->getDefaultURIResolver()));
    document_ = 0;
    currentNode_ = 0;
  }
}

void XercesSequenceBuilder::commentEvent(const XMLCh *value)
{
  if(document_ == 0) {
    document_ = new (context_->getMemoryManager()) XPathDocumentImpl(XQillaImplementation::getDOMImplementationImpl(), context_->getMemoryManager());
  }

  DOMComment *comment = document_->createComment(value);

  if(currentParent_ != 0)
    currentParent_->appendChild(comment);
  currentNode_ = comment;

  if(currentParent_ == 0) {
    seq_.addItem(new XercesNodeImpl(currentNode_, (XercesURIResolver*)context_->getDefaultURIResolver()));
    document_ = 0;
    currentNode_ = 0;
  }
}

void XercesSequenceBuilder::setAttributeTypeInfo(DOMAttr *attr, const XMLCh *typeURI, const XMLCh *typeName)
{
  // TBD detect xs:untypedAtomic? - jpcs
  DOMDocument *document = attr->getOwnerDocument();
  DOMTypeInfoImpl* pInfo = new (document) DOMTypeInfoImpl(typeURI, typeName);
  pInfo->setNumericProperty(DOMPSVITypeInfo::PSVI_Validity, PSVIItem::VALIDITY_VALID);
#if _XERCES_VERSION >= 30000
  ((DOMAttrImpl*)attr)->setSchemaTypeInfo(pInfo);
#else
  ((DOMAttrImpl*)attr)->setTypeInfo(pInfo);
#endif
}

void XercesSequenceBuilder::attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                                           const XMLCh *typeURI, const XMLCh *typeName)
{
  if(document_ == 0) {
    document_ = new (context_->getMemoryManager()) XPathDocumentImpl(XQillaImplementation::getDOMImplementationImpl(), context_->getMemoryManager());
  }

  DOMAttr *attr = document_->createAttributeNS(uri, localname);
  if(prefix != 0)
    attr->setPrefix(prefix);
  attr->setValue(value);

  if(typeName)
    setAttributeTypeInfo(attr, typeURI, typeName);
  else
    setAttributeTypeInfo(attr, SchemaSymbols::fgURI_SCHEMAFORSCHEMA, ATUntypedAtomic::fgDT_UNTYPEDATOMIC);

  if(currentParent_ != 0)
    currentParent_->getAttributes()->setNamedItemNS(attr);
  currentNode_ = attr;

  if(currentParent_ == 0) {
    seq_.addItem(new XercesNodeImpl(currentNode_, (XercesURIResolver*)context_->getDefaultURIResolver()));
    document_ = 0;
    currentNode_ = 0;
  }
}

void XercesSequenceBuilder::namespaceEvent(const XMLCh *prefix, const XMLCh *uri)
{
  if(currentParent_ != 0) {
    DOMAttr *attr = document_->createAttributeNS(XMLUni::fgXMLNSURIName, prefix == 0 ? XMLUni::fgXMLNSString : prefix);
    if(prefix != 0) attr->setPrefix(XMLUni::fgXMLNSString);
    attr->setValue(uri);

    currentParent_->getAttributes()->setNamedItemNS(attr);
    currentNode_ = attr;
  } else {
    document_ = new (context_->getMemoryManager()) XPathDocumentImpl(XQillaImplementation::getDOMImplementationImpl(), context_->getMemoryManager());

    DOMXPathNamespace *ns = new (document_, (DOMDocumentImpl::NodeObjectType)XPathNamespaceImpl::XPATH_NAMESPACE_OBJECT)
      XPathNamespaceImpl(prefix, uri, 0, document_);

    seq_.addItem(new XercesNodeImpl(ns, (XercesURIResolver*)context_->getDefaultURIResolver()));
    document_ = 0;
    currentNode_ = 0;
  }
}

void XercesSequenceBuilder::atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value, const XMLCh *typeURI,
                                            const XMLCh *typeName)
{
  assert(currentParent_ == 0);

  seq_.addItem(context_->getItemFactory()->createDerivedFromAtomicType(type, typeURI, typeName, value, context_));
}

