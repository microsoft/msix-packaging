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

#include <xqilla/ast/XQTypeswitch.hpp>
#include <xqilla/context/VariableStore.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/ast/ContextTuple.hpp>
#include <xqilla/ast/LetTuple.hpp>
#include <xqilla/ast/XQReturn.hpp>
#include <xqilla/runtime/ResultBuffer.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/update/PendingUpdateList.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

XQTypeswitch::  XQTypeswitch(ASTNode *expr, Cases *cases, Case *defaultCase, XPath2MemoryManager *mm)
  : ASTNodeImpl(TYPESWITCH, mm),
    expr_(expr),
    cases_(cases),
    default_(defaultCase)
{
}

ASTNode* XQTypeswitch::staticResolution(StaticContext *context)
{
  // Statically resolve the test expression
  expr_ = expr_->staticResolution(context);

  // Call static resolution on the clauses
  for(Cases::iterator it = cases_->begin(); it != cases_->end(); ++it) {
    (*it)->staticResolution(context);
  }

  default_->staticResolution(context);

  return this;
}

static const XMLCh no_err[] = { 0 };

ASTNode *XQTypeswitch::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  // Statically resolve the test expression
  const StaticAnalysis &exprSrc = expr_->getStaticAnalysis();

  if(exprSrc.isUpdating()) {
    XQThrow(StaticErrorException,X("XQTypeswitch::staticTyping"),
            X("It is a static error for the operand expression of a typeswitch expression "
              "to be an updating expression [err:XUST0001]"));
  }

  // Call static resolution on the clauses
  bool possiblyUpdating = true;
  _src.add(exprSrc);

  default_->staticTyping(expr_->getStaticAnalysis(), context, _src, possiblyUpdating, /*first*/true);

  Cases::iterator it;
  for(it = cases_->begin(); it != cases_->end(); ++it) {
    (*it)->staticTyping(expr_->getStaticAnalysis(), context, _src, possiblyUpdating, /*first*/false);
  }

  if(!context) {
    return this;
  }

  XPath2MemoryManager *mm = context->getMemoryManager();

  if(exprSrc.isUsed()) {
    // Do basic static type checking on the clauses,
    // to eliminate ones which will never be matches,
    // and find ones which will always be matched.
    const StaticType &sType = expr_->getStaticAnalysis().getStaticType();

    bool found = false;
    Cases newCases = Cases(XQillaAllocator<Case*>(mm));
    for(it = cases_->begin(); it != cases_->end(); ++it) {
      StaticType::TypeMatch match = (*it)->getTreatType().matches(sType);
      if(found || match.type == StaticType::NEVER || match.cardinality == StaticType::NEVER) {
        // It never matches
        (*it)->getExpression()->release();
        mm->deallocate(*it);
      }
      else if((*it)->getIsExact() && match.type == StaticType::ALWAYS &&
         match.cardinality == StaticType::ALWAYS) {
        // It always matches, so set this clause as the
        // default clause and remove all clauses after it
        default_->getExpression()->release();
        mm->deallocate(default_);
        default_ = *it;
        found = true;
      }
      else {
        newCases.push_back(*it);
      }
    }

    if(newCases.size() == cases_->size()) {
      // No change
      return this;
    }

    *cases_ = newCases;
  }
  else {
    // If it's constant, we can narrow it down to the correct clause
    AutoDelete<DynamicContext> dContext(context->createDynamicContext());
    dContext->setMemoryManager(mm);

    Sequence value;
    Case *match = (Case*)chooseCase(dContext, value);    

    // Replace the default with the matched clause and
    // remove the remaining clauses, as they don't match
    for(it = cases_->begin(); it != cases_->end(); ++it) {
      if((*it) != match) {
        (*it)->getExpression()->release();
        mm->deallocate(*it);
      }
    }
    if(default_ != match) {
      default_->getExpression()->release();
      mm->deallocate(default_);
    }

    default_ = match;
    cases_->clear();
  }

  // Call static resolution on the new clauses
  possiblyUpdating = true;
  _src.clear();
  _src.add(exprSrc);

  default_->staticTyping(expr_->getStaticAnalysis(), context, _src, possiblyUpdating, /*first*/true);

  for(it = cases_->begin(); it != cases_->end(); ++it) {
    (*it)->staticTyping(expr_->getStaticAnalysis(), context, _src, possiblyUpdating, /*first*/false);
  }

  if(cases_->empty()) {
    if(default_->isVariableUsed()) {
      TupleNode *tuple = new (mm) ContextTuple(mm);
      tuple->setLocationInfo(this);
      tuple = new (mm) LetTuple(tuple, default_->getURI(), default_->getName(), expr_, mm);
      tuple->setLocationInfo(this);
      ASTNode *result = new (mm) XQReturn(tuple, default_->getExpression(), mm);
      result->setLocationInfo(this);
      const_cast<StaticAnalysis&>(result->getStaticAnalysis()).copy(_src);
      expr_ = 0;
      default_->setExpression(0);
      this->release();
      return result;
    }
    else if(!expr_->getStaticAnalysis().isNoFoldingForced()) {
      ASTNode *result = default_->getExpression();
      default_->setExpression(0);
      this->release();
      return result;
    }
  }

  return this;
}

XQTypeswitch::Case::Case(const XMLCh *qname, SequenceType *seqType, ASTNode *expr)
  : qname_(qname),
    uri_(0),
    name_(0),
    seqType_(seqType),
    isExact_(false),
    expr_(expr)
{
}

XQTypeswitch::Case::Case(const XMLCh *qname, const XMLCh *uri, const XMLCh *name, SequenceType *seqType,
                         const StaticType &treatType, bool isExact, ASTNode *expr)
  : qname_(qname),
    uri_(uri),
    name_(name),
    seqType_(seqType),
    treatType_(treatType),
    isExact_(isExact),
    expr_(expr)
{
}

void XQTypeswitch::Case::staticResolution(StaticContext* context)
{
  if(seqType_) {
    seqType_->staticResolution(context);
    seqType_->getStaticType(treatType_, context, isExact_, this);
  }
  expr_ = expr_->staticResolution(context);

  if(qname_ != 0) {
    uri_ = context->getUriBoundToPrefix(XPath2NSUtils::getPrefix(qname_, context->getMemoryManager()), this);
    name_ = XPath2NSUtils::getLocalName(qname_);
  }
}

void XQTypeswitch::Case::staticTyping(const StaticAnalysis &var_src, StaticContext* context,
                                      StaticAnalysis &src, bool &possiblyUpdating, bool first)
{
  if(seqType_ != 0) {
    if(src.isUpdating()) {
      if(!expr_->getStaticAnalysis().isUpdating() && !expr_->getStaticAnalysis().isPossiblyUpdating())
        XQThrow(StaticErrorException, X("XQTypeswitch::Case::staticTyping"),
                X("Mixed updating and non-updating operands [err:XUST0001]"));
    }
    else {
      if(expr_->getStaticAnalysis().isUpdating() && !possiblyUpdating)
        XQThrow(StaticErrorException, X("XQTypeswitch::Case::staticTyping"),
                X("Mixed updating and non-updating operands [err:XUST0001]"));
    }
  }

  if(qname_ != 0) {
    // Remove the local variable from the StaticAnalysis
    if(!expr_->getStaticAnalysis().isVariableUsed(uri_, name_)) {
      // If the variable isn't used, don't bother setting it when we execute
      qname_ = 0;
    }
  }

  if(possiblyUpdating)
    possiblyUpdating = expr_->getStaticAnalysis().isPossiblyUpdating();

  if(first) {
    src.getStaticType() = expr_->getStaticAnalysis().getStaticType();
    src.setProperties(expr_->getStaticAnalysis().getProperties());
  } else {
    src.getStaticType() |= expr_->getStaticAnalysis().getStaticType();
    src.setProperties(src.getProperties() & expr_->getStaticAnalysis().getProperties());
  }
  if(qname_ != 0) {
    src.addExceptVariable(uri_, name_, expr_->getStaticAnalysis());
  }
  else {
    src.add(expr_->getStaticAnalysis());
  }
}

const XQTypeswitch::Case *XQTypeswitch::chooseCase(DynamicContext *context, Sequence &resultSeq) const
{
  // retrieve the value of the operand expression
//   ResultBuffer value(expr_->createResult(context));
  Sequence value = expr_->createResult(context)->toSequence(context);

  const Case *cse = 0;

  // find the effective case
  for(Cases::const_iterator it = cases_->begin(); it != cases_->end(); ++it) {
    try {
//       (*it)->getSequenceType()->matches(value.createResult(), (*it)->getSequenceType(), no_err)->toSequence(context);
      (*it)->getSequenceType()->matches(value, (*it)->getSequenceType(), no_err)->toSequence(context);
      cse = *it;
      break;
    }
    catch(const XPath2TypeMatchException &ex) {
      // Well, it doesn't match that one then...
    }
  }

  // if no case is satisfied, use the default one
  if(cse == 0) {
      cse = default_;
  }

  // Bind the variable
  if(cse->isVariableUsed()) {
//     resultSeq = value.createResult()->toSequence(context);
    resultSeq = value;

//     varStore->declareVar(cse->getURI(), cse->getName(), value.createResult()->toSequence(context), context);
  }

  return cse;
}

EventGenerator::Ptr XQTypeswitch::generateEvents(EventHandler *events, DynamicContext *context,
                                            bool preserveNS, bool preserveType) const
{
  SingleVarStore scope;
  const Case *cse = chooseCase(context, scope.value);

  AutoVariableStoreReset reset(context);
  if(cse->isVariableUsed())
    scope.setAsVariableStore(cse->getURI(), cse->getName(), context);

  return new ClosureEventGenerator(cse->getExpression(), context, preserveNS, preserveType);
}

PendingUpdateList XQTypeswitch::createUpdateList(DynamicContext *context) const
{
  SingleVarStore scope;
  const Case *cse = chooseCase(context, scope.value);

  AutoVariableStoreReset reset(context);
  if(cse->isVariableUsed())
    scope.setAsVariableStore(cse->getURI(), cse->getName(), context);

  return cse->getExpression()->createUpdateList(context);
}

Result XQTypeswitch::createResult(DynamicContext *context, int flags) const
{
  SingleVarStore scope;
  const Case *cse = chooseCase(context, scope.value);

  AutoVariableStoreReset reset(context);
  if(cse->isVariableUsed())
    scope.setAsVariableStore(cse->getURI(), cse->getName(), context);

  return cse->getExpression()->createResult(context);
}

