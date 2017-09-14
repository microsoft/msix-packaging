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

#include <xqilla/ast/CountTuple.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/ItemFactory.hpp>

CountTuple::CountTuple(TupleNode *parent, const XMLCh *varQName, XPath2MemoryManager *mm)
  : TupleNode(COUNT, parent, mm),
    varQName_(varQName),
    varURI_(0),
    varName_(0),
    varSrc_(mm)
{
}

CountTuple::CountTuple(TupleNode *parent, const XMLCh *varURI, const XMLCh *varName, XPath2MemoryManager *mm)
  : TupleNode(COUNT, parent, mm),
    varQName_(0),
    varURI_(varURI),
    varName_(varName),
    varSrc_(mm)
{
}

TupleNode *CountTuple::staticResolution(StaticContext *context)
{
  parent_ = parent_->staticResolution(context);

  if(varName_ == 0) {
    varURI_ = context->getUriBoundToPrefix(XPath2NSUtils::getPrefix(varQName_, context->getMemoryManager()), this);
    varName_ = XPath2NSUtils::getLocalName(varQName_);
  }

  return this;
}

static bool canPushCountBack(TupleNode *ancestor)
{
  switch(ancestor->getType()) {
  case TupleNode::COUNT: {
    return canPushCountBack(ancestor->getParent());
  }
  case TupleNode::ORDER_BY:
  case TupleNode::WHERE:
  case TupleNode::FOR:
  case TupleNode::CONTEXT_TUPLE:
    break;
  case TupleNode::LET:
  case TupleNode::DEBUG_HOOK:
    return true;
  }
  return false;
}

TupleNode *CountTuple::staticTypingImpl(StaticContext *context)
{
  // Push back if possible
  if(canPushCountBack(parent_)) {
    // Swap parent_ and this LetTuple, re-executing their staticTypingImpl() methods
    TupleNode *tmp = parent_;
    parent_ = tmp->getParent();
    tmp->setParent(this->staticTypingImpl(context));
    return tmp->staticTypingImpl(context);
  }

  min_ = parent_->getMin();
  max_ = parent_->getMax();

  return this;
}

TupleNode *CountTuple::staticTypingTeardown(StaticContext *context, StaticAnalysis &usedSrc)
{
  // Remove our binding variable from the StaticAnalysis data (removing it if it's not used)
  if(varName_ && !usedSrc.removeVariable(varURI_, varName_)) {
    varURI_ = 0;
    varName_ = 0;
  }

  parent_ = parent_->staticTypingTeardown(context, usedSrc);

  if(varName_ == 0)
    return parent_;

  return this;
}

class CountTupleResult : public TupleResult
{
public:
  CountTupleResult(const CountTuple *ast, const TupleResult::Ptr &parent)
    : TupleResult(ast),
      ast_(ast),
      parent_(parent),
      position_(0)      
  {
  }

  virtual Result getVar(const XMLCh *namespaceURI, const XMLCh *name) const
  {
    if(XPath2Utils::equals(name, ast_->getVarName()) && XPath2Utils::equals(namespaceURI, ast_->getVarURI()))
      return posItem_;

    return parent_->getVar(namespaceURI, name);
  }

  virtual void getInScopeVariables(std::vector<std::pair<const XMLCh*, const XMLCh*> > &variables) const
  {
    variables.push_back(std::pair<const XMLCh*, const XMLCh*>(ast_->getVarURI(), ast_->getVarName()));

    parent_->getInScopeVariables(variables);
  }

  virtual bool next(DynamicContext *context)
  {
    context->testInterrupt();

    if(!parent_->next(context))
      return false;

    posItem_ = context->getItemFactory()->createInteger(++position_, context);
    return true;
  }

private:
  const CountTuple *ast_;
  TupleResult::Ptr parent_;
  Item::Ptr posItem_;
  int position_;
};

TupleResult::Ptr CountTuple::createResult(DynamicContext* context) const
{
  return new CountTupleResult(this, parent_->createResult(context));
}

