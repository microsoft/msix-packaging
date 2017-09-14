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
#include <set>

#include <xqilla/update/UTransform.hpp>
#include <xqilla/context/VariableStore.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/ast/XQCopyOf.hpp>
#include <xqilla/context/UpdateFactory.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/update/PendingUpdateList.hpp>

CopyBinding::CopyBinding(XPath2MemoryManager* memMgr, 
                         const XMLCh* variable, 
                         ASTNode* allValues)
  : qname_(memMgr->getPooledString(variable)),
    uri_(0),
    name_(0),
    src_(memMgr),
    expr_(allValues)
{
}

CopyBinding::CopyBinding(XPath2MemoryManager* memMgr, const CopyBinding &o)
  : qname_(o.qname_),
    uri_(o.uri_),
    name_(o.name_),
    src_(o.src_, memMgr),
    expr_(o.expr_)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

UTransform::UTransform(VectorOfCopyBinding* bindings, ASTNode *modifyExpr, ASTNode* returnExpr, XPath2MemoryManager* mm)
  : ASTNodeImpl(UTRANSFORM, mm),
    bindings_(bindings),
    modify_(modifyExpr),
    return_(returnExpr),
    valMode_(DocumentCache::VALIDATION_SKIP)
{
}

UTransform::UTransform(VectorOfCopyBinding* bindings, ASTNode *modifyExpr, ASTNode* returnExpr, DocumentCache::ValidationMode valMode, XPath2MemoryManager* mm)
  : ASTNodeImpl(UTRANSFORM, mm),
    bindings_(bindings),
    modify_(modifyExpr),
    return_(returnExpr),
    valMode_(valMode)
{
}

static const XMLCh err_XUTY0013[] = { 'e', 'r', 'r', ':', 'X', 'U', 'T', 'Y', '0', '0', '1', '3', 0 };

ASTNode *UTransform::staticResolution(StaticContext* context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  VectorOfCopyBinding::iterator end = bindings_->end();
  for(VectorOfCopyBinding::iterator it0 = bindings_->begin(); it0 != end; ++it0) {
    // Work out the uri and localname of the variable binding
    const XMLCh* prefix=XPath2NSUtils::getPrefix((*it0)->qname_, mm);
    if(prefix && *prefix)
      (*it0)->uri_ = context->getUriBoundToPrefix(prefix, this);
    (*it0)->name_ = XPath2NSUtils::getLocalName((*it0)->qname_);

    SequenceType *copyType = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_NODE),
                                                   SequenceType::EXACTLY_ONE);
    copyType->setLocationInfo(this);

    // call static resolution on the value
    (*it0)->expr_ = new (mm) XQCopyOf((*it0)->expr_, /*copyNamespaces*/true, mm);
    (*it0)->expr_->setLocationInfo(this);
    (*it0)->expr_ = new (mm) XQTreatAs((*it0)->expr_, copyType, mm, err_XUTY0013);
    (*it0)->expr_->setLocationInfo(this);
    (*it0)->expr_ = (*it0)->expr_->staticResolution(context);
  }

  // Call staticResolution on the modify expression
  modify_ = modify_->staticResolution(context);

  // Call staticResolution on the return expression
  return_ = return_->staticResolution(context);

  valMode_ = context->getRevalidationMode();

  return this;
}

ASTNode *UTransform::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  // Add all the binding variables to the new scope
  VectorOfCopyBinding::iterator end = bindings_->end();
  for(VectorOfCopyBinding::iterator it0 = bindings_->begin(); it0 != end; ++it0) {
    if((*it0)->expr_->getStaticAnalysis().isUpdating()) {
      XQThrow(StaticErrorException,X("UTransform::staticTyping"),
              X("It is a static error for the copy expression of a transform expression "
                "to be an updating expression [err:XUST0001]"));
    }
  }

  // Call staticTyping on the modify expression
  _src.add(modify_->getStaticAnalysis());
  _src.updating(false);

  if(!modify_->getStaticAnalysis().isUpdating() && !modify_->getStaticAnalysis().isPossiblyUpdating())
    XQThrow(StaticErrorException, X("UTransform::staticTyping"),
            X("The modify expression is not an updating expression [err:XUST0002]"));

  // Call staticResolution on the return expression
  _src.getStaticType() = return_->getStaticAnalysis().getStaticType();
  _src.setProperties(return_->getStaticAnalysis().getProperties());
  _src.add(return_->getStaticAnalysis());

  if(return_->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("UTransform::staticTyping"),
            X("It is a static error for the return expression of a transform expression "
              "to be an updating expression [err:XUST0001]"));
  }

  VectorOfCopyBinding *newBindings = context == 0 ? 0 :
    new (context->getMemoryManager()) VectorOfCopyBinding(XQillaAllocator<CopyBinding*>(context->getMemoryManager()));

  VectorOfCopyBinding::reverse_iterator rend = bindings_->rend();
  for(VectorOfCopyBinding::reverse_iterator it = bindings_->rbegin(); it != rend; ++it) {
    // Remove our binding variable from the StaticAnalysis data (removing it if it's not used)
    if(!_src.removeVariable((*it)->uri_, (*it)->name_)) {
      (*it)->qname_ = 0;
    }

    _src.add((*it)->expr_->getStaticAnalysis());

    // Add the new VB at the front of the new Bindings
    // (If it's a let binding, and it's variable isn't used, don't add it - there's no point)
    if(newBindings && (*it)->qname_) {
      CopyBinding *newVB = new (context->getMemoryManager()) CopyBinding(context->getMemoryManager(), **it);
      newVB->setLocationInfo(*it);
      newBindings->insert(newBindings->begin(), newVB);
    }
  }

  // Overwrite our bindings with the new ones
  if(newBindings)
    bindings_ = newBindings;

  return this;
}

class nodecompare {
public:
  nodecompare(const DynamicContext *context)
    : context_(context) {}

  bool operator()(const Node::Ptr &first, const Node::Ptr &second) const
  {
    return first->uniqueLessThan(second, context_);
  }

private:
  const DynamicContext *context_;
};

typedef std::set<Node::Ptr, nodecompare> NodeSet;

Result UTransform::createResult(DynamicContext* context, int flags) const
{
  context->testInterrupt();

  VarStoreImpl scope(context->getMemoryManager(), context->getVariableStore());
  AutoVariableStoreReset reset(context, &scope);

  NodeSet copiedNodes = NodeSet(nodecompare(context));

  VectorOfCopyBinding::const_iterator end = getBindings()->end();
  for(VectorOfCopyBinding::const_iterator it = getBindings()->begin();
      it != end; ++it) {
    if((*it)->qname_ == 0) continue;
      
    Sequence values = (*it)->expr_->createResult(context)->toSequence(context);

    // Keep a record of the nodes that have been copied
    Result valIt = values;
    Item::Ptr val;
    while((val = valIt->next(context)).notNull()) {
      copiedNodes.insert((Node*)val.get());
    }

    scope.setVar((*it)->uri_, (*it)->name_, values);
  }

  // Get the pending update list
  PendingUpdateList pul = getModifyExpr()->createUpdateList(context);

  // Check that the targets of the pending updates are copied nodes
  for(PendingUpdateList::const_iterator i = pul.begin(); i != pul.end(); ++i) {
    Node::Ptr target = i->getTarget();
    while(copiedNodes.find(target) == copiedNodes.end()) {
      target = target->dmParent(context);
      if(target.isNull()) {
        XQThrow3(StaticErrorException,X("UTransform::staticTyping"),
                 X("The target node of an update expression in the transform expression is not a node from the copy clauses [err:XUDY0014]"), &(*i));
      }
    }
  }

  // Apply the updates
  AutoDelete<UpdateFactory> ufactory(context->createUpdateFactory());
  ufactory->applyUpdates(pul, context, getRevalidationMode());

  // Execute the return expression
  return getReturnExpr()->createResult(context);
}
