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

#include "FastXDMDocument.hpp"

#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/utils/XMLChCompare.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/schema/DocumentCache.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

#include <set>

#define INITIAL_NODES 1000
#define INITIAL_ATTRIBUTES 500
#define INITIAL_NAMESPACES 5

XERCES_CPP_NAMESPACE_USE;

FastXDMDocument::FastXDMDocument(XPath2MemoryManager *mm)
  : elementStack_(21, mm),
    prevNode_((unsigned int)-1),
    textBuffer_(1023, mm),
    textToCreate_(false),
    nodes_((Node*)mm->allocate(INITIAL_NODES * sizeof(Node))),
    numNodes_(0),
    maxNodes_(INITIAL_NODES),
    attributes_((Attribute*)mm->allocate(INITIAL_ATTRIBUTES * sizeof(Attribute))),
    numAttributes_(0),
    maxAttributes_(INITIAL_ATTRIBUTES),
    namespaces_((Namespace*)mm->allocate(INITIAL_NAMESPACES * sizeof(Namespace))),
    numNamespaces_(0),
    maxNamespaces_(INITIAL_NAMESPACES),
    mm_(mm)
{
}

FastXDMDocument::FastXDMDocument(unsigned int numNodes, unsigned int numAttributes, unsigned int numNamespaces, XPath2MemoryManager *mm)
  : elementStack_(21, mm),
    prevNode_((unsigned int)-1),
    textBuffer_(1023, mm),
    textToCreate_(false),
    nodes_((Node*)mm->allocate(numNodes * sizeof(Node))),
    numNodes_(0),
    maxNodes_(numNodes),
    attributes_((Attribute*)mm->allocate(numAttributes * sizeof(Attribute))),
    numAttributes_(0),
    maxAttributes_(numAttributes),
    namespaces_((Namespace*)mm->allocate(numNamespaces * sizeof(Namespace))),
    numNamespaces_(0),
    maxNamespaces_(numNamespaces),
    mm_(mm)
{
}

FastXDMDocument::~FastXDMDocument()
{
  mm_->deallocate(nodes_);
  mm_->deallocate(attributes_);
  mm_->deallocate(namespaces_);
}

FastXDMDocument::Node *FastXDMDocument::getNode(unsigned int i)
{
  assert(i < numNodes_);

  return nodes_ + i;
}

const FastXDMDocument::Node *FastXDMDocument::getNode(unsigned int i) const
{
  assert(i < numNodes_);

  return nodes_ + i;
}

const FastXDMDocument::Attribute *FastXDMDocument::getAttribute(unsigned int i) const
{
  assert(i < numAttributes_);

  return attributes_ + i;
}

FastXDMDocument::Attribute *FastXDMDocument::getAttribute(unsigned int i)
{
  assert(i < numAttributes_);

  return attributes_ + i;
}

FastXDMDocument::Namespace *FastXDMDocument::getNamespace(unsigned int i)
{
  assert(i < numNamespaces_);

  return namespaces_ + i;
}

#define checkTextBuffer() \
{ \
  if(textToCreate_) { \
    if(numNodes_ == 0 || !textBuffer_.isEmpty()) { \
      if(numNodes_ == maxNodes_) resizeNodes(); \
\
      nodes_[numNodes_].setText((unsigned int) elementStack_.size(), mm_->getPooledString(textBuffer_.getRawBuffer())); \
\
      if(prevNode_ != (unsigned int)-1) \
        getNode(prevNode_)->nextSibling.index = numNodes_; \
\
      prevNode_ = numNodes_; \
      ++numNodes_; \
    } \
\
    textBuffer_.reset(); \
    textToCreate_ = false; \
  } \
}

void FastXDMDocument::startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding)
{
  elementStack_.removeAllElements();
  textBuffer_.reset();
  textToCreate_ = false;

  if(numNodes_ == maxNodes_) resizeNodes();

  nodes_[numNodes_].setDocument(mm_->getPooledString(documentURI),
                                mm_->getPooledString(encoding));
  elementStack_.push(numNodes_);
  prevNode_ = (unsigned int)-1;

  ++numNodes_;
}

void FastXDMDocument::endDocumentEvent()
{
  checkTextBuffer();
  prevNode_ = elementStack_.pop();
}

void FastXDMDocument::endEvent()
{
  checkTextBuffer();
  elementStack_.removeAllElements();

  // Add the end of nodes marker
  if(numNodes_ > 0) {
    if(numNodes_ == maxNodes_) resizeNodes();
    nodes_[numNodes_].setMarker();
    ++numNodes_;
  }

  // Add the end of attributes marker
  if(numAttributes_ > 0) {
	  if(numAttributes_ == maxAttributes_) resizeAttributes();
	  attributes_[numAttributes_].setMarker();
	  ++numAttributes_;
  }

  // Add the end of namespaces marker
  if(numNamespaces_ > 0) {
	  if(numNamespaces_ == maxNamespaces_) resizeNamespaces();
	  namespaces_[numNamespaces_].setMarker();
	  ++numNamespaces_;
  }

  // Rewrite all the indexes to pointers
  Node *nend = nodes_ + numNodes_;
  for(Node *node = nodes_; node != nend; ++node) {
    if(node->nextSibling.index != (unsigned int)-1)
      node->nextSibling.ptr = nodes_ + node->nextSibling.index;
    else node->nextSibling.ptr = 0;

    if(node->nodeKind == ELEMENT) {
      if(node->data.element.attributes.index != (unsigned int)-1)
        node->data.element.attributes.ptr = attributes_ + node->data.element.attributes.index;
      else node->data.element.attributes.ptr = 0;

      if(node->data.element.namespaces.index != (unsigned int)-1)
        node->data.element.namespaces.ptr = namespaces_ + node->data.element.namespaces.index;
      else node->data.element.namespaces.ptr = 0;
    }
  }

  Attribute *aend = attributes_ + numAttributes_;
  for(Attribute *attr = attributes_; attr != aend; ++attr) {
    if(attr->owner.index != (unsigned int)-1)
      attr->owner.ptr = nodes_ + attr->owner.index;
    else attr->owner.ptr = 0;
  }

  Namespace *nmend = namespaces_ + numNamespaces_;
  for(Namespace *ns = namespaces_; ns != nmend; ++ns) {
    if(ns->owner.index != (unsigned int)-1)
      ns->owner.ptr = nodes_ + ns->owner.index;
    else ns->owner.ptr = 0;
  }
}

void FastXDMDocument::startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
{
  checkTextBuffer();

  if(numNodes_ == maxNodes_) resizeNodes();

  nodes_[numNodes_].setElement((unsigned int) elementStack_.size(), mm_->getPooledString(prefix),
                               mm_->getPooledString(uri),  mm_->getPooledString(localname));

  if(prevNode_ != (unsigned int)-1)
    getNode(prevNode_)->nextSibling.index = numNodes_;

  elementStack_.push(numNodes_);
  prevNode_ = (unsigned int)-1;

  ++numNodes_;
}

void FastXDMDocument::endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                                      const XMLCh *typeURI, const XMLCh *typeName)
{
  checkTextBuffer();
  prevNode_ = elementStack_.pop();

  if(typeName) {
    getNode(prevNode_)->setElementType(typeURI, typeName);
  }
  else {
    getNode(prevNode_)->setElementType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, DocumentCache::g_szUntyped);
  }
}

void FastXDMDocument::piEvent(const XMLCh *target, const XMLCh *value)
{
  checkTextBuffer();

  if(numNodes_ == maxNodes_) resizeNodes();

  nodes_[numNodes_].setPI((unsigned int) elementStack_.size(), mm_->getPooledString(target),
                          mm_->getPooledString(value));

  if(prevNode_ != (unsigned int)-1)
    getNode(prevNode_)->nextSibling.index = numNodes_;

  prevNode_ = numNodes_;

  ++numNodes_;
}

void FastXDMDocument::textEvent(const XMLCh *value)
{
  textBuffer_.append(value);
  textToCreate_ = true;
}

void FastXDMDocument::textEvent(const XMLCh *chars, unsigned int length)
{
  textBuffer_.append(chars, length);
  textToCreate_ = true;
}

void FastXDMDocument::commentEvent(const XMLCh *value)
{
  checkTextBuffer();

  if(numNodes_ == maxNodes_) resizeNodes();

  nodes_[numNodes_].setComment((unsigned int) elementStack_.size(), mm_->getPooledString(value));

  if(prevNode_ != (unsigned int)-1)
    getNode(prevNode_)->nextSibling.index = numNodes_;

  prevNode_ = numNodes_;

  ++numNodes_;
}

void FastXDMDocument::attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                                     const XMLCh *typeURI, const XMLCh *typeName)
{
  unsigned int owner = (unsigned int)-1;
  if(!elementStack_.empty()) {
    owner = elementStack_.peek();
  }

  if(numAttributes_ == maxAttributes_) resizeAttributes();

  if(typeName) {
    attributes_[numAttributes_].set(owner, mm_->getPooledString(prefix), mm_->getPooledString(uri), mm_->getPooledString(localname),
                                    mm_->getPooledString(value), mm_->getPooledString(typeURI), mm_->getPooledString(typeName));
  }
  else {
    attributes_[numAttributes_].set(owner, mm_->getPooledString(prefix), mm_->getPooledString(uri), mm_->getPooledString(localname),
                                    mm_->getPooledString(value), mm_->getPooledString(SchemaSymbols::fgURI_SCHEMAFORSCHEMA),
                                    mm_->getPooledString(ATUntypedAtomic::fgDT_UNTYPEDATOMIC));
  }

  if(owner != (unsigned int)-1) {
    Node *node = getNode(owner);
    if(node->data.element.attributes.index == (unsigned int)-1)
      node->data.element.attributes.index = numAttributes_;
  }

  ++numAttributes_;
}

void FastXDMDocument::namespaceEvent(const XMLCh *prefix, const XMLCh *uri)
{
  assert(!elementStack_.empty());

  if(numNamespaces_ == maxNamespaces_) resizeNamespaces();

  namespaces_[numNamespaces_].set(elementStack_.peek(), mm_->getPooledString(prefix), mm_->getPooledString(uri));

  Node *node = getNode(elementStack_.peek());
  if(node->data.element.namespaces.index == (unsigned int)-1)
    node->data.element.namespaces.index = numNamespaces_;

  ++numNamespaces_;
}

void FastXDMDocument::resizeNodes()
{
  AutoDeallocate<Node> newMem(mm_, maxNodes_ * 2 * sizeof(Node));
  memcpy(newMem.get(), nodes_, numNodes_ * sizeof(Node));
  nodes_ = newMem.swap(nodes_);
  maxNodes_ *= 2;
}

void FastXDMDocument::resizeAttributes()
{
  AutoDeallocate<Attribute> newMem(mm_, maxAttributes_ * 2 * sizeof(Attribute));
  memcpy(newMem.get(), attributes_, numAttributes_ * sizeof(Attribute));
  attributes_ = newMem.swap(attributes_);
  maxAttributes_ *= 2;
}

void FastXDMDocument::resizeNamespaces()
{
  AutoDeallocate<Namespace> newMem(mm_, maxNamespaces_ * 2 * sizeof(Namespace));
  memcpy(newMem.get(), namespaces_, numNamespaces_ * sizeof(Namespace));
  namespaces_ = newMem.swap(namespaces_);
  maxNamespaces_ *= 2;
}

static void outputInheritedNamespaces(const FastXDMDocument::Node *node, EventHandler *events)
{
  typedef std::set<const XMLCh*, XMLChSort> DoneSet;
  DoneSet done;

  while(node != 0 && node->nodeKind == FastXDMDocument::ELEMENT) {

    if(done.insert(node->data.element.prefix).second && node->data.element.uri != 0) {
      events->namespaceEvent(node->data.element.prefix, node->data.element.uri);
    }

    FastXDMDocument::Attribute *attr = node->data.element.attributes.ptr;
    while(attr != 0 && attr->owner.ptr == node) {
      if(attr->uri != 0 && done.insert(attr->prefix).second) {
        events->namespaceEvent(attr->prefix, attr->uri);
      }
      ++attr;
    }

    FastXDMDocument::Namespace *ns = node->data.element.namespaces.ptr;
    while(ns != 0 && ns->owner.ptr == node) {
      if(done.insert(ns->prefix).second && ns->uri != 0) {
        events->namespaceEvent(ns->prefix, ns->uri);
      }
      ++ns;
    }

    node = FastXDMDocument::getParent(node);
  }
}

static const FastXDMDocument::Node *toEventsImpl(const FastXDMDocument::Node *node, EventHandler *events,
                                                 bool outputNamespaces, bool preserveType, bool inheritedNamespaces)
{
  switch(node->nodeKind) {
  case FastXDMDocument::DOCUMENT: {
    events->startDocumentEvent(node->data.document.documentURI, node->data.document.encoding);
    const FastXDMDocument::Node *child = node + 1;
    while(child->level > node->level) {
      child = toEventsImpl(child, events, outputNamespaces, preserveType, false);
    }
    events->endDocumentEvent();
    return child;
  }
  case FastXDMDocument::ELEMENT: {
    events->startElementEvent(node->data.element.prefix, node->data.element.uri, node->data.element.localname);

    if(node->data.element.attributes.ptr != 0) {
      const FastXDMDocument::Attribute *attr = node->data.element.attributes.ptr;
      while(attr->owner.ptr == node) {
        if(preserveType)
          events->attributeEvent(attr->prefix, attr->uri, attr->localname, attr->value, attr->typeURI, attr->typeName);
        else
          events->attributeEvent(attr->prefix, attr->uri, attr->localname, attr->value,
                                 SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                 ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
        ++attr;
      }
    }

    if(outputNamespaces) {
      if(inheritedNamespaces) {
        outputInheritedNamespaces(node, events);
      }
      else if(node->data.element.namespaces.ptr != 0) {
        const FastXDMDocument::Namespace *ns = node->data.element.namespaces.ptr;
        while(ns->owner.ptr == node) {
          events->namespaceEvent(ns->prefix, ns->uri);
          ++ns;
        }
      }
    }

    const FastXDMDocument::Node *child = node + 1;
    while(child->level > node->level) {
      child = toEventsImpl(child, events, outputNamespaces, preserveType, false);
    }

    if(preserveType)
      events->endElementEvent(node->data.element.prefix, node->data.element.uri, node->data.element.localname,
                              node->data.element.typeURI, node->data.element.typeName);
    else
      events->endElementEvent(node->data.element.prefix, node->data.element.uri, node->data.element.localname,
                              SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                              DocumentCache::g_szUntyped);
    return child;
  }
  case FastXDMDocument::TEXT:
    events->textEvent(node->data.other.value);
    return node + 1;
  case FastXDMDocument::COMMENT:
    events->commentEvent(node->data.other.value);
    return node + 1;
  case FastXDMDocument::PROCESSING_INSTRUCTION:
    events->piEvent(node->data.other.target, node->data.other.value);
    return node + 1;
  case FastXDMDocument::MARKER:
    break;
  }
  return node;
}

void FastXDMDocument::toEvents(const Node *node, EventHandler *events, bool preserveNS, bool preserveType)
{
  toEventsImpl(node, events, preserveNS, preserveType, preserveNS);
}

void FastXDMDocument::toEvents(const Attribute *attr, EventHandler *events, bool preserveType)
{
  if(preserveType)
    events->attributeEvent(attr->prefix, attr->uri, attr->localname, attr->value, attr->typeURI, attr->typeName);
  else
    events->attributeEvent(attr->prefix, attr->uri, attr->localname, attr->value,
                           SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                           ATUntypedAtomic::fgDT_UNTYPEDATOMIC);
}

void FastXDMDocument::toEvents(const Namespace *ns, EventHandler *events)
{
  events->namespaceEvent(ns->prefix, ns->uri);
}
