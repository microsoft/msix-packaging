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

#include <xqilla/ast/XQMap.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/runtime/SequenceResult.hpp>
#include <xqilla/ast/XQNav.hpp>
#include <xqilla/ast/XQPredicate.hpp>
#include <xqilla/ast/XQVariable.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

XERCES_CPP_NAMESPACE_USE

XQMap::XQMap(ASTNode *arg1, ASTNode *arg2, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(MAP, memMgr),
    arg1_(arg1),
    arg2_(arg2),
    uri_(0),
    name_(0),
    varSrc_(memMgr)
{
}

XQMap::XQMap(ASTNode *arg1, ASTNode *arg2, const XMLCh *uri, const XMLCh *name, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(MAP, memMgr),
    arg1_(arg1),
    arg2_(arg2),
    uri_(uri),
    name_(name),
    varSrc_(memMgr)
{
}

ASTNode *XQMap::staticResolution(StaticContext *context)
{
  arg1_ = arg1_->staticResolution(context);
  arg2_ = arg2_->staticResolution(context);
  return this;
}

ASTNode *XQMap::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  const StaticAnalysis &arg2Src = arg2_->getStaticAnalysis();
  if(name_ == 0) {
    _src.addExceptContextFlags(arg2Src);
  } else {
    _src.addExceptVariable(uri_, name_, arg2Src);
  }

  const StaticAnalysis &arg1Src = arg1_->getStaticAnalysis();
  _src.add(arg1Src);

  _src.getStaticType() = arg2Src.getStaticType();
  _src.getStaticType().multiply(arg1Src.getStaticType().getMin(), arg1Src.getStaticType().getMax());

  _src.setProperties(XQNav::combineProperties(arg1Src.getProperties(), arg2Src.getProperties()));

  if(name_ == 0) {
    if(arg2_->getType() == CONTEXT_ITEM)
      return arg1_;
  } else {
    if(arg2_->getType() == VARIABLE &&
       XPath2Utils::equals(((XQVariable*)arg2_)->getURI(), uri_) &&
       XPath2Utils::equals(((XQVariable*)arg2_)->getName(), name_))
      return arg1_;
  }

  return this;
}

Result XQMap::createResult(DynamicContext* context, int flags) const
{
  Result result = arg1_->createResult(context);

  if(name_ != 0) {
    return ClosureResult::create(arg2_->getStaticAnalysis(), context,
      new MapResult(result, arg2_, uri_, name_));
  }

  if(arg2_->getStaticAnalysis().isContextSizeUsed()) {
    // We need the context size, so convert to a Sequence to work it out
    Sequence seq(result->toSequence(context));
    result =  ClosureResult::create(arg2_->getStaticAnalysis(), context,
      new NavStepResult(new SequenceResult(this, seq), arg2_, seq.getLength()));
  } else {
    result = ClosureResult::create(arg2_->getStaticAnalysis(), context,
      new NavStepResult(result, arg2_, 0));
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MapResult::MapResult(const Result &parent, const ASTNode *step, const XMLCh *uri, const XMLCh *name)
  : ResultImpl(step),
    parent_(parent),
    step_(step),
    uri_(uri),
    name_(name),
    stepResult_(0),
    item_(0),
    scope_(0)
{
}

Result MapResult::getVar(const XMLCh *namespaceURI, const XMLCh *name) const
{
  if(XPath2Utils::equals(name, name_) && XPath2Utils::equals(namespaceURI, uri_))
    return item_;

  return scope_->getVar(namespaceURI, name);
}

void MapResult::getInScopeVariables(std::vector<std::pair<const XMLCh*, const XMLCh*> > &variables) const
{
  variables.push_back(std::pair<const XMLCh*, const XMLCh*>(uri_, name_));
  scope_->getInScopeVariables(variables);
}

Item::Ptr MapResult::next(DynamicContext *context)
{
  if(scope_ == 0) {
    scope_ = context->getVariableStore();
  }

  AutoVariableStoreReset reset(context, this);

  Item::Ptr result;
  while((result = stepResult_->next(context)).isNull()) {
    context->testInterrupt();

    context->setVariableStore(scope_);
    item_ = parent_->next(context);
    if(item_.isNull()) {
      parent_ = 0;
      return 0;
    }

    context->setVariableStore(this);
    stepResult_ = step_->createResult(context);
  }

  return result;
}

