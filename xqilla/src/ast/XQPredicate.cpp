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

#include <xqilla/ast/XQPredicate.hpp>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/ast/XQEffectiveBooleanValue.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/runtime/SequenceResult.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

XQPredicate::XQPredicate(ASTNode *predicate, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(PREDICATE, memMgr),
    expr_(0),
    predicate_(predicate),
    reverse_(false)
{
}

XQPredicate::XQPredicate(ASTNode* expr, ASTNode *predicate, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(PREDICATE, memMgr),
    expr_(expr),
    predicate_(predicate),
    reverse_(false)
{
}

XQPredicate::XQPredicate(ASTNode *expr, ASTNode *predicate, bool reverse, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(PREDICATE, memMgr),
    expr_(expr),
    predicate_(predicate),
    reverse_(reverse)
{
}

ASTNode* XQPredicate::staticResolution(StaticContext *context)
{
  expr_ = expr_->staticResolution(context);

  {
    AutoNodeSetOrderingReset orderReset(context);
    predicate_ = predicate_->staticResolution(context);
  }

  return this;
}

ASTNode *XQPredicate::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.copy(expr_->getStaticAnalysis());
  _src.getStaticType().multiply(0, 1);

  // Remove context item usage
  _src.addExceptContextFlags(predicate_->getStaticAnalysis());

  return this;
}

Result XQPredicate::createResult(DynamicContext* context, int flags) const
{
  const StaticAnalysis &src = predicate_->getStaticAnalysis();

  Result parent = expr_->createResult(context, flags);

  size_t contextSize = 0;
  if(src.isContextSizeUsed()) {
    // We need the context size, so convert to a Sequence to work it out
    Sequence seq(parent->toSequence(context));
    contextSize = seq.getLength();
    parent = new SequenceResult(this, seq);
  }
  if(src.getStaticType().isType(StaticType::NUMERIC_TYPE) &&
     src.getStaticType().getMin() <= 1 &&
     src.getStaticType().getMax() >= 1 &&
     !src.isContextItemUsed() && !src.isContextPositionUsed()) {
    // It only contains numeric type results, and doesn't use the context
    // item or position
    // TBD Fix this StaticAnalysis - jpcs
    return ClosureResult::create(predicate_->getStaticAnalysis(), context,
      new NumericPredicateFilterResult(parent, predicate_, contextSize));
  }
  else if(!src.getStaticType().containsType(StaticType::NUMERIC_TYPE) ||
          src.getStaticType().getMin() > 1 ||
          src.getStaticType().getMax() < 1) {
    // It only contains non-numeric results
    return ClosureResult::create(predicate_->getStaticAnalysis(), context,
      new NonNumericPredicateFilterResult(parent, predicate_, contextSize));
  }
  else {
    return ClosureResult::create(predicate_->getStaticAnalysis(), context,
      new PredicateFilterResult(parent, predicate_, contextSize));
  }
}

Result XQPredicate::iterateResult(const Result &contextItems, DynamicContext *context) const
{
  const StaticAnalysis &src = predicate_->getStaticAnalysis();

  if((src.getStaticType().containsType(StaticType::NUMERIC_TYPE) &&
      src.getStaticType().getMin() <= 1 &&
      src.getStaticType().getMax() >= 1)
     || src.isContextPositionUsed() || src.isContextSizeUsed()) {
    return ASTNodeImpl::iterateResult(contextItems, context);
  }

  Result parent = expr_->iterateResult(contextItems, context);
  // It only contains non-numeric results
  return ClosureResult::create(predicate_->getStaticAnalysis(), context,
    new NonNumericPredicateFilterResult(parent, predicate_, 0));
}

ASTNode *XQPredicate::addPredicates(ASTNode *expr, VectorOfPredicates *preds)
{
  VectorOfPredicates::iterator i = preds->begin();
  VectorOfPredicates::iterator end = preds->end();
  for(; i != end; ++i) {
    (*i)->setExpression(expr);
    expr = *i;
  }
  return expr;
}

ASTNode *XQPredicate::addReversePredicates(ASTNode *expr, VectorOfPredicates *preds)
{
  VectorOfPredicates::iterator i = preds->begin();
  VectorOfPredicates::iterator end = preds->end();
  for(; i != end; ++i) {
    (*i)->setReverse(true);
    (*i)->setExpression(expr);
    expr = *i;
  }
  return expr;
}

/////////////////////////////////////
// PredicateFilterResult
/////////////////////////////////////

PredicateFilterResult::PredicateFilterResult(const Result &parent, const ASTNode *pred, size_t contextSize)
  : ResultImpl(pred),
    todo_(true),
    parent_(parent),
    pred_(pred),
    contextPos_(0),
    contextSize_(contextSize)
{
}

Item::Ptr PredicateFilterResult::next(DynamicContext *context)
{
  AutoContextInfoReset autoReset(context);

  bool contextUsed = pred_->getStaticAnalysis().isContextItemUsed() ||
    pred_->getStaticAnalysis().isContextPositionUsed();

  Item::Ptr result = 0;
  while(result.isNull()) {
    result = parent_->next(context);
    if(result.isNull()) {
      parent_ = 0;
      return 0;
    }

    ++contextPos_;

    if(todo_ || contextUsed) {
      todo_ = false;

      context->setContextSize(contextSize_);
      context->setContextPosition(contextPos_);
      context->setContextItem(result);

      Result pred_result = pred_->createResult(context);
      first_ = pred_result->next(context);
      if(first_.notNull()) {
        second_ = pred_result->next(context);
      }

      autoReset.resetContextInfo();
    }

    // 3.2.2 ...
    // The predicate truth value is derived by applying the following rules, in order:
    // 1) If the value of the predicate expression is an atomic value of a numeric type, the predicate truth
    // value is true if and only if the value of the predicate expression is equal to the context position.
    if(first_.notNull() && second_.isNull() && first_->isAtomicValue() &&
       ((const AnyAtomicType::Ptr)first_)->isNumericValue()) {
      const Numeric::Ptr num = (const Numeric::Ptr)first_;
      if(!num->equals((const AnyAtomicType::Ptr)context->getItemFactory()->createInteger((long)contextPos_, context), context)) {
        result = 0;
      }
      else if(!contextUsed) {
        parent_ = 0;
      }
    }
    else {
      // 2) Otherwise, the predicate truth value is the effective boolean value of the predicate expression
      if(!XQEffectiveBooleanValue::get(first_, second_, context, this)) {
        result = 0;
      }
    }
  }

  return result;
}

std::string PredicateFilterResult::asString(DynamicContext *context, int indent) const
{
  return "predicatefilterresult";
}

/////////////////////////////////////
// NonNumericPredicateFilterResult
/////////////////////////////////////

NonNumericPredicateFilterResult::NonNumericPredicateFilterResult(const Result &parent, const ASTNode *pred, size_t contextSize)
  : ResultImpl(pred),
    todo_(true),
    parent_(parent),
    pred_(pred),
    contextPos_(0),
    contextSize_(contextSize)
{
}

Item::Ptr NonNumericPredicateFilterResult::next(DynamicContext *context)
{
  AutoContextInfoReset autoReset(context);

  bool contextUsed = pred_->getStaticAnalysis().isContextItemUsed() ||
    pred_->getStaticAnalysis().isContextPositionUsed();

  Item::Ptr result = 0;
  while(result.isNull()) {
    result = parent_->next(context);
    if(result.isNull()) {
      parent_ = 0;
      return 0;
    }

    ++contextPos_;

    if(todo_ || contextUsed) {
      todo_ = false;

      context->setContextSize(contextSize_);
      context->setContextPosition(contextPos_);
      context->setContextItem(result);

      // 2) Otherwise, the predicate truth value is the effective boolean value of the predicate expression
      Result predResult = pred_->createResult(context);
      Item::Ptr first = predResult->next(context);
      if(first.isNull() || !XQEffectiveBooleanValue::get(first, predResult->next(context), context, this)) {
        result = 0;
        if(!contextUsed) {
          parent_ = 0;
        }
      }

      autoReset.resetContextInfo();
    }
  }

  return result;
}

std::string NonNumericPredicateFilterResult::asString(DynamicContext *context, int indent) const
{
  return "nonnumericpredicatefilterresult";
}

/////////////////////////////////////
// NumericPredicateFilterResult
/////////////////////////////////////

NumericPredicateFilterResult::NumericPredicateFilterResult(const Result &parent, const ASTNode *pred, size_t contextSize)
  : ResultImpl(pred),
    parent_(parent),
    pred_(pred),
    contextSize_(contextSize)
{
}

Item::Ptr NumericPredicateFilterResult::nextOrTail(Result &tail, DynamicContext *context)
{
  AutoContextInfoReset autoReset(context);
  context->setContextSize(contextSize_);

  // Set the context item to something other than null,
  // since fn:last() checks to see that there is actually
  // a context item
  context->setContextItem(context->getMemoryManager()->createInteger(1));

  Result pred_result = pred_->createResult(context);
  Numeric::Ptr first = (Numeric::Ptr)pred_result->next(context);
  if(first.isNull()) {
    // The effective boolean value is therefore false
    tail = 0;
    return 0;
  }

  Item::Ptr second = pred_result->next(context);
  if(second.notNull()) {
    // The effective boolean value causes an error -
    // so call it to get the correct error
    XQEffectiveBooleanValue::get(first, second, context, this);
    tail = 0;
    return 0;
  }

  autoReset.resetContextInfo();

  if(!first->isInteger() || first->isZero() || first->isNegative()) {
    tail = 0;
    return 0;
  }

  parent_->skip(first->asInt() - 1, context);
  Item::Ptr result = parent_->next(context);
  tail = 0;
  return result;
}
