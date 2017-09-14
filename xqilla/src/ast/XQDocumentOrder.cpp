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

#include <xqilla/ast/XQDocumentOrder.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/runtime/Sequence.hpp>

XQDocumentOrder::XQDocumentOrder(ASTNode* expr, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(DOCUMENT_ORDER, memMgr),
    expr_(expr),
    unordered_(false)
{
}

XQDocumentOrder::XQDocumentOrder(ASTNode* expr, bool unordered, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(DOCUMENT_ORDER, memMgr),
    expr_(expr),
    unordered_(unordered)
{
}

ASTNode* XQDocumentOrder::staticResolution(StaticContext *context)
{
  unordered_ = context->getNodeSetOrdering() == StaticContext::ORDERING_UNORDERED;

  expr_ = expr_->staticResolution(context);

  return this;
}

ASTNode *XQDocumentOrder::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(expr_->getStaticAnalysis());

  _src.getStaticType() = expr_->getStaticAnalysis().getStaticType();
  if(_src.getStaticType().getMin() > 0)
    _src.getStaticType().setCardinality(1, _src.getStaticType().getMax());

  // Check if nodes will be returned
  if(!_src.getStaticType().containsType(StaticType::NODE_TYPE)) {
    return substitute(expr_);
  }

  // Check if it's already in document order
  if((expr_->getStaticAnalysis().getProperties() & StaticAnalysis::DOCORDER) != 0) {
    return substitute(expr_);
  }

  _src.setProperties(expr_->getStaticAnalysis().getProperties()
                     | StaticAnalysis::DOCORDER
                     | StaticAnalysis::GROUPED);

  return this;
}

Result XQDocumentOrder::createResult(DynamicContext* context, int flags) const
{
  if(unordered_) {
    return new UniqueNodesResult(this, expr_->createResult(context, flags), context);
  }
  else {
    return new DocumentOrderResult(this, expr_->createResult(context, flags));
  }
}

Item::Ptr DocumentOrderResult::nextOrTail(Result &tail, DynamicContext *context)
{
  Sequence seq = parent_->toSequence(context);
  seq.sortIntoDocumentOrder(context);
  tail = seq;
  return 0;
}

Item::Ptr UniqueNodesResult::next(DynamicContext *context)
{
  Item::Ptr result = parent_->next(context);

  if(nTypeOfItemsInLastStep_ == 0 && result.notNull())
	  nTypeOfItemsInLastStep_ = result->isNode() ? 1 : 2;

  if(nTypeOfItemsInLastStep_ == 1) {
	  while(result.notNull()) {
		  if(noDups_.insert(result).second) break;
		  else result = parent_->next(context);
	  }
  }

  return result;
}
