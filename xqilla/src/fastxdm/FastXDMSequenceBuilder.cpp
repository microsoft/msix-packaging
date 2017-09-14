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

#include "FastXDMSequenceBuilder.hpp"
#include "FastXDMNodeImpl.hpp"

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>

FastXDMSequenceBuilder::FastXDMSequenceBuilder(const DynamicContext *context)
  : context_(context),
    level_(0),
    document_(0),
    seq_(context->getMemoryManager())
{
}

void FastXDMSequenceBuilder::startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding)
{
  if(level_ == 0) {
    document_ = new FastXDMDocument(context_->getMemoryManager());
  }

  document_->startDocumentEvent(documentURI, encoding);

  ++level_;
}

void FastXDMSequenceBuilder::endDocumentEvent()
{
  --level_;

  document_->endDocumentEvent();

  if(level_ == 0) {
    document_->endEvent();
    seq_.addItem(new FastXDMNodeImpl(document_, document_->getNode(0)));
    document_ = 0;
  }
}

void FastXDMSequenceBuilder::endEvent()
{
}

void FastXDMSequenceBuilder::startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
{
  if(level_ == 0) {
    document_ = new FastXDMDocument(context_->getMemoryManager());
  }

  document_->startElementEvent(prefix, uri, localname);

  ++level_;
}

void FastXDMSequenceBuilder::endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                                         const XMLCh *typeURI, const XMLCh *typeName)
{
  --level_;

  document_->endElementEvent(prefix, uri, localname, typeURI, typeName);

  if(level_ == 0) {
    document_->endEvent();
    seq_.addItem(new FastXDMNodeImpl(document_, document_->getNode(0)));
    document_ = 0;
  }
}

void FastXDMSequenceBuilder::piEvent(const XMLCh *target, const XMLCh *value)
{
  if(level_ == 0) {
    document_ = new FastXDMDocument(1, 0, 0, context_->getMemoryManager());
  }

  document_->piEvent(target, value);

  if(level_ == 0) {
    document_->endEvent();
    seq_.addItem(new FastXDMNodeImpl(document_, document_->getNode(0)));
    document_ = 0;
  }
}

void FastXDMSequenceBuilder::textEvent(const XMLCh *value)
{
  if(level_ == 0) {
    document_ = new FastXDMDocument(1, 0, 0, context_->getMemoryManager());
  }

  document_->textEvent(value);

  if(level_ == 0) {
    document_->endEvent();
    seq_.addItem(new FastXDMNodeImpl(document_, document_->getNode(0)));
    document_ = 0;
  }
}

void FastXDMSequenceBuilder::textEvent(const XMLCh *chars, unsigned int length)
{
  if(level_ == 0) {
    document_ = new FastXDMDocument(1, 0, 0, context_->getMemoryManager());
  }

  document_->textEvent(chars, length);

  if(level_ == 0) {
    document_->endEvent();
    seq_.addItem(new FastXDMNodeImpl(document_, document_->getNode(0)));
    document_ = 0;
  }
}

void FastXDMSequenceBuilder::commentEvent(const XMLCh *value)
{
  if(level_ == 0) {
    document_ = new FastXDMDocument(1, 0, 0, context_->getMemoryManager());
  }

  document_->commentEvent(value);

  if(level_ == 0) {
    document_->endEvent();
    seq_.addItem(new FastXDMNodeImpl(document_, document_->getNode(0)));
    document_ = 0;
  }
}

void FastXDMSequenceBuilder::attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                                        const XMLCh *typeURI, const XMLCh *typeName)
{
  if(level_ == 0) {
    document_ = new FastXDMDocument(0, 1, 0, context_->getMemoryManager());
  }

  document_->attributeEvent(prefix, uri, localname, value, typeURI, typeName);

  if(level_ == 0) {
    document_->endEvent();
    seq_.addItem(new FastXDMAttributeNodeImpl(document_, document_->getAttribute(0)));
    document_ = 0;
  }
}

void FastXDMSequenceBuilder::namespaceEvent(const XMLCh *prefix, const XMLCh *uri)
{
  if(level_ == 0) {
    seq_.addItem(new FastXDMNamespaceNodeImpl(0, prefix, uri, 0));
  } else {
    document_->namespaceEvent(prefix, uri);
  }
}

void FastXDMSequenceBuilder::atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value, const XMLCh *typeURI,
                                             const XMLCh *typeName)
{
  assert(level_ == 0);

  seq_.addItem(context_->getItemFactory()->createDerivedFromAtomicType(type, typeURI, typeName, value, context_));
}

