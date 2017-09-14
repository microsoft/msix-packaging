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

#include <xqilla/ast/ForTuple.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/utils/XStr.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

XERCES_CPP_NAMESPACE_USE;

ForTuple::ForTuple(TupleNode *parent, const XMLCh *varQName, const XMLCh *posQName,
                   ASTNode *expr, XPath2MemoryManager *mm)
  : TupleNode(FOR, parent, mm),
    varQName_(varQName),
    varURI_(0),
    varName_(0),
    posQName_(posQName),
    posURI_(0),
    posName_(0),
    varSrc_(mm),
    posSrc_(mm),
    expr_(expr)
{
}

ForTuple::ForTuple(TupleNode *parent, const XMLCh *varURI, const XMLCh *varName,
                   const XMLCh *posURI, const XMLCh *posName, ASTNode *expr, XPath2MemoryManager *mm)
  : TupleNode(FOR, parent, mm),
    varQName_(0),
    varURI_(varURI),
    varName_(varName),
    posQName_(0),
    posURI_(posURI),
    posName_(posName),
    varSrc_(mm),
    posSrc_(mm),
    expr_(expr)
{
}

TupleNode *ForTuple::staticResolution(StaticContext *context)
{
  parent_ = parent_->staticResolution(context);

  varURI_ = context->getUriBoundToPrefix(XPath2NSUtils::getPrefix(varQName_, context->getMemoryManager()), this);
  varName_ = XPath2NSUtils::getLocalName(varQName_);

  if(posQName_ && *posQName_) {
    posURI_ = context->getUriBoundToPrefix(XPath2NSUtils::getPrefix(posQName_, context->getMemoryManager()), this);
    posName_ = XPath2NSUtils::getLocalName(posQName_);

    if(XPath2Utils::equals(posName_, varName_) && XPath2Utils::equals(posURI_, varURI_)) {
      XMLBuffer errMsg;
      errMsg.set(X("The positional variable with name {"));
      errMsg.append(posURI_);
      errMsg.append(X("}"));
      errMsg.append(posName_);
      errMsg.append(X(" conflicts with the iteration variable [err:XQST0089]"));
      XQThrow(StaticErrorException,X("ForTuple::staticResolution"), errMsg.getRawBuffer());
    }
  }

  expr_ = expr_->staticResolution(context);

  return this;
}

TupleNode *ForTuple::staticTypingImpl(StaticContext *context)
{
  if(expr_->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("ForTuple::staticTypingSetup"),
            X("It is a static error for the for expression of a FLWOR expression "
              "to be an updating expression [err:XUST0001]"));
  }

  const StaticType &sType = expr_->getStaticAnalysis().getStaticType();

  min_ = parent_->getMin() * sType.getMin();
  if(parent_->getMax() == StaticType::UNLIMITED || sType.getMax() == StaticType::UNLIMITED)
    max_ = StaticType::UNLIMITED;
  else max_ = parent_->getMax() * sType.getMax();

  return this;
}

TupleNode *ForTuple::staticTypingTeardown(StaticContext *context, StaticAnalysis &usedSrc)
{
  // Remove our binding variable from the StaticAnalysis data (removing it if it's not used)
  if(varName_ && !usedSrc.removeVariable(varURI_, varName_)) {
    varURI_ = 0;
    varName_ = 0;
  }

  // Remove our positional variable from the StaticAnalysis data (removing it if it's not used)
  if(posName_ && !usedSrc.removeVariable(posURI_, posName_)) {
    posURI_ = 0;
    posName_ = 0;
  }

  usedSrc.add(expr_->getStaticAnalysis());
  parent_ = parent_->staticTypingTeardown(context, usedSrc);

  const StaticType &sType = expr_->getStaticAnalysis().getStaticType();
  if(varName_ == 0 && posName_ == 0 && sType.getMin() == 1 && sType.getMax() == 1)
    return parent_;

  return this;
}

class ForTupleResult : public TupleResult
{
public:
  ForTupleResult(const ForTuple *ast, const TupleResult::Ptr &parent)
    : TupleResult(ast),
      ast_(ast),
      parent_(parent),
      values_(0),
      position_(0)
  {
  }

  virtual Result getVar(const XMLCh *namespaceURI, const XMLCh *name) const
  {
    if(XPath2Utils::equals(name, ast_->getVarName()) && XPath2Utils::equals(namespaceURI, ast_->getVarURI()))
      return item_;

    if(XPath2Utils::equals(name, ast_->getPosName()) && XPath2Utils::equals(namespaceURI, ast_->getPosURI()))
      return posItem_;

    return parent_->getVar(namespaceURI, name);
  }

  virtual void getInScopeVariables(std::vector<std::pair<const XMLCh*, const XMLCh*> > &variables) const
  {
    variables.push_back(std::pair<const XMLCh*, const XMLCh*>(ast_->getVarURI(), ast_->getVarName()));
    if(ast_->getPosName())
      variables.push_back(std::pair<const XMLCh*, const XMLCh*>(ast_->getPosURI(), ast_->getPosName()));

    parent_->getInScopeVariables(variables);
  }

  virtual bool next(DynamicContext *context)
  {
    AutoVariableStoreReset reset(context, parent_);
    while((item_ = values_->next(context)).isNull()) {
      context->testInterrupt();

      reset.reset();
      if(!parent_->next(context)) return false;

      context->setVariableStore(parent_);
      values_ = ast_->getExpression()->createResult(context);
      position_ = 0;
    }

    if(ast_->getPosName())
      posItem_ = context->getItemFactory()->createInteger(++position_, context);
    return true;
  }

private:
  const ForTuple *ast_;
  TupleResult::Ptr parent_;
  Result values_;
  Item::Ptr item_;
  Item::Ptr posItem_;
  int position_;
};

TupleResult::Ptr ForTuple::createResult(DynamicContext* context) const
{
  return new ForTupleResult(this, parent_->createResult(context));
}

