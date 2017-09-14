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
#include <assert.h>
#include <sstream>

#include <xqilla/ast/XQCopy.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/events/SequenceBuilder.hpp>
#include <xqilla/ast/XQContextItem.hpp>
#include <xqilla/ast/XQElementConstructor.hpp>
#include <xqilla/ast/XQDocumentConstructor.hpp>
#include <xqilla/runtime/Result.hpp>
#include <xqilla/runtime/Sequence.hpp>

#include "../events/NoInheritFilter.hpp"

#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_USE
using namespace std;

XQCopy::XQCopy(XPath2MemoryManager* memMgr)
  : ASTNodeImpl(COPY, memMgr),
    expr_(0),
    children_(XQillaAllocator<ASTNode*>(memMgr)),
    copyNamespaces_(true),
    inheritNamespaces_(true)
{
}

XQCopy::XQCopy(ASTNode *expr, const VectorOfASTNodes &children, bool copyNamespaces, bool inheritNamespaces, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(COPY, memMgr),
    expr_(expr),
    children_(XQillaAllocator<ASTNode*>(memMgr)),
    copyNamespaces_(copyNamespaces),
    inheritNamespaces_(inheritNamespaces)
{
  children_ = children;
}

ASTNode* XQCopy::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  expr_ = expr_->staticResolution(context);

  unsigned int i;
  for(i = 0; i < children_.size(); ++i) {
    children_[i] = new (mm) XQContentSequence(children_[i], mm);
    children_[i]->setLocationInfo(this);

    children_[i] = children_[i]->staticResolution(context);
  }

  return this;
}

ASTNode *XQCopy::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.copy(expr_->getStaticAnalysis());

  unsigned int i;
  for(i = 0; i < children_.size(); ++i) {
    _src.add(children_[i]->getStaticAnalysis());
  }

  if(!_src.getStaticType().containsType(StaticType::NODE_TYPE)) {
    return expr_;
  }

  return this;
}

Result XQCopy::createResult(DynamicContext* context, int flags) const
{
  Item::Ptr toBeCopied = getExpression()->createResult(context)->next(context);

  if(!toBeCopied->isNode())
    return toBeCopied;

  AutoDelete<SequenceBuilder> builder(context->createSequenceBuilder());
  EventGenerator::generateAndTailCall(generateEventsImpl(toBeCopied, builder.get(), context, true, true),
                                      builder.get(), context);
  builder->endEvent();
  return builder->getSequence();
}

EventGenerator::Ptr XQCopy::generateEvents(EventHandler *events, DynamicContext *context,
                                           bool preserveNS, bool preserveType) const
{
  return generateEventsImpl(expr_->createResult(context)->next(context), events,
                            context, preserveNS, preserveType);
}

EventGenerator::Ptr XQCopy::generateEventsImpl(const Item::Ptr &toBeCopied, EventHandler *events, DynamicContext *context,
                                               bool preserveNS, bool preserveType) const
{
  if(!toBeCopied->isNode()) {
    toBeCopied->generateEvents(events, context, preserveNS, preserveType);
    return 0;
  }

  Node *node = (Node*)toBeCopied.get();

  if(node->dmNodeKind() == Node::element_string) {
    NoInheritFilter niFilter(events, context->getMemoryManager());
    if(!inheritNamespaces_) events = &niFilter;

    AnyAtomicType::Ptr itemName = node->dmNodeName(context);
    const ATQNameOrDerived *pQName = (const ATQNameOrDerived*)itemName.get();
    const XMLCh *prefix = emptyToNull(pQName->getPrefix());
    const XMLCh *uri = emptyToNull(pQName->getURI());
    const XMLCh *localname = pQName->getName();

    events->startElementEvent(prefix, uri, localname);

    ElemConstructFilter elemFilter(events, this, context->getMemoryManager());

    if(copyNamespaces_) {
      Result nsnodes = node->dmNamespaceNodes(context, this);
      Item::Ptr ns;
      while((ns = nsnodes->next(context)).notNull()) {
        ns->generateEvents(&elemFilter, context, preserveNS, preserveType);
      }
    }

    for(VectorOfASTNodes::const_iterator itCont = children_.begin(); itCont != children_.end (); ++itCont) {
      (*itCont)->generateAndTailCall(&elemFilter, context, preserveNS, preserveType);
    }

    // TBD validation and type - jpcs
    const XMLCh *typeURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
    const XMLCh *typeName = DocumentCache::g_szUntyped;

    events->endElementEvent(prefix, uri, localname, typeURI, typeName);
  }
  else if(node->dmNodeKind() == Node::document_string) {
    events->startDocumentEvent(0, 0);

    DocConstructFilter filter(events, this);

    for(VectorOfASTNodes::const_iterator itCont = children_.begin(); itCont != children_.end (); ++itCont) {
      (*itCont)->generateAndTailCall(&filter, context, preserveNS, preserveType);
    }

    events->endDocumentEvent();
  }
  else {
    node->generateEvents(events, context, preserveNS, preserveType);
  }

  return 0;
}

